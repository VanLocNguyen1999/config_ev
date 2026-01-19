/*
 * sm_bp_retain.c
 *
 *  Created on: 20 Nov 2025
 *      Author: My PC
 */
#include "sm_bp_retain.h"
#include "sm_core_co.h"

#define BP_RETAIN_DISCHAR_TIME_M			10
#define BP_RETAIN_DISCHAR_TIME_MS			(10*60*1000)
#define BP_TIME_MS_RATIO					1.0f/(BP_RETAIN_DISCHAR_TIME_MS)

#define _impl(p)    ((sm_bp_retain_impl_t*)(p))

typedef struct bp_retain_list{
    elapsed_timer_t m_timeout;
    int32_t index;
}bp_retain_list_t;

typedef struct{

	sm_bp_t* m_bps;
	sm_sv_bp_retain_cb_t m_cb;
	void* m_arg;
	uint8_t bp_retain_numbs;

	int32_t m_paused;

	bp_retain_list_t list[SM_SV_BP_NUMBER_DEFAULT];
}sm_bp_retain_impl_t;

static sm_bp_retain_impl_t g_bp_retain;
static inline int32_t sm_bp_retain_move_bp_to_list(void *_list, uint8_t id);
static inline int32_t sm_bp_retain_remove_bp_from_list(void *_list, uint8_t id);
static inline int32_t sm_bp_retain_get_list_id(void *_list, uint8_t _bp_id);

static inline void sm_bp_enable_retain_discharge(int32_t _id, void* _arg){


	sm_sv_bp_t* _bp = (sm_sv_bp_t*)_arg;
    sm_sv_bp_set_cmd(_bp, _id, BP_CMD_RETAIN_DISCHARGE, NULL, NULL, NULL);
}

static inline void sm_bp_disable_retain_discharge(int32_t _id, void* _arg){


	sm_sv_bp_t* _bp = (sm_sv_bp_t*)_arg;
    sm_sv_bp_set_cmd(_bp, _id, BP_CMD_DISCHARGE, NULL, NULL, NULL);
}

static sm_sv_bp_retain_cb_t g_bp_retain_callback = {
		.enable_retain_cb = sm_bp_enable_retain_discharge,
		.disable_retain_cb = sm_bp_disable_retain_discharge
};
static void sm_bp_enable_retain(sm_bp_retain_impl_t *_this) {

	if (_this == NULL)
		return;

	for (uint8_t id = 0; id < SM_SV_BP_NUMBER_DEFAULT; id++) {
		sm_bp_t* _bp = &_this->m_bps[id];

		if (_bp->m_retain_st == BP_SUPPORT_RETAIN
				&& _bp->m_data.m_state == BP_STATE_DISCHARGING
				&& _bp->m_is_connected) {

			if (_this->m_cb.enable_retain_cb) {

				_this->m_cb.enable_retain_cb(id, _this->m_arg);
			}
		}
	}
}

static void sm_bp_disable_retain(sm_bp_retain_impl_t *_this) {

	if (_this == NULL)
		return;

	for (uint8_t id = 0; id < SM_SV_BP_NUMBER_DEFAULT; id++) {
		sm_bp_t *_bp = &_this->m_bps[id];

		if (_bp->m_retain_st == BP_ENABLE_RETAIN
				&& _bp->m_data.m_state == BP_STATE_DISCHARGING
				&& _bp->m_is_connected) {

			if (_this->m_cb.disable_retain_cb) {

				_this->m_cb.disable_retain_cb(id, _this->m_arg);
			}
		}
	}
}

static inline int32_t sm_bp_retain_move_bp_to_list(void *_list, uint8_t id) {

    bp_retain_list_t* list = (bp_retain_list_t*) _list;
    if (!list)
        return -1;
    for (int i = 0; i < SM_BP_NUMBER_DEFAULT - 1; i++) {
        if (list[i].index == -1) {
            list[i].index = id;
            return 0;
        }
    }
    return 0;
}
static inline int32_t sm_bp_retain_remove_bp_from_list(void *_list, uint8_t id){

    bp_retain_list_t *list = (bp_retain_list_t*) _list;
    if (!list)
        return -1;
    for (int i = 0; i < SM_BP_NUMBER_DEFAULT; i++){
        if (list[i].index == id){

            list[i].index = -1;
            for (int j = i; j < SM_BP_NUMBER_DEFAULT - 1; j++){
                list[j] = list[j + 1];
            }
            list[SM_BP_NUMBER_DEFAULT - 1].index = -1;
            return 0;
        }
    }
    return 0;
}

