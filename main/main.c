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

static const char* TAG = "app_main";


// void ultrasonic_test(void *pvParameters)
// {
//     ultrasonic_sensor_t* pSensor;
//     pSensor = pvParameters;

//     ultrasonic_init(pSensor);

//     while (true)
//     {
//         float distance;
//         esp_err_t res = ultrasonic_measure(pSensor, CONFIG_MAX_DISTANCE_CM, &distance);
//         if (res != ESP_OK) {
//             printf("Error %d: ", res);
//             switch (res)
//             {
//                 case ESP_ERR_ULTRASONIC_PING:
//                     // printf("Cannot ping (device is in invalid state)\n");
//                     printf("-1\n");
//                     break;
//                 case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
//                     // printf("Ping timeout (no device found)\n");
//                     printf("-2\n");
//                     break;
//                 case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
//                     // printf("Echo timeout (i.e. distance too big)\n");
//                     printf("-3\n");
//                     break;
//                 default:
//                     printf("%s\n", esp_err_to_name(res));
//             }
//         }
//         else {
//             distance_cm = (uint8_t)(distance*100);
//             // printf("Distance: %d cm\n", distance_cm);
//             printf("%d\n", distance_cm);
//         }

//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

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

    lErr = bumperInit();
    if(lErr) {
        ESP_LOGE(TAG, "Failed to initialize bumper. Code: 0x%X", lErr);
    } else {
        ESP_LOGI(TAG, "Bumper initialized!");
    }
    
}
