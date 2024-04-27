#include <stdio.h>
#include "bumper.h"
#include "device_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "esp_log.h"
#include "communication.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"


static const char* TAG = "bumper";

// Communication syncrhonization - Event Group Config

#define BIT_SENSOR_LEFT     BIT0
#define BIT_SENSOR_MID      BIT1
#define BIT_SENSOR_RIGHT    BIT2

#define BUFFER_SIZE                 (1024)
#define COMM_PACKET_HEADER          (0xAB3412FE)
#define COMM_PACKET_TERMINATOR      (0x0A0D)

static EventGroupHandle_t s_pEventGroup = NULL;
static StaticEventGroup_t s_sEventGroupBuffer;
static TickType_t s_ulCondition = BIT_SENSOR_LEFT | BIT_SENSOR_MID | BIT_SENSOR_RIGHT;


// Shared resource access control
static SemaphoreHandle_t s_sMutexBumper = NULL;
static StaticSemaphore_t s_sMutexBumperBuffer;


// Static vars
static Bumper_t s_sBumper;
static uint8_t* pData;
static uint32_t s_ulHeader = COMM_PACKET_HEADER;
static uint16_t s_ulTerminator = COMM_PACKET_TERMINATOR;


// Buffer conversion functions
static uint8_t* s_intToBytes(uint32_t* pValue)
{
    return (uint8_t*) pValue;
}

static uint8_t* s_intToBytesShort(uint16_t* pValue)
{
    return (uint8_t*) pValue;
}

static void s_printBumperPacket(void) {
    printf("[%s]\tDistance:\t%d\n", s_sBumper.sLeft.szName, s_sBumper.sLeft.ubDistance);
    printf("[%s]\tDistance:\t%d\n", s_sBumper.sMid.szName, s_sBumper.sMid.ubDistance);
    printf("[%s]\tDistance:\t%d\n", s_sBumper.sRight.szName, s_sBumper.sRight.ubDistance);
    printf("--------------------------------------\n");
}

static esp_err_t s_sendBumperPacket(Bumper_t* pBumper) {
    esp_err_t lErr = ESP_FAIL;
    int lIterator = 0;
    // Access Tx buffer
    xSemaphoreTake(s_sMutexBumper, portMAX_DELAY);

    size_t packetSize = sizeof(uint32_t) + 3*sizeof(uint8_t) + sizeof(uint16_t);

    if(NULL == pData) {
        pData = malloc(packetSize);
        if(NULL == pData) {
            ESP_LOGE(TAG, "Failed to allocate memory for tx buffer!");
            lErr =  ESP_ERR_NO_MEM;
            goto abort_send_packet;
        }
        // Serialize header
        memcpy(&pData[lIterator], s_intToBytes(&s_ulHeader), sizeof(uint32_t));
        lIterator += sizeof(uint32_t);

        // serialize sensor values
        memcpy(&pData[lIterator], &s_sBumper.sLeft.ubDistance, sizeof(uint8_t));
        lIterator += sizeof(uint8_t);
        memcpy(&pData[lIterator], &s_sBumper.sMid.ubDistance, sizeof(uint8_t));
        lIterator += sizeof(uint8_t);
        memcpy(&pData[lIterator], &s_sBumper.sRight.ubDistance, sizeof(uint8_t));
        lIterator += sizeof(uint8_t);

        // serialize terminator
        memcpy(&pData[lIterator], s_intToBytesShort(&s_ulTerminator), sizeof(uint16_t));
        lIterator += sizeof(uint16_t);

#ifdef DEBUG
        ESP_LOGI(TAG, "Packed %d bytes into buffer!", lIterator);
        printf("Buffer:\n");
        for(int i = 0; i < lIterator+1; i++) {
            printf("%X | %d\n", pData[i], pData[i]);
        }
#endif
        lErr = communicationWrite(pData, packetSize);
    }

abort_send_packet:
    if(NULL != pData) {
        free(pData);
    }
    xSemaphoreGive(s_sMutexBumper);
    return lErr;
}

