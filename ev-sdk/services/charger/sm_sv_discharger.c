#include "sm_sv_discharger.h"
#include "sm_elapsed_timer.h"
#include "sm_logger.h"
#include "sm_pms_controller.h"
#define _impl(x) ((sm_sv_discharger_impl_t*)(x))
typedef struct {
    sm_sv_discharger_prof_t* 	m_prof;
    sm_sv_discharger_config_t*  m_config;
    sm_sv_bp_t*                 m_bpm;
    sm_pms_ctl_t*               m_ctl;
    int32_t                     m_port_enable[SM_DISCHARGER_SUPPORT_BP_NUMBER];
    uint8_t						m_numb_dischar;

    elapsed_timer_t             m_timeout;

    int32_t                     m_counter;
    int32_t                     m_forced_bp;
    uint8_t                     m_in_process;
    int32_t                     m_err;
    uint8_t                     m_paused;

    uint8_t                     m_paused_sw;


    int32_t						m_dischar_cur;

    struct{

        int32_t index;
        int32_t vol;
    }m_next_bp[SM_DISCHARGER_SUPPORT_BP_NUMBER];
    void                          			*m_event_arg;
    sm_sv_discharger_event_cb_fn_t   		*m_event_cb;

    sm_discharger_on_force_charging_cb_fn_t m_force_cb;
    void*                                	m_force_arg;
} sm_sv_discharger_impl_t;


static sm_sv_discharger_impl_t g_discharger = {
        .m_bpm = NULL,
        .m_prof = NULL,
        .m_ctl = NULL,
        .m_event_cb = NULL,
        .m_event_arg = NULL,
        .m_paused = 0,
		.m_paused_sw = 0,
        .m_forced_bp = -1,
        .m_port_enable = {SM_SV_DISCHARGER_PORT_ENABLE,
        				  SM_SV_DISCHARGER_PORT_ENABLE,
						  SM_SV_DISCHARGER_PORT_ENABLE},
		.m_next_bp = {
					{ .index = -1, .vol = 0 },
					{ .index = -1, .vol = 0 },
					{ .index = -1, .vol = 0 },
					},
        .m_err = 0,
        .m_in_process = 0,
        .m_counter    = 0,
		.m_dischar_cur = 0,
		.m_numb_dischar = 0
};

static sm_sv_discharger_prof_t g_discharger_prof_default = {
		.m_max_cur[0] = SM_SV_DISCHARGER_MAX_CUR_1BP,
		.m_max_cur[1] = SM_SV_DISCHARGER_MAX_CUR_2BP,
		.m_max_cur[2] = SM_SV_DISCHARGER_MAX_CUR_3BP,
		.m_min_cur = SM_SV_DISCHARGER_MIN_CUR,
		.m_temp_active_limit = SM_SV_DISCHARGER_ACTIVE_TEMP_LIMIT,
		.m_temp_deactive_limit = SM_SV_DISCHARGER_DEACTIVE_TEMP_LIMIT
};

static sm_sv_discharger_config_t g_discharger_config_default = {
		.m_min_volt_diff = SM_SV_DISCHARGER_DIFF_VOL_DEFAULT,
		.m_min_dischar_cur = SM_SV_DISCHARGER_MIN_CUR
};

sm_sv_discharger_t* sm_sv_discharger_create(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp) {

    if(!_ctl || !_bp){
        LOG_ERR(TAG, "Param is INVALID");
        return NULL;
    }

    sm_sv_discharger_impl_t* discharger = &g_discharger;

    discharger->m_bpm = _bp;
    discharger->m_ctl = _ctl;
    discharger->m_config = &g_discharger_config_default;
    discharger->m_prof = &g_discharger_prof_default;

    return discharger;
}

int32_t sm_sv_discharger_pause(sm_sv_discharger_t* _this) {
    if (!_this) {
        return -1;
    }
//    (void)_this;
    sm_pms_ctl_release(_impl(_this)->m_ctl);
    _impl(_this)->m_in_process = 0;
    _impl(_this)->m_paused = 1;
    return 0;
}

int32_t sm_sv_discharger_is_paused(sm_sv_discharger_t* _this){
    if (!_this) {
        return -1;
    }
    return _impl(_this)->m_paused;
}

int32_t sm_sv_discharger_resume(sm_sv_discharger_t* _this) {
    if (!_this) {
        return -1;
    }
//    (void)_this;
    _impl(_this)->m_paused = 0;
    return 0;
}

