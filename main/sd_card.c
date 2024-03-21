#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <time.h>

#include <diskio.h>
#include "ff.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_common.h"
#include "hal/spi_types.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "pin_diagrams.h"

#define EXAMPLE_MAX_CHAR_SIZE 64
#define MOUNT_POINT "/sdcard"



static const char *TAG = "sdcard";

static void init_sd_card() 
{
    esp_err_t ret;


    // Init SPI
    // check device
    // mount fs


    // sdspi_device_config_t sd_spi_config = {
    //   //  .host_id = ,

    // };

    sdspi_device_config_t host_conf = {
      // .slot = SDMMC_HOST_SLOT_0,
      // .frequency = SDMMC_FREQ_xDEFAULT, // Adjust if needed
      // .clk_flags = SDMMC_CLK_FLAG_HALF_SPEED, // Adjust if needed
      // .mode = SDMMC_HOST_MODE_SPI,
      // .dma_intr_enable = false, // Adjust if using DMA
      // .fifo_depth = 128, // Adjust if needed
      // .append_bits = 8,
    };

    ESP_ERROR_CHECK(sdspi_host_init_device(&host_conf));

}