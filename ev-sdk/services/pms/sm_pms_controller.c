//
// Created by vnbk on 25/03/2025.
//
#include "sm_pms_controller.h"
#include "sm_logger.h"

#define TAG "SM_PMS_CONTROLLER"

#define _impl(x) ((sm_pms_ctl_impl_t*)(x))

const char* SM_BP_CMD_STR[] = {
        "BP_CMD_REBOOT",
        "BP_CMD_CHARGE",
        "BP_CMD_ONLY_DISCHARGE",
        "BP_CMD_DISCHARGE",
        "BP_CMD_STANDBY",
        "BP_CMD_READ_SN",
        "BP_CMD_READ_ASSIGNED_DEV",
        "BP_CMD_WRITE_ASSIGNED_DEV",
        "BP_CMD_READ_VERSION",
        "BP_CMD_RECONFIG_ID",
        "BP_CMD_NUMBER"
};

typedef enum {
    SM_PMS_ST_IDLE = 0,
    SM_PMS_ST_PRE_SWITCH,
    SM_PMS_ST_MAIN_SWITCH,
    SM_PMS_ST_FIN_SWITCH,
    SM_PMS_ST_MERGE,
    SM_PMS_ST_RECOVERY,
    SM_PMS_ST_FAIL
}SM_PMS_ST;

enum {
    PMS_CMD_IDLE = 0,
    PMS_CMD_IN_PROCESS,
};

typedef struct {
    sm_sv_bp_t              *m_bp;
    SM_PMS_ST			     m_state;
    elapsed_timer_t         m_timeout;

    uint8_t                 m_shadow_state[SM_BP_NUMBER_DEFAULT];
    struct {
        uint8_t m_cmd;
        uint8_t m_cmd_in_process;
        int32_t m_current_bp;
        int32_t m_next_bp;

        sm_pms_ctl_on_switched m_cb;
        void* m_cb_arg;
    }m_cmd;

}sm_pms_ctl_impl_t;

static sm_pms_ctl_impl_t g_pms_ctl_default = {
        .m_bp = NULL,
        .m_state = SM_PMS_ST_IDLE,
        .m_shadow_state = {
                BP_STATE_IDLE,
                BP_STATE_IDLE,
                BP_STATE_IDLE},
        .m_cmd = {
                .m_cmd = BP_CMD_NUMBER,
                .m_cmd_in_process = PMS_CMD_IDLE,
                .m_current_bp = -1,
                .m_next_bp = -1,
                .m_cb = NULL,
                .m_cb_arg = NULL
        }
};

static void pms_set_state(sm_pms_ctl_impl_t* _this, uint8_t _state);
static void pms_reset_cmd(sm_pms_ctl_impl_t* _this);

static void pms_on_cmd_bp(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg) {
    (void)_data;
    sm_pms_ctl_impl_t* _this = (sm_pms_ctl_impl_t*) _arg;

    LOG_INF(TAG, "PMS ON CMD %s BP %d %s", SM_BP_CMD_STR[_cmd], _id, _err ? "FAIL" : "SUCCESS");

    if(_cmd != BP_CMD_ONLY_DISCHARGE && _cmd != BP_CMD_DISCHARGE && _cmd != BP_CMD_CHARGE && _cmd != BP_CMD_STANDBY){
        return;
    }

    if (_err == SM_BP_CMD_SUCCESS){
        switch (_this->m_state) {
            case SM_PMS_ST_PRE_SWITCH:
                if(_cmd == BP_CMD_ONLY_DISCHARGE){
                    _this->m_shadow_state[_id] = BP_STATE_ONLY_DISCHARGING;
                }
                break;
            case SM_PMS_ST_RECOVERY:
                pms_set_state(_this, SM_PMS_ST_IDLE);
                break;
            case SM_PMS_ST_MAIN_SWITCH:
            case SM_PMS_ST_MERGE:
                _this->m_shadow_state[_id] = BP_STATE_DISCHARGING;
                break;
            case SM_PMS_ST_FIN_SWITCH:
                _this->m_shadow_state[_id] = BP_STATE_STANDBY;
                break;
            case SM_PMS_ST_IDLE:
            default:
                break;
        }
    }

    _this->m_cmd.m_cmd_in_process = PMS_CMD_IDLE;
}

