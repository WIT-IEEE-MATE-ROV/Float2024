/*
By: Tedi Qafko
Date: 1/28/2024
Description: Float Server SoftAP server in ESP32 using RTOS and 
EPS-IDF applicationto recieve get and post requests via the HTTP protocol.
*/
// #include "uri_handlers.c"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <time.h>
#include <sys/time.h>
#include "pin_diagrams.c"
#include "driver/i2c.h" /*  Needed for I2C */
#include "webserver.h"
#include "MS5837.h"

const char *TAG = "main";

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

static void configure_led (void){
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

// uint32_t D1_pres, D2_temp;
// int32_t TEMP;

void app_main(void)
{
    configure_led();
    void *param = NULL; 
    
    TaskHandle_t ws_task = NULL;
    // TaskHandle_t pressure_task = NULL;
    printf("Started!!\n\n");
    xTaskCreate(ws_run, "WEBSERVER", 3584, param, 1, &ws_task);
    // xTaskCreate(get_pressure_data, "Pressure", 3584, param, 1, &pressure_task);
    uint8_t data[3];
    init(data); /* Start i2c device and check if it works*/
    printf("This is the pressure data: %d\n",(int)(data));

    // double temp, pres;

    configASSERT(ws_task);
    // while(1){
        // get_pressure_data(&temp, &pres);
        // double depth = convert_depth(pres);
        // printf("Temperature: %.2f deg C\n",temp/100);
        // printf("Pressure: %.2f mbar\n",pres/100);
        // printf("Depth: %.2f m\n\n", depth);
        // vTaskDelay(100);
    // }
    while(1)
    vTaskDelay(10);
}



