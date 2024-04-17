#ifndef __BUMPER_H__
#define __BUMPER_H__

#include "ultrasonic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" 


typedef struct {
    TaskHandle_t pHandle;
    char szName[16];
    uint8_t ubDistance;
    ultrasonic_sensor_t sSensor;
} SensorModule_t;

typedef struct {
    SensorModule_t sLeft;
    SensorModule_t sMid;
    SensorModule_t sRight;
} Bumper_t;


esp_err_t bumperInit(void);
    
#endif /* __BUMPER_H__ */