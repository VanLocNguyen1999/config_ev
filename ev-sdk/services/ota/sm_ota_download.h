//
// Created by vnbk on 16/09/2024.
//

#ifndef EV_SDK_SM_OTA_DOWNLOAD_H
#define EV_SDK_SM_OTA_DOWNLOAD_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_http_client.h"
#include "sm_sv_ota.h"

#define SM_OTA_DOWNLOAD_SEG_FRAME_SIZE   1024

enum {
    SM_OTA_DOWNLOAD_FAILED = -1,
    SM_OTA_DOWNLOAD_NONE = 0,
    SM_OTA_DOWNLOAD_IN_PROCESS = 1,
    SM_OTA_DOWNLOAD_DONE = 2
};

typedef struct sm_ota_download sm_ota_download_t;

typedef struct {
    int32_t (*init)(sm_ota_download_t*, sm_ota_fw_storage_t*, int32_t);
    int32_t (*free)(sm_ota_download_t*);
    int32_t (*reg_cb)(sm_ota_download_t*, void (*on_err)(int32_t, bool, int32_t, const char*, void*), void*);
    int32_t (*set_param)(sm_ota_download_t*, void*);
    int32_t (*download)(sm_ota_download_t*, void*);
    int32_t (*validate)(sm_ota_download_t*, uint16_t);
}sm_ota_download_proc_t;

struct sm_ota_download{
    const sm_ota_download_proc_t* m_proc;
};

sm_ota_download_t* sm_ota_http_download_create(sm_http_client_t* _http_client);

sm_ota_download_t* sm_ota_ble_download_create(void* _ble);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_OTA_DOWNLOAD_H