static int32_t pms_set_cmd(sm_pms_ctl_impl_t* _this, SM_BP_CMD _cmd, int32_t _id) {
    sm_sv_bp_set_cmd(_this->m_bp,
                     _id,
                     _cmd,
                     NULL,
                     pms_on_cmd_bp,
                     _this);
    _this->m_cmd.m_cmd_in_process = PMS_CMD_IN_PROCESS;
    LOG_INF(TAG, "PMS SET CMD : %s, BP: %d", SM_BP_CMD_STR[_cmd], _id);
    return 0;
}

sm_pms_ctl_t* sm_pms_ctl_create(sm_sv_bp_t* _bpm){
    if (_bpm == NULL){
        return NULL;
    }
    sm_pms_ctl_impl_t *this = &g_pms_ctl_default;

    this->m_bp = _bpm;

    this->m_state = SM_PMS_ST_IDLE;

    for(uint8_t i = 0; i < SM_BP_NUMBER_DEFAULT; i++) {
        this->m_shadow_state[i] = BP_STATE_IDLE;
    }

    elapsed_timer_resetz(&this->m_timeout, 5000);
    return (sm_pms_ctl_impl_t*)this;
}

int32_t  sm_pms_ctl_destroy(sm_pms_ctl_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_bp = NULL;

    _impl(_this)->m_cmd.m_cmd = BP_CMD_NUMBER;
    _impl(_this)->m_cmd.m_cb = NULL;
    _impl(_this)->m_cmd.m_cb_arg = NULL;
    _impl(_this)->m_cmd.m_next_bp = -1;
    _impl(_this)->m_cmd.m_current_bp = -1;

    _impl(_this)->m_state = SM_PMS_ST_IDLE;

    return 0;
}

static void pms_set_state(sm_pms_ctl_impl_t* _this, uint8_t _state){
    _this->m_state = _state;
    elapsed_timer_reset(&_this->m_timeout);

    if(_state == SM_PMS_ST_IDLE){
        const sm_bp_data_t* bp_data = NULL;
        for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
            bp_data = sm_sv_bp_get_data(_this->m_bp, index);
            if(bp_data){
                _this->m_shadow_state[index] = (uint8_t)bp_data->m_state;
            }else{
                _this->m_shadow_state[index] = BP_STATE_IDLE;
            }
        }
    }
}

static void pms_reset_cmd(sm_pms_ctl_impl_t* _this){
    _this->m_cmd.m_cmd = BP_CMD_NUMBER;
    _this->m_cmd.m_next_bp = -1;
    _this->m_cmd.m_current_bp = -1;
    _this->m_cmd.m_cb = NULL;
    _this->m_cmd.m_cb_arg = NULL;
    _this->m_cmd.m_cmd_in_process = PMS_CMD_IDLE;
}

static void pms_recovery(sm_pms_ctl_impl_t* _this) {
    if(!elapsed_timer_get_remain(&_this->m_timeout)){
        LOG_ERR(TAG, "PMS recovery timeout");
        pms_set_state(_this, SM_PMS_ST_FAIL);
        return;
    }

    if(_this->m_cmd.m_cmd_in_process){
        return;
    }

    if (_this->m_cmd.m_current_bp != -1 && _this->m_cmd.m_current_bp < SM_BP_NUMBER_DEFAULT){
        pms_set_cmd(_this, _this->m_cmd.m_cmd, _this->m_cmd.m_current_bp);
    }
}

