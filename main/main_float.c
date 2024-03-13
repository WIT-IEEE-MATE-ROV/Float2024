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
//#include "stepper.h"

static const char *TAG = "main";


#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

esp_err_t i2c_param_config(i2c_port_t i2c_num, const i2c_config_t *i2c_conf);

i2c_config_t conf = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = 21,
	.scl_io_num = 22,
	.sda_pullup_en = GPIO_PULLUP_ENABLE,
	.scl_pullup_en = GPIO_PULLUP_ENABLE,
	.master.clk_speed = I2C_CLK_SRC_DEFAULT,
};


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

static void configure_led (void){
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

static void configure_motorDriver(void){
    gpio_reset_pin(dirPin);
    gpio_reset_pin(stepPin);
    gpio_set_direction(dirPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(stepPin, GPIO_MODE_OUTPUT);
}

static void spin_motor(void){
    gpio_set_level(dirPin, 1);
    for(int i = rotations; i > 0; i--){
        for(int x = 0; x < stepsPerRevolution; x++){
            gpio_set_level(stepPin, 1);
            vTaskDelay(10/portTICK_PERIOD_MS);
            gpio_set_level(stepPin, 0);
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }
    // vTaskDelay(1000/portTICK_PERIOD_MS);
}

void task1(void *p){
    int var = 0;
    while(1){
        printf("111111: %d\n", var);
        var += 1;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void task2(void *p){
    int var2 = 0;
    while(1){
        printf("2222222: %d\n", var2);
        var2 += 1;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

static esp_err_t root_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "LED Turned OFF");
    const char *response = (const char *) req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if (error != ESP_OK){
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    } else {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}



static esp_err_t ledON_handler(httpd_req_t *req)
{
    esp_err_t error;
    gpio_set_level(LED, 1);
    // xTaskCreate(spin_motor(),"Spin Motor",1000, NULL, 1, NULL);
    if(task2_handle != NULL){
        vTaskSuspend(task2_handle);
    }
    xTaskCreate(task1, "task 1", 4096, NULL, 1, &task1_handle);
    ESP_LOGI(TAG, "LED Turned ON");
    const char *response = (const char *) req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if (error != ESP_OK){
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    } else {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}



static esp_err_t ledOFF_handler(httpd_req_t *req)
{
    esp_err_t error;
    gpio_set_level(LED, 0);
    if(task1_handle != NULL){
        vTaskSuspend(task1_handle);
    }
    xTaskCreate(task2, "task 2", 4096, NULL, 1, &task2_handle);
    ESP_LOGI(TAG, "LED Turned OFF");
    const char *response = (const char *) req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if (error != ESP_OK){
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    } else {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}


static esp_err_t stop_webserver(httpd_handle_t server)
{
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}



void app_main(void)
{
    // static httpd_handle_t server = NULL;
    // configure_led();
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
    TaskHandle_t stepper_init_task = NULL;
    TaskHandle_t ws_task = NULL;

    xTaskCreate(ws_run, "WEBSERVER", 3584, param, 1, &ws_task);
    // xTaskCreate(stepper_init, "STEPPER", 3584, param, 1, &stepper_init_task);

    configASSERT(ws_task);
    //configASSERT(stepper_init_task);
    while(1)
    vTaskDelay(10);
}



