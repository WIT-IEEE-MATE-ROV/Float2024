#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <inttypes.h>
#include "MS5837.h"

// const char *TAG = "pressure";

// static const float Pa = 100.0f;
// static const float bar = 0.001f;
// static const float mbar = 1.0f;

uint8_t data[3];
uint32_t D1_pres, D2_temp;
double TEMP;
double P;
uint8_t _model;
uint16_t C[8];
double fluidDensity = 1023.6; // kg/m^3

/* Versions */
const uint8_t MS5837_02BA01 = 0x00; // Sensor version: From MS5837_02BA datasheet Version PROM Word 0
const uint8_t MS5837_02BA21 = 0x15; // Sensor version: From MS5837_02BA datasheet Version PROM Word 0
const uint8_t MS5837_30BA26 = 0x1A; // Sensor version: From MS5837_30BA datasheet Version PROM Word 0

/**
 * @brief Read a sequence of bytes from a MS5837 sensor registers
 */
esp_err_t reg_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MS5837_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MS5837 sensor register
 */
esp_err_t write_byte(uint8_t device_address, uint8_t data)
{
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, device_address, &data, 1 * sizeof(uint8_t), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    // printf("Write ret value: %u\n", ret);
    ESP_ERROR_CHECK(ret);
    return ret;
}

