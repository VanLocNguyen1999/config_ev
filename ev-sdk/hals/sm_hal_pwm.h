#ifndef INCLUDE_SM_HAL_PWM_H_
#define INCLUDE_SM_HAL_PWM_H_

#include <stdint.h>

typedef void sm_hal_pwm_t;

/**
 * @fn sm_hal_pwm_t sm_hal_pwm_init*(sm_hal_pwm_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_pwm_t* sm_hal_pwm_init(uint32_t _freq, uint32_t _duty);

/**
 * @fn void sm_hal_pwm_deinit(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_pwm_deinit(sm_hal_pwm_t *_this);

/**
 * @fn int32_t sm_hal_pwm_set_freq(sm_hal_pwm_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param freq
 * @return
 */
int32_t sm_hal_pwm_set_freq(sm_hal_pwm_t *_this, uint32_t _freq);

/**
 * @fn int32_t sm_hal_pwm_set_dutycycles(sm_hal_pwm_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param dutycycle
 * @return
 */
int32_t sm_hal_pwm_set_duty_cycle(sm_hal_pwm_t *_this, uint32_t _duty_cycle);

/**
 * @fn int32_t sm_hal_pwm_set_start(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_pwm_start(sm_hal_pwm_t *_this);

/**
 * @fn int32_t sm_hal_pwm_set_stop(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_pwm_stop(sm_hal_pwm_t *_this);

#endif
