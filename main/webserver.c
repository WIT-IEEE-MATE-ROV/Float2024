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
#include <cJSON.h> // Include cJSON library for JSON manipulation

#include "esp_log.h"
#include "esp_app_trace.h"
#include "pin_diagrams.c"
#include "stepper.h"
#include "MS5837.h"


static const char *TAG = "webserver";

#define SSID "FloatWIT" 

// Lets html to be loaded from seprate file
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

// Sends the html page when root ip is requested
esp_err_t index_get_handler(httpd_req_t *req)
{
	esp_err_t error = httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
	if (error != ESP_OK){
        ESP_LOGI(TAG, "Error %d while sending Response", error);
    } else {
        ESP_LOGI(TAG, "Response sent Successfully");
    }
    return error;
}

//----------callback functions----------//
static esp_err_t dirHIGH_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST responce";
	ESP_LOGI(TAG, "Recieved dirhigh request");
	stp_set_dir(1);
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t dirLOW_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST responce";
	ESP_LOGI(TAG, "Recieved dirlow request");
	stp_set_dir(0);
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;	
}

static esp_err_t ledOFF_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved ledoff request");
	gpio_set_level(LED, 0);
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t ledON_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved ledon request");
	gpio_set_level(LED, 1);
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t start_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved start float request");
	// TODO: Control of Servo
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}
	
static esp_err_t downloadData_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved download data request");
	// TODO: Retrive data from sd card using wifi 
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t enable_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved enable stepper request");
	stp_enable();
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

static esp_err_t disable_handler(httpd_req_t *req)
{
	const char resp[] = "URI POST Response";
	ESP_LOGI(TAG, "Recieved disable stepper request");
	stp_disable();
	httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

void double_to_json_string(double value, char* json_string, size_t size) {
    // Format the double value as a JSON string
    snprintf(json_string, size, "%.15g", value);
}

static esp_err_t pressure_temp_handler(httpd_req_t *req)
{
	cJSON *root = cJSON_CreateObject();
	// httpd_resp_set_type(req, "application/json");
	ESP_LOGI(TAG, "Recieved pressure request");
	
	double temp, pres;
	get_pressure_data(&temp, &pres);
	double depth = convert_depth(pres);
    // printf("Temperature: %.2f deg C\n",temp/100);
    // printf("Pressure: %.2f mbar\n",pres/100);
    // printf("Depth: %.2f m\n\n", depth);
	pres /= 100;
	temp /= 100;
	char json_string_temp[8];
	char json_string_pres[8];
	char json_string_depth[8];
	double_to_json_string(temp, json_string_temp, sizeof(json_string_temp));
	double_to_json_string(pres, json_string_pres, sizeof(json_string_pres));
	double_to_json_string(depth, json_string_depth, sizeof(json_string_depth));

	cJSON_AddStringToObject(root, "units", "deg C, mBar, m");
	cJSON_AddStringToObject(root, "temperature", json_string_temp);
	cJSON_AddStringToObject(root, "pressure", json_string_pres);
	cJSON_AddStringToObject(root, "depth", json_string_depth);
	 
	// Convert cJSON object to a JSON-formatted string
    char *json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "json_string\n%s", json_string);
	httpd_resp_send(req, json_string, strlen(json_string));

	// Free allocated memory
    cJSON_Delete(root);
    cJSON_free(json_string);
	return ESP_OK;
}

//---------- uri handlers ----------//
httpd_uri_t index_get_uri = {
	.uri	  = "/",
	.method   = HTTP_GET,
	.handler  = index_get_handler,
	.user_ctx = NULL
};

httpd_uri_t ledon_uri = {
	.uri	  = "/ledon",
	.method   = HTTP_POST,
	.handler  = ledON_handler,
	.user_ctx = NULL
};

httpd_uri_t lefoff_uri = {
	.uri	  = "/ledoff",
	.method   = HTTP_POST,
	.handler  = ledOFF_handler,
	.user_ctx = NULL
};

httpd_uri_t dirhigh_uri ={
	.uri	  = "/dirhigh",
	.method   = HTTP_POST,
	.handler  = dirHIGH_handler,
	.user_ctx = NULL  
};

httpd_uri_t dirlow_uri ={
	.uri	  = "/dirlow",
	.method   = HTTP_POST,
	.handler  = dirLOW_handler,
	.user_ctx = NULL  
};

httpd_uri_t start_uri ={
	.uri	  = "/start",
	.method   = HTTP_POST,
	.handler  = start_handler,
	.user_ctx = NULL  
};

httpd_uri_t downloadData_uri ={
	.uri	  = "/downloadData",
	.method   = HTTP_POST,
	.handler  = downloadData_handler,
	.user_ctx = NULL  
};

httpd_uri_t enable_uri ={
	.uri	  = "/enable",
	.method   = HTTP_POST,
	.handler  = enable_handler,
	.user_ctx = NULL  
};

httpd_uri_t disable_uri ={
	.uri	  = "/disable",
	.method   = HTTP_POST,
	.handler  = disable_handler,
	.user_ctx = NULL  
};

httpd_uri_t pressure_uri = {
	.uri	  = "/pressure",
	.method   = HTTP_GET,
	.handler  = pressure_temp_handler,
	.user_ctx = NULL
};

// httpd_uri_t url = {
// 	.uri	  = "/ledon",
// 	.method   = HTTP_GET,
// 	.handler  = ledON_handler,
// 	.user_ctx = NULL
// };

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
		httpd_register_uri_handler(http_server, &ledon_uri);
		httpd_register_uri_handler(http_server, &lefoff_uri);
		httpd_register_uri_handler(http_server, &dirlow_uri);
		httpd_register_uri_handler(http_server, &dirhigh_uri);
		httpd_register_uri_handler(http_server, &disable_uri);
		httpd_register_uri_handler(http_server, &enable_uri);
		httpd_register_uri_handler(http_server, &pressure_uri);

		

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

	// Task that inits the stepper code when the server is initialized
    void *param = NULL; 
    TaskHandle_t stepper_init_task = NULL;
	xTaskCreate(stepper_init, "STEPPER", 3584, param, 1, &stepper_init_task);
	while(1) vTaskDelay(10);
}