static inline int32_t sm_bp_retain_get_list_id(void *_list, uint8_t _bp_id){

    bp_retain_list_t *list = (bp_retain_list_t*) _list;
    if (!list)
        return -1;
    for (int i = 0; i < SM_BP_NUMBER_DEFAULT; i++){
        if (list[i].index == _bp_id){
            return i;
        }
    }
    return 0;
}

sm_bp_retain_t* sm_bp_retain_create(sm_bp_t* bp_list, void* _arg){

	g_bp_retain.m_cb = g_bp_retain_callback;
	g_bp_retain.m_arg = _arg;
	g_bp_retain.bp_retain_numbs = 0;
	g_bp_retain.m_bps = bp_list;
	g_bp_retain.m_paused = 0;
	for(uint8_t i = 0; i < SM_SV_BP_NUMBER_DEFAULT; i++){

	    g_bp_retain.list[i].index = -1;
		elapsed_timer_resetz(&g_bp_retain.list[i].m_timeout, BP_RETAIN_DISCHAR_TIME_MS);
	}


	return &g_bp_retain;
}
sm_bp_retain_t* sm_bp_retain_get(void) {

    return &g_bp_retain;
}

int32_t sm_bp_retain_pause(sm_bp_retain_t *_this){

    if(!_this){

        return -1;
    }

    _impl(_this)->m_paused = 1;
    return _impl(_this)->m_paused;
}

int32_t sm_bp_retain_resume(sm_bp_retain_t *_this){

    if(!_this){

        return -1;
    }

    _impl(_this)->m_paused = 0;
    return _impl(_this)->m_paused;
}

int32_t sm_bp_active_retain(sm_bp_retain_t *_this, uint8_t id) {

    if(!_this){

        return -1;
    }

	sm_bp_t* _bp = &g_bp_retain.m_bps[id];
	if (_bp->m_retain_st == BP_ENABLE_RETAIN) {

		_bp->m_retain_st = BP_ACTIVE_RETAIN;
		_bp->m_data.m_state = BP_ST_RETAIN_DISCHARGE;

		sm_bp_retain_move_bp_to_list(&g_bp_retain.list,id);
	}

	return sm_bp_get_active_retain_numbs(_this);
}

int32_t sm_bp_deactive_retain(sm_bp_retain_t *_this, uint8_t id) {

    if(!_this){

        return -1;
    }

	sm_bp_t* _bp = &g_bp_retain.m_bps[id];
	if (_bp->m_retain_st == BP_ACTIVE_RETAIN) {

		_bp->m_retain_st = BP_NO_SUPPORT_RETAIN;
		sm_bp_retain_remove_bp_from_list(&g_bp_retain.list,id);
	}

	return sm_bp_get_active_retain_numbs(_this);
}

int32_t sm_bp_retain_process(sm_bp_retain_t* _this){
	if(!_this){

		return -1;
	}
	if(_impl(_this)->m_paused){

		sm_bp_disable_retain(_impl(_this));
	}else {

		sm_bp_enable_retain(_impl(_this));
	}

	return sm_bp_get_active_retain_numbs(_this);
}


int32_t sm_bp_get_active_retain_numbs(sm_bp_retain_t* _this) {

	if(!_this){

		return -1;
	}
	uint8_t bp_retain_numbs = 0;
	for (uint8_t i = 0; i < SM_SV_BP_NUMBER_DEFAULT; i++) {

	    sm_bp_t* _bp = &g_bp_retain.m_bps[i];
	    int32_t index = sm_bp_retain_get_list_id(&g_bp_retain.list,i);
		if (!elapsed_timer_get_remain(&g_bp_retain.list[index].m_timeout)&&_bp->m_retain_st == BP_ACTIVE_RETAIN) {

			_bp->m_retain_st = BP_DEACTIVE_RETAIN;
			sm_bp_retain_remove_bp_from_list(&g_bp_retain.list,i);
		}

		if(_bp->m_retain_st == BP_ACTIVE_RETAIN){

			bp_retain_numbs++;
		}
	}
	g_bp_retain.bp_retain_numbs  = bp_retain_numbs;
	return g_bp_retain.bp_retain_numbs;
}

float sm_bp_retain_get_elapsed_ratio(sm_bp_retain_t *_this) {
	if (_impl(_this)->list[0].index == -1)
		return 0;
	return (BP_TIME_MS_RATIO
			* (float) (elapsed_timer_get_remain(&_impl(_this)->list[0].m_timeout)));
}




