/*
 * uart_module.c
 *
 *  Created on: 24 Sept 2025
 *      Author: My PC
 */

#include <stdlib.h>
#include "uart_module.h"
#include "sm_elapsed_timer.h"


static void uart_module_recv_data_callback(uint8_t _data, void* _arg) {

	if (!_arg) return;
	uart_module_t *modem = (uart_module_t*) _arg;
	 char* buff = (char*)modem->buff;

    if (_data == '<') {
    	modem->is_new_data = true;
    	modem->buff_index = 0;
        buff[modem->buff_index++] = (char) _data;
    }
    else if (_data == '>') {
        if (modem->is_new_data) {
            if (modem->buff_index < modem->buff_size - 1) {
                buff[modem->buff_index++] = (char) _data;
            }
            buff[modem->buff_index] = '\0';
            modem->is_new_data = false;
            modem->rx_flag = true;
        }
    }
    else {
        if (modem->is_new_data) {
            if (modem->buff_index < modem->buff_size - 1) {
                buff[modem->buff_index++] = (char) _data;
            } else {
            	modem->is_new_data = false;
            	modem->buff_index = 0;
            }
        }
    }
}


int32_t uart_module_init(uart_module_t *uart_module, sm_hal_uart_t *driver, uint32_t _size){

    if (!uart_module) {

        return -1;
    }
    uart_module->m_uart_port = driver;
    uart_module->buff_size =_size;
    uart_module->buff_index = 0;

    sm_hal_uart_set_rx_cb(driver, uart_module_recv_data_callback, uart_module);
    uart_module->buff = (char*)malloc(_size);
    uart_module->rx_flag = false;
    uart_module->lock = 0;
    uart_module->is_new_data = false;
    return 0;
}

int32_t uart_module_set_cmd(uart_module_t *uart_module, char* cmd, int32_t timeout){

    if (!uart_module || ! cmd) {

        return -1;
    }

    while (uart_module->lock) {
        sm_hal_delay_ms(1);
    }
    uart_module->lock = 1;


    elapsed_timer_t time;
    elapsed_timer_resetz(&time, timeout);

    memset(uart_module->buff, 0, uart_module->buff_size);
    uart_module->buff_index = 0;

    sm_hal_uart_write(uart_module->m_uart_port, (uint8_t*)cmd, strlen(cmd));

    int32_t res = 0;
    while (1) {
        if (!elapsed_timer_get_remain(&time)) {
            res = -1;
            break;
        }
        if (uart_module->rx_flag) {
            uart_module->rx_flag = false;
            res = 0;
            break;
        }
        sm_hal_delay_ms(1);
    }

    uart_module->lock = 0;
    return res;
}

int32_t uart_module_send_recv(uart_module_t *uart_module, char* cmd,char* data_recv ,int32_t timeout){

    if (!uart_module || ! cmd) {

        return -1;
    }

    while (uart_module->lock) {
        sm_hal_delay_ms(1);
    }
    uart_module->lock = 1;


    elapsed_timer_t time;
    elapsed_timer_resetz(&time, timeout);

    memset(uart_module->buff, 0, uart_module->buff_size);
    uart_module->buff_index = 0;

    sm_hal_uart_write(uart_module->m_uart_port, (uint8_t*)cmd, strlen(cmd));

    int32_t res = 0;
    while (1) {
        if (!elapsed_timer_get_remain(&time)) {
            res = -1;
            break;
        }
        if (uart_module->rx_flag) {
            uart_module->rx_flag = false;
            res = 0;
            break;
        }
        sm_hal_delay_ms(1);
    }

    uart_module->lock = 0;
    strncpy(data_recv, uart_module->buff, uart_module->buff_index);
    data_recv[uart_module->buff_index] = '\0';
    return res;
}

int32_t uart_module_write(uart_module_t *uart_module, char* cmd){
    if (!uart_module) {
        return -1;
    }
    while (uart_module->lock) {
        sm_hal_delay_ms(1);
    }
    uart_module->lock = 1;
    int32_t rc = sm_hal_uart_write(uart_module->m_uart_port, (uint8_t*) cmd,strlen(cmd));
    uart_module->lock = 0;
    return rc;
}

int32_t uart_module_read(uart_module_t *uart_module, char* cmd, uint32_t len){
    if (!uart_module) {
        return -1;
    }
    while (uart_module->lock) {
        sm_hal_delay_ms(1);
    }
    uart_module->lock = 1;
    uint32_t length = 0;

    if (len <= uart_module->buff_size && uart_module->buff_index != 0){
        length = (len <= (uart_module->buff_index + 1)) ? len : (uart_module->buff_index + 1);
        memcpy(cmd, uart_module->buff, length);

        memset(uart_module->buff, 0, uart_module->buff_size);
        uart_module->buff_index = 0;
    }

    uart_module->lock = 0;
    return (int32_t)length;
}

int32_t uart_module_is_busy(uart_module_t *uart_module){

    if (!uart_module) {
        return -1;
    }

    return (uart_module->lock == 0) ? 1: 0;
}