int32_t sm_sv_discharger_set_sw_state(sm_sv_discharger_t* _this, uint8_t state) {
    if (!_this) {
        return -1;
    }
    g_discharger.m_paused_sw = (state) ? 0 : 1;
    return 0;
}
int32_t sm_sv_discharger_set_profile(sm_sv_discharger_t *_this,
		sm_sv_discharger_prof_t *_prof) {
	if (!_this) {
		return -1;
	}
	_impl(_this)->m_prof = _prof;
	return 0;
}

const sm_sv_discharger_prof_t* sm_sv_discharger_get_profile(
		sm_sv_discharger_t *_this) {
	if (!_this) {
		return NULL;
	}
	return _impl(_this)->m_prof;
}

int32_t sm_sv_get_discharger_bp_num(sm_sv_discharger_t* _this){
    if (!_this) {
        return -1;
    }
    const sm_bp_data_t* bp_data = NULL;
    int32_t number = 0;
    for(uint8_t i = 0; i < SM_DISCHARGER_SUPPORT_BP_NUMBER; ++i) {
        if(!sm_sv_bp_is_connected(_impl(_this)->m_bpm, i)){
            continue;
        }
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, i);
        if(bp_data->m_state == BP_STATE_DISCHARGING){
            number++;
        }
    }
    _impl(_this)->m_numb_dischar = (uint8_t)number;
    return number;
}
static void discharger_reset_force_bp(sm_sv_discharger_impl_t* _this){
    _this->m_forced_bp = -1;
    _this->m_force_arg = NULL;
    _this->m_force_cb = NULL;
}

static void discharger_ctl_on_power_off(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg){
    (void)_id;
    (void)_cmd;
    (void)_err;
    (void)_data;
    if (!_arg) {
        return;
    }
    sm_sv_discharger_impl_t* this = (sm_sv_discharger_impl_t*) _arg;
    this->m_in_process = 0;
}

static void discharger_ctl_on_switched_merged_cb(int32_t _success, uint8_t _id, void* _arg){
    (void)_success;
    (void)_id;
    if (!_arg) {
        return;
    }
    sm_sv_discharger_impl_t* this = (sm_sv_discharger_impl_t*) _arg;
    this->m_in_process = 0;

    if(_success == SM_PMS_CTL_SUCCESS && this->m_forced_bp >= 0){
        if(this->m_event_cb && this->m_event_cb->on_switch_forced){
            this->m_event_cb->on_switch_forced(SM_SV_DISCHARGER_EVENT_SUCCESS,
                                                 (uint8_t)this->m_forced_bp,
                                                this->m_force_arg);
        }
        if(this->m_force_cb){
            this->m_force_cb(SM_SV_DISCHARGER_EVENT_SUCCESS,
                             (uint8_t)this->m_forced_bp,
                             this->m_force_arg);
        }
    }
}

