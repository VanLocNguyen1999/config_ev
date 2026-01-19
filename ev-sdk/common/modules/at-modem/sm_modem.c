/*
 * sm_modem.c
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */
#include <string.h>
#include "sm_modem.h"
#include "sm_hal_delay.h"
#include "sm_logger.h"
#include "sm_memory.h"
#include "sm_elapsed_timer.h"

#define TAG  "MODEM"

static void modem_recv_data_callback(uint8_t _data, void* _arg){
    /// Call in interrupt
    if (!_arg)
        return;
    sm_modem_t *modem = (sm_modem_t*) _arg;
    char* buff = (char*)modem->buff;
    if(modem->buff_index < modem->buff_size){
        buff[modem->buff_index++] = (char)_data;
    }
}

void sm_modem_init(sm_modem_t *modem, sm_hal_io_t *io, sm_hal_uart_t *driver, uint32_t _size){
	if (!modem) {
		return;
	}
    modem->reset_pin = io;
    modem->driver = driver;
    modem->buff_size = _size;
    modem->buff_index = 0;
    if(modem->buff == NULL){
        modem->buff = (char*)mem_alloc(_size);
    }
    sm_hal_uart_set_rx_cb(driver, modem_recv_data_callback, modem);
    modem->lock = 0;
}

void sm_modem_deinit(sm_modem_t *modem){
	if (!modem) {
		return;
	}
    modem->reset_pin = NULL;
    modem->driver = NULL;
}

int32_t sm_modem_send_cmd(sm_modem_t *modem, char* cmd, char* res_succ, char* res_fail, uint32_t timeout){
	if (!modem || !cmd || !res_succ || !res_fail) {
		return -1;
	}
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;

	elapsed_timer_t time;
	elapsed_timer_resetz(&time, timeout);

    memset(modem->buff, 0, modem->buff_size);
    modem->buff_index = 0;

    sm_hal_uart_write(modem->driver, (uint8_t*)cmd, strlen(cmd));
    int32_t res = 0;
    while(1){
        if (!elapsed_timer_get_remain(&time)) {
			LOG_ERR(TAG, "cmd: %s >>>>>> timeout", cmd);
			res = -1;
			break;
		}
        if (strstr(modem->buff, res_succ) != NULL) {
            res = 0;
            break;
        }
        if (strstr(modem->buff, res_fail) != NULL) {
            res = -2;
            break;
        }
        sm_hal_delay_ms(1);
    }
//	LOG_DBG(TAG, "cmd: %s res: %s len: %d time: %ld", cmd, modem->buff, strlen(modem->buff), timeout-elapsed_timer_get_remain(&time));
	modem->lock = 0;
    return res;
}

int32_t sm_modem_send_recv(sm_modem_t *modem, char *cmd,
							char *res_succ, char *res_fail,
							char *res_buff, uint32_t res_buff_size,
							uint32_t timeout) {
	if (!modem || !cmd) {
		return -1;
	}
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;

	elapsed_timer_t time;
	elapsed_timer_resetz(&time, timeout);

	memset(modem->buff, 0, modem->buff_size);
    modem->buff_index = 0;

	sm_hal_uart_write(modem->driver, (uint8_t*)cmd, strlen(cmd));
	int32_t res = 0;
	while(1){
		if (!elapsed_timer_get_remain(&time)) {
			LOG_ERR(TAG, "cmd: %s >>>>>> timeout", cmd);
			res = -1;
			break;
		}
		if (strstr(modem->buff, res_succ) != NULL) {
			res = 0;
			break;
		}
		if (strstr(modem->buff, res_fail) != NULL) {
			res = -2;
			break;
		}
		sm_hal_delay_ms(1);
	}

//	LOG_DBG(TAG, "cmd: %s res: %s len: %d time: %d",cmd, modem->buff, strlen(modem->buff), timeout - elapsed_timer_get_remain(&time));
	memcpy(res_buff, modem->buff, res_buff_size);
	modem->lock = 0;
	return res;
}

int32_t sm_modem_write(sm_modem_t *modem, char* data, int32_t len){
	if (!modem) {
		return -1;
	}
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;
	int32_t rc = sm_hal_uart_write(modem->driver, (uint8_t*) data, (uint32_t) len);
	modem->lock = 0;
	return rc;
}

void sm_modem_hw_reset(sm_modem_t *modem, uint32_t duration_ms, int logic) {
	if (!modem) {
		return;
	}
	if (logic) {
        sm_hal_io_set_value(modem->reset_pin, 0);
		sm_hal_delay_ms(duration_ms);
        sm_hal_io_set_value(modem->reset_pin, 1);
	} else {
        sm_hal_io_set_value(modem->reset_pin, 1);
		sm_hal_delay_ms(duration_ms);
        sm_hal_io_set_value(modem->reset_pin, 0);
	}
}

int32_t sm_modem_read(sm_modem_t *modem, char* data, uint32_t len){
	if (!modem) {
		return -1;
	}
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;
    uint32_t length = 0;

	if (len <= modem->buff_size && modem->buff_index != 0){
        length = (len <= (modem->buff_index + 1)) ? len : (modem->buff_index + 1);
	    memcpy(data, modem->buff, length);

	    memset(modem->buff, 0, modem->buff_size);
        modem->buff_index = 0;
	}

	modem->lock = 0;
	return (int32_t)length;
}

