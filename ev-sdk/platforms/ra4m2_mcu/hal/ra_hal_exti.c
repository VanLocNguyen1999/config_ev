/*
 * ra_hal_exti.c
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */
#include "common_data.h"
#include "sm_hal_exti.h"

#define _impl(x)			((sm_hal_flash_impl_t*)(x))
typedef struct{
	const external_irq_instance_t* m_instance;
	sm_hal_exti_cb_fn_t m_cb;
	void* arg;
}sm_hal_flash_impl_t;

sm_hal_exti_t* sm_hal_exti_init(const void* _instance){

	if(!_instance) return NULL;
	sm_hal_flash_impl_t* this = (sm_hal_flash_impl_t*)malloc(sizeof(sm_hal_flash_impl_t));
	this->m_instance = _instance;
	this->m_cb = NULL;
	this->arg = NULL;
	return (sm_hal_exti_t*)this;
}

void sm_hal_pwm_deinit(sm_hal_exti_t *_this);

int32_t sm_hal_exti_start(sm_hal_exti_t *_this){

	if(!_this) return -1;
	fsp_err_t err = FSP_SUCCESS;
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

