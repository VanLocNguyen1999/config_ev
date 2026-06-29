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
typedef void (*sm_hal_exti_cb_fn_t)(void*);


sm_hal_exti_t* sm_hal_exti_init(const void* _instance);

void sm_hal_exti_deinit(sm_hal_exti_t *_this);

int32_t sm_hal_exti_start(sm_hal_exti_t *_this);

int32_t sm_hal_exti_stop(sm_hal_exti_t *_this);

#endif /* SM_HAL_EXTI_H_ */