static void s_communicationTask(void* pArg) {
    ESP_LOGI(TAG, "Started communication task");

    EventBits_t bits;
    while(true) {
        bits = xEventGroupWaitBits(s_pEventGroup, s_ulCondition, pdTRUE, pdTRUE, portMAX_DELAY);
        if ((bits & s_ulCondition) == s_ulCondition) {
            s_sendBumperPacket(&s_sBumper);
            s_printBumperPacket();
            xEventGroupClearBits(s_pEventGroup, s_ulCondition);
        } else {
            ESP_LOGW(TAG, "Timed out waiting for condition! %ld != %ld", bits, s_ulCondition);
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

static void s_bumperTask(void* pArg) {
    SensorModule_t* pModule;
    pModule = pArg;

    bool isLoop = true;

    esp_err_t lErr = ultrasonic_init(&pModule->sSensor);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to initialize ultrasonic sensor! Aborting...");
        isLoop = false;
    }

    while (isLoop)
    {
        float fDistance;
        esp_err_t lErr = ultrasonic_measure(&pModule->sSensor, CONFIG_MAX_DISTANCE_CM, &fDistance);
 
        if (lErr != ESP_OK) {
            printf("Error %d: ", lErr);
            switch (lErr)
            {
                case ESP_ERR_ULTRASONIC_PING: {
                    ESP_LOGE(TAG, "Cannot ping (device is in invalid state)");
                    break;
                }

                case ESP_ERR_ULTRASONIC_PING_TIMEOUT: {
                    ESP_LOGE(TAG, "Ping timeout (no device found)");
                    break;
                }

                case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT: {
                    ESP_LOGE(TAG, "Echo timeout (no device found)");
                    break;
                }

                default: {
                    ESP_LOGE(TAG, "Sensor error! Code: 0x%X", lErr);
                }
            }
        }
        else {
            // Record value
            pModule->ubDistance = (uint8_t)(fDistance*100);

            // Set respective bit
            EventBits_t sBits;
            if(!strcmp(pModule->szName, "left")) {
                sBits = BIT_SENSOR_LEFT;
            } else if(!strcmp(pModule->szName, "mid")) {
                sBits = BIT_SENSOR_MID;
            } else {
                sBits = BIT_SENSOR_RIGHT;
            }

            xEventGroupSetBits(s_pEventGroup, sBits);

#ifdef DEBUG
            ESP_LOGW(TAG, "%f", fDistance*100);
#endif
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static esp_err_t s_createBumperTasks(Bumper_t* pBumper) {
    esp_err_t lErr = ESP_FAIL;

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sMid.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sMid, 
                                            5, 
                                            &pBumper->sMid.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sMid.szName);
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sLeft.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sLeft, 
                                            5, 
                                            &pBumper->sLeft.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sLeft.szName);
        goto end_create_task;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_bumperTask, 
                                            pBumper->sRight.szName, 
                                            configMINIMAL_STACK_SIZE * 3, 
                                            &pBumper->sRight, 
                                            5, 
                                            &pBumper->sRight.pHandle, 
                                            1)) {
        ESP_LOGE(TAG, "Failed to start %s sensor! Aborting...", pBumper->sRight.szName);
        goto end_create_task;
    }

    // Set all sensor bits
    xEventGroupSetBits(s_pEventGroup, s_ulCondition);

    ESP_LOGI(TAG, "Initialized bumper sensor array");
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
    strcpy(pBumper->sRight.szName, "right");

    return ESP_OK;
}

static void s_startCountdown(uint8_t ulTimeSeconds) {
    ESP_LOGI(TAG, "Starting in");
    for(int i = 0; i < ulTimeSeconds; i++) {
        vTaskDelay(ulTimeSeconds*200 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "%d", ulTimeSeconds-(i+1));
    }
}

esp_err_t bumperInit(void) {
    esp_err_t lErr = ESP_FAIL;

    s_sMutexBumper = xSemaphoreCreateMutexStatic(&s_sMutexBumperBuffer);
    if(NULL == s_sMutexBumper) {
        ESP_LOGE(TAG, "Failed to initialize bumper mutex! Aborting...");
        goto abort_init;
    }

    s_pEventGroup = xEventGroupCreateStatic(&s_sEventGroupBuffer);
    if(NULL == s_pEventGroup) {
        ESP_LOGE(TAG, "Failed to create event group! Aborting...");
        goto abort_init;
    }
    
    lErr = s_configureModules(&s_sBumper);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure sensor modules! Code: 0x%X. Aborting...", lErr);
        goto abort_init;
    }

    s_startCountdown(5);

    lErr = s_createBumperTasks(&s_sBumper);
    if(lErr) {
        goto abort_init;
    }

    if(pdPASS != xTaskCreatePinnedToCore(s_communicationTask,
                                            "tx_loop",
                                            configMINIMAL_STACK_SIZE*5,
                                            NULL,
                                            3,
                                            NULL,
                                            0)) {
        ESP_LOGE(TAG, "Failed to start communication task! Aborting...");
        lErr = ESP_FAIL;
        goto abort_init;
    }

    // pData = (uint8_t*)malloc((sizeof(uint8_t)*3)+sizeof(s_ulHeader)+sizeof(s_ulTerminator));
    // if(NULL == pData) {
    //     ESP_LOGE(TAG, "Failed to allocate memory for Tx buffer! Aborting...");
    //     lErr = ESP_ERR_NO_MEM;
    //     goto abort_init;
    // }

    lErr = ESP_OK;
abort_init:

    return lErr;
}