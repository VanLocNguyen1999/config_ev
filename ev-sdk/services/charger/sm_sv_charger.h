//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_CHARGER_H
#define EV_SDK_SM_SV_CHARGER_H

#include "sm_types.h"
#include "sm_sv_bp.h"
#include "sm_pms_controller.h"

#define SM_CHARGER_SUPPORT_BP_NUMBER    SM_BP_NUMBER_DEFAULT

#define SM_SV_CHARGER_PORT_DISABLE            (0)
#define SM_SV_CHARGER_PORT_ENABLE             (!SM_SV_CHARGER_PORT_DISABLE)

#define SM_SV_CHARGER_MAX_VOL                   70000   // mV
#define SM_SV_CHARGER_MIN_VOL                   45000   // mV
#define SM_SV_CHARGER_MAX_CUR                   12000   // mA
#define SM_SV_CHARGER_STB_TIME                  5000    // ms
#define SM_SV_CHARGER_MAX_TEMP                  70      // oC
#define SM_SV_CHARGER_MIN_CUR                   500     // mA

#define SM_SV_CHARGER_DIFF_VOL_DEFAULT              2000    // mV
#define SM_SV_CHARGER_MIN_CUR_CUTOFF_DEFAULT        500     // mA
#define SM_SV_CHARGER_DETECT_CUTOFF_TIME_DEFAULT    20000   //20s
#define SM_SV_CHARGER_DETECT_PLUG_TIME_DEFAULT      2000    //2s

#define SM_SV_CHARGER_ERROR				      		4    //2s

enum {
    SM_CHARGER_OVER_CUR = 0,
    SM_CHARGER_OVER_VOL,
    SM_CHARGER_UNDER_CUR,
    SM_CHARGER_UNDER_VOL,
    SM_CHARGER_OVER_TEMP,
    SM_CHARGER_UNKNOWN,
};

#define SM_CHARGER_IS_PLUGGED       (1)
#define SM_CHARGER_IS_UNPLUGGED     (0)

#define SM_SV_CHARGER_EVENT_SUCCESS     SM_PMS_CTL_SUCCESS
#define SM_SV_CHARGER_EVENT_FAILURE     SM_PMS_CTL_FAILURE

#define SM_CHARGER_DETECT_PLUGGED_COUNTER       50

typedef void (*sm_charger_on_force_charging_cb_fn_t)(int32_t, uint8_t, void*);

typedef struct {
    void (*on_charger_is_plugged)(uint8_t, void*);
    void (*on_charged)(void*);
    void (*on_stop_charging)(void*);
    void (*on_err)(int32_t, void*);
    void (*on_forced_charging)(int32_t, uint8_t, void*);
    void (*on_released_charging)(int32_t, uint8_t, void*);
}sm_sv_charger_event_cb_fn_t;

typedef struct {
	int32_t m_min_volt;
	int32_t m_max_volt;
    int32_t	m_max_cur;
    int32_t	m_min_cur;
    int16_t	m_max_temp;
} sm_sv_charger_prof_t;

typedef struct{
    int32_t m_min_volt_diff;
    int32_t m_min_cur_cutoff;
    int32_t m_detect_plug_time;
    int32_t m_detect_cutoff_time;
}sm_sv_charger_config_t;

typedef struct {
    int32_t (*get_charger_vol)();
    int32_t (*ctl_charger_power)(uint8_t);
}sm_sv_charger_if_t;

typedef void sm_sv_charger_t;

/**
 * @brief:
 * @param _bp
 * @param _if
 * @return
 */
sm_sv_charger_t* sm_sv_charger_create(sm_sv_charger_if_t* _if,
                                       sm_pms_ctl_t* _ctl,
                                       sm_sv_charger_config_t* _config,
                                       sm_sv_bp_t* _bp);

int32_t sm_sv_charger_is_charging(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_destroy(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_pause(sm_sv_charger_t* _this);

/**
 *
 * @param _this
 * @return
 */
int32_t sm_sv_charger_is_paused(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_resume(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @param _fn_cb 
 * @param _arg 
 * @return 
 */
int32_t sm_sv_charger_reg_event(sm_sv_charger_t* _this, sm_sv_charger_event_cb_fn_t* _fn_cb, void* _arg);

/**
 * 
 * @param _this 
 * @param _prof 
 * @return 
 */
int32_t sm_sv_charger_set_profile(sm_sv_charger_t* _this, sm_sv_charger_prof_t* _prof);

/**
 * 
 * @param _this 
 * @return 
 */
const sm_sv_charger_prof_t* sm_sv_charger_get_profile(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_get_bp_num(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_get_cur(sm_sv_charger_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_get_volt(sm_sv_charger_t* _this);
/**
 *
 * @param _this
 * @return
 */
void sm_sv_charger_set_volt(sm_sv_charger_t* _this, int32_t vol);
/**
 *
 * @param _this
 * @param _bp_id
 * @return
 */
int32_t sm_sv_charger_force_bp(sm_sv_charger_t* _this,
                               uint8_t _bp_id,
                               sm_charger_on_force_charging_cb_fn_t _cb,
                               void* _arg);
/**
 *
 * @param _this
 * @return
 */
int32_t sm_sv_charger_release_bp(sm_sv_charger_t* _this);

/**
 *
 * @param _this
 * @param _bp_id
 * @return
 */
int32_t sm_sv_charger_enable_port(sm_sv_charger_t* _this, uint8_t _port, uint8_t _enable);

int32_t sm_sv_charger_is_err(sm_sv_charger_t* _this);
/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_sv_charger_process(sm_sv_charger_t* _this);
#endif //EV_SDK_SM_SV_CHARGER_H
