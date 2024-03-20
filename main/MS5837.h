#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <inttypes.h>

#define I2C_MASTER_SCL_IO GPIO_NUM_22 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_21 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0              /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 100000     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define MS5837_SENSOR_ADDR 0x76 /*!< Slave address of the MPU9250 sensor */
#define MS5837_RESET 0x1E
#define MS5837_ADC_READ 0x00
#define MS5837_PROM_READ 0xA0
#define MS5837_CONVERT_D1_8192 0x4A /* Idd = 20.09 uA, Td = 16.44 ms, Resolution RMS 0.20 mbar, 0.0022 degree C */
#define MS5837_CONVERT_D2_8192 0x5A /* Idd = 20.09 uA, Td = 16.44 ms, Resolution RMS 0.20 mbar, 0.0022 degree C */
#define MS5837_CONVERT_D1_2048 0x46
#define MS5837_CONVERT_D2_2048 0x56

#define MS_CMD_INIT_PROM_READ 0xA0 // 0xA6

esp_err_t reg_read(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t write_byte(uint8_t device_address, uint8_t data);
esp_err_t i2c_read(uint8_t *data, size_t len);
esp_err_t i2c_master_init(void);
float pressure(float conversion);
uint8_t crc4(uint16_t n_prom[]);
void calculate();
void init(uint8_t *data);
void get_pressure_data(double *t, double *p);
double convert_depth(double pressure);