//
// Created by vnbk on 27/03/2025.
//

#include "sm_sv_charger.h"
#include "sm_elapsed_timer.h"
#include "sm_logger.h"

#define TAG  "SV_CHARGER"

#define _impl(x) ((sm_sv_charger_impl_t*)(x))

typedef struct {
    sm_sv_charger_prof_t* 	    m_prof;
    sm_sv_charger_if_t*         m_if;
    sm_sv_bp_t*                 m_bpm;
    sm_pms_ctl_t*               m_ctl;
    sm_sv_charger_config_t*     m_config;
    int32_t                     m_port_enable[SM_CHARGER_SUPPORT_BP_NUMBER];
    uint8_t						m_numb_charger;
    elapsed_timer_t             m_timeout;
    int32_t                     volt_charger;
    struct {
        uint8_t m_plugged           :1;
        uint8_t m_instant_plug      :1;
        uint8_t m_cutoff            :1;
        uint8_t m_instant_cutoff    :1;
    }m_flags;

    int32_t                     m_counter;
    int32_t                     m_forced_bp;
    uint8_t                     m_in_process;
    int32_t                     m_err;
    uint8_t                     m_paused;

    void                          *m_event_arg;
    sm_sv_charger_event_cb_fn_t   *m_event_cb;

    sm_charger_on_force_charging_cb_fn_t m_force_cb;
    void*                                m_force_arg;
} sm_sv_charger_impl_t;

static void    charger_reset_force_bp(sm_sv_charger_impl_t* _this);
static int32_t charger_forcing_handled(sm_sv_charger_impl_t* _this);
static int32_t charger_check_switch_merge_condition(sm_sv_charger_impl_t* _this);
static int32_t charger_check_plugging_status(sm_sv_charger_impl_t* _this);
static int32_t charger_current_monitor(sm_sv_charger_impl_t* _this);

static void charger_ctl_on_switched_merged_cb(int32_t _success, uint8_t _id, void* _arg);
static void charger_ctl_on_power_off(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg);

static sm_sv_charger_impl_t g_charger_default = {
        .m_bpm = NULL,
        .m_if = NULL,
        .m_prof = NULL,
        .m_ctl = NULL,
        .m_config = NULL,
        .m_event_cb = NULL,
        .m_event_arg = NULL,
        .m_paused = 0,
        .m_forced_bp = -1,
        .m_port_enable = {SM_SV_CHARGER_PORT_ENABLE,
                          SM_SV_CHARGER_PORT_ENABLE,
                          SM_SV_CHARGER_PORT_ENABLE},
        .m_flags = {SM_CHARGER_IS_UNPLUGGED,
                    SM_CHARGER_IS_UNPLUGGED,
                    0,
                    0},
        .m_err = 0,
        .m_in_process = 0,
        .m_counter    = 0,
		.m_numb_charger = 0,
		.volt_charger = 0,
};

static sm_sv_charger_prof_t g_charger_prof_default = {
		.m_max_volt			= SM_SV_CHARGER_MAX_VOL,
		.m_min_volt			= SM_SV_CHARGER_MIN_VOL,
        .m_max_cur          = SM_SV_CHARGER_MAX_CUR,
        .m_min_cur          = SM_SV_CHARGER_MIN_CUR,
        .m_max_temp         = SM_SV_CHARGER_MAX_TEMP,
};

static sm_sv_charger_config_t g_charger_config_default = {
        .m_min_volt_diff = SM_SV_CHARGER_DIFF_VOL_DEFAULT,
        .m_min_cur_cutoff = SM_SV_CHARGER_MIN_CUR_CUTOFF_DEFAULT,
        .m_detect_plug_time = SM_SV_CHARGER_DETECT_PLUG_TIME_DEFAULT,
        .m_detect_cutoff_time = SM_SV_CHARGER_DETECT_CUTOFF_TIME_DEFAULT,
};

