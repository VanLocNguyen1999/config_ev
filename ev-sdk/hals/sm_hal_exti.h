/*
 * ra_hal_exti.h
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */

#ifndef SM_HAL_EXTI_H_
#define SM_HAL_EXTI_H_
#include <stdint.h>

typedef void sm_hal_exti_t;
typedef void (*sm_hal_exti_cb_fn_t)(uint32_t,void*);
/**
 *
 * @param _instance
 * @return
 */
sm_hal_exti_t* sm_hal_exti_init(const void* _instance, uint32_t _pin);
/**
 *
 * @param _this
 */
void sm_hal_pwm_deinit(sm_hal_exti_t *_this);
/**
 *
 * @param _this
 * @return
 */
int32_t sm_hal_exti_start(sm_hal_exti_t *_this);
/**
 *
 * @param _this
 * @return
 */
int32_t sm_hal_exti_stop(sm_hal_exti_t *_this);
/**
 *
 * @param _this
 * @param _cb
 * @param _arg
 */
void sm_hal_exti_set_callback(sm_hal_exti_t *_this, sm_hal_exti_cb_fn_t _cb, void *_arg);
/**
 *
 * @param _this
 */
void sm_hal_exti_irq(sm_hal_exti_t *_this);

#endif /* SM_HAL_EXTI_H_ */