static void pms_pre_switch(sm_pms_ctl_impl_t* _this) {
    if(!elapsed_timer_get_remain(&_this->m_timeout)){
        LOG_ERR(TAG, "PMS Pre-Switch timeout");
        pms_set_state(_this, SM_PMS_ST_FAIL);
        return;
    }

    if(_this->m_cmd.m_cmd_in_process){
        return;
    }

    const sm_bp_data_t* bp_data = NULL;
    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        if(index == _this->m_cmd.m_next_bp || _this->m_shadow_state[index] == BP_STATE_ONLY_DISCHARGING){
            continue;
        }
        bp_data = sm_sv_bp_get_data(_this->m_bp, index);
        if(_this->m_cmd.m_cmd == BP_CMD_DISCHARGE && (bp_data->m_state == BP_STATE_DISCHARGING || bp_data->m_state == BP_STATE_CHARGING)){
            pms_set_cmd(_this, BP_CMD_ONLY_DISCHARGE, index);
            return;
        }
        if(_this->m_cmd.m_cmd == BP_CMD_CHARGE && (bp_data->m_state == BP_STATE_DISCHARGING || bp_data->m_state == BP_STATE_CHARGING)){
            pms_set_cmd(_this, BP_CMD_STANDBY, index);
            return;
        }
    }

    LOG_INF(TAG, "PMS switch to MAIN Switch State");
    pms_set_state(_this, SM_PMS_ST_MAIN_SWITCH);
}

static void pms_main_switch(sm_pms_ctl_impl_t *_this) {
    if (!elapsed_timer_get_remain(&_this->m_timeout)) {
        LOG_ERR(TAG, "PMS Main switch timeout");
        pms_set_state(_this, SM_PMS_ST_FAIL);
        return;
    }

    if (_this->m_cmd.m_cmd_in_process) {
        return;
    }

    const sm_bp_data_t *bp_data = sm_sv_bp_get_data(_this->m_bp, _this->m_cmd.m_next_bp);
    if (_this->m_shadow_state[_this->m_cmd.m_next_bp] != BP_STATE_DISCHARGING && bp_data->m_state == BP_STATE_STANDBY) {
        LOG_DBG(TAG, "In %s State: Set Discharge BP %d",
                _this->m_state == SM_PMS_ST_MAIN_SWITCH ? "MAIN SWITCH" : "MERGER",
                _this->m_cmd.m_next_bp);
        pms_set_cmd(_this, _this->m_cmd.m_cmd, _this->m_cmd.m_next_bp);
        return;
    }

    if (bp_data->m_state == BP_STATE_DISCHARGING || bp_data->m_state == BP_STATE_CHARGING) {
        if (_this->m_state == SM_PMS_ST_MERGE) {
            if (_this->m_cmd.m_cb) {
                _this->m_cmd.m_cb(SM_PMS_CTL_SUCCESS, (uint8_t)_this->m_cmd.m_next_bp, _this->m_cmd.m_cb_arg);
            }
            pms_reset_cmd(_this);
            pms_set_state(_this, SM_PMS_ST_IDLE);
        }else if(_this->m_state == SM_PMS_ST_MAIN_SWITCH){
            pms_set_state(_this, SM_PMS_ST_FIN_SWITCH);
        }
    }
}

static void pms_finish_switch(sm_pms_ctl_impl_t* _this) {
    if(!elapsed_timer_get_remain(&_this->m_timeout)){
        LOG_ERR(TAG, "PMS finish switch timeout, Recovery previous state");
        pms_set_state(_this, SM_PMS_ST_RECOVERY);
        return;
    }

    if(_this->m_cmd.m_cmd_in_process){
        return;
    }

    const sm_bp_data_t* bp_data = NULL;
    uint8_t check = 0;
    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        if(index == _this->m_cmd.m_next_bp){
            continue;
        }
        if(_this->m_shadow_state[index] == BP_STATE_ONLY_DISCHARGING){
            pms_set_cmd(_this, BP_CMD_STANDBY, index);
            return;
        }

        bp_data = sm_sv_bp_get_data(_this->m_bp, index);
        if(bp_data->m_state <= BP_STATE_IDLE || bp_data->m_state == BP_STATE_STANDBY){
            check++;
        }
    }

    if(check < 2){
    	return;
    }

    if(_this->m_cmd.m_cb){
        _this->m_cmd.m_cb(SM_PMS_CTL_SUCCESS, (uint8_t)_this->m_cmd.m_next_bp, _this->m_cmd.m_cb_arg);
    }

    LOG_DBG(TAG, "Finished switch process. Switch to IDLE state");
    pms_reset_cmd(_this);
    pms_set_state(_this, SM_PMS_ST_IDLE);
}

