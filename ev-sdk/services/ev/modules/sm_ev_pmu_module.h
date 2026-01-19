//
// Created by vnbk on 17/08/2024.
//

#ifndef EV_SDK_SM_EV_PMU_MODULE_H
#define EV_SDK_SM_EV_PMU_MODULE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_ev_module.h"
#include "sm_core_co.h"

#include "sm_ev_cmd.h"

#define EV_PMU_AUTH_SUPPORT         1
#define EV_PMU_AUTH_NOT_SUPPORT     (!EV_PMU_AUTH_SUPPORT)

#define EV_PMU_VER_AUTH_SUPPORT     "1.1."

typedef struct {
    void (*on_drive_mode_signal)(uint8_t, void*);
    void (*on_backward_mode_signal)(uint8_t, void*);
    void (*on_port_stated_changed)(uint8_t, uint8_t, void*);
    void (*on_err)(uint8_t, void*);
}sm_pmu_event_t;

sm_ev_module_t* sm_pmu_create(void* _owner, sm_co_t* _co, sm_pmu_event_t* _event_handle, void* _arg);
void*  sm_pmu_get_data(sm_ev_module_t* _this);
int32_t sm_pmu_destroy(sm_ev_module_t* _this);

int32_t sm_pmu_auth_support(sm_ev_module_t* _this);

int32_t sm_pmu_ctl_left_signal(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_right_signal(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_high_beam(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_low_beam(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_horn(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_find_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_lock_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_block_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_ctl_anti_theft_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_config_verify_bp_offline(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_set_lock_port(sm_ev_module_t* _this, uint8_t _port, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);

int32_t sm_pmu_set_enable_port(sm_ev_module_t* _this, uint8_t _port, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg);


#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_PMU_MODULE_H
