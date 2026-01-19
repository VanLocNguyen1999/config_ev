#include "sm_sv_io.h"
//#include "sm_sv_bt.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"
#include "sm_memory.h"

//static const char* TAG = "SM_EV_IO";

#define _impl(x)      ((sm_sv_io_impl_t*)(x))

enum {
    BT_PARKING = 0,
    BT_DRIVE_MODE,
    BT_NUMBER
};

typedef struct{
	int32_t (*input_if)();
    uint8_t m_enable;

    uint8_t m_real_state;
    uint8_t m_debounce_value;
    elapsed_timer_t m_debounce_time;

    uint8_t m_event;
    sm_sv_io_event_cb_fn_t m_event_cb;
    void* m_event_arg;
}sm_input_t;

typedef struct sm_output sm_output_t;

struct sm_output{
    int32_t (*output_if)(uint8_t);
    void (*behavior)(sm_output_t*);

    sm_input_t* m_sw;

    uint8_t m_state;

    uint8_t m_enable;
    uint8_t m_op_state;

    uint8_t m_force_value;

    int32_t m_repeat;
    int32_t m_blink_count;
    elapsed_timer_t m_duration;

    uint8_t m_event;
    sm_sv_io_event_cb_fn_t m_event_cb;
    void* m_event_arg;
};

typedef struct {
    sm_sv_io_if_t* m_if;

    sm_input_t m_input[IN_NUMBER];
    sm_output_t m_output[OUT_NUMBER];
//    sm_sv_bt_t* m_bt_service;

    struct {
        int32_t m_id;

        uint8_t m_event;
        sm_sv_io_event_cb_fn_t m_event_cb;
        void* m_event_arg;
    }m_button[BT_NUMBER];

}sm_sv_io_impl_t;

static sm_sv_io_impl_t g_io = {
//        .m_bt_service = NULL,
        .m_if = NULL,
};
//static void on_bt_event_cb(uint8_t _btn_id, uint8_t _event, void *_arg);
static void sm_io_input_process(sm_sv_io_impl_t* _this){
    sm_input_t* input = NULL;
    uint8_t current_state;
    for(int index = 0; index < IN_NUMBER; index++){
        input = &_this->m_input[index];
        if(!input->m_enable || !input->input_if){
            continue;
        }
        current_state = (uint8_t)input->input_if();
        if(current_state != input->m_debounce_value){
            input->m_debounce_value = current_state;
            elapsed_timer_reset(&input->m_debounce_time);
        }

        if(!elapsed_timer_get_remain(&input->m_debounce_time) && input->m_debounce_value != input->m_real_state){
            input->m_real_state = input->m_debounce_value;
            if(input->m_event_cb){
                input->m_event_cb(input->m_event, input->m_real_state, input->m_event_arg);
            }
        }
    }
}
static void sm_output_reset(sm_output_t* _this){
	if(_this->output_if){
	    _this->output_if(SM_SV_IO_DEACTIVATE);
	}

    if(_this->m_state != SM_SV_IO_DEACTIVATE){
        _this->m_state = SM_SV_IO_DEACTIVATE;
        if(_this->m_event_cb){
            _this->m_event_cb(_this->m_event, _this->m_state, _this->m_event_arg);
        }
    }

    _this->m_repeat = SM_SV_IO_BLINK_REPEAT_TIME_FOREVER;
    _this->m_blink_count = 0;
    elapsed_timer_resetz(&_this->m_duration, SM_SV_IO_BLINK_DURATION_DEFAULT);
}
static void sm_output_blink(sm_output_t* _this){
    bool blink_condition = (_this->m_repeat == SM_SV_IO_BLINK_REPEAT_TIME_FOREVER) || (_this->m_blink_count <= _this->m_repeat);
    if(!elapsed_timer_get_remain(&_this->m_duration) && blink_condition){
        _this->m_state = !_this->m_state;

        _this->output_if(_this->m_state);
        if(_this->m_event_cb){
            _this->m_event_cb(_this->m_event, _this->m_state, _this->m_event_arg);
        }

        _this->m_blink_count++;

        elapsed_timer_reset(&_this->m_duration);
    }
}

#if 0
static void sm_output_force(sm_sv_io_t* _this){
    if(!_this){
        return NULL;
    }
    _impl(_this)->m_output[_io].m_op_state = OP_STATE_NORMAL;
    _impl(_this)->m_output[_io].m_force_value = SM_SV_IO_ACTIVATE;
}
static void sm_output_blink(sm_output_t* _this){
    bool blink_condition = (_this->m_repeat == SM_SV_IO_BLINK_REPEAT_TIME_FOREVER) || (_this->m_blink_count <= _this->m_repeat);
    if(!elapsed_timer_get_remain(&_this->m_duration) && blink_condition){
        _this->m_state = !_this->m_state;

        _this->output_if(_this->m_state);
        if(_this->m_event_cb){
            _this->m_event_cb(_this->m_event, _this->m_state, _this->m_event_arg);
        }

        _this->m_blink_count++;

        elapsed_timer_reset(&_this->m_duration);
    }
}

