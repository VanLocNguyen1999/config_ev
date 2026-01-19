/*
 * sm_hal_i2c.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_I2C_H_
#define HAL_INCLUDE_SM_HAL_I2C_H_

#include <stdint.h>

typedef void sm_hal_i2c_t;

typedef void (*sm_hal_i2c_irq_fn_t)(sm_hal_i2c_t*, uint8_t, void*);
/**
 * @fn sm_hal_i2c_t sm_hal_i2c_init*(sm_hal_i2c_proc_t*, void*)
 * @brief
 * @param _addr
 * @return
 */
sm_hal_i2c_t* sm_hal_i2c_init(void* _channel, uint8_t _addr);

/**
 * @fn void sm_hal_i2c_deinit(sm_hal_i2c_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_i2c_deinit(sm_hal_i2c_t *_this);

void sm_hal_i2c_set_callback(sm_hal_i2c_t *_this, sm_hal_i2c_irq_fn_t _cb, void*_arg);

/**
 * @fn int32_t sm_hal_i2c_set_addr(sm_hal_i2c_t*, uint8_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @return
 */
int32_t sm_hal_i2c_set_addr(sm_hal_i2c_t *_this, uint8_t _addr);

/**
 * @fn int32_t sm_hal_i2c_open(sm_hal_i2c_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_i2c_open(sm_hal_i2c_t *_this);

/**
 * @fn int32_t sm_hal_i2c_close(sm_hal_i2c_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_i2c_close(sm_hal_i2c_t *_this);

/**
 * @fn int32_t sm_hal_i2c_write(sm_hal_i2c_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param data
 * @param len
 * @return
 */
int32_t sm_hal_i2c_write(sm_hal_i2c_t *_this, const uint8_t *_data, uint32_t _len);

/**
 * @fn int32_t sm_hal_i2c_read(sm_hal_i2c_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param data
 * @param len
 * @return
 */
int32_t sm_hal_i2c_read(sm_hal_i2c_t *_this, uint8_t *_data, uint32_t _len);

/**
 *
 * @param _this
 * @param _data
 * @param _data_len
 * @param _buf
 * @param _recv_len
 * @return
 */
int32_t sm_hal_i2c_write_read(sm_hal_i2c_t *_this,
                              const uint8_t *_data,
                              uint32_t _data_len,
                              uint8_t* _buf,
                              uint32_t* _recv_len);


/**
 * @brief This is call in interrupt
 * @param _this
 * @param event
 * @return
 */
int32_t sm_hal_i2c_callback(sm_hal_i2c_t *_this, uint8_t event);

#endif /* HAL_INCLUDE_SM_HAL_I2C_H_ */
