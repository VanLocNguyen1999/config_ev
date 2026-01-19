//
// Created by vnbk on 30/09/2024.
//
#include "sm_net_monitor.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"

#include "sm_ec200.h"

#define TAG "SM_NET_MONITOR"

#define NET_SYNC_LOST_CONNECTION_TIME   5000 /// 5s
#define NET_LOST_CONNECTION_COUNT_MAX   (10)

#define _impl(x)        ((sm_net_monitor_impl_t*)(x))

enum {
    NET_STATE_INITIALIZE = 0,
    NET_STATE_LOSING,
    NET_STATE_LOST,
    NET_STATE_RECOVERING,
    NET_STATE_RECOVERED,
};

typedef struct {
    int32_t m_detected_time;
    elapsed_timer_t m_timer;

    int32_t m_cur_state;
    uint8_t m_lost_count;

    sm_net_monitor_event_fn_t m_event_handle;
    void* m_event_arg;

    sm_mutex m_lock;
}sm_net_monitor_impl_t;

static sm_net_monitor_impl_t g_net_monitor = {
        .m_detected_time = SM_NET_MONITOR_DETECTED_TIME_DEFAULT,
        .m_event_handle = NULL,
        .m_event_arg = NULL,
        .m_cur_state = NET_STATE_INITIALIZE,
        .m_lost_count = 0,
};

sm_net_monitor_t* sm_net_monitor_create(int32_t _detected_timeout,
                                        sm_net_monitor_event_fn_t _event_handle,
                                        void* _event_arg){
    if(_detected_timeout <= 0){
        g_net_monitor.m_detected_time = SM_NET_MONITOR_DETECTED_TIME_DEFAULT;
    }else{
        g_net_monitor.m_detected_time = _detected_timeout;
    }

    g_net_monitor.m_cur_state = NET_STATE_INITIALIZE;
    g_net_monitor.m_lost_count = 0;

    g_net_monitor.m_event_handle = _event_handle;
    g_net_monitor.m_event_arg = _event_arg;

    elapsed_timer_resetz(&g_net_monitor.m_timer, g_net_monitor.m_detected_time);

    MUTEX_INIT(g_net_monitor.m_lock);

    return &g_net_monitor;
}

int32_t sm_net_monitor_destroy(sm_net_monitor_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_event_arg = NULL;
    _impl(_this)->m_event_handle = NULL;

    return 0;
}

int32_t sm_net_monitor_set_cb(sm_net_monitor_t* _this,
                              sm_net_monitor_event_fn_t _event_handle,
                              void* _event_arg){
    if(!_this || !_event_handle){
        return -1;
    }
    _impl(_this)->m_event_handle = _event_handle;
    _impl(_this)->m_event_arg = _event_arg;
    return 0;
}

int32_t sm_net_monitor_update_state(sm_net_monitor_t* _this, int32_t _state){
    if(!_this){
        return -1;
    }
    if(_state == NET_RECOVERING && _impl(_this)->m_cur_state != NET_STATE_RECOVERED){
        LOG_INF(TAG, "Recovered Network");

        ENTER_CRITICAL(_impl(_this)->m_lock);

        _impl(_this)->m_cur_state = NET_STATE_RECOVERED;
        _impl(_this)->m_lost_count = 0;

        EXIT_CRITICAL(_impl(_this)->m_lock);

        if(_impl(_this)->m_event_handle){
            _impl(_this)->m_event_handle(NET_RECOVERED_CONNECTION,
                                         _impl(_this)->m_event_arg);
        }
        return 0;
    }

    if(_state == NET_LOSING && _impl(_this)->m_cur_state >= NET_STATE_RECOVERING){
        LOG_DBG(TAG, "Start monitor LOST network connection state");

        ENTER_CRITICAL(_impl(_this)->m_lock);

        _impl(_this)->m_cur_state = NET_STATE_LOST;
        elapsed_timer_resetz(&_impl(_this)->m_timer, _impl(_this)->m_detected_time);

        EXIT_CRITICAL(_impl(_this)->m_lock);
    }

    return 0;
}

int32_t sm_net_monitor_get_state(sm_net_monitor_t* _this){
    if(!_this){
        return -1;
    }
    return _impl(_this)->m_cur_state >= NET_STATE_RECOVERED ? NET_RECOVERED_CONNECTION : NET_LOST_CONNECTION;
}

void sm_net_monitor_process(sm_net_monitor_t* _this){
    if(!_this){
        return;
    }
    if((_impl(_this)->m_cur_state <= NET_STATE_LOST) && !elapsed_timer_get_remain(&_impl(_this)->m_timer)){
//        LOG_ERR(TAG, "Network is lost connection");

        _impl(_this)->m_lost_count++;
        if(_impl(_this)->m_lost_count >= NET_LOST_CONNECTION_COUNT_MAX){
            if(_impl(_this)->m_event_handle){
                _impl(_this)->m_event_handle(NET_RESET_CONNECTION,
                                             _impl(_this)->m_event_arg);
            }
            _impl(_this)->m_lost_count = 0;
        }

        if(_impl(_this)->m_event_handle){
            _impl(_this)->m_event_handle(NET_LOST_CONNECTION,
                                         _impl(_this)->m_event_arg);
        }

        elapsed_timer_resetz(&_impl(_this)->m_timer, NET_SYNC_LOST_CONNECTION_TIME);
    }
}
