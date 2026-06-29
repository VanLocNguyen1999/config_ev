/*
 * ra_hal_timer.c
 *
 *  Created on: Sep 19, 2024
 *      Author: admin
 */
#include "sm_hal_timer.h"
#include "hal_data.h"
#include <stdlib.h>


typedef struct {
    timer_instance_t        *m_channel;
    uint32_t                m_period;
    sm_hal_timer_cb_fn_t    m_cb;
    void                    *m_arg;
}ra_timer_t;
#define impl(x) ((ra_timer_t*)(x))

sm_hal_timer_t* sm_hal_timer_init(const void* _timer, uint32_t _period){
    if (!_timer)
        return NULL;
    ra_timer_t *timer = malloc(sizeof(ra_timer_t));
    timer->m_channel = (timer_instance_t*)_timer;
    timer->m_period = _period;
    timer->m_cb = NULL;
    R_GPT_Open( timer->m_channel->p_ctrl,  timer->m_channel->p_cfg);
    return (sm_hal_timer_t*) timer;
}

void sm_hal_timer_deinit(sm_hal_timer_t *_this){
    if(!_this)
        return;
    free(_this);
}

int32_t sm_hal_timer_set_period(sm_hal_timer_t *_this, uint32_t _period){
    if (!_this)
        return -1;
    return R_GPT_PeriodSet(impl(_this)->m_channel->p_ctrl, _period);
}

void sm_hal_timer_set_callback(sm_hal_timer_t *_this, sm_hal_timer_cb_fn_t _cb, void *_arg){
    if (!_this)
        return;
    impl(_this)->m_cb = _cb;
    impl(_this)->m_arg = _arg;

}

int32_t sm_hal_timer_start(sm_hal_timer_t *_this){
    if (!_this)
        return -1;
    return R_GPT_Start(impl(_this)->m_channel->p_ctrl);
}

int32_t sm_hal_timer_stop(sm_hal_timer_t *_this){
    if (!_this)
          return -1;
    return R_GPT_Stop(impl(_this)->m_channel->p_ctrl);
}

void sm_hal_timer_irq(sm_hal_timer_t *_this){
    ra_timer_t* this =  impl(_this);
    if (this && this->m_cb){
        this->m_cb(this->m_arg);
    }
}

int32_t sm_hal_timer_set_counter(sm_hal_timer_t *_this, uint32_t counter){

    if (!_this)
        return -1;
    return R_GPT_CounterSet(impl(_this)->m_channel->p_ctrl, counter);
}
uint32_t sm_hal_timer_get_counter(sm_hal_timer_t *_this){

	if(!_this) return 0;
	timer_status_t time_status;
	fsp_err_t err = R_GPT_StatusGet (impl(_this)->m_channel->p_ctrl, &time_status);
	return time_status.counter;
}
