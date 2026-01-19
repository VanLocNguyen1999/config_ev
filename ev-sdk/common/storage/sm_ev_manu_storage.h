//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_EV_MANU_STORAGE_H
#define EV_SDK_SM_EV_MANU_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_storage.h"
#include "sm_hal_flash.h"
#include "sm_ev_data.h"

#define DEVICE_KEY_LENGTH           9
#define DEVICE_KEY_DEFAULT          "selex123@"

sm_storage_t* sm_ev_manu_storage_create(sm_hal_flash_t* _data_flash, uint32_t _start_address);

int32_t sm_ev_manu_storage_validate(sm_storage_t* _this, const uint8_t* _data, int32_t _len);

#ifdef __cplusplus
extern "C" {
#endif

#endif //EV_SDK_SM_EV_MANU_STORAGE_H
