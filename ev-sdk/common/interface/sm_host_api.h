#ifndef EV_SDK_SM_HOST_API_H
#define EV_SDK_SM_HOST_API_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"

#define SM_HOST_SYNC_TIME_DEFAULT     (35) //seconds

typedef struct {
    void (*on_connected)(int32_t, void*);
    void (*on_disconnected)(int32_t, void*);
    int32_t (*on_cmd_from_host)(int32_t, void*, void*);
    int32_t (*on_cfg_from_host)(int32_t, void*, void*);
    int32_t (*on_cfg_completed)(void*);
}sm_host_event_t;

typedef struct {
    void* (*get_ev_manu)(void*);
    void* (*get_ev_module_info)(int32_t, void*);
    void* (*get_ev_data)(void*);
    void* (*get_ev_config)(void*);
    void* (*get_ev_bp_data)(int32_t, void*);
    int32_t (*get_gps_data)(void*, void*);
    int32_t (*get_network_data)(int32_t*, char*, void*);
    int32_t (*get_optional_data)(char*, void*);
    void* m_arg;
}sm_host_sync_data_if_t;

typedef struct sm_host_api sm_host_api_t;

typedef struct {
    int32_t (*init)(sm_host_api_t*, sm_host_sync_data_if_t*, int32_t);
    int32_t (*free)(sm_host_api_t*);

    int32_t (*sync_pause)(sm_host_api_t*);
    int32_t (*sync_resume)(sm_host_api_t*);

    int32_t (*reg_event_handle)(sm_host_api_t*, sm_host_event_t*, void*);
    int32_t (*unreg_event_handle)(sm_host_api_t*);

    int32_t (*is_connected)(sm_host_api_t*);

    int32_t (*update_data_to_host)(sm_host_api_t*, const char*, const char*);
    int32_t (*update_datas_to_host)(sm_host_api_t*, void*);

    int32_t (*push_event_to_host)(sm_host_api_t*, void*);

    int32_t (*process)(sm_host_api_t*);
}sm_host_api_proc_t;

struct sm_host_api{
   const sm_host_api_proc_t* m_proc;

    const sm_host_sync_data_if_t* m_sync_if;
    sm_host_event_t* m_host_event;
    void* m_host_event_arg;

    int32_t m_sync_time;
    bool m_sync;
};

static inline int32_t sm_host_api_init(sm_host_api_t* _this, sm_host_sync_data_if_t* _sync_if, int32_t _sync_time){
    if(!_this || !_sync_if){
        return -1;
    }
    _this->m_sync_if = _sync_if;
    if(_sync_time <= 0){
        _this->m_sync = SM_HOST_SYNC_TIME_DEFAULT;
    }
    return 0;
}

static inline int32_t sm_host_api_reg_event_handle(sm_host_api_t* _this, sm_host_event_t* _event_handle, void* _event_arg){
    if(!_this || !_event_handle){
        return -1;
    }
    _this->m_host_event = _event_handle;
    _this->m_host_event_arg = _event_arg;
    return 0;
}

static inline int32_t sm_host_api_unreg_event_handle(sm_host_api_t* _this){
    if(!_this || !_this->m_host_event){
        return -1;
    }
    _this->m_host_event = NULL;
    _this->m_host_event_arg = NULL;
    return 0;
}

static inline int32_t sm_host_api_sync_pause(sm_host_api_t* _this){
    if(!_this){
        return -1;
    }
    _this->m_sync = false;
    return 0;
}

static inline int32_t sm_host_api_sync_resume(sm_host_api_t* _this){
    if(!_this){
        return -1;
    }
    _this->m_sync = true;
    return 0;
}

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_HOST_API_H
