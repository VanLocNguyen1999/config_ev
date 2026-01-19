/*
 * sp_hal_can.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_CAN_H_
#define HAL_INCLUDE_SM_HAL_CAN_H_

#include <stdint.h>

typedef void sm_hal_can_t;
typedef struct sm_hal_can_msg sm_hal_can_msg_t;

typedef void (*sm_hal_can_rx_cb_fn_t)(sm_hal_can_t*, sm_hal_can_msg_t*, void*);
typedef void (*sm_hal_can_tx_cb_fn_t)(sm_hal_can_t*, sm_hal_can_msg_t*, void*);

/**
 * @struct sm_hal_can_msg
 * @brief can message format
 *
 */
struct sm_hal_can_msg{
    uint32_t id;
    uint8_t data[8];
    uint16_t length;
};


/**
 * @fn sm_hal_can_t sm_hal_can_init*(sm_hal_can_proc_t*, void*)
 * @brief init hal CAN's driver
 *
 * @param m_proc CAN's function
 * @param handle MCU's CAN driver
 * @return CAN's driver pointer
 */
sm_hal_can_t* sm_hal_can_init(const void *_channel, uint32_t _bitrate);
/**
 * @fn void sm_hal_can_deinit(sm_hal_can_t*)
 * @brief clear hal CAN's driver
 *
 * @param _this hal CAN's driver
 */
void sm_hal_can_deinit(sm_hal_can_t *_this);

/**
 *
 * @param _this
 * @param callback
 */
void sm_hal_can_set_rx_cb(sm_hal_can_t *_this, sm_hal_can_rx_cb_fn_t _cb, void* _arg);

/**
 * 
 * @param _this
 * @param callback
 */
void sm_hal_can_set_tx_cb(sm_hal_can_t *_this, sm_hal_can_tx_cb_fn_t _cb, void* _arg);

/**
 * @fn int32_t sm_hal_can_set_baudrate(sm_hal_can_t*, uint32_t)
 * @brief set CAN's baudrate
 * * @param _this
 * @param baudrate
 * @return 0: success | -1 : false
 */
int32_t sm_hal_can_set_bit_rate(sm_hal_can_t *_this, uint32_t _bit_rate);

/**
 * @fn int32_t sm_hal_can_write(sm_hal_can_t*, sm_hal_can_msg_t*)
 * @brief Write CAN Message
 *
 * @param _this
 * @param mgs message
 * @return 0: success | -1 : false
 */
int32_t sm_hal_can_write(sm_hal_can_t *_this, sm_hal_can_msg_t *_mgs);
/**
 * @fn int32_t sm_hal_can_read(sm_hal_can_t*, sm_hal_can_msg_t*)
 * @brief Read CAN message
 *
 * @param _this
 * @param mgs
 * @return 0: success | -1 : false
 */
int32_t sm_hal_can_read(sm_hal_can_t *_this, sm_hal_can_msg_t *_mgs);

/**
 * @fn int32_t sm_hal_can_open(sm_hal_can_t*)
 * @brief Open CAN's driver
 * @param _this
 * @return 0: success | -1 : false
 */
int32_t sm_hal_can_open(sm_hal_can_t *_this);
/**
 * @fn int32_t sm_hal_can_close(sm_hal_can_t*)
 * @brief Close CAN's driver
 * @param _this
 * @return 0: success | -1 : false
 */
int32_t sm_hal_can_close(sm_hal_can_t *_this);

/**
 * @brief sm_hal_can_irq: Call in interrupt CAN
 * @param _this
 * @param _data
 * @return
 */
int32_t sm_hal_can_rx_irq(sm_hal_can_t* _this, sm_hal_can_msg_t*);

/**
 * @brief sm_hal_uart_irq: Call in interrupt CAN
 * @param _this
 * @param _data
 * @return
 */
int32_t sm_hal_can_tx_irq(sm_hal_can_t* _this, sm_hal_can_msg_t*);

#endif /* hal_INCLUDE_SM_HAL_CAN_H_ */
