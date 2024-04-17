#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "nvs_storage.h"
#include "inttypes.h"
#include "esp_log.h"
#include "device_config.h"
#include "nvs_storage.h"
#include "ultrasonic.h"
#include "string.h"
#include "bumper.h"
#include "esp_err.h"
#include "communication.h"

static const char* TAG = "app_main";


static void s_checkFirmwareAndPartition(void)
{
    ESP_LOGW(TAG, "ESP-IDF ver: %s", esp_get_idf_version());
    ESP_LOGW(TAG, "Firmware ver: %s", CONFIG_DEVICE_FIRMWARE_REV);
	const esp_partition_t *partition_running = esp_ota_get_running_partition();
	if(partition_running) {
        ESP_LOGW(TAG, "Current partition: %s", partition_running->label);
	}
}

void app_main(void)
{
    s_checkFirmwareAndPartition();

    esp_err_t lErr = ESP_OK;

     lErr = communicationInit();
    if(lErr) {
        ESP_LOGE(TAG, "Failed to initialize UART communication. Code: 0x%X", lErr);
    } else {
        ESP_LOGI(TAG, "UART peripheral initialized!");
    }

    lErr = bumperInit();
    if(lErr) {
        ESP_LOGE(TAG, "Failed to initialize bumper. Code: 0x%X", lErr);
    } else {
        ESP_LOGI(TAG, "Bumper initialized!");
    }
    
}