static void sm_output_behavior_default(sm_output_t* _this){
	if(_this->m_state != SM_SV_IO_ACTIVATE && _this->output_if){
		_this->output_if(SM_SV_IO_ACTIVATE);
		_this->m_state = SM_SV_IO_ACTIVATE;

		if(_this->m_event_cb){
		    _this->m_event_cb(_this->m_event, _this->m_state, _this->m_event_arg);
		}
	}
}
static void on_bt_event_cb(uint8_t _btn_id, uint8_t _event, void *_arg){
    if(_arg && _impl(_arg)->m_button[_btn_id].m_event_cb){
        _impl(_arg)->m_button[_btn_id].m_event_cb(_impl(_arg)->m_button[_btn_id].m_event,
                                               _event,
                                               _impl(_arg)->m_button[_btn_id].m_event_arg);
    }
}
#endif
static void sm_io_output_process(sm_sv_io_impl_t* _this){

    sm_output_t* output = NULL;
    for(int index = 0; index < OUT_NUMBER; index++){
        output = &_this->m_output[index];
        if(!output->m_enable || !output->output_if){
            continue;
        }
        if(output->m_op_state == OP_STATE_FORCE){
            if(output->m_force_value != output->m_state){
                output->output_if(output->m_force_value);
                output->m_state = output->m_force_value;
                if(output->m_event_cb){
                    output->m_event_cb(output->m_event,
                                       output->m_state,
                                       output->m_event_arg);
                }
            }
        }else if(output->m_op_state == OP_STATE_FORCE_BLINK){
            sm_output_blink(output);
            if(output->m_repeat > 0 && output->m_blink_count >= output->m_repeat){
                sm_output_reset(output);
                output->m_op_state = OP_STATE_NORMAL;
            }
        }else if(output->m_op_state == OP_STATE_NORMAL){
            if(output->m_sw->m_real_state == SM_SV_IO_ACTIVATE){
                output->behavior(output);
//                sm_output_blink(output);
            }else{
                if(output->m_state != SM_SV_IO_DEACTIVATE){
                    output->output_if(SM_SV_IO_DEACTIVATE);
                    output->m_state = SM_SV_IO_DEACTIVATE;
                    output->m_blink_count = 0;
                }
            }
        }
    }
}

static inline int32_t sm_sv_input_create(sm_sv_io_impl_t* _this){
    for(int index = 0; index < IN_NUMBER; index++){
        _this->m_input[index].m_real_state = SM_SV_IO_DEACTIVATE;
        _this->m_input[index].m_debounce_value = SM_SV_IO_DEACTIVATE;
        _this->m_input[index].m_event_cb = NULL;
        _this->m_input[index].input_if = NULL;
        _this->m_input[index].m_event = SM_SV_IO_EVENT_NUMBER;
        _this->m_input[index].m_event_arg = NULL;
        _this->m_input[index].m_enable = SM_SV_IO_ENABLE_DEFAULT;
        elapsed_timer_resetz(&_this->m_input[index].m_debounce_time, SM_SV_IO_DEBOUNCE_TIME_DEFAULT);
    }

    _this->m_input[IN_12V_DET].input_if = _this->m_if->in_12v_det_fn_t;
    return 0;
}

static inline int32_t sm_sv_output_create(sm_sv_io_impl_t* _this){
    for(int index = 0; index < OUT_NUMBER; index++){
        _this->m_output[index].m_state = SM_SV_IO_DEACTIVATE;
        _this->m_output[index].m_op_state = OP_STATE_NORMAL;
        _this->m_output[index].m_event_cb = NULL;
        _this->m_output[index].m_event_arg = NULL;
        _this->m_output[index].m_event = SM_SV_IO_EVENT_NUMBER;
        _this->m_output[index].output_if = NULL;
        _this->m_output[index].m_enable = SM_SV_IO_ENABLE_DEFAULT;
        _this->m_output[index].m_repeat = SM_SV_IO_BLINK_REPEAT_TIME_FOREVER;
        _this->m_output[index].m_force_value = SM_SV_IO_DEACTIVATE;

        elapsed_timer_resetz(&_this->m_output[index].m_duration, SM_SV_IO_BLINK_DURATION_DEFAULT);
    }

    _this->m_output[OUT_12V_ACT].output_if = _this->m_if->out_actice_12V_fn_t;
    _this->m_output[OUT_12V_ACT].m_op_state = OP_STATE_FORCE;
    return 0;
}

sm_sv_io_t* sm_sv_io_create(void* _if){
    if(!_if){
        LOG_ERR(TAG, "Missing IO interface");
        return NULL;
    }
    sm_sv_io_impl_t* io_service = &g_io;

    io_service->m_if = (sm_sv_io_if_t*) _if;

    /// Create Input
    sm_sv_input_create(io_service);

    /// Create Output
    sm_sv_output_create(io_service);

    /// Create Button service
//    io_service->m_bt_service = sm_sv_bt_create(NULL);
    return (sm_sv_io_t*) &g_io;
}

