#include <stdio.h>
#include "bumper.h"
#include "device_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "bumper";

static Bumper_t s_sBumper;


static void s_bumperTask(void* pArg) {
    SensorModule_t* pModule;
    pModule = pArg;

    ultrasonic_init(&pModule->sSensor);

    while (true)
    {
        float fDistance;
        esp_err_t lErr = ultrasonic_measure(&pModule->sSensor, CONFIG_MAX_DISTANCE_CM, &fDistance);
        if (lErr != ESP_OK) {
            printf("Error %d: ", lErr);
            switch (lErr)
            {
                case ESP_ERR_ULTRASONIC_PING:
                    // printf("Cannot ping (device is in invalid state)\n");
                    printf("-1\n");
                    break;
                case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    // printf("Ping timeout (no device found)\n");
                    printf("-2\n");
                    break;
                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    // printf("Echo timeout (i.e. distance too big)\n");
                    printf("-3\n");
                    break;
                default:
                    printf("%s\n", esp_err_to_name(lErr));
            }
        }
        else {
            pModule->ubDistance = (uint8_t)(fDistance*100);
            // printf("Distance: %d cm\n", distance_cm);
            // printf("%d\n", pModule->ubDistance);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static esp_err_t s_createBumperTask(Bumper_t* pBumper) {
    esp_err_t lErr = ESP_FAIL;

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            "mid", 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sMid, 
                                            5, 
                                            pBumper->sMid.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start mid sensor! Aborting...");
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            "left", 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sLeft, 
                                            5, 
                                            pBumper->sLeft.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start left sensor! Aborting...");
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            "right", 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sRight, 
                                            5, 
                                            pBumper->sRight.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start right sensor! Aborting...");
        goto end_create_task;
    }

    lErr = ESP_OK;

end_create_task:

    return lErr;
}

esp_err_t bumperInit(void) {
    esp_err_t lErr = ESP_OK;
    s_sBumper.sLeft.sSensor.echo_pin = CONFIG_PIN_SENSOR_LEFT_ECHO;
    s_sBumper.sLeft.sSensor.trigger_pin = CONFIG_PIN_SENSOR_LEFT_TRIGGER;
    strcpy(s_sBumper.sLeft.szName, "left");

    s_sBumper.sMid.sSensor.echo_pin = CONFIG_PIN_SENSOR_FRONT_ECHO;
    s_sBumper.sMid.sSensor.trigger_pin = CONFIG_PIN_SENSOR_FRONT_TRIGGER;
    strcpy(s_sBumper.sMid.szName, "left");

    s_sBumper.sRight.sSensor.echo_pin = CONFIG_PIN_SENSOR_RIGHT_ECHO;
    s_sBumper.sRight.sSensor.trigger_pin = CONFIG_PIN_SENSOR_RIGHT_TRIGGER;
    strcpy(s_sBumper.sMid.szName, "left");


    return s_createBumperTask(&s_sBumper);
}