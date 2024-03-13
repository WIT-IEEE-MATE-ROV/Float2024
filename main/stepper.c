#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "math.h"

static const char *TAG = "stepper";

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_OUTPUT_IO (5) // The step pin
#define DIR_PIN (4) // the dir pin
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_14_BIT // Set duty resolution to 13 bits
// #define LEDC_DUTY               (4096)            // Set duty to 50%. (2 ** LEDC_DUTY_RES) * 50% = 4096
#define LEDC_FREQUENCY 400 // Frequency in Hertz. Set frequency at 4 kHz

uint32_t LEDC_DUTY = (powf(2.f, 20.f) / 2.f);


// sets the dir pin and returns the state
// 1 HIGH 0 LOW 
// Direction will depend on the driver you use
int stp_set_dir(uint8_t dir)
{
    ESP_ERROR_CHECK(gpio_set_level(DIR_PIN, dir));
    return dir;
}

// set direction of and speed stepper bases on floating poinr valS from -1 to 1
// 1 +max speed
// 0 not moving
// -1 -max speed
void stp_set_output(float output)
{
    if (output > 0.f)
    {
        gpio_set_level(LEDC_OUTPUT_IO, 0);
    }
    else
    {
        gpio_set_level(LEDC_OUTPUT_IO, 1);
    }
}


void stepper_init()
{
    //Pin init
    gpio_reset_pin(DIR_PIN);
    gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LEDC_OUTPUT_IO);
    gpio_set_direction(LEDC_OUTPUT_IO, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY, // Set output frequency at 4 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 4096, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 4096);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    //int freq_step = 0;
    //int ret;
    while(1) vTaskDelay(10);
//     while (1)
//     {
//         vTaskDelay(500);
//         //ledc_set_freq(LEDC_MODE, LEDC_CHANNEL, 100 + freq_step);
//         ret =stp_set_dir (0);
//         vTaskDelay(500);
//         ret = stp_set_dir(1);

//         ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
//         if (freq_step > 1000)
//         {
//             freq_step = 0;
//         }
//         freq_step += 10;
//     }
   }


