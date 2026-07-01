/*
 * sm_hal_timer_agt_low_pw.h
 *
 *  Created on: Dec 12, 2024
 *      Author: admin
 */

#ifndef HALS_SM_HAL_TIMER_AGT_H_
#define HALS_SM_HAL_TIMER_AGT_H_

#include "sm_types.h"

typedef void* sm_hal_timer_agt_t;
typedef void (*sm_hal_timer_agt_cb_fn_t)(void*);

/**
 * @fn sm_hal_timer_agt_t sm_hal_timer_agt_init()
 * @brief
 *@param _timer
 * @param _period
 * @return
 */
sm_hal_timer_agt_t* sm_hal_timer_agt_init(const void* _timer, uint32_t _period);

/**
 * @fn void sm_hal_timer_agt_deinit(sm_hal_timer_agt_t*)
 * @brief
 * @param _this
 */
void sm_hal_timer_agt_deinit(sm_hal_timer_agt_t *_this);

/**
 * @fn int32_t sm_hal_timer_agt_set_period
 * @brief
 * @param _this
 * @param _period
 * @return
 */
int32_t sm_hal_timer_agt_set_period(sm_hal_timer_agt_t *_this, uint32_t _period);

/**
 * @fn void sm_hal_timer_agt_set_callback()
 * @brief
 * @param _this
 * @param _cb
 * @param arg
 */
void sm_hal_timer_agt_set_callback(sm_hal_timer_agt_t *_this, sm_hal_timer_agt_cb_fn_t _cb, void *arg);

/**
 * @fn int32_t sm_hal_timer_agt_start(sm_hal_timer_agt_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_timer_agt_start(sm_hal_timer_agt_t *_this);

/**
 * @fn int32_t sm_hal_timer_agt_stop(sm_hal_timer_agt_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_timer_agt_stop(sm_hal_timer_agt_t *_this);

/**
 * @fn void sm_hal_timer_agt_callback(sm_hal_timer_agt_t*)
 * @brief
 * @param _this
 */
void sm_hal_timer_agt_irq(sm_hal_timer_agt_t *_this); // Calling function in timer interrupt
#endif /* HALS_SM_HAL_TIMER_AGT_H_ */
