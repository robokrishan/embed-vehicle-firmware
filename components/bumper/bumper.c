#include <stdio.h>
#include "bumper.h"
#include "device_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

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
                case ESP_ERR_ULTRASONIC_PING: {
                    ESP_LOGE(TAG, "Cannot ping (device is in invalid state)");
                    pModule->ubDistance = 0;
                    break;
                }

                case ESP_ERR_ULTRASONIC_PING_TIMEOUT: {
                    ESP_LOGE(TAG, "Ping timeout (no device found)");
                    pModule->ubDistance = 0;
                    break;
                }

                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT: {
                    ESP_LOGE(TAG, "Echo timeout (no device found)");
                    pModule->ubDistance = 0;
                    break;
                }

                default: {
                    ESP_LOGE(TAG, "Sensor error! Code: 0x%X", lErr);
                }
            }
        }
        else {
            pModule->ubDistance = (uint8_t)(fDistance*100);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static esp_err_t s_createBumperTasks(Bumper_t* pBumper) {
    esp_err_t lErr = ESP_FAIL;

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sMid.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sMid, 
                                            5, 
                                            pBumper->sMid.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sMid.szName);
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sLeft.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sLeft, 
                                            5, 
                                            pBumper->sLeft.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sLeft.szName);
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sRight.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sRight, 
                                            5, 
                                            pBumper->sRight.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sRight.szName);
        goto end_create_task;
    }

    lErr = ESP_OK;

end_create_task:

    return lErr;
}

static esp_err_t s_configureModules(Bumper_t* pBumper) {
    pBumper->sLeft.sSensor.echo_pin = CONFIG_PIN_SENSOR_LEFT_ECHO;
    pBumper->sLeft.sSensor.trigger_pin = CONFIG_PIN_SENSOR_LEFT_TRIGGER;
    strcpy(pBumper->sLeft.szName, "left");

    pBumper->sMid.sSensor.echo_pin = CONFIG_PIN_SENSOR_FRONT_ECHO;
    pBumper->sMid.sSensor.trigger_pin = CONFIG_PIN_SENSOR_FRONT_TRIGGER;
    strcpy(pBumper->sMid.szName, "mid");

    pBumper->sRight.sSensor.echo_pin = CONFIG_PIN_SENSOR_RIGHT_ECHO;
    pBumper->sRight.sSensor.trigger_pin = CONFIG_PIN_SENSOR_RIGHT_TRIGGER;
    strcpy(pBumper->sMid.szName, "right");

    return ESP_OK;
}

esp_err_t bumperInit(void) {
    esp_err_t lErr = ESP_FAIL;
    
    lErr = s_configureModules(&s_sBumper);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure sensor modules! Code: 0x%X", lErr);
        goto abort_init;
    }

    lErr = s_createBumperTasks(&s_sBumper);
    if(lErr) {
        goto abort_init;
    }

    lErr = ESP_OK;
abort_init:

    return lErr;
}