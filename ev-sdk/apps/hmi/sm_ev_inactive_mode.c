//
// Created by vnbk on 01/10/2024.
//
#include "sm_ev_inactive_mode.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"

#include "sm_ev_opt_storage.h"
#include "sm_net_monitor.h"

#define TAG "SM_EV_INACTIVE_MODE"

#define _impl(x)        ((sm_inactive_mode_impl_t*)(x))

#define KM_TO_M     1000

typedef struct {
    sm_storage_t* m_storage;

    sm_sv_ev_t* m_ev_service;

    sm_ev_inactive_mode_config_t* m_config;

//    elapsed_timer_t m_timer;
    int32_t m_odo_start;
    bool m_active_state;
    uint8_t m_lost_connection;

    sm_inactive_mode_on_event_fn_t m_cb;
    void* m_arg;

    sm_mutex m_lock;
}sm_inactive_mode_impl_t;

static sm_inactive_mode_impl_t g_inactive_mode = {
        .m_storage = NULL,
        .m_ev_service = NULL,
        .m_active_state = SM_INACTIVE_MODE_DEFAULT,
        .m_lost_connection = NET_RESET_CONNECTION,
        .m_cb = NULL,
        .m_arg = NULL
};

static void sm_inactive_mode_load_db(sm_inactive_mode_impl_t* _this){
    uint32_t odo = 0;
    int32_t ret = sm_ev_opt_load_inactive_mode(_this->m_storage, &odo);
    if(ret < 0){
        LOG_ERR(TAG, "Could NOT load INACTIVE mode");
        _this->m_active_state = ret;
        _this->m_odo_start = 0;
        sm_ev_opt_store_inactive_mode(_this->m_storage, SM_INACTIVE_MODE_DEFAULT, 0);
        return;
    }
    _this->m_active_state = ret;
    _this->m_odo_start = (int32_t)odo;
}

sm_inactive_mode_t* sm_inactive_mode_create(sm_sv_ev_t* _ev_service,
                                            sm_storage_t* _storage,
                                            sm_ev_inactive_mode_config_t* _config,
                                            sm_inactive_mode_on_event_fn_t _cb,
                                            void* _arg){
    if(!_ev_service || !_storage || !_config){
        LOG_ERR(TAG, "Params INVALID");
        return NULL;
    }

    g_inactive_mode.m_storage = _storage;
    g_inactive_mode.m_config = _config;
    g_inactive_mode.m_cb = _cb;
    g_inactive_mode.m_arg = _arg;
    g_inactive_mode.m_ev_service = _ev_service;

    sm_inactive_mode_load_db(&g_inactive_mode);

    if(g_inactive_mode.m_active_state == EV_INACTIVE_STATE){
        if(g_inactive_mode.m_cb){
            g_inactive_mode.m_cb(SM_EV_INACTIVE_EXECUTED, g_inactive_mode.m_arg);
        }
    }

    MUTEX_INIT(g_inactive_mode.m_lock);

    return &g_inactive_mode;
}

int32_t sm_inactive_mode_destroy(sm_inactive_mode_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_storage = NULL;
    _impl(_this)->m_cb = NULL;
    _impl(_this)->m_arg = NULL;
    _impl(_this)->m_ev_service = NULL;
    return 0;
}

