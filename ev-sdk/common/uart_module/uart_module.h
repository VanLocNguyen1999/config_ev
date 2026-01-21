/*
 * sm_uart_if.h
 *
 *  Created on: 18 Sept 2025
 *      Author: My PC
 */

#ifndef UART_MODULE_H_
#define UART_MODULE_H_

#include "sm_types.h"
#include <string.h>
#include "sm_hal.h"

#define SM_UART_FUNCTION_CALLBACK_NUM     4
#define SM_QUEUE_UART_MSG                4

typedef struct uart_module uart_module_t;

struct uart_module {
    sm_hal_uart_t*  m_uart_port;
    char*           buff;
    uint32_t        buff_index;
    volatile bool   lock;
    bool            is_new_data;
    bool            rx_flag;
    uint32_t        buff_size;
};

int32_t uart_module_init(uart_module_t *uart_module, sm_hal_uart_t *driver, uint32_t _size);
int32_t uart_module_set_cmd(uart_module_t *uart_module, char* cmd, int32_t timeout);
int32_t uart_module_send_recv(uart_module_t *uart_module, char* cmd,char* data_recv ,int32_t timeout);
int32_t uart_module_write(uart_module_t *uart_module, char* cmd);

int32_t uart_module_read(uart_module_t *uart_module, char* cmd, uint32_t len);
int32_t uart_module_is_busy(uart_module_t *uart_module);

int32_t uart_module_get_data(uart_module_t *uart_module,char* data_recv);
int32_t uart_module_clear_data(uart_module_t *uart_module);

#endif /* UART_MODULE_H_ */
