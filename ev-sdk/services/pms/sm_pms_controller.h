//
// Created by vnbk on 25/03/2025.
//

#ifndef EV_SDK_SM_PMS_CONTROLLER_H
#define EV_SDK_SM_PMS_CONTROLLER_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_sv_bp.h"

#define SM_PMS_CTL_SUCCESS   (0)
#define SM_PMS_CTL_FAILURE   (-1)

typedef void sm_pms_ctl_t;

/**
 * @brief
 * @param int32_t: SUCCESS/FAILURE
 * @param uint8_t: port
 * @param void*  : arg;
 * @return
 */
typedef void (*sm_pms_ctl_on_switched)(int32_t, uint8_t, void*);
typedef sm_pms_ctl_on_switched sm_pms_ctl_on_merged;

typedef sm_bp_on_cmd_fn_t sm_pms_ctl_on_power_off;

sm_pms_ctl_t* sm_pms_ctl_create(sm_sv_bp_t* _bpm);

int32_t sm_pms_ctl_destroy(sm_pms_ctl_t* _this);

int32_t sm_pms_ctl_switch(sm_pms_ctl_t* _this, uint8_t _port, SM_BP_CMD _cmd, sm_pms_ctl_on_switched _cb, void* _arg);

int32_t sm_pms_ctl_merge(sm_pms_ctl_t* _this, uint8_t _port, SM_BP_CMD _cmd, sm_pms_ctl_on_merged _cb, void* _arg);

int32_t sm_pms_ctl_power_off(sm_pms_ctl_t* _this, uint8_t _port, sm_pms_ctl_on_power_off, void* _arg);

int32_t sm_pms_ctl_release(sm_pms_ctl_t* _this);

int32_t sm_pms_ctl_process(sm_pms_ctl_t* _this);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMS_CONTROLLER_H
