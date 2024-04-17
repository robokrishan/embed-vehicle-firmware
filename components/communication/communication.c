#include <stdio.h>
#include "communication.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "device_config.h"
#include "esp_log.h"

static const char* TAG = "communication";

esp_err_t communicationInit(void) {
    esp_err_t lErr = ESP_OK;
    
    uart_config_t sConfig = {
        .baud_rate = 115200,
    };
    
    lErr = uart_param_config(UART_NUM_2, &sConfig);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to configure UART. Code: 0x%X", lErr);
    }

    return lErr;
}