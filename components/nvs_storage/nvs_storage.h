#ifndef COMPONENTS_NVS_STORAGE_NVS_STORAGE_H_
#define COMPONENTS_NVS_STORAGE_NVS_STORAGE_H_

#include "esp_err.h"

typedef enum {
    NVS_INT8_T,
    NVS_UINT8_T,
    NVS_INT16_T,
    NVS_UINT16_T,
    NVS_INT32_T,
    NVS_UINT32_T,
    NVS_INT64_T,
    NVS_UINT64_T,
    NVS_FLOAT,
    NVS_DOUBLE,
    NVS_TYPE_MAX,
} nvsType_t;

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param eType - value type
 * @param pData - value data

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsSave(char* szNamespace, char* szKey, nvsType_t eType, void* pData);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param eType - value type
 * @param pData - value data

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsRead(char* szNamespace, char* szKey, nvsType_t eType, void* pData);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param szStr - string value

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsSaveString(char* szNamespace, char* szKey, char* szStr);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param szStr - string value
 * @param pSize - size of string value

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsReadString(char* szNamespace, char* szKey, char* szStr, size_t* pSize);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param pData - value data
 * @param ulSize - value data size

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsSaveBlob(char* szNamespace, char* szKey, void* pData, size_t ulSize);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param pData - value data
 * @param pSize - value data size

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsReadBlob(char* szNamespace, char* szKey, void* pData, size_t* pSize);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param pSize - string value data size

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsGetStringSize(char* szNamespace, char* szKey, size_t* pSize);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key
 * @param pSize - value data size

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsGetBlobSize(char* szNamespace, char* szKey, size_t* pSize);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsEraseNamespace(char* szNamespace);

/**
 * @brief process data from buffer
 * 
 * @param szNamespace - namespace
 * @param szKey - key

 * @return ESP_OK if success, otherwise error code
 */
esp_err_t nvsEraseKey(char* szNamespace, char* szKey);

#endif