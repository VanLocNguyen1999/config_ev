//
// Created by vnbk on 13/09/2024.
//
#include <sm_hal_delay.h>
#include <stdlib.h>
#include <sm_fifo.h>
#include "sm_hal_uart.h"
#include "linux_serial.h"

#define TAG "SM_HAL_UART_LINUX"
#define USB0 "/dev/ttyUSB0"

#define _impl(x) ((sm_hal_uart_linux_t*)(x))
int32_t g_fd;
typedef struct {
    int32_t *m_fd;
    uint32_t m_baud;
    uint8_t m_is_locked;
    sm_fifo_handle_t m_buff;
    uint8_t m_data_available;
    sm_hal_uart_rx_irq_fn_t rx_irq_cb;
    sm_hal_uart_tx_irq_fn_t tx_irq_cb;
    void *m_rx_cb_arg;
    void *m_tx_cb_arg;
} sm_hal_uart_linux_t;

/**
 * @fn sm_hal_uart_t sm_hal_uart_init*(sm_hal_uart_proc_t*, void*)
 * @brief
 *
 * @param fun
 * @param handle
 * @return
 */
sm_hal_uart_t* sm_hal_uart_init(const void* _channel, uint32_t _baud, uint8_t _stop_bit, uint8_t _data_bit){
    sm_hal_uart_linux_t* _this = malloc(sizeof(sm_hal_uart_linux_t));
     _this->m_fd = &g_fd;
    *_this->m_fd = serial_init(USB0, _baud, SERIAL_FLAG_BLOCKING);
    if (*_this->m_fd<0) return NULL;
    sm_fifo_init(&_this->m_buff, _size, sizeof(uint8_t));

    return (sm_hal_uart_t*) _this;
}

/**
 * @fn void sm_hal_uart_deinit(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_uart_deinit(sm_hal_uart_t *_this){
  sm_fifo_deinit(&_impl(_this)->m_buff);
 free(_this);
}

/**
 * @fn int32_t sm_hal_uart_config
 * @brief
 * @param _this
 * @param _baud
 * @param _stop_bit
 * @param _data_bit
 * @return
 */
int32_t sm_hal_uart_config(sm_hal_uart_t *_this, uint32_t _baud, uint8_t _stop_bit, uint8_t _data_bit){
 return 0;
}

/**
 * @fn int32_t sm_hal_uart_write(sm_hal_uart_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_uart_write(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len){
    while (_impl(_this)->m_is_locked) {
        sm_hal_delay_ms(1);
    }
    _impl(_this)->m_is_locked = 1;
    int32_t ret = serial_send_bytes(*_impl(_this)->m_fd, (uint8_t*) _buff, _len);
    sm_hal_delay_ms(1);
   _impl(_this)->m_is_locked = 0;
    return ret > 0 ? 0 : -1;
}

/**
 * @fn int32_t sm_hal_uart_read(sm_hal_uart_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_uart_read(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len){
    int32_t lenght = 0;
    while (lenght < _len) {
        if (sm_fifo_pop(&_impl(_this)->m_buff, &_buff[lenght])) {
          lenght++;
        }else {
          _impl(_this)->m_data_available = 0;
          break;
        }
    }
    return lenght;
}

/**
 * @fn void sm_hal_uart_set_rx_callback(sm_hal_uart_t*, sm_hal_uart_rx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_rx_cb(sm_hal_uart_t *_this, sm_hal_uart_rx_irq_fn_t _cb, void* _arg){
    if(!_this) return;
    _impl(_this)->rx_irq_cb = _cb;
    _impl(_this)->m_rx_cb_arg = _arg;
}

/**
 * @fn void sm_hal_uart_set_tx_callback(sm_hal_uart_t*, sm_hal_uart_tx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_tx_cb(sm_hal_uart_t *_this, sm_hal_uart_tx_irq_fn_t _cb, void* _arg){
    if(!_this) return;
    _impl(_this)->tx_irq_cb = _cb;
    _impl(_this)->m_tx_cb_arg = _arg;
}

/**
 * @fn int32_t sm_hal_uart_open(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_uart_open(sm_hal_uart_t *_this){

}

/**
 * @fn int32_t sm_hal_uart_close(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_uart_close(sm_hal_uart_t *_this){

}

int32_t sm_hal_uart_rx_irq(sm_hal_uart_t* _this, uint8_t _data) {
    if(!_this) return 0;
    static uint8_t temp_data;
    temp_data = _data;
    sm_fifo_push_irq(&_impl(_this)->m_buff, &temp_data);
    if (_impl(_this)->rx_irq_cb) {
      _impl(_this)->rx_irq_cb(_this, _data, _impl(_this)->m_rx_cb_arg);
    }
}


int32_t sm_hal_uart_tx_irq(sm_hal_uart_t* _this, uint8_t _data) {
    if(!_this) return 0;
    if (_impl(_this)->tx_irq_cb) {
        _impl(_this)->tx_irq_cb(_this, _impl(_this)->m_tx_cb_arg);
    }
}