//
// Created by vnbk on 30/09/2024.
//

#ifndef EV_SDK_SM_NET_MONITOR_H
#define EV_SDK_SM_NET_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_types.h"

#define SM_NET_MONITOR_DETECTED_TIME_DEFAULT    (30*1000) /// 20s

/// NET EVENT
enum {
    NET_RESET_CONNECTION = 0,
    NET_LOST_CONNECTION = 1,
    NET_RECOVERED_CONNECTION = 2,
};

#define NET_LOSING              0
#define NET_RECOVERING          (!NET_LOSING)

typedef void sm_net_monitor_t;
typedef void (*sm_net_monitor_event_fn_t)(int32_t, void*);

sm_net_monitor_t* sm_net_monitor_create(int32_t _detected_timeout,
                                        sm_net_monitor_event_fn_t _event_handle,
                                        void* _event_arg);

int32_t sm_net_monitor_destroy(sm_net_monitor_t* _this);

int32_t sm_net_monitor_set_cb(sm_net_monitor_t* _this,
                              sm_net_monitor_event_fn_t _event_handle,
                              void* _event_arg);

int32_t sm_net_monitor_update_state(sm_net_monitor_t* _this, int32_t _state);

int32_t sm_net_monitor_get_state(sm_net_monitor_t* _this);

void sm_net_monitor_process(sm_net_monitor_t* _this);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_NET_MONITOR_H
