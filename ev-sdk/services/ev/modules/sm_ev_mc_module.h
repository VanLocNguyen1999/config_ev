//
// Created by vnbk on 17/08/2024.
//

#ifndef EV_SDK_SM_EV_MC_MODULE_H
#define EV_SDK_SM_EV_MC_MODULE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_ev_module.h"
#include "sm_core_co.h"
#include "sm_ev_cmd.h"

#define SM_MC_ENTER_STOP_MODE   (1)
#define SM_MC_EXIT_STOP_MODE    (2)

sm_ev_module_t* sm_mc_create(void* _owner, sm_co_t* _co);

int32_t sm_mc_destroy(sm_ev_module_t* _this);

int32_t sm_mc_set_drive_mode(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_mc_set_max_speed(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_mc_set_anti_theft_state(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_mc_set_stop_mode(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

void*  sm_mc_get_data(sm_ev_module_t* _this);
void sm_mc_co_received_data(uint32_t _can_id, uint8_t* _data, void* _arg);
#ifdef __cplusplus
};
#endif


#endif //EV_SDK_SM_EV_MC_MODULE_H
