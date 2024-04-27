#include <stdio.h>
#include "communication.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "device_config.h"
#include "esp_log.h"
#include "string.h"


static const char* TAG = "communication";


esp_err_t communicationWrite(uint8_t* pData, size_t ulLen) {
    esp_err_t lErr = ESP_OK;

    for(int i = 0; i < ulLen; i++) {
        printf("%02X", pData[i]);
    }
    printf("\n");

    return lErr;
}

esp_err_t communicationInit(void) {
    esp_err_t lErr = ESP_FAIL;
    
    uart_config_t sConfig = {
        .baud_rate = 115200,
    };

    lErr = uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to install UART driver! Code: 0x%X", lErr);
        goto abort_init;
    }
    
    lErr = uart_param_config(UART_NUM_0, &sConfig);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure UART! Code: 0x%X", lErr);
        goto abort_init;
    }

    lErr = ESP_OK;
abort_init:

    return lErr;
}