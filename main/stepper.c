#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "math.h"
#include "pin_diagrams.h"

static const char *TAG = "stepper";

#define LEDC_TIMER                              LEDC_TIMER_0
#define LEDC_MODE                               LEDC_HIGH_SPEED_MODE
// #define STEP_PIN                             (2) // The step pin
// #define DIR_PIN                              (4) // the dir pin
#define LEDC_CHANNEL                            LEDC_CHANNEL_0
#define LEDC_DUTY_RES                           LEDC_TIMER_13_BIT 
#define LEDC_MAX_FREQUENCY                      5000 // Frequency in Hertz
#define LEDC_MIN_FREQUENCY                      0
#define LEDC_DEFAULT_FREQUENCY                  LEDC_MAX_FREQUENCY / 2 

int max_duty = pow(2.0, LEDC_DUTY_RES);

// sets the dir pin and returns the state
// 1 HIGH 0 LOW 
// Direction will depend on the driver you use
int stp_set_dir(uint8_t dir)
{
    ESP_ERROR_CHECK(gpio_set_level(DIR_PIN, dir));
    return dir;
}

void stp_enable()
{
    ESP_ERROR_CHECK(gpio_set_level(ENABLE, 0));
}

void stp_disable()
{
    ESP_ERROR_CHECK(gpio_set_level(ENABLE, 1));
}


// sets the frequency of the pwm signal 
uint16_t stp_set_speed(float val)
{
    // max 500 hz
    if (val > 500)
    {
        val = 500;
    }
    // uint16_t freq = (val * (LEDC_MAX_FREQUENCY - LEDC_MIN_FREQUENCY) / 100 + LEDC_MIN_FREQUENCY);
    uint16_t freq = val;
    // ledc_set_freq(LEDC_MODE, LEDC_CHANNEL, 100 + freq);
    ledc_set_freq(LEDC_MODE, LEDC_CHANNEL, val);
    return freq;
}


void stepper_init()
{
    printf("MAX DUTY: %d\n", max_duty);
    // GPIO Initilization
    gpio_reset_pin(DIR_PIN);
    gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(STEP_PIN);
    gpio_set_direction(STEP_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ENABLE);
    gpio_set_direction(ENABLE, GPIO_MODE_OUTPUT);
    printf("A4998 stepper driver Initilized.\n");
    stp_disable();
    printf("A4998 stepper driver disabled.\n");

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_DEFAULT_FREQUENCY, // Set output frequency at some freq (hz)
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = STEP_PIN,
        .duty = max_duty / 2, // Set duty to 50%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, max_duty / 2);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    stp_set_speed(500);

    //int ret;
    while(1) vTaskDelay(10);

    //speed test very fast
    // while (1)
    // {
    //     vTaskDelay(100);
    //     stp_set_speed(10 + freq_step);
    //     freq_step += 10;
    //     vTaskDelay(100);
    //     if(freq_step > 100){freq_step = 0;}
    // }
   }