static int32_t discharger_force_switch_handle(sm_sv_discharger_impl_t* _this){
	  if(_this->m_in_process) {
	        return -1;
	    }

	    if(!sm_sv_bp_is_connected(_this->m_bpm, _this->m_forced_bp)){
	        LOG_ERR(TAG, "Forcing BP is disconnected. Release Forcing BP");
	        if(_this->m_event_cb && _this->m_event_cb->on_switch_released){
	            _this->m_event_cb->on_switch_released(SM_SV_DISCHARGER_EVENT_FAILURE,
	                                                  (uint8_t)_this->m_forced_bp,
	                                                  _this->m_force_arg);
	        }
	        discharger_reset_force_bp(_this);
	        return -2;
	    }

	    const sm_bp_data_t *bp_data = NULL;
	    bp_data = sm_sv_bp_get_data(_this->m_bpm, _this->m_forced_bp);
	    if(bp_data->m_state != BP_STATE_DISCHARGING){
	        _this->m_in_process = 1;
	        sm_pms_ctl_switch(_this->m_ctl, (uint8_t)_this->m_forced_bp,
	                          BP_CMD_DISCHARGE,
							  discharger_ctl_on_switched_merged_cb,
	                          _this);

	        return 0;
	    }

	    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
	        if(index == _this->m_forced_bp){
	            continue;
	        }
	        bp_data = sm_sv_bp_get_data(_this->m_bpm, index);
	        if(bp_data->m_state == BP_STATE_DISCHARGING){
	            _this->m_in_process = 1;
	            sm_pms_ctl_power_off(_this->m_ctl,
	                                 index,
									 discharger_ctl_on_power_off,
	                                 _this);
	            break;
	        }
	    }

	    return 0;
}
int32_t sm_sv_discharger_force_bp(sm_sv_discharger_t* _this,
                               uint8_t _bp_id,
							   sm_discharger_on_force_charging_cb_fn_t _cb,
                               void* _arg){
    if (!_this) {
        return -1;
    }

    if(_impl(_this)->m_paused ||
       !_impl(_this)->m_port_enable[_bp_id] ||
       !sm_sv_bp_is_connected(_impl(_this)->m_bpm, _bp_id)){
        return -2;
    }

    const sm_bp_data_t *bp_data = NULL;
    bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _bp_id);

    if(_bp_id == _impl(_this)->m_forced_bp && bp_data->m_state == BP_STATE_DISCHARGING){
        LOG_DBG(TAG, "BP is forcing charging BP: %d that in charging already", _bp_id);
        if(_cb) {
            _cb(SM_SV_DISCHARGER_EVENT_SUCCESS, _bp_id, _arg);
        }
        if(_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_forced_discharging) {
            _impl(_this)->m_event_cb->on_forced_discharging(SM_SV_DISCHARGER_EVENT_SUCCESS,
                                                       _bp_id,
                                                       _impl(_this)->m_event_arg);
        }
        return 0;
    }
    _impl(_this)->m_forced_bp = _bp_id;
    _impl(_this)->m_force_cb = _cb;
    _impl(_this)->m_force_arg = _arg;
    return 0;
}

int32_t sm_sv_discharger_release_bp(sm_sv_discharger_t* _this){
    if (_this == NULL || _impl(_this)->m_paused) {
        return -1;
    }
    _impl(_this)->m_forced_bp = -1;
    return 0;
}

static int32_t discharger_check_switch_merge_condition(sm_sv_discharger_impl_t* _this){
    const sm_bp_data_t* bp_data = NULL;
    int32_t vol_diff = 0, vol = 0, vol_discharging = 0;
    int32_t cur_allowed = 0;
    int32_t next_bp = -1;

    if(_this->m_in_process) {
        return 0;
    }

    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++) {
        if(!sm_sv_bp_is_connected(_this->m_bpm, index)){
            continue;
        }

        bp_data = sm_sv_bp_get_data(_this->m_bpm, index);
        if (bp_data->m_state == BP_STATE_STANDBY && vol < bp_data->m_vol) {
            vol = bp_data->m_vol;
            next_bp = index;
        }

        if(bp_data->m_state == BP_STATE_DISCHARGING || bp_data->m_state == BP_STATE_CHARGING){
            vol_discharging = bp_data->m_vol;
        }
        cur_allowed += bp_data->m_cur;
    }

    if (next_bp == -1){
        return -1;
    }

    vol_diff = vol - vol_discharging;
    if (abs(vol_diff) < _this->m_config->m_min_volt_diff) {
        LOG_INF(TAG, "Switch to PMS Merging state. Next BP in process: %d", next_bp);

        _this->m_in_process = 1;
        return sm_pms_ctl_merge(_this->m_ctl, (uint8_t)next_bp, BP_CMD_DISCHARGE, discharger_ctl_on_switched_merged_cb, _this);
    }else if (vol_diff > _this->m_config->m_min_volt_diff) {
        if(abs(cur_allowed) >= _this->m_config->m_min_dischar_cur){
            return -1;
        }
        LOG_INF(TAG, "Switch to PMS Pre-Switch state. Next BP in process: %d", next_bp);

        if(_this->m_paused_sw) return -1;
        _this->m_in_process = 1;
        return sm_pms_ctl_switch(_this->m_ctl, (uint8_t)next_bp, BP_CMD_DISCHARGE, discharger_ctl_on_switched_merged_cb, _this);
    }

    return -1;
}

int32_t sm_sv_discharger_process(sm_sv_discharger_t *_this) {
	if (!_this) {
		return -1;
	}
	sm_sv_discharger_impl_t *discharger = _impl(_this);
	if (discharger->m_paused) {
		return -2;
	}

	if(discharger->m_forced_bp > 0){

		discharger_force_switch_handle(_this);
		return 0;
	}
	discharger_check_switch_merge_condition(_this);
	return sm_sv_get_discharger_bp_num(_this);
}
