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
#include "pin_diagrams.h"
#include "driver/i2c.h" /*  Needed for I2C */
#include "webserver.h"
#include "sd_card.h"
//#include "stepper.h"

static const char *TAG = "main";


#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

static void configure_led (void){
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    // static httpd_handle_t server = NULL;
    configure_led();
     
    // // configure_motorDriver();
    // esp_err_t ret = nvs_flash_init();
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //   ESP_ERROR_CHECK(nvs_flash_erase());
    //   ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);
    // ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");

    // wifi_init_softap();
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));

    // i2c_param_config(I2C_NUM_0, &conf);
	// ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    void *param = NULL; 
    TaskHandle_t ws_task = NULL;
//    TaskHandle_t sd_card_init_task = NULL;

    xTaskCreate(ws_run, "WEBSERVER", 3584, param, 1, &ws_task);
    //xTaskCreate(stepper_init, "STEPPER", 3584, param, 1, &stepper_init_task);
    configASSERT(ws_task);
    //configASSERT(stepper_init_task);

    while(1)
    vTaskDelay(10);
}