int32_t sm_sv_io_destroy(sm_sv_io_t* _this){
    if (_this == NULL) {
        return -1;
    }
    _impl(_this)->m_if = NULL;

    if(_this != &g_io){
        mem_free(_this);
    }

    _this = NULL;
    return 0;
}

int32_t sm_sv_io_reg_event(sm_sv_io_t *_this, uint8_t _event, sm_sv_io_event_cb_fn_t _cb_fn, void *_arg){
    if(!_this){
        return -1;
    }
    sm_sv_io_impl_t* io_service = _impl(_this);
    switch (_event) {
        case SM_SV_IO_EVENT_12V_DET_CHANGED:
            io_service->m_input[IN_12V_DET].m_event = SM_SV_IO_EVENT_12V_DET_CHANGED;
            io_service->m_input[_event].m_event_cb = _cb_fn;
            io_service->m_input[_event].m_event_arg =_arg;
            break;
        case SM_SV_IO_EVENT_ACTIVE_12V_CHANGED:
            io_service->m_button[OUT_12V_ACT].m_event = SM_SV_IO_EVENT_ACTIVE_12V_CHANGED;
            io_service->m_button[OUT_12V_ACT].m_event_cb = _cb_fn;
            io_service->m_button[OUT_12V_ACT].m_event_arg = _arg;
            return 0;
        default:
            return -1;
    }

    return 0;
}

int32_t sm_sv_io_set_if(sm_sv_io_t *_this, sm_sv_io_if_t *_io_if){
    if (_this == NULL || _io_if == NULL) {
        return -1;
    }

    _impl(_this)->m_if = _io_if;
    return 0;
}
/**
 * @brief sm_sv_io_get_io_sw_state
 * @param _this
 * @return
 */
int32_t sm_sv_io_get_io_sw_state(sm_sv_io_t *_this, uint8_t _io){
    if(!_this){
        return -1;
    }
    switch (_io) {
        case IN_12V_DET:
            return _impl(_this)->m_input[IN_12V_DET].m_real_state;
        default:
            break;
    }
    return -1;
}

int32_t sm_sv_io_get_state(sm_sv_io_t* _this, uint8_t _io){
    if(!_this || _io >= IN_NUMBER){
        return -1;
    }
    return _impl(_this)->m_input[_io].m_real_state;
}

int32_t sm_sv_io_enable(sm_sv_io_t* _this, uint8_t _io, uint8_t _enable){
    if(!_this){
        return -1;
    }
    sm_output_reset(&_impl(_this)->m_output[_io]);
    _impl(_this)->m_output[_io].m_enable = _enable;
    return 0;
}

int32_t sm_sv_io_force(sm_sv_io_t* _this, uint8_t _io, uint8_t _value){
    if(!_this){
        return -1;
    }
//    sm_sv_io_impl_t* g_io_sv = (sm_sv_io_impl_t*)_this;
    sm_output_reset(&g_io.m_output[_io]);
    g_io.m_output[_io].m_op_state = OP_STATE_FORCE;
    g_io.m_output[_io].m_force_value = _value;

    return 0;
}

int32_t sm_sv_io_release(sm_sv_io_t* _this, uint8_t _io){
    if(!_this){
        return -1;
    }

    sm_output_reset(&_impl(_this)->m_output[_io]);
    _impl(_this)->m_output[_io].m_op_state = OP_STATE_NORMAL;
    _impl(_this)->m_output[_io].m_force_value = SM_SV_IO_DEACTIVATE;

    return 0;
}

int32_t sm_sv_io_blink(sm_sv_io_t* _this, uint8_t _io, int32_t _repeat, int32_t _duration){
    if(!_this){
        return -1;
    }

    sm_output_reset(&_impl(_this)->m_output[_io]);
    _impl(_this)->m_output[_io].m_op_state = OP_STATE_FORCE_BLINK;
    _impl(_this)->m_output[_io].m_repeat = _repeat;
    elapsed_timer_resetz(&_impl(_this)->m_output[_io].m_duration, _duration);

    return 0;
}

int32_t sm_sv_io_cancel_blink(sm_sv_io_t* _this, uint8_t _io){
    if(!_this){
        return -1;
    }
    sm_output_reset(&_impl(_this)->m_output[_io]);
    _impl(_this)->m_output[_io].m_op_state = OP_STATE_NORMAL;

    return 0;
}

void sm_sv_io_process(sm_sv_io_t *_this){
	if (_this == NULL)
		return;
//    	sm_sv_bt_process(_impl(_this)->m_bt_service);
	sm_io_input_process(_impl(_this));
	sm_io_output_process(_impl(_this));

}
