/*
 * sm_sv_discharger.h
 *
 *  Created on: 13 Oct 2025
 *      Author: My PC
 */

#ifndef SM_SV_DISCHARGER_H_
#define SM_SV_DISCHARGER_H_
#include "sm_types.h"
#include "sm_sv_bp.h"
#include "sm_pms_controller.h"

#define PMU_S2				(1)
#define PMU_S3				(!PMU_S2)

#define SM_SV_DISCHARGER_EVENT_SUCCESS     SM_PMS_CTL_SUCCESS
#define SM_SV_DISCHARGER_EVENT_FAILURE     SM_PMS_CTL_FAILURE

#define SM_DISCHARGER_SUPPORT_BP_NUMBER    SM_BP_NUMBER_DEFAULT

#define SM_SV_DISCHARGER_PORT_DISABLE            (0)
#define SM_SV_DISCHARGER_PORT_ENABLE             (!SM_SV_DISCHARGER_PORT_DISABLE)

#if PMU_S2
#define SM_SV_DISCHARGER_MAX_CUR_1BP                   50000   // mA
#define SM_SV_DISCHARGER_MAX_CUR_2BP                   45000   // mA
#define SM_SV_DISCHARGER_MAX_CUR_3BP                   35000   // mA
#else
#define SM_SV_DISCHARGER_MAX_CUR_1BP                   65000   // mA
#define SM_SV_DISCHARGER_MAX_CUR_2BP                   45000   // mA
#define SM_SV_DISCHARGER_MAX_CUR_3BP                   35000   // mA
#endif
#define SM_SV_DISCHARGER_ACTIVE_TEMP_LIMIT             50      // oC
#define SM_SV_DISCHARGER_DEACTIVE_TEMP_LIMIT           45      // oC
#define SM_SV_DISCHARGER_MIN_CUR                   	   5000     // mA

#define SM_SV_DISCHARGER_DIFF_VOL_DEFAULT              2000    // mV

typedef void (*sm_discharger_on_force_charging_cb_fn_t)(int32_t, uint8_t, void*);

typedef struct {
    void (*on_switched)(int32_t, uint8_t, void*);
    void (*on_merged)(int32_t, uint8_t, void*);
    void (*on_switch_forced)(int32_t, uint8_t, void*);
    void (*on_switch_released)(int32_t, uint8_t, void*);
    void (*on_paused)(int32_t, void*);
    void (*on_resumed)(int32_t, void*);
    void (*on_err)(int32_t, void*);
    void (*on_forced_discharging)(int32_t, uint8_t, void*);
    void (*on_released_discharging)(int32_t, uint8_t, void*);
}sm_sv_discharger_event_cb_fn_t;
typedef struct {
    int32_t	m_max_cur[SM_DISCHARGER_SUPPORT_BP_NUMBER];
    int32_t	m_min_cur;
    int16_t	m_temp_active_limit;
    int16_t	m_temp_deactive_limit;
} sm_sv_discharger_prof_t;

typedef struct{
    int32_t m_min_volt_diff;
    int32_t m_min_dischar_cur;
}sm_sv_discharger_config_t;

typedef void sm_sv_discharger_t;

sm_sv_discharger_t* sm_sv_discharger_create(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp);
int32_t sm_sv_discharger_pause(sm_sv_discharger_t* _this) ;
int32_t sm_sv_discharger_is_paused(sm_sv_discharger_t* _this);
int32_t sm_sv_discharger_resume(sm_sv_discharger_t* _this);
int32_t sm_sv_discharger_set_sw_state(sm_sv_discharger_t* _this, uint8_t state);
int32_t sm_sv_discharger_set_profile(sm_sv_discharger_t *_this,
		sm_sv_discharger_prof_t *_prof) ;
const sm_sv_discharger_prof_t* sm_sv_discharger_get_profile(
		sm_sv_discharger_t *_this);
int32_t sm_sv_get_discharger_bp_num(sm_sv_discharger_t* _this);
int32_t sm_sv_discharger_force_bp(sm_sv_discharger_t* _this,
                               uint8_t _bp_id,
							   sm_discharger_on_force_charging_cb_fn_t _cb,
                               void* _arg);
int32_t sm_sv_discharger_release_bp(sm_sv_discharger_t* _this);
int32_t sm_sv_discharger_process(sm_sv_discharger_t *_this);
#endif /* SM_SV_DISCHARGER_H_ */
