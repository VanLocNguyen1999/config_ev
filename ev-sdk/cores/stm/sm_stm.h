//
// Created by vnbk on 25/05/2024.
//

#ifndef EV_SDK_SM_STM_H
#define EV_SDK_SM_STM_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
//#include "sm_stm_def.h"

#define SM_STM_STATE_NUMBER_SUPPORT     12
#define SM_STM_EVENT_NUMBER_SUPPORT     20

typedef struct {
    int32_t m_state;

    int32_t (*entry)(void*);
    int32_t (*process)(void*);
    int32_t (*exit)(void*);

    void* m_arg;
}sm_stm_state_t;

static inline int32_t sm_stm_clone_state(sm_stm_state_t* _dst, const sm_stm_state_t* _src){
    if(!_src || !_dst){
        return -1;
    }
    _dst->m_state = _src->m_state;

    _dst->entry = _src->entry;
    _dst->process = _src->process;
    _dst->exit = _src->exit;

    _dst->m_arg = _src->m_arg;
    return 0;
}

/**
 * @brief: Event handler
 * @param: argument of event.
 * @return: Next state.
 */
typedef int32_t (*sm_stm_event_handler_fn_t)(void*);

typedef struct {
    int32_t m_id;
    sm_stm_event_handler_fn_t m_handler;
    void* m_arg;
}sm_stm_event_t;

static inline int32_t sm_stm_clone_event(sm_stm_event_t* _dst, const sm_stm_event_t* _src){
    if(!_src || !_dst){
        return -1;
    }
    _dst->m_id = _src->m_id;
    _dst->m_handler = _src->m_handler;
    _dst->m_arg = _src->m_arg;

    return 0;
}

typedef void sm_stm_t;

sm_stm_t* sm_stm_create_default(int32_t _state_number, int32_t _event_number);
sm_stm_t* sm_stm_create(int32_t _state_number, int32_t _event_number);
int32_t   sm_stm_destroy(sm_stm_t* _this);

int32_t sm_stm_add_state(sm_stm_t* _this, const sm_stm_state_t* _state);
int32_t sm_stm_remove_state(sm_stm_t* _this, int32_t _state);

int32_t sm_stm_set_event(int32_t _event);

int32_t sm_stm_get_current_state(sm_stm_t* _this);
int32_t sm_stm_set_current_state(sm_stm_t* _this, int32_t m_state);
int32_t sm_stm_process(sm_stm_t* _this);

int32_t sm_stm_config_transition(sm_stm_t* _this, int32_t _state, sm_stm_event_t* _event);
#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_STM_H
