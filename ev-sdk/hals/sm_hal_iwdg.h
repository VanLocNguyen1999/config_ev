/*
 * sm_hal_iwdg.h
 *
 *  Created on: Jul 12, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_IWDG_H_
#define HAL_INCLUDE_SM_HAL_IWDG_H_

#include <stdint.h>

typedef void sm_hal_iwdg_t;

typedef void (*sm_hal_iwdg_cb_fn_t)();

sm_hal_iwdg_t* sm_hal_iwdg_init(int32_t _period);

int32_t sm_hal_iwdg_deinit(sm_hal_iwdg_t* _this);

int32_t sm_hal_iwdg_set_cb(sm_hal_iwdg_t* _this, sm_hal_iwdg_cb_fn_t _fn);

int32_t sm_hal_iwdg_start(sm_hal_iwdg_t *_this);

int32_t sm_hal_iwdg_stop(sm_hal_iwdg_t *_this);

int32_t sm_hal_iwdg_reset(sm_hal_iwdg_t *_this);

#endif /* hal_INCLUDE_SM_HAL_IWDG_H_ */
