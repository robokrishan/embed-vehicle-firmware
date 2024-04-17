#include <stdio.h>
#include "communication.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "device_config.h"
#include "esp_log.h"


static const char* TAG = "communication";


esp_err_t communicationWrite(uint8_t* pData, size_t ulLen) {
    size_t ulWritten = uart_write_bytes(UART_NUM_1, pData, ulLen);
    if(ulLen == ulWritten) {\
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "Failed to write bytes to UART. Expected: %d | Written: %d", ulLen, ulWritten);
        return ESP_FAIL;
    }
}

esp_err_t communicationInit(void) {
    esp_err_t lErr = ESP_FAIL;
    
    uart_config_t sConfig = {
        .baud_rate = 115200,
    };

    lErr = uart_driver_install(UART_NUM_1, BUFFER_SIZE, 0, 0, NULL, 0);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to install UART driver! Code: 0x%X", lErr);
        goto abort_init;
    }
    
    lErr = uart_param_config(UART_NUM_1, &sConfig);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure UART! Code: 0x%X", lErr);
        goto abort_init;
    }

    lErr = uart_set_pin(UART_NUM_1, CONFIG_PIN_UART_TX1, CONFIG_PIN_UART_RX1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure UART pins! Code: 0x%X", lErr);
        goto abort_init;
    }

    lErr = ESP_OK;
abort_init:

    return lErr;
}