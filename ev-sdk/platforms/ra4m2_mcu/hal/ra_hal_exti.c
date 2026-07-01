/*
 * ra_hal_exti.c
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */
#include "stdlib.h"
#include "common_data.h"
#include "sm_hal_exti.h"

#define _impl(x)			((sm_hal_flash_impl_t*)(x))
typedef struct{
	const external_irq_instance_t* m_instance;
	uint32_t m_pin;
	sm_hal_exti_cb_fn_t m_cb;
	void* m_arg;
}sm_hal_flash_impl_t;

sm_hal_exti_t* sm_hal_exti_init(const void* _instance, uint32_t _pin){

	if(!_instance) return NULL;
	sm_hal_flash_impl_t* this = (sm_hal_flash_impl_t*)malloc(sizeof(sm_hal_flash_impl_t));
	this->m_instance = _instance;
	this->m_pin = _pin;
	this->m_cb = NULL;
	this->m_arg = NULL;
	return (sm_hal_exti_t*)this;
}

void sm_hal_pwm_deinit(sm_hal_exti_t *_this){

    if(!_this)
        return;
    free(_this);
}

int32_t sm_hal_exti_start(sm_hal_exti_t *_this){

	if(!_this) return -1;
	fsp_err_t err = FSP_SUCCESS;
	R_IOPORT_Open(&g_ioport_ctrl, &g_bsp_pin_cfg);
	err = R_ICU_ExternalIrqOpen(_impl(_this)->m_instance->p_ctrl, _impl(_this)->m_instance->p_cfg);
	if(err != FSP_SUCCESS) return -1;
    // Bật ngắt
	err = R_ICU_ExternalIrqEnable(_impl(_this)->m_instance->p_ctrl);
	if(err != FSP_SUCCESS) return -1;
	return 0;
}

int32_t sm_hal_exti_stop(sm_hal_exti_t *_this){

	if(!_this) return -1;
	fsp_err_t err = FSP_SUCCESS;
    // Tắt ngắt
	err = R_ICU_ExternalIrqDisable(_impl(_this)->m_instance->p_ctrl);
	if(err != FSP_SUCCESS) return -1;
	return 0;
}

void sm_hal_exti_set_callback(sm_hal_exti_t *_this, sm_hal_exti_cb_fn_t _cb, void *_arg){
    if (!_this)
        return;
    _impl(_this)->m_cb = _cb;
    _impl(_this)->m_arg = _arg;
}

void sm_hal_exti_irq(sm_hal_exti_t *_this) {
	if (_this && _impl(_this)->m_cb) {
		bsp_io_level_t level;
		R_IOPORT_PinRead(&g_ioport_ctrl, _impl(_this)->m_pin, &level);
		_impl(_this)->m_cb(level, _impl(_this)->m_arg);
	}
}