sm_sv_charger_t* sm_sv_charger_create(sm_sv_charger_if_t* _if,
                                       sm_pms_ctl_t* _ctl,
                                       sm_sv_charger_config_t* _config,
                                       sm_sv_bp_t* _bp){

//    (void) _if;
    if(!_ctl || !_bp){
        LOG_ERR(TAG, "Param is INVALID");
        return NULL;
    }
/*    if(!_if->get_charger_vol || !_if->ctl_charger_power){
        LOG_ERR(TAG, "Charger Interface is NOT setting up");
        return NULL;
    }*/
    sm_sv_charger_impl_t* charger = &g_charger_default;

    charger->m_if = _if;
    charger->m_bpm = _bp;
    charger->m_ctl = _ctl;

    if(!_config){
        charger->m_config = &g_charger_config_default;
    }else{
        charger->m_config = _config;
    }

    charger->m_prof = &g_charger_prof_default;

    return charger;
}

int32_t sm_sv_charger_destroy(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    _impl(_this)->m_if = NULL;
    _impl(_this)->m_bpm = NULL;
    _impl(_this)->m_ctl = NULL;
    _impl(_this)->m_prof = NULL;
    _impl(_this)->m_event_cb = NULL;
    _impl(_this)->m_event_arg = NULL;
    return 0;
}

int32_t sm_sv_charger_is_charging(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    return _impl(_this)->m_flags.m_plugged;
}

int32_t sm_sv_charger_pause(sm_sv_charger_t* _this) {
    if (!_this) {
        return -1;
    }
    sm_pms_ctl_release(_impl(_this));
    _impl(_this)->m_in_process = 0;
    _impl(_this)->m_paused = 1;
    return 0;
}

int32_t sm_sv_charger_is_paused(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    return _impl(_this)->m_paused;
}

int32_t sm_sv_charger_resume(sm_sv_charger_t* _this) {
    if (!_this) {
        return -1;
    }
    _impl(_this)->m_paused = 0;
    return 0;
}

int32_t sm_sv_charger_reg_event(sm_sv_charger_t* _this,
                                sm_sv_charger_event_cb_fn_t* _cb_fn,
                                void* _arg){
    if(!_this || !_cb_fn){
        return -1;
    }
    _impl(_this)->m_event_arg = _arg;
    _impl(_this)->m_event_cb = _cb_fn;
    return 0;
}

int32_t sm_sv_charger_set_profile(sm_sv_charger_t* _this, sm_sv_charger_prof_t* _prof){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_prof = _prof;
    return 0;
}

const sm_sv_charger_prof_t* sm_sv_charger_get_profile(sm_sv_charger_t* _this){
    if(!_this){
        return NULL;
    }
    return _impl(_this)->m_prof;
}

int32_t sm_sv_charger_get_bp_num(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    const sm_bp_data_t* bp_data = NULL;
    int32_t number = 0;
    for(uint8_t i = 0; i < SM_CHARGER_SUPPORT_BP_NUMBER; ++i) {
        if(!sm_sv_bp_is_connected(_impl(_this)->m_bpm, i)){
            continue;
        }
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, i);
        if(bp_data->m_state == BP_STATE_CHARGING){
            number++;
        }
    }
    _impl(_this)->m_numb_charger = (uint8_t)number;
    return number;
}

int32_t sm_sv_charger_get_cur(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    const sm_bp_data_t* bp_data = NULL;
    int32_t cur = 0;
    for(uint8_t i = 0; i < SM_CHARGER_SUPPORT_BP_NUMBER; ++i) {
        if(!sm_sv_bp_is_connected(_impl(_this)->m_bpm, i)){
            continue;
        }
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, i);
        cur += bp_data->m_cur;
    }
    return cur;
}

int32_t sm_sv_charger_get_volt(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    return _impl(_this)->volt_charger;
}

void sm_sv_charger_set_volt(sm_sv_charger_t* _this, int32_t vol){

    if (!_this) {
        return;
    }
    _impl(_this)->volt_charger = vol;
}

