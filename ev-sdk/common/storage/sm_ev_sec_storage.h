//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_EV_SEC_STORAGE_H
#define EV_SDK_SM_EV_SEC_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_storage.h"
#include "sm_hal_flash.h"
#include "sm_ev_data.h"

typedef sm_storage_t sm_sec_storage_t;

sm_sec_storage_t* sm_ev_security_storage_create_default(sm_hal_flash_t* _flash, uint32_t _address, uint32_t _size);

sm_sec_storage_t* sm_ev_security_storage_create(sm_hal_flash_t* _flash, uint32_t _address, uint32_t _size);

int32_t sm_ev_security_storage_load(sm_sec_storage_t* _this, const char* _pass, void* _data);

int32_t sm_ev_security_storage_store(sm_sec_storage_t* _this, const char* _pass, const void* _data);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_SEC_STORAGE_H
