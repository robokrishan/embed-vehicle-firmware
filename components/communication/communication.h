#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include "esp_err.h"

#define BUFFER_SIZE                 (1024)
#define COMM_PACKET_HEADER          (0xAB3412FE)
#define COMM_PACKET_TERMINATOR      (0x0A0D)

esp_err_t communicationWrite(uint8_t* pData, size_t ulLen);

esp_err_t communicationInit(void);

#endif /* __COMMUNICATION_H__ */