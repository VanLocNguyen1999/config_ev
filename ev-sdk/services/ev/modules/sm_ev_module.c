//
// Created by vnbk on 16/08/2024.
//
#include "sm_ev_module.h"
#include "sm_logger.h"

#define TAG "SM_EV_MODULE"

int32_t sm_module_get_id(sm_ev_module_t* _this){
    return _this->m_id;
}

int32_t sm_module_reg_event_callback(sm_ev_module_t* _this, sm_module_event_cb_fn_t _callback_fn, void* _arg){
    if(!_this || !_callback_fn){
        return -1;
    }
    _this->m_callback_fn = _callback_fn;
    _this->m_arg = _arg;
    return 0;
}

int32_t sm_module_process(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    if(_this->m_connection_state == MODULE_STATE_CONNECTED && !elapsed_timer_get_remain(&_this->m_connected_timeout)){
        _this->m_connection_state = MODULE_STATE_DISCONNECTED;
        if(_this->m_callback_fn){
            _this->m_callback_fn(_this->m_id, MODULE_EVENT_DISCONNECTED, NULL, _this->m_arg);
        }
        elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
    }
    if(_this->m_connection_state == MODULE_EVENT_CONNECTED && !_this->m_sync_info && _this->m_version[0] == '\0'){
        _this->m_proc->read_info(_this);
        _this->m_sync_info = true;
    }
    return 0;
}