int32_t sm_sv_charger_force_bp(sm_sv_charger_t* _this,
                               uint8_t _bp_id,
                               sm_charger_on_force_charging_cb_fn_t _cb,
                               void* _arg){
    if (!_this) {
        return -1;
    }

    if(_impl(_this)->m_paused ||
       !_impl(_this)->m_port_enable[_bp_id] ||
       !sm_sv_bp_is_connected(_impl(_this)->m_bpm, _bp_id)){
        LOG_ERR(TAG, "BP port INVALID, Please check again");
        return -2;
    }

    const sm_bp_data_t *bp_data = NULL;
    bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _bp_id);

    if(_bp_id == _impl(_this)->m_forced_bp && bp_data->m_state == BP_STATE_DISCHARGING){
        LOG_DBG(TAG, "BP is forcing charging BP: %d that in charging already", _bp_id);
        if(_cb) {
            _cb(SM_SV_CHARGER_EVENT_SUCCESS, _bp_id, _arg);
        }
        if(_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_forced_charging) {
            _impl(_this)->m_event_cb->on_forced_charging(SM_SV_CHARGER_EVENT_SUCCESS,
                                                       _bp_id,
                                                       _impl(_this)->m_event_arg);
        }
        return 0;
    }
    LOG_DBG(TAG, "Set force charging BP: %d", _bp_id);
    _impl(_this)->m_forced_bp = _bp_id;
    _impl(_this)->m_force_cb = _cb;
    _impl(_this)->m_force_arg = _arg;
    return 0;
}

int32_t sm_sv_charger_release_bp(sm_sv_charger_t* _this){
    if (_this == NULL || _impl(_this)->m_paused) {
        return -1;
    }
    _impl(_this)->m_forced_bp = -1;
    return 0;
}

int32_t sm_sv_charger_enable_port(sm_sv_charger_t* _this, uint8_t _port, uint8_t _enable){
    if(!_this || _port >= SM_CHARGER_SUPPORT_BP_NUMBER) {
        return -1;
    }

    if(_enable == SM_SV_CHARGER_PORT_DISABLE){
        const sm_bp_data_t* bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _port);
        if(bp_data->m_state == BP_STATE_CHARGING || bp_data->m_state == BP_STATE_DISCHARGING){
            sm_pms_ctl_power_off(_impl(_this)->m_ctl, _port, NULL, NULL);
        }

        if(_impl(_this)->m_forced_bp == _port){
            LOG_WRN(TAG, "Reset forcing BP because this BP is disable");
            if (_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_released_charging) {
                _impl(_this)->m_event_cb->on_released_charging(SM_SV_CHARGER_EVENT_SUCCESS,
                                                               (uint8_t)_impl(_this)->m_forced_bp,
                                                               _impl(_this)->m_event_arg);
            }

            charger_reset_force_bp(_impl(_this));
        }
    }

    _impl(_this)->m_port_enable[_port] = _enable;
    return 0;
}

int32_t sm_sv_charger_process(sm_sv_charger_t* _this){
    if (!_this){
        return -1;
    }
    sm_sv_charger_impl_t* charger = _impl(_this);
    if(charger->m_paused){
        return -2;
    }

    if(charger_check_plugging_status(charger) == SM_CHARGER_IS_UNPLUGGED){
        return -3;
    }

    if(charger->m_forced_bp >= 0){
        charger_forcing_handled(charger);
    }else{
        charger_check_switch_merge_condition(charger);
    }

    return charger_current_monitor(charger);

}

/**********************************************************************************************************************/
static int32_t charger_current_monitor(sm_sv_charger_impl_t* _this){
    int32_t cur = sm_sv_charger_get_cur(_this);
    cur = abs(cur);
    if(cur > _this->m_prof->m_max_cur){
        if(_this->m_event_cb && _this->m_event_cb->on_err){
            _this->m_event_cb->on_err(SM_CHARGER_OVER_CUR,
                                      _this->m_event_arg);
        }
    }

    /// Handle cutoff current
    if(abs(cur) < _this->m_config->m_min_cur_cutoff){
        if(!_this->m_flags.m_instant_cutoff){
            elapsed_timer_resetz(&_this->m_timeout, _this->m_config->m_detect_cutoff_time);
            _this->m_flags.m_instant_cutoff = 1;
        }

        if(!elapsed_timer_get_remain(&_this->m_timeout) && _this->m_flags.m_instant_plug && !_this->m_flags.m_cutoff){
            _this->m_flags.m_cutoff = 1;
            if(_this->m_event_cb && _this->m_event_cb->on_stop_charging){
                _this->m_event_cb->on_stop_charging(_this->m_event_arg);
            }
        }
    }else{
        if(_this->m_flags.m_instant_cutoff){
            _this->m_flags.m_instant_cutoff = 0;
            _this->m_flags.m_cutoff = 0;
        }
    }

    return cur;
}

