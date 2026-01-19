//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_EV_OPT_STORAGE_H
#define EV_SDK_SM_EV_OPT_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_storage.h"
#include "sm_hal_flash.h"

typedef void sm_opt_storage_t;

sm_opt_storage_t* sm_ev_opt_storage_create_default(sm_hal_flash_t* _flash, uint32_t _start_address);

int32_t sm_ev_opt_clear_storage(sm_storage_t* _this);

int32_t sm_ev_opt_load_max_speed(sm_storage_t* _this);
int32_t sm_ev_opt_store_max_speed(sm_storage_t* _this, int32_t _max_speed);

int32_t sm_ev_opt_load_drive_mode(sm_storage_t* _this);
int32_t sm_ev_opt_store_drive_mode(sm_storage_t* _this, int32_t _driver_mode);

int32_t sm_ev_opt_load_inactive_mode(sm_storage_t* _this, uint32_t* _odo_storage);
int32_t sm_ev_opt_store_inactive_mode(sm_storage_t* _this, int32_t _inactive_mode, uint32_t _odo);

int32_t sm_ev_opt_load_ble_uuid(sm_storage_t* _this, uint8_t* _uuid);
int32_t sm_ev_opt_store_ble_uuid(sm_storage_t* _this, const uint8_t* _uuid, int32_t _len);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_OPT_STORAGE_H
