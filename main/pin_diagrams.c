#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include "esp_tls_crypto.h"
#include "esp_tls.h"
#include "esp_check.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <esp_system.h>
#include "esp_eth.h"
#include "driver/gpio.h"

#define LED 15
#define dirPin 4
#define stepPin 2
#define stepsPerRevolution 200
#define rotations 5
#define delays 1500

TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;

static const char *TAG = "Float Server";