static int32_t sm_pms_ctl_check_cmd(sm_pms_ctl_impl_t* _this, uint8_t _port, SM_BP_CMD _cmd){
    if(!_this || _port >= SM_BP_NUMBER_DEFAULT){
        return -1;
    }

    if(_cmd != BP_CMD_DISCHARGE && _cmd != BP_CMD_CHARGE){
        LOG_ERR(TAG, "PMS Controller NOT Support");
        return -1;
    }

    sm_pms_ctl_impl_t* pms_ctl = _impl(_this);

    if(!sm_sv_bp_is_connected(pms_ctl->m_bp, _port)){
        LOG_ERR(TAG, "BP is disconnected, please check again");
        return -1;
    }

    if(pms_ctl->m_cmd.m_cmd < BP_CMD_NUMBER){
        LOG_ERR(TAG, "PMS Controller is busy");
        return -2;
    }

    return 0;
}

int32_t sm_pms_ctl_switch(sm_pms_ctl_t* _this, uint8_t _port, SM_BP_CMD _cmd, sm_pms_ctl_on_switched _cb, void* _arg){
    sm_pms_ctl_impl_t* pms_ctl = _impl(_this);
    if(sm_pms_ctl_check_cmd(pms_ctl, _port, _cmd) < 0){
        if(_cb){
            _cb(SM_PMS_CTL_FAILURE, _port, _arg);
        }
        return -1;
    }

    const sm_bp_data_t* bp_data = sm_sv_bp_get_data(pms_ctl->m_bp, _port);
    if((bp_data->m_state == BP_STATE_DISCHARGING && _cmd == BP_CMD_DISCHARGE) ||
            (bp_data->m_state == BP_STATE_CHARGING && _cmd == BP_CMD_CHARGE)){
        LOG_INF(TAG, "BP %d is already in the desired", _port);
        if(_cb){
            _cb(SM_PMS_CTL_SUCCESS, _port, _arg);
        }
        return 0;
    }

    pms_ctl->m_cmd.m_cmd = _cmd;
    pms_ctl->m_cmd.m_cmd_in_process = PMS_CMD_IDLE;
    pms_ctl->m_cmd.m_next_bp = _port;
    pms_ctl->m_cmd.m_cb = _cb;
    pms_ctl->m_cmd.m_cb_arg = _arg;

    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        if(index == _port){
            continue;
        }
        bp_data = sm_sv_bp_get_data(pms_ctl->m_bp, index);
        if((bp_data->m_state == BP_STATE_DISCHARGING && _cmd == BP_CMD_DISCHARGE) ||
                (bp_data->m_state == BP_STATE_CHARGING && _cmd == BP_CMD_CHARGE)) {
            pms_ctl->m_cmd.m_current_bp = index;
            break;
        }
    }

    pms_set_state(pms_ctl, SM_PMS_ST_PRE_SWITCH);

    return 0;
}

