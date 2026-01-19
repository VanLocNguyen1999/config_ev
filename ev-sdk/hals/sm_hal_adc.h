/*
 * sm_hal_adc.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_ADC_H_
#define SM_HAL_ADC_H_

#include <stdint.h>

typedef void sm_hal_adc_t;

/**
 * @brief sm_hal_adc_init
 * @param _proc
 * @param _handle
 * @return *sm_hal_adc_t
 */
sm_hal_adc_t* sm_hal_adc_init(void* _handle, int32_t _channel, uint32_t _flag);

/**
 * @brief sm_hal_adc_deinit
 * @param _this
 */
void sm_hal_adc_deinit(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_read
 * @param _this
 * @return
 */
uint16_t sm_hal_adc_read(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_start
 * @param _this
 * @return
 */
int32_t sm_hal_adc_start(sm_hal_adc_t *_this);

/**
 * @brief
 * sm_hal_adc_stop
 * @param _this
 * @return
 */
int32_t sm_hal_adc_stop(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_callback
 * @param _this
 * @param _value
 */
void sm_hal_adc_set_cb(sm_hal_adc_t *_this, void(*on_converted_fn_t)(uint16_t));

/**
 * @brief sm_hal_adc_open
 * @param _this
 * @return 0
 */

int32_t sm_hal_adc_open(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_close
 * @param _this
 * @return 0
 */
int32_t sm_hal_adc_close(sm_hal_adc_t *_this);

#endif /* SM_HAL_ADC_H_ */