int32_t sm_inactive_mode_update_net_state(sm_inactive_mode_t* _this, int32_t _net_state){
    sm_inactive_mode_impl_t* active_mode = (sm_inactive_mode_impl_t*)_this;

    if(active_mode->m_active_state == EV_INACTIVE_STATE){
        return 0;
    }

    if(_net_state == NET_LOST_CONNECTION && active_mode->m_lost_connection == NET_RESET_CONNECTION){
        ENTER_CRITICAL(_impl(_this)->m_lock);
        active_mode->m_lost_connection = NET_LOST_CONNECTION;
        active_mode->m_odo_start = (int32_t)sm_sv_ev_get_data(active_mode->m_ev_service)->m_odo;
        EXIT_CRITICAL(_impl(_this)->m_lock);
        return 0;
    }

    if(_net_state == NET_LOST_CONNECTION && active_mode->m_lost_connection != NET_LOST_CONNECTION){
        ENTER_CRITICAL(_impl(_this)->m_lock);

        active_mode->m_odo_start = (int32_t)sm_sv_ev_get_data(active_mode->m_ev_service)->m_odo;
        if(sm_ev_opt_store_inactive_mode(active_mode->m_storage,
                                          EV_ACTIVE_STATE,
                                          active_mode->m_odo_start) < 0){
            LOG_ERR(TAG, "Could NOT store inactive mode");
            EXIT_CRITICAL(_impl(_this)->m_lock);
            return -1;
        }
        active_mode->m_lost_connection = NET_LOST_CONNECTION;

        EXIT_CRITICAL(_impl(_this)->m_lock);
        return 0;
    }

    if(_net_state == NET_RECOVERED_CONNECTION && active_mode->m_lost_connection != NET_RECOVERED_CONNECTION){
        ENTER_CRITICAL(_impl(_this)->m_lock);
        active_mode->m_odo_start = 0;
        active_mode->m_lost_connection = NET_RECOVERED_CONNECTION;
        EXIT_CRITICAL(_impl(_this)->m_lock);

        if(active_mode->m_cb){
            active_mode->m_cb(SM_EV_INACTIVE_RESET, active_mode->m_arg);
        }
    }

    return 0;
}

int32_t sm_inactive_mode_reset(sm_inactive_mode_t* _this){
    if(!_this){
        return -1;
    }

    ENTER_CRITICAL(_impl(_this)->m_lock);

    if(sm_ev_opt_store_inactive_mode(_impl(_this)->m_storage, EV_ACTIVE_STATE, 0) < 0){
        LOG_ERR(TAG, "Could NOT store inactive mode");
        EXIT_CRITICAL(_impl(_this)->m_lock);
        return -1;
    }

    _impl(_this)->m_active_state = EV_ACTIVE_STATE;
    _impl(_this)->m_odo_start = 0;

    _impl(_this)->m_lost_connection = NET_RECOVERED_CONNECTION;

    EXIT_CRITICAL(_impl(_this)->m_lock);

    if( _impl(_this)->m_cb){
        _impl(_this)->m_cb(SM_EV_INACTIVE_RESET,  _impl(_this)->m_arg);
    }

    return 0;
}

int32_t sm_inactive_mode_process(sm_inactive_mode_t* _this){
    if(!_this){
        return -1;
    }

    if(_impl(_this)->m_active_state == EV_INACTIVE_STATE){
        if(_impl(_this)->m_cb){
            _impl(_this)->m_cb(SM_EV_INACTIVE_EXECUTED, _impl(_this)->m_arg);
        }
        return 0;
    }

    if(_impl(_this)->m_lost_connection == NET_LOST_CONNECTION){
        int32_t current_odo = (int32_t)sm_sv_ev_get_data(_impl(_this)->m_ev_service)->m_odo;
        int32_t distance = current_odo - _impl(_this)->m_odo_start;

        if(distance > 0 && distance >= (_impl(_this)->m_config->m_km_warning * KM_TO_M)){
            if(_impl(_this)->m_cb){
                _impl(_this)->m_cb(SM_EV_INACTIVE_WARNING, _impl(_this)->m_arg);
            }
        }

        if(distance > 0 && distance >= (_impl(_this)->m_config->m_km_force_stop * KM_TO_M)){

            ENTER_CRITICAL(_impl(_this)->m_lock);
            if(sm_ev_opt_store_inactive_mode(_impl(_this)->m_storage, EV_INACTIVE_STATE, current_odo) < 0){
                LOG_ERR(TAG, "Could NOT store inactive mode");
                EXIT_CRITICAL(_impl(_this)->m_lock);
                return -1;
            }
            _impl(_this)->m_active_state = EV_INACTIVE_STATE;
            _impl(_this)->m_odo_start = current_odo;

            EXIT_CRITICAL(_impl(_this)->m_lock);

            if(_impl(_this)->m_cb){
                _impl(_this)->m_cb(SM_EV_INACTIVE_EXECUTED, _impl(_this)->m_arg);
            }
        }
    }

    return 0;
}