int32_t sm_pms_ctl_merge(sm_pms_ctl_t* _this, uint8_t _port, SM_BP_CMD _cmd,  sm_pms_ctl_on_merged _cb, void* _arg){
    sm_pms_ctl_impl_t* pms_ctl = _impl(_this);
    if(sm_pms_ctl_check_cmd(pms_ctl, _port, _cmd) < 0){
        if(_cb){
            _cb(SM_PMS_CTL_FAILURE, _port, _arg);
        }
        return -1;
    }

    const sm_bp_data_t* bp_data = sm_sv_bp_get_data(pms_ctl->m_bp, _port);
    if((bp_data->m_state == BP_STATE_DISCHARGING && _cmd == BP_CMD_DISCHARGE) ||
       (bp_data->m_state == BP_STATE_CHARGING && _cmd == BP_CMD_CHARGE)){
        LOG_INF(TAG, "BP %d is already in the desired", _port);
        if(_cb){
            _cb(SM_PMS_CTL_SUCCESS, _port, _arg);
        }
        return 0;
    }

    pms_ctl->m_cmd.m_cmd = _cmd;
    pms_ctl->m_cmd.m_cmd_in_process = PMS_CMD_IDLE;
    pms_ctl->m_cmd.m_next_bp = _port;
    pms_ctl->m_cmd.m_cb = _cb;
    pms_ctl->m_cmd.m_cb_arg = _arg;

    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        if(index == _port){
            continue;
        }
        bp_data = sm_sv_bp_get_data(pms_ctl->m_bp, index);
        if((bp_data->m_state == BP_STATE_DISCHARGING && _cmd == BP_CMD_DISCHARGE) ||
           (bp_data->m_state == BP_STATE_CHARGING && _cmd == BP_CMD_CHARGE)) {
            pms_ctl->m_cmd.m_current_bp = index;
            break;
        }
    }

    pms_set_state(pms_ctl, SM_PMS_ST_MERGE);

    return 0;
}

int32_t sm_pms_ctl_release(sm_pms_ctl_t* _this){
    sm_pms_ctl_impl_t* pms_ctl = (sm_pms_ctl_impl_t*)_this;
    if(!pms_ctl){
        return -1;
    }

    if(pms_ctl->m_cmd.m_cmd >= BP_CMD_NUMBER){
        LOG_WRN(TAG, "PMS controller did NOT handle any commands");
        return 0;
    }

    if(pms_ctl->m_cmd.m_cb){
        pms_ctl->m_cmd.m_cb(SM_PMS_CTL_FAILURE, (uint8_t)pms_ctl->m_cmd.m_next_bp, pms_ctl->m_cmd.m_cb_arg);
    }

    pms_reset_cmd(pms_ctl);
    pms_set_state(pms_ctl, SM_PMS_ST_IDLE);

    return 0;
}

int32_t sm_pms_ctl_power_off(sm_pms_ctl_t* _this, uint8_t _port, sm_pms_ctl_on_power_off _cb, void* _arg){
    sm_pms_ctl_impl_t* pms_ctl = (sm_pms_ctl_impl_t*)_this;
    if(!pms_ctl || _port >= SM_BP_NUMBER_DEFAULT){
        return -1;
    }

    return sm_sv_bp_set_cmd(pms_ctl->m_bp, _port, BP_CMD_STANDBY, NULL, _cb, _arg);
}

int32_t sm_pms_ctl_process(sm_pms_ctl_t* _this){
    sm_pms_ctl_impl_t* pms_ctl = (sm_pms_ctl_impl_t*)_this;
    if(!pms_ctl){
        return -1;
    }

    switch (pms_ctl->m_state) {
        case SM_PMS_ST_IDLE:
            break;
        case SM_PMS_ST_RECOVERY:
            pms_recovery(pms_ctl);
            break;
        case SM_PMS_ST_PRE_SWITCH:
            pms_pre_switch(pms_ctl);
            break;
        case SM_PMS_ST_MAIN_SWITCH:
        case SM_PMS_ST_MERGE:
            pms_main_switch(pms_ctl);
            break;
        case SM_PMS_ST_FIN_SWITCH:
            pms_finish_switch(pms_ctl);
            break;
        case SM_PMS_ST_FAIL:
            if(pms_ctl->m_cmd.m_cb){
                pms_ctl->m_cmd.m_cb(SM_PMS_CTL_FAILURE,
                                    (uint8_t)pms_ctl->m_cmd.m_next_bp,
                                    pms_ctl->m_cmd.m_cb_arg);
            }

            pms_reset_cmd(pms_ctl);
            pms_set_state(pms_ctl, SM_PMS_ST_IDLE);
            break;
        default:
            break;
    }
    return 0;
}
