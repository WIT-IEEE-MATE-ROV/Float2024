#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_wifi.h>
#include <time.h>
#include <sys/time.h>


#define SSID "FloatWIT" 

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");


esp_err_t index_get_handler(httpd_req_t *req)
{
	error = httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
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



httpd_uri_t index_get_uri = {
	.uri	  = "/",
	.method   = HTTP_GET,
	.handler  = index_get_handler,
	.user_ctx = NULL
};

httpd_uri_t ledon_uri = {
	.uri	  = "/ledon",
	.method   = HTTP_GET,
	.handler  = ledON_handler,
	.user_ctx = NULL
};

httpd_uri_t lefoff_uri = {
	.uri	  = "/ledoff",
	.method   = HTTP_GET,
	.handler  = ledON_handler,
	.user_ctx = NULL
};

httpd_uri_t url = {
	.uri	  = "/ledon",
	.method   = HTTP_GET,
	.handler  = ledON_handler,
	.user_ctx = NULL
};

// httpd_uri_t sync_post = {
// 	.uri	  = "/sync",
// 	.method   = HTTP_POST,
// 	.handler  = sync_post_handler,
// 	.user_ctx = NULL
// };



static esp_err_t http_server_init(void)
{
	static httpd_handle_t http_server = NULL;

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	if (httpd_start(&http_server, &config) == ESP_OK) {
		httpd_register_uri_handler(http_server, &index_get_uri);
		//httpd_register_uri_handler(http_server, &sync_post);

	}

	return http_server == NULL ? ESP_FAIL : ESP_OK;
}


static esp_err_t softap_init(void)
{
	esp_err_t res = ESP_OK;

	res |= esp_netif_init();
	res |= esp_event_loop_create_default();
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	res |= esp_wifi_init(&cfg);

	wifi_config_t wifi_config = {
		.ap = {
			.ssid = SSID,
			.ssid_len = strlen(SSID),
			.channel = 6,
			.authmode = WIFI_AUTH_OPEN,
			.max_connection = 10
		},
	};

	res |= esp_wifi_set_mode(WIFI_MODE_AP);
	res |= esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
	res |= esp_wifi_start();

	return res;
}

void ws_run(void* a) {
	esp_err_t ret = nvs_flash_init();

	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	ESP_ERROR_CHECK(ret);
	ESP_ERROR_CHECK(softap_init());
	ESP_ERROR_CHECK(http_server_init());

	while(1) vTaskDelay(10);
}