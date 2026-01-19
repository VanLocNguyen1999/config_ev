//
// Created by vnbk on 10/06/2023.
//

#ifndef SM_EV_MODULE_H
#define SM_EV_MODULE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include "memory.h"
#include "sm_elapsed_timer.h"
#include "sm_ev_data.h"
#include "sm_bp_data.h"

#ifdef MUTILPLE_THREAD
#include "threading/thread_utils.h"
#endif

#define SM_MODULE_CONNECTED_TIMEOUT     5000

typedef enum {
    MODULE_EVENT_DISCONNECTED,
    MODULE_EVENT_CONNECTED,
    MODULE_EVENT_UPDATED_DATA,
    MODULE_EVENT_NUMBER
}SM_MODULE_EVENT;

typedef void (*sm_module_event_cb_fn_t)(uint8_t, uint8_t, void*, void*);

typedef struct sm_ev_module sm_ev_module_t;

typedef struct {
    int32_t (*init)(sm_ev_module_t*);
    int32_t (*free)(sm_ev_module_t*);
    int32_t (*reboot)(sm_ev_module_t*);
    const char* (*get_name)(sm_ev_module_t*);
    int32_t (*read_info)(sm_ev_module_t*);
    void*   (*get_data)(sm_ev_module_t*);
    void    (*reset_data)(sm_ev_module_t*);
    int32_t (*handle_data)(sm_ev_module_t*, int32_t, const uint8_t*, int32_t);
    int32_t (*process)(sm_ev_module_t*);
}sm_module_proc_t;

struct sm_ev_module{
    const sm_module_proc_t* m_proc;

    uint8_t m_id;
    sm_module_info_t m_info;
    bool m_sync_info;

    char m_version[EV_VERSION_STRING_LENGTH];

    uint8_t m_connection_state;
    elapsed_timer_t m_connected_timeout;

    sm_module_event_cb_fn_t m_callback_fn;
    void* m_arg;

#ifdef MUTILPLE_THREAD
    sm_mutex m_lock;
#endif
};

int32_t sm_module_get_id(sm_ev_module_t* _this);
int32_t sm_module_reg_event_callback(sm_ev_module_t* _this, sm_module_event_cb_fn_t _callback_fn, void* _arg);
int32_t sm_module_process(sm_ev_module_t* _this);
static inline void sm_module_set_state_connect(sm_ev_module_t* _this, SM_MODULE_STATE state){

    _this->m_connection_state = state;
}
#ifdef __cplusplus
};
#endif

#endif //SM_EV_MODULE_H