esp_err_t i2c_read(uint8_t *data, size_t len)
{
    esp_err_t ret = i2c_master_read_from_device(I2C_MASTER_NUM, MS5837_SENSOR_ADDR, data, len * sizeof(uint8_t), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(ret);
    return ret;
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

float pressure(float conversion) {
    return P*conversion/100.0f;
}

uint8_t crc4(uint16_t n_prom[])
{
    uint16_t n_rem = 0;

    n_prom[0] = ((n_prom[0]) & 0x0FFF);
    n_prom[7] = 0;

    for (uint8_t i = 0; i < 16; i++)
    {
        if (i % 2 == 1)
        {
            n_rem ^= (uint16_t)((n_prom[i >> 1]) & 0x00FF);
        }
        else
        {
            n_rem ^= (uint16_t)(n_prom[i >> 1] >> 8);
        }
        for (uint8_t n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else
            {
                n_rem = (n_rem << 1);
            }
        }
    }

    n_rem = ((n_rem >> 12) & 0x000F);

    return n_rem ^ 0x00;
}

void calculate()
{
    // Given C1-C6 and D1, D2, calculated TEMP and P
    // Do conversion first and then second order temp compensation

    int32_t dT = 0;
    int64_t SENS = 0;
    int64_t OFF = 0;
    int32_t SENSi = 0;
    int32_t OFFi = 0;
    int32_t Ti = 0;
    int64_t OFF2 = 0;
    int64_t SENS2 = 0;

    // Terms called
    dT = D2_temp - (uint32_t)(C[5]) * 256l;
    SENS = (int64_t)(C[1]) * 65536l + ((int64_t)(C[3]) * dT) / 128l;
    OFF = (int64_t)(C[2]) * 131072l + ((int64_t)(C[4]) * dT) / 64l;
    P = (D1_pres * SENS / (2097152l) - OFF) / (32768l);

    // Temp conversion
    TEMP = 2000l + (int64_t)(dT)*C[6] / 8388608LL;

    // Second order compensation
    if ((TEMP / 100) < 20)
    { // Low temp
        Ti = (11 * (int64_t)(dT) * (int64_t)(dT)) / (34359738368LL);
        OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
        SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
    }
    OFF2 = OFF - OFFi; // Calculate pressure and temp second order
    SENS2 = SENS - SENSi;
    TEMP = (TEMP - Ti);
    P = (((D1_pres * SENS2) / 2097152l - OFF2) / 32768l);
    // P = (((D1_pres*SENS2)/2097152l-OFF2)/32768l);
}

void init(uint8_t *data)
{
    ESP_ERROR_CHECK(i2c_master_init()); /* Initilize */
    // ESP_LOGI(TAG, "I2C initialized successfully");

    write_byte(MS5837_SENSOR_ADDR, MS5837_RESET); /* Reset */
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // ESP_LOGI(TAG, "MS5837 reset successfully");
    // ESP_LOGI(TAG, "Initializing PROM read...");


    uint16_t prom_data[7];
    uint8_t read_bytes[2];
    for (uint8_t i = 0; i < 7; i++)
    { /* Read calibration from PROM */
        /* First bits are 15-8 of register, and 7-0 are second bits sent, data is 16 bits for most */
        reg_read(MS_CMD_INIT_PROM_READ + i * 2, read_bytes, 2);
        prom_data[i] = ((uint16_t) read_bytes[0] << 8) | (uint16_t) read_bytes[1];
        C[i] = (read_bytes[0] << 8) | read_bytes[1];
    }
    // ESP_LOGI(TAG, "PROM Read: %X, %X, %X, %X, %X, %X, %X", prom_data[0], prom_data[1], prom_data[2], prom_data[3], prom_data[4], prom_data[5], prom_data[6]);
    uint8_t crcRead = C[0] >> 12; /* Factory defined data */
    uint8_t crcCalculated = crc4(C);

    if (crcCalculated != crcRead)
    {
        // ESP_LOGE(TAG, "CRC Fail");
    }

    // ESP_LOGI(TAG, "CRC Worked!");

    uint8_t version = (C[0] >> 5) & 0x7F; // Extract the sensor version from PROM Word 0

    // Set _model according to the sensor version
    if (version == MS5837_02BA01)
    {
        // ESP_LOGI(TAG, "Version MS5837_02BA");
    }
    else if (version == MS5837_02BA21)
    {
        // ESP_LOGI(TAG, "Version MS5837_02BA");
    }
    else if (version == MS5837_30BA26)
    {
        // ESP_LOGI(TAG, "Version MS5837_30BA");
    }
    else
    {
        // ESP_LOGE(TAG, "Version MS5837_UNRECOGNISED");
    }
}

// The pressure sensor measures absolute pressure, so it will measure the atmospheric pressure + water pressure
// We subtract the atmospheric pressure to calculate the depth with only the water pressure
// The average atmospheric pressure of 1013.25  mbar is used for the calcuation, but atmospheric pressure varies
// If the atmospheric pressure is not 1013.25  at the time of reading, the depth reported will be offset
// In order to calculate the correct depth, the actual atmospheric pressure should be measured once in air, and
// that value should subtracted for subsequent depth calculations.
double convert_depth(double pressure) 
{
	return (pressure-1013.25)/(fluidDensity*9.80665); // mbar / (kg/m^3 * m/s^2)
}

void get_pressure_data(double *t, double *p)
{
    uint8_t data[3];
    uint32_t D1_pres, D2_temp;

    write_byte(MS5837_SENSOR_ADDR, MS5837_CONVERT_D1_8192);
    vTaskDelay(20 /portTICK_PERIOD_MS);
    reg_read(MS5837_ADC_READ, data, 3);

    D1_pres = 0;
    D1_pres = data[0];
    D1_pres = (D1_pres << 8) | data[1];
    D1_pres = (D1_pres << 8) | data[2];

    write_byte(MS5837_SENSOR_ADDR, MS5837_CONVERT_D2_8192);
    vTaskDelay(20/portTICK_PERIOD_MS);
    reg_read(MS5837_ADC_READ, data, 3);

    D2_temp = 0;
    D2_temp = data[0];
    D2_temp = (D2_temp << 8) | data[1];
    D2_temp = (D2_temp << 8) | data[2];

    // Given C1-C6 and D1, D2, calculated TEMP and P
    // Do conversion first and then second order temp compensation

    int32_t dT = 0;
    int64_t SENS = 0;
    int64_t OFF = 0;
    int32_t SENSi = 0;
    int32_t OFFi = 0;
    int32_t Ti = 0;
    int64_t OFF2 = 0;
    int64_t SENS2 = 0;

    // Terms called
    dT = D2_temp - (uint32_t)(C[5]) * 256l;
    SENS = (int64_t)(C[1]) * 65536l + ((int64_t)(C[3]) * dT) / 128l;
    OFF = (int64_t)(C[2]) * 131072l + ((int64_t)(C[4]) * dT) / 64l;
    P = (D1_pres * SENS / (2097152l) - OFF) / (32768l);

    // Temp conversion
    TEMP = 2000l + (int64_t)(dT)*C[6] / 8388608LL;

    // Second order compensation
    if ((TEMP / 100) < 20)
    { // Low temp
        Ti = (11 * (int64_t)(dT) * (int64_t)(dT)) / (34359738368LL);
        OFFi = (31 * (TEMP - 2000) * (TEMP - 2000)) / 8;
        SENSi = (63 * (TEMP - 2000) * (TEMP - 2000)) / 32;
    }
    OFF2 = OFF - OFFi; // Calculate pressure and temp second order
    SENS2 = SENS - SENSi;
    TEMP = (TEMP - Ti);
    P = (((D1_pres * SENS2) / 2097152l - OFF2) / 32768l);
    // P = (((D1_pres*SENS2)/2097152l-OFF2)/32768l);

    *t = TEMP;
    *p = P;
}