static int32_t charger_check_plugging_status(sm_sv_charger_impl_t* _this){

	   int32_t vol = _impl(_this)->volt_charger;
	    if(vol > _impl(_this)->m_prof->m_min_volt && vol < _impl(_this)->m_prof->m_max_volt){ /// Detect plugging charger
	        if(_impl(_this)->m_flags.m_instant_plug == SM_CHARGER_IS_UNPLUGGED){
	            _impl(_this)->m_flags.m_instant_plug = SM_CHARGER_IS_PLUGGED;
	            elapsed_timer_reset(&_impl(_this)->m_timeout);
	        }

	        if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout) &&
	                _impl(_this)->m_flags.m_instant_plug == SM_CHARGER_IS_PLUGGED &&
	                _impl(_this)->m_flags.m_plugged == SM_CHARGER_IS_UNPLUGGED){
	            _impl(_this)->m_flags.m_plugged = SM_CHARGER_IS_PLUGGED;
	            if(_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_charger_is_plugged){
	                _impl(_this)->m_event_cb->on_charger_is_plugged(SM_CHARGER_IS_PLUGGED,
	                                                                _impl(_this)->m_event_arg);
	            }

	        }
	    }else if(vol <= _impl(_this)->m_prof->m_min_volt){ /// Detect unplugging charger

	        if (_impl(_this)->m_flags.m_instant_plug == SM_CHARGER_IS_PLUGGED){

	            _impl(_this)->m_flags.m_instant_plug = SM_CHARGER_IS_UNPLUGGED;
	            elapsed_timer_reset (&_impl(_this)->m_timeout);
	        }
	        if (!elapsed_timer_get_remain (&_impl(_this)->m_timeout) &&
	                _impl(_this)->m_flags.m_instant_plug == SM_CHARGER_IS_UNPLUGGED &&
	                _impl(_this)->m_flags.m_plugged == SM_CHARGER_IS_PLUGGED) {

	            _impl(_this)->m_flags.m_plugged = SM_CHARGER_IS_UNPLUGGED;
	            if (_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_charger_is_plugged){
	                _impl(_this)->m_event_cb->on_charger_is_plugged (SM_CHARGER_IS_UNPLUGGED, _impl(_this)->m_event_arg);
	            }

	        }
	    }else {

	        if(_impl(_this)->m_err == 0){

	            _impl(_this)->m_err = 1;
	            elapsed_timer_reset (&_impl(_this)->m_timeout);
	        }

	        if(!elapsed_timer_get_remain (&_impl(_this)->m_timeout) && (_impl(_this)->m_err == 1)){

	            sm_sv_charger_pause(_this);
	            if(_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_err){
	                _impl(_this)->m_event_cb->on_err(_impl(_this)->m_err,
	                                                 _impl(_this)->m_event_arg);
	            }
	        }
	    }

	    return _impl(_this)->m_flags.m_plugged;
}

static int32_t charger_forcing_handled(sm_sv_charger_impl_t* _this){
    if(_this->m_in_process) {
        return -1;
    }
    if(!sm_sv_bp_is_connected(_this->m_bpm, _this->m_forced_bp)){
        LOG_ERR(TAG, "Force charging BP is disconnected. Release Forcing BP");
        if(_this->m_event_cb && _this->m_event_cb->on_released_charging){
            _this->m_event_cb->on_released_charging(SM_SV_CHARGER_EVENT_FAILURE,
                                                    (uint8_t)_this->m_forced_bp,
                                                    _this->m_force_arg);
        }
        charger_reset_force_bp(_this);
        return -2;
    }

    const sm_bp_data_t *bp_data = NULL;
    /// Off all other PIN
    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        if(index == _this->m_forced_bp){
            continue;
        }
        bp_data = sm_sv_bp_get_data(_this->m_bpm, index);
        if(bp_data->m_state == BP_STATE_CHARGING || bp_data->m_state == BP_STATE_DISCHARGING){
            _this->m_in_process = 1;
            sm_pms_ctl_power_off(_this->m_ctl,
                                 index,
                                 charger_ctl_on_power_off,
                                 _this);
            break;
        }
    }

    /// Charging forcing BP
    bp_data = sm_sv_bp_get_data(_this->m_bpm, _this->m_forced_bp);
    if(bp_data->m_state != BP_STATE_DISCHARGING){
        _this->m_in_process = 1;
        sm_pms_ctl_switch(_this->m_ctl, (uint8_t)_this->m_forced_bp,
                          BP_CMD_CHARGE,
                          charger_ctl_on_switched_merged_cb,
                          _this);
    }

    return 0;
}

static void charger_reset_force_bp(sm_sv_charger_impl_t* _this){
    _this->m_forced_bp = -1;
    _this->m_force_arg = NULL;
    _this->m_force_cb = NULL;
}

static void charger_ctl_on_power_off(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg){
    (void)_id;
    (void)_cmd;
    (void)_err;
    (void)_data;
    if (!_arg) {
        return;
    }
    sm_sv_charger_impl_t* this = (sm_sv_charger_impl_t*) _arg;
    this->m_in_process = 0;
}

static void charger_ctl_on_switched_merged_cb(int32_t _success, uint8_t _id, void* _arg){
    (void)_success;
    (void)_id;
    if (!_arg) {
        return;
    }
    sm_sv_charger_impl_t* this = (sm_sv_charger_impl_t*) _arg;
    this->m_in_process = 0;

    if(_success == SM_PMS_CTL_SUCCESS && this->m_forced_bp >= 0){
        if(this->m_event_cb && this->m_event_cb->on_forced_charging){
            this->m_event_cb->on_forced_charging(SM_SV_CHARGER_EVENT_SUCCESS,
                                                 (uint8_t)this->m_forced_bp,
                                                this->m_force_arg);
        }
        if(this->m_force_cb){
            this->m_force_cb(SM_SV_CHARGER_EVENT_SUCCESS,
                             (uint8_t)this->m_forced_bp,
                             this->m_force_arg);
        }
    }
}

#if 0
static void charger_on_cmd_cb(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg){

    (void)_id;
    (void)_cmd;
    (void)_err;
    (void)_data;
    if (!_arg) {
        return;
    }
    sm_sv_charger_impl_t* this = (sm_sv_charger_impl_t*) _arg;
    this->m_in_process = 0;
}
#endif

static int32_t charger_check_switch_merge_condition(sm_sv_charger_impl_t* _this){
    const sm_bp_data_t* bp_data = NULL;
    int32_t vol_diff = 0, vol = 0, vol_charging = 0;
    int32_t next_bp = -1;

    if(_this->m_in_process){
        return 0;
    }

    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++) {
        if(_this->m_port_enable[index] == SM_SV_CHARGER_PORT_DISABLE || !sm_sv_bp_is_connected(_this->m_bpm, index)){
            continue;
        }

        bp_data = sm_sv_bp_get_data(_this->m_bpm, index);
        if((bp_data->m_state == BP_STATE_DISCHARGING || bp_data->m_state == BP_STATE_CHARGING) && vol_charging <= 0){
            vol_charging = bp_data->m_vol;
        }

        if (bp_data->m_state == BP_STATE_STANDBY) {
            if(vol <= 0 || vol > bp_data->m_vol){
                vol = bp_data->m_vol;
                next_bp = index;
            }
        }
    }

    if (next_bp == -1){
        return -1;
    }

    if(vol_charging <= 0){
        LOG_INF(TAG, "Charger: Switch to BP that voltage is minimum", next_bp);

        _this->m_in_process = 1;
        return sm_pms_ctl_merge(_this->m_ctl, (uint8_t)next_bp, BP_CMD_CHARGE, charger_ctl_on_switched_merged_cb, _this);
    }
    vol_diff = vol_charging - vol;
    if (abs(vol_diff) < _this->m_config->m_min_volt_diff) {
        LOG_INF(TAG, "Charger: Switch to Merging state. Next BP in process: %d", next_bp);

        _this->m_in_process = 1;
        return sm_pms_ctl_merge(_this->m_ctl, (uint8_t)next_bp, BP_CMD_CHARGE, charger_ctl_on_switched_merged_cb, _this);
    }else if (vol_diff > _this->m_config->m_min_volt_diff) {
        LOG_INF(TAG, "Charger: Switch to PMS Pre-Switch state. Next BP in process: %d", next_bp);

        _this->m_in_process = 1;
        return sm_pms_ctl_switch(_this->m_ctl, (uint8_t)next_bp, BP_CMD_CHARGE, charger_ctl_on_switched_merged_cb, _this);
    }

    return -1;
}


int32_t sm_sv_charger_is_err(sm_sv_charger_t* _this){

	if(!_this) return -1;
    return (_impl(_this)->m_err) ? -1 : 0;
}
