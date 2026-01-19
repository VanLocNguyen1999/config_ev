//
// Created by vnbk on 25/05/2024.
//
#include "sm_stm.h"
#include "sm_utils.h"
#include "sm_logger.h"
#include "sm_memory.h"

#define TAG "SM_STM"

//#define _impl(p)        ((sm_stm_impl_t*)(p))

typedef struct {

    sm_stm_state_t* m_current_state;
    sm_stm_state_t m_states[SM_STM_STATE_NUMBER_SUPPORT];
    struct {
        sm_stm_event_t _event[SM_STM_EVENT_NUMBER_SUPPORT];
    } m_stm_transitions[SM_STM_STATE_NUMBER_SUPPORT];
    int32_t m_state_number;
    int32_t m_event;
    int32_t m_event_pre;
    int32_t m_event_number;

}sm_stm_impl_t;

static sm_stm_impl_t g_stm_default = {
        .m_current_state = NULL,
        .m_state_number = 0,
		.m_event = 0,
		.m_event_pre = 0,
		.m_event_number = 0
};

sm_stm_t* sm_stm_create_default(int32_t _state_number, int32_t _event_number){
    sm_stm_impl_t* stm = &g_stm_default;
    stm->m_state_number = _state_number;
    stm->m_current_state = 0;
    stm->m_event = 0;
    stm->m_event_pre = 0;
    stm->m_event_number = _event_number;
    for(int index = 0; index < _state_number; index++){
        stm->m_states[index].m_state = 0;
        stm->m_states[index].m_arg = NULL;
        stm->m_states[index].process = NULL;
        stm->m_states[index].exit = NULL;
        stm->m_states[index].entry = NULL;
    }
//    LOG_INF(TAG, "Application State Machine is initializing.....");
    return (sm_stm_t*) stm;
}

sm_stm_t* sm_stm_create(int32_t _state_number, int32_t _event_number){

	static sm_stm_impl_t g_stm;
	sm_stm_impl_t* stm = &g_stm;
    if(!stm){
        LOG_ERR(TAG, "Could NOT create State Machine Object. Allocate memory FAILURE");
        return NULL;
    }
    stm->m_state_number = _state_number;
    stm->m_current_state = NULL;
    stm->m_event = 0;
    stm->m_event_pre = 0;
    stm->m_event_number = _event_number;
    for(int index = 0; index < _state_number; index++){
        stm->m_states[index].m_state = _state_number;
        stm->m_states[index].m_arg = NULL;
        stm->m_states[index].process = NULL;
        stm->m_states[index].exit = NULL;
        stm->m_states[index].entry = NULL;
    }
    for(int i = 0; i < SM_STM_STATE_NUMBER_SUPPORT; i++){
        for(int j = 0; j < SM_STM_EVENT_NUMBER_SUPPORT; j++){
            stm->m_stm_transitions[i]._event[j].m_id = SM_STM_EVENT_NUMBER_SUPPORT;
            stm->m_stm_transitions[i]._event[j].m_handler = NULL;
            stm->m_stm_transitions[i]._event[j].m_arg = NULL;
        }
    }
//    LOG_INF(TAG, "Application State Machine is initializing.....");
    return stm;
}

int32_t sm_stm_destroy(sm_stm_t* _this){
    if(!_this){
        return -1;
    }

    mem_free(_this);
    LOG_WRN(TAG, "Application State Machine is free");
    return 0;
}

int32_t sm_stm_add_state(sm_stm_t* _this, const sm_stm_state_t* _state){
    if(!_this){
        return -1;
    }
    sm_stm_impl_t* _impl = (sm_stm_t*) _this;
    if(_state && _state->m_state < _impl->m_state_number){
        return sm_stm_clone_state(&_impl->m_states[_state->m_state], _state);
    }
    return -1;
}

int32_t sm_stm_config_transition(sm_stm_t* _this, int32_t _state, sm_stm_event_t* _event){
    if(!_this){
        return -1;
    }
    sm_stm_impl_t* _impl = (sm_stm_t*) _this;
    if(_state >= _impl->m_state_number || _event->m_id >= _impl->m_event_number){
        LOG_ERR(TAG, "State Machine Transition is INVALID");
        return -1;
    }

    return sm_stm_clone_event( &_impl->m_stm_transitions[_state]._event[_event->m_id], _event);
}

int32_t sm_stm_remove_state(sm_stm_t* _this, int32_t _state){
    if(!_this){
        return -1;
    }

    sm_stm_impl_t* _impl = (sm_stm_t*) _this;
    if(_state < _impl->m_state_number){
    	_impl->m_states[_state].m_arg = NULL;
    	_impl->m_states[_state].m_state = SM_STM_STATE_NUMBER_SUPPORT;
    	_impl->m_states[_state].process = NULL;
    	_impl->m_states[_state].entry = NULL;
    	_impl->m_states[_state].exit = NULL;
        return 0;
    }
    return -1;
}
int32_t sm_stm_set_event(int32_t _event){

	sm_stm_impl_t* stm = &g_stm_default;
    if(stm == NULL){
        return -1;
    }

    stm->m_event_pre = stm->m_event;
    stm->m_event = _event;
    return 0;
}

int32_t sm_stm_get_current_state(sm_stm_t* _this){
    if(!_this){
        return -1;
    }
    sm_stm_impl_t* _impl = (sm_stm_t*) _this;
    return _impl->m_current_state->m_state;
}

int32_t sm_stm_set_current_state(sm_stm_t* _this, int32_t m_state){
    if(!_this){
        return -1;
    }
	(void) _this;
	g_stm_default.m_current_state = &g_stm_default.m_states[m_state];

    return 0;
}

int32_t sm_stm_process(sm_stm_t* _this){
    if(!_this){
        return -1;
    }
    sm_stm_impl_t* _impl = (sm_stm_t*) _this;
    sm_stm_state_t* current_state = _impl->m_current_state;
    if(!current_state){
        LOG_ERR(TAG, "Current State is NOT set.");
        return -1;
    }

    int32_t state = current_state->m_state;
       int32_t event = _impl->m_event;

       if(event < _impl->m_event_number){
           event = _impl->m_event;
           if(_impl->m_stm_transitions[state]._event[event].m_handler != NULL){

               if(_impl->m_current_state->exit){
            	   _impl->m_current_state->exit(_impl->m_current_state->m_arg);
               }

               state = _impl->m_stm_transitions[state]._event[event].m_handler(_impl->m_stm_transitions[state]._event[event].m_arg);

               _impl->m_current_state = &_impl->m_states[state];

               if(_impl->m_current_state->entry){
                   _impl->m_current_state->entry(_impl->m_current_state->m_arg);
               }
           }
           _impl->m_event = SM_STM_EVENT_NUMBER_SUPPORT;
       }

    if(_impl->m_current_state->process != NULL){
    	_impl->m_current_state->process(_impl->m_current_state->m_arg);
    }

    return 0;
}
