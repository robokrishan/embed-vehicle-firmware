#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "esp_err.h"


esp_err_t communicationWrite(uint8_t* pData, size_t ulLen);

esp_err_t communicationInit(void);

#endif /* __COMMUNICATION_H__ */