#include <stdio.h>
#include "nvs_storage.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char* TAG = "nvs_storage";

typedef enum {
    NVS_OPERATION_SIMPLE,
    NVS_OPERATION_STRING,
    NVS_OPERATION_BLOB
} nvsOperation_t;


// Static functions

static esp_err_t s_nvsSetSimple(nvs_handle ulHandle, char* szKey, nvsType_t eType, void* pData) {
    switch(eType) {
        case NVS_INT8_T:
            return nvs_set_i8(ulHandle, szKey, *((int8_t*)pData));
        case NVS_UINT8_T:
            return nvs_set_u8(ulHandle, szKey, *((uint8_t*)pData));
        case NVS_INT16_T:
            return nvs_set_i16(ulHandle, szKey, *((int16_t*)pData));
        case NVS_UINT16_T:
            return nvs_set_u16(ulHandle, szKey, *((uint16_t*)pData));
        case NVS_INT32_T:
            return nvs_set_i32(ulHandle, szKey, *((int32_t*)pData));
        case NVS_FLOAT:
        case NVS_UINT32_T:
            return nvs_set_u32(ulHandle, szKey, *((uint32_t*)pData));
        case NVS_INT64_T:
            return nvs_set_i64(ulHandle, szKey, *((int64_t*)pData));
        case NVS_UINT64_T:
        case NVS_DOUBLE:
            return nvs_set_u64(ulHandle, szKey, *((uint64_t*)pData));
        case NVS_TYPE_MAX:
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t s_nvsGetSimple(nvs_handle ulHandle, char* szKey, nvsType_t eType, void* pData) {
    switch(eType) {
        case NVS_INT8_T:
            return nvs_get_i8(ulHandle, szKey, (int8_t*)pData);
        case NVS_UINT8_T:
            return nvs_get_u8(ulHandle, szKey, (uint8_t*)pData);
        case NVS_INT16_T:
            return nvs_get_i16(ulHandle, szKey, (int16_t*)pData);
        case NVS_UINT16_T:
            return nvs_get_u16(ulHandle, szKey, (uint16_t*)pData);
        case NVS_INT32_T:
            return nvs_get_i32(ulHandle, szKey, (int32_t*)pData);
        case NVS_FLOAT:
        case NVS_UINT32_T:
            return nvs_get_u32(ulHandle, szKey,  (uint32_t*)pData);
        case NVS_INT64_T:
            return nvs_get_i64(ulHandle, szKey,  (int64_t*)pData);
        case NVS_UINT64_T:
        case NVS_DOUBLE:
            return nvs_get_u64(ulHandle, szKey,  (uint64_t*)pData);
        case NVS_TYPE_MAX:
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t s_nvsSaveBasic(char* szNamespace, char* szKey, nvsOperation_t eOperation, nvsType_t eType, void* pData, size_t ulSize) {
    nvs_handle ulHandle;

    esp_err_t lErr = nvs_open(szNamespace, NVS_READWRITE, &ulHandle);
    if(lErr) {
        ESP_LOGE(TAG, "Failed to get NVS handler! Code: 0x%X", lErr);
        return lErr;
    }

    switch(eOperation) {
        case NVS_OPERATION_SIMPLE:
            lErr = s_nvsSetSimple(ulHandle, szKey, eType, pData);
            break;
        
        case NVS_OPERATION_STRING:
            lErr = nvs_set_str(ulHandle, szKey, (const char*)pData);
            break;

        case NVS_OPERATION_BLOB:
            lErr = nvs_set_blob(ulHandle, szKey, pData, ulSize);
            break;
    }

    if(ESP_OK == lErr) {
        lErr = nvs_commit(ulHandle);
        if(lErr) {
            ESP_LOGE(TAG, "Failed to commit set name: %s | key: %s | error: 0x%X", szNamespace, szKey, lErr);
        }
    } else {
        ESP_LOGE(TAG, "Unable to set name: %s | key: %s | error: 0x%X", szNamespace, szKey, lErr);
    }

    nvs_close(ulHandle);

    return lErr;
}

static esp_err_t s_nvsReadBasic(char *szNamespace, char *szKey, nvsOperation_t eOperation, nvsType_t eType, void *pData, size_t *pSize) {
	nvs_handle ulHandle;
	esp_err_t lErr = nvs_open(szNamespace, NVS_READWRITE, &ulHandle);
	if(lErr) {
		ESP_LOGE(TAG, "unable to get NVS handler, error 0x%x", lErr);
		return lErr;
	}

	switch(eOperation) {
		case NVS_OPERATION_SIMPLE:
			lErr = s_nvsGetSimple(ulHandle, szKey, eType, pData);
			break;
		case NVS_OPERATION_STRING:
			lErr = nvs_get_str(ulHandle, szKey, (char*) pData, pSize);
			break;
		case NVS_OPERATION_BLOB:
			lErr = nvs_get_blob(ulHandle, szKey, pData, pSize);
			break;
	}
	if(ESP_ERR_NVS_NOT_FOUND == lErr) {
		ESP_LOGD(TAG, "get name: %s key: %s not found", szNamespace, szKey);
	} else if(lErr) {
		ESP_LOGE(TAG, "unable to get name: %s key: %s, error 0x%x", szNamespace, szKey, lErr);
	}

	nvs_close(ulHandle);
	return lErr;
}


// Public Functions

esp_err_t nvsSave(char *szNamespace, char *szKey, nvsType_t eType, void *pData)
{
	return s_nvsSaveBasic(szNamespace, szKey, NVS_OPERATION_SIMPLE, eType, pData, 0);
}

esp_err_t nvsRead(char *szNamespace, char *szKey, nvsType_t eType, void *pData)
{
	return s_nvsReadBasic(szNamespace, szKey, NVS_OPERATION_SIMPLE, eType, pData, NULL);
}

esp_err_t nvsSaveString(char *szNamespace, char *szKey, char *szStr)
{
	return s_nvsSaveBasic(szNamespace, szKey, NVS_OPERATION_STRING, 0, szStr, 0);
}

esp_err_t nvsReadString(char *szNamespace, char *szKey, char *szStr, size_t *pSize)
{
	return s_nvsReadBasic(szNamespace, szKey, NVS_OPERATION_STRING, 0, szStr, pSize);
}

esp_err_t nvsSaveBlob(char *szNamespace, char *szKey, void *pData, size_t ulSize)
{
	return s_nvsSaveBasic(szNamespace, szKey, NVS_OPERATION_BLOB, 0, pData, ulSize);
}

esp_err_t nvsReadBlob(char *szNamespace, char *szKey, void *pData, size_t *pSize)
{
	return s_nvsReadBasic(szNamespace, szKey, NVS_OPERATION_BLOB, 0, pData, pSize);
}

esp_err_t nvsGetStringSize(char *szNamespace, char *szKey, size_t *pSize)
{
	return s_nvsReadBasic(szNamespace, szKey, NVS_OPERATION_STRING, 0, NULL, pSize);
}

esp_err_t nvsGetBlobSize(char *szNamespace, char *szKey, size_t *pSize)
{
	return s_nvsReadBasic(szNamespace, szKey, NVS_OPERATION_BLOB, 0, NULL, pSize);
}

esp_err_t nvsEraseNamespace(char *szNamespace)
{
	nvs_handle ulHandle;
	esp_err_t lErr = nvs_open(szNamespace, NVS_READWRITE, &ulHandle);
	if(lErr) {
		ESP_LOGE(TAG, "unable to get NVS handler, error 0x%x", lErr);
		return lErr;
	}

	lErr = nvs_erase_all(ulHandle);
	if(ESP_OK == lErr) {
		lErr = nvs_commit(ulHandle);
		if(lErr) {
			ESP_LOGE(TAG, "unable to commit erase name: %s, error 0x%x", szNamespace, lErr);
		}
	} else if(ESP_ERR_NVS_NOT_FOUND == lErr) {
		ESP_LOGD(TAG, "erase name: %s not found", szNamespace);
	} else {
		ESP_LOGE(TAG, "unable to erase name: %s , error 0x%x", szNamespace, lErr);
	}

	nvs_close(ulHandle);
	return lErr;
}

esp_err_t nvsEraseKey(char *szNamespace, char *szKey)
{
	nvs_handle ulHandle;
	esp_err_t lErr = nvs_open(szNamespace, NVS_READWRITE, &ulHandle);
	if(lErr) {
		ESP_LOGE(TAG, "unable to get NVS handler, error 0x%x", lErr);
		return lErr;
	}

	lErr = nvs_erase_key(ulHandle, szKey);
	if(ESP_OK == lErr) {
		lErr = nvs_commit(ulHandle);
		if(lErr) {
			ESP_LOGE(TAG, "unable to commit erase name: %s key: %s, error 0x%x", szNamespace, szKey, lErr);
		}
	} else if(ESP_ERR_NVS_NOT_FOUND == lErr) {
		ESP_LOGD(TAG, "erase name: %s key: %s not found", szNamespace, szKey);
	} else {
		ESP_LOGE(TAG, "unable to erase name: %s key: %s, error 0x%x", szNamespace, szKey, lErr);
	}

	nvs_close(ulHandle);
	return lErr;
}

