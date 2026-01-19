//
// Created by vnbk on 15/08/2024.
//

#ifndef EV_SDK_SM_EV_SERVICE_H
#define EV_SDK_SM_EV_SERVICE_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"
#include "sm_core_co.h"
#include "sm_ev_odo_storage.h"

#include "sm_bp_data.h"
#include "sm_sv_bp.h"
#include "sm_ev_data.h"
#include "sm_ev_config.h"
#include "sm_ev_cmd.h"
#include "sm_storage.h"
#include "sm_ev_sec_storage.h"
#include "modules/sm_ev_module.h"

typedef void sm_sv_ev_t;

typedef void (*sm_sv_ev_on_cmd_fn_t)(int32_t, uint8_t, void*, void*);

enum{
    SM_MOTOR_ACTIVE_CONDITION_EXIT_PACKING              = 0x01,
    SM_MOTOR_ACTIVE_CONDITION_OFFLINE                   = 0x02,
    SM_MOTOR_ACTIVE_CONDITION_BP_OFFLINE_INVALID        = 0x04,
    SM_MOTOR_ACTIVE_CONDITION_BP_ONLINE_INVALID         = 0x08,
    SM_MOTOR_ACTIVE_CONDITION_EV_LOCKING                = 0x10,
};

enum {
    SM_EV_EVENT_KEY_CHANGED,
    SM_EV_EVENT_LEFT_SIGNAL_CHANGED,
    SM_EV_EVENT_RIGHT_SIGNAL_CHANGED,
    SM_EV_EVENT_PARKING_CHANGED,
    SM_EV_EVENT_DRIVE_MODE_SIGNAL_CHANGED,
    SM_EV_EVENT_DRIVE_MODE_CHANGED,
    SM_EV_EVENT_PHASE_LIGHT_CHANGED,
    SM_EV_EVENT_COS_LIGHT_CHANGED,
    SM_EV_EVENT_RANGE_CHANGED,
    SM_EV_EVENT_BACKWARD_MODE_SIGNAL_CHANGED,
    SM_EV_EVENT_BACKWARD_MODE_CHANGED,
    SM_EV_EVENT_ANTI_THEFT_STATUS_CHANGED,
    SM_EV_EVENT_LOCK_MODE_CHANGED,
    SM_EV_EVENT_BLOCK_MODE_CHANGED,
    SM_EV_EVENT_ANTI_THEFT_MODE_CHANGED,
    SM_EV_EVENT_INACTIVE_MODE_CHANGED,
    SM_EV_EVENT_ODO_CHANGED,
    SM_EV_EVENT_SPEED_CHANGED,
    SM_EV_EVENT_MC_STATUS_CHANGED,
    SM_EV_EVENT_TRIP_CHANGED,
    SM_EV_EVENT_ERROR_CHANGED,
    SM_EV_EVENT_NUMBER
};

enum {
    SM_BP_EVENT_LOCK_PORT_CHANGED,
    SM_BP_EVENT_VALIDATE_CHANGED,
    SM_BP_EVENT_CONNECTED,
    SM_BP_EVENT_DISCONNECTED,
    SM_BP_EVENT_STATE_CHANGED,
    SM_BP_EVENT_NUMBER
};

typedef struct{
    void (*on_ev_event)(int32_t, int32_t, void*);
    void (*on_bp_event)(int32_t, int32_t, int32_t, void*);
}sm_sv_ev_event_t;

sm_sv_ev_t* sm_sv_ev_create(sm_ev_manu_t* _ev_manu,
                            sm_co_t* _co,
                            sm_sv_bp_t* _bp_manager,
                            sm_ev_config_t * _ev_config,
                            sm_sec_storage_t* _backup_odo_storage,
                            sm_odo_storage_t* _odo_storage,
                            sm_storage_t* _opt_storage);

int32_t sm_sv_ev_destroy(sm_sv_ev_t* _this);

int32_t sm_sv_ev_reg_event(sm_sv_ev_t* _this, sm_sv_ev_event_t* _event_handle, void* _arg);

int32_t sm_sv_ev_set_cmd(sm_sv_ev_t* _this, uint8_t _cmd, void* _value, sm_sv_ev_on_cmd_fn_t _cb, void* _arg);

int32_t sm_sv_ev_reboot_module(sm_sv_ev_t* _this, int32_t _module, uint32_t _delay_time);
int32_t sm_sv_ev_reboot_bp(sm_sv_ev_t* _this, int32_t _slot);

int32_t sm_sv_ev_set_odo(sm_sv_ev_t* _this, uint32_t _odo, const char* _pass);

int32_t sm_sv_ev_set_motor_active_condition(sm_sv_ev_t* _this, uint8_t _condition);
int32_t sm_sv_ev_reset_motor_active_condition(sm_sv_ev_t* _this, uint8_t _condition);

int32_t sm_sv_ev_get_module_type_by_name(sm_sv_ev_t* _this, const char* _name);
const char* sm_sv_ev_get_module_name_by_type(sm_sv_ev_t* _this, int32_t _module);

const sm_module_info_t* sm_sv_ev_get_module_info(sm_sv_ev_t* _this, int32_t _module);
int32_t sm_sv_ev_reset_module_info(sm_sv_ev_t* _this, int32_t _module);
const char* sm_sv_ev_get_module_version(sm_sv_ev_t* _this, int32_t _module);
int32_t sm_ev_check_module_pre_download_condition_by_name(sm_sv_ev_t* _this, const char* _name);
int32_t sm_ev_check_module_pre_upgrade_condition_by_type(sm_sv_ev_t* _this, int32_t _module);

const char* sm_sv_ev_get_module_version_by_name(sm_sv_ev_t* _this, const char* _name);

sm_sv_bp_t* sm_sv_ev_get_bp_sv(sm_sv_ev_t* _this);
const sm_ev_data_t* sm_sv_ev_get_data(sm_sv_ev_t* _this);
const sm_bp_data_t* sm_sv_ev_get_bp_data(sm_sv_ev_t* _this, uint8_t _slot);
int32_t sm_ev_check_bp_general_ota_condition(sm_sv_ev_t* _this, uint8_t _slot, const char* _new_ver);
int32_t sm_ev_check_bp_version_different(sm_sv_ev_t* _this, uint8_t _slot, const char* _version);


int32_t sm_sv_ev_process(sm_sv_ev_t* _this);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_SERVICE_H
