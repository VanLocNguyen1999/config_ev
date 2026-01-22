/*
 * sm_pmu_uart.c
 *
 *  Created on: 25 Sept 2025
 *      Author: My PC
 */

#ifndef SM_PMU_UART_C_
#define SM_PMU_UART_C_

#include "sm_bsp_pmu.h"
#include "sm_pmu_uart.h"
#include "sm_pmu_app.h"

#include "sm_fifo.h"
#include "ev_io_define.h"

#include "stdlib.h"
#include "string.h"
#include <ctype.h>

#define _impl(x) ((sm_uart*)(x))
#define MAX_UART_CMD		4
#define UART_EXIT			0

#define CMD_SUCCESS    (0)
#define CMD_FAILURE    (-1)
const char *UART_CMD[CMD_NUMBER] = {
		"READ",
		"WRITE",
		"UV",
		"I_BAT",
		"ECO",
		"SPORT",
		"EXIT", };

typedef struct sm_uart sm_uart_t;
typedef void (*msg_handler_t)(const char* data, void* _arg);
typedef struct {

	uart_module_t* m_base;

    ascii_field_cb_t m_extract_frame_cb;
    sm_ev_config_para_t *m_ev_config;
    sm_fifo_handle_t cmd;
    char 	m_cmd_current[400];
    int8_t m_cmd_in_process;
    int8_t m_pause_poll;

    int8_t m_force;
    UART_CFG_STATE m_state;
    UART_WRITE_STATE m_write_state;
}sm_uart ;

static sm_uart g_sm_uart = {
		.m_base = NULL,
		.m_ev_config = NULL,
		.m_cmd_in_process = 0,
		.m_pause_poll = 0,
		.m_state = UART_CFG_IDLE,
		.m_write_state = UART_WRITE_IDLE,
		.m_force = 0
};

static void msg_cmd_read(const char* data, void* _arg);
static void msg_cmd_write(const char* data, void* _arg);
static void msg_cmd_write_uv(const char* data, void* _arg);
static void msg_cmd_write_ibat(const char* data, void* _arg);
static void msg_cmd_write_eco_sport(const char* data, void* _arg);
static void msg_cmd_write_speed_sport(const char* data, void* _arg);
static void msg_cmd_exit(const char* data, void* _arg);
msg_handler_t msg_idle_state_handler[CMD_NUMBER] ={

        msg_cmd_read,
        msg_cmd_write,
		msg_cmd_write_uv,
		msg_cmd_write_ibat,
		msg_cmd_write_eco_sport,
		msg_cmd_write_speed_sport,
		msg_cmd_exit
};

static void sm_uart_cmd_queue_init(sm_uart* _this){
    sm_fifo_init(&_impl(_this)->cmd, MAX_UART_CMD, MAX_ASCII_MSG_SIZE);
}

bool sm_cmd_push(sm_pmu_uart_t* _this, char* data) {

	(void)_this;
    return sm_fifo_push(&g_sm_uart.cmd, data);
}

bool sm_cmd_pop(sm_pmu_uart_t* _this, char* data) {

	(void)_this;
    return sm_fifo_pop(&g_sm_uart.cmd, data);
}
sm_pmu_uart_t* sm_pmu_uart_create_default(sm_ev_config_para_t *m_ev_config){

	sm_uart *this = &g_sm_uart;
    uart_module_t * uart = (uart_module_t*) malloc(sizeof(uart_module_t));
    if (!uart)
    {
        return NULL;
    }

    this->m_base = uart;
    this->m_ev_config = m_ev_config;
    memset(g_sm_uart.m_cmd_current, 0, sizeof(g_sm_uart.m_cmd_current));

    sm_hal_uart_t *driver = sm_bsp_pmu_get_uart_port();
    uart_module_init(this->m_base, driver, MAX_ASCII_MSG_SIZE);
    sm_uart_cmd_queue_init(this);

    return this;
}

int32_t sm_pmu_uart_polling_msg(sm_pmu_uart_t* _this){

	if(!_this || (_impl(_this)->m_pause_poll && !_impl(_this)->m_force)){

		return -1;
	}
	_impl(_this)->m_force = 0;
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;
	char _msg[300] = "";
	strcpy(_msg, "\r\n============================================\r\n"
            		 "======        TRANG THAI CHO        ========\r\n"
                     "============================================\r\n"
                     "	Gưi \"READ\" de doc thong so dang kiem \r\n"
                     "	Gưi \"WRITE\" de ghi thong so dang kiem \r\n"
            		 "============================================\r\n");
	uart_module_write(uart_module, _msg);
	return 0;
}

static char* trim_string(char *str) {
	char *end;
	while (isspace((unsigned char) *str))
		str++;
	if (*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char) *end))
		end--;
	*(end + 1) = 0;
	return str;
}

static int32_t parse_command(char *data_recv, char *out_cmd, char *out_data) {
    char *temp_cmd = strtok(data_recv, "=");
    char *temp_data = strtok(NULL, "=");

    if (temp_cmd == NULL && temp_data == NULL) {
        return -1;
    }
	strcpy(out_cmd, trim_string(temp_cmd));
	strcpy(out_data, trim_string(temp_data));
    return 0;
}
int32_t sm_uart_process(sm_pmu_uart_t* _this, void* _arg){

	(void)_arg;
	if(!_this){

		return -1;
	}
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;
	if(_impl(_this)->m_cmd_current[0] != '\0'){

		uart_module_write(uart_module, _impl(_this)->m_cmd_current);
		_impl(_this)->m_cmd_current[0] = '\0';
	}
	char data_recv[100] = "";
	if(uart_module_get_data(uart_module,data_recv) < 0){
	    return -1;
	}
	char cmd_buf[10] = "";
	char data_buf[10]= "";
	parse_command(data_recv, cmd_buf, data_buf);
	for (size_t i = 0; i < CMD_NUMBER; i++) {
		if (strcmp(cmd_buf, UART_CMD[i]) == 0) {
			msg_idle_state_handler[i](data_buf, _this);
			break;
		}
	}

	return 0;
}
static void msg_cmd_write(const char* data, void* _arg) {

	(void) data;
	uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
	_impl(_arg)->m_pause_poll = 1;
	char _msg[400] = "";
	_impl(_arg)->m_state = UART_CFG_WRITE;
	strcpy(_msg, "\r\n============================================\r\n"
                     "	Gưi \"UV\" de ghi bao ve thap ap \r\n"
					 "	Gưi \"I_BAT\" de ghi gioi han dong PIN \r\n"
                     "	Gưi \"ECO\" de ghi toc ECO \r\n"
            		 "	Gưi \"SPORT\" de ghi toc SPORT \r\n"
					 "	Gưi \"EXIT\" de thoat \r\n"
            		 "============================================\r\n");
	uart_module_write(uart_module, _msg);
}
static void msg_cmd_read(const char* data, void* _arg)  {

    (void)data;
    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
    _impl(_arg)->m_pause_poll = 1;
    char _msg[400] = "";
    _impl(_arg)->m_state = UART_CFG_READ;

    float sport_speed   = 50.0;
    float eco_speed     = 35.0;
    float current_limit = 49.5;
    float low_voltage   = 48.5;

    snprintf(_msg, sizeof(_msg),
             "\r\n============================================\r\n"
                 "	Toc do toi da eco la  : %0.2f kmh\r\n"
                 "	Toc do toi da sport la: %0.2f kmh\r\n"
                 "	Gioi han dong PIN la  : %0.2f A\r\n"
                 "	Bao ve thap ap la     : %0.2f V\r\n"
                 "	Gưi \"EXIT\" de thoat\r\n"
                 "============================================\r\n",
				 eco_speed,sport_speed, current_limit, low_voltage);

    uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_uv_res_cb(void* data, int32_t res, void *_arg) {

	uart_module_t *uart_module = (uart_module_t*) _impl(_arg)->m_base;
	char _msg[400] = "";
	float value = 30.0f;//*(float*)data;
	if (res == CMD_SUCCESS) {

		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi thanh cong UV = %0.2f (V)\r\n"
						"============================================\r\n",
						value);
	} else if (res == CMD_FAILURE) {
		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi that bai UV = %0.2f (V)\r\n"
						"============================================\r\n",
						value);
	}

	memcpy(_impl(_arg)->m_cmd_current,_msg,sizeof(_msg));
}

static void msg_cmd_write_uv(const char *data, void *_arg) {
	float value = (float) atof(data);
	sm_ev_set_uv_protect(_impl(_arg)->m_ev_config, (float) value);

	sm_ev_config_cmd_t cmd;
	cmd.m_arg = _arg;
	cmd.m_cmd = EV_CFG_UV_PROTECT;
	cmd.m_cb = msg_cmd_write_uv_res_cb,
	cmd.m_data = &value;
	sm_ev_config_co_set_cmd(_impl(_arg)->m_ev_config, cmd);

//    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
//    char _msg[400] = "";
//	snprintf(_msg, sizeof(_msg),
//             "\r\n============================================\r\n"
//                 "	Ghi thanh cong UV = %0.2f (V)\r\n"
//                 "============================================\r\n",
//				 value);
//    uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_ibat_limit_res_cb(void* data, int32_t res, void *_arg) {

	uart_module_t *uart_module = (uart_module_t*) _impl(_arg)->m_base;
	char _msg[400] = "";

	float value = *(float*)data;
	if (res == CMD_SUCCESS) {

		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi thanh cong I_BAT = %0.2f (A)\r\n"
						"============================================\r\n",
						value);
	} else if (res == CMD_FAILURE) {
		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi that bai I_BAT = %0.2f (A)\r\n"
						"============================================\r\n",
						value);
	}

	uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_ibat(const char *data, void *_arg) {
	float value = (float)atof(data);
	sm_ev_set_ibat_limit(_impl(_arg)->m_ev_config, (float) value);

	sm_ev_config_cmd_t cmd;
	cmd.m_arg = _arg;
	cmd.m_cmd = EV_CFG_IBAT_LIMIT;
	cmd.m_cb = msg_cmd_write_ibat_limit_res_cb;
	cmd.m_data = &value;
	sm_ev_config_co_set_cmd(_impl(_arg)->m_ev_config, cmd);

//    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
//    char _msg[400] = "";
//    snprintf(_msg, sizeof(_msg),
//             "\r\n============================================\r\n"
//                 "	Ghi thanh cong I_BAT = %0.2f (A)\r\n"
//                 "============================================\r\n",
//				 value);
//
//    uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_eco_sport_res_cb(void* data, int32_t res, void *_arg) {

	uart_module_t *uart_module = (uart_module_t*) _impl(_arg)->m_base;
	char _msg[400] = "";
	uint8_t value = *(uint8_t*)data;
	if (res == CMD_SUCCESS) {

		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi thanh cong ECO_SPEED = %02d (km/h)\r\n"
						"============================================\r\n",
						value);
	} else if (res == CMD_FAILURE) {
		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi that bai ECO_SPEED = %02d (km/h)\r\n"
						"============================================\r\n",
						value);
	}

	uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_eco_sport(const char *data, void *_arg) {
	uint8_t value = (uint8_t)atoi(data);
	sm_ev_set_eco_speed(_impl(_arg)->m_ev_config, (uint8_t) value);

	sm_ev_config_cmd_t cmd;
	cmd.m_arg = _arg;
	cmd.m_cmd = EV_CFG_ECO_SPEED;
	cmd.m_cb = msg_cmd_write_eco_sport_res_cb;
	cmd.m_data = &value;
	sm_ev_config_co_set_cmd(_impl(_arg)->m_ev_config, cmd);

//    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
//    char _msg[400] = "";
//    snprintf(_msg, sizeof(_msg),
//             "\r\n============================================\r\n"
//                 "	Ghi thanh cong ECO_SPEED = %02d (km/h)\r\n"
//                 "============================================\r\n",
//				 value);
//
//    uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_speed_sport_res_cb(void* data, int32_t res, void *_arg) {

	uart_module_t *uart_module = (uart_module_t*) _impl(_arg)->m_base;
	char _msg[400] = "";
	uint8_t value = *(uint8_t*)data;
	if (res == CMD_SUCCESS) {

		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi thanh cong SPORT_SPEED = %02d (km/h)\r\n"
						"============================================\r\n",
						value);
	} else if (res == CMD_FAILURE) {
		snprintf(_msg, sizeof(_msg),
				"\r\n============================================\r\n"
						"	Ghi that bai SPORT_SPEED = %02d (km/h)\r\n"
						"============================================\r\n",
						value);
	}

	uart_module_write(uart_module, _msg);
}

static void msg_cmd_write_speed_sport(const char *data, void *_arg) {

	uint8_t value = (uint8_t)atoi(data);
	sm_ev_set_sport_speed(_impl(_arg)->m_ev_config, (uint8_t) value);

	sm_ev_config_cmd_t cmd;
	cmd.m_arg = _arg;
	cmd.m_cmd = EV_CFG_SPORT_SPEED;
	cmd.m_cb = msg_cmd_write_speed_sport_res_cb;
	cmd.m_data = &value;
	sm_ev_config_co_set_cmd(_impl(_arg)->m_ev_config, cmd);
//    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
//    char _msg[400] = "";
//    snprintf(_msg, sizeof(_msg),
//             "\r\n============================================\r\n"
//                 "	Ghi thanh cong SPORT_SPEED = %02d (km/h)\r\n"
//                 "============================================\r\n",
//				 value);
//
//    uart_module_write(uart_module, _msg);
}

static void msg_cmd_exit(const char* data, void* _arg){

	(void)data;
	 _impl(_arg)->m_force = 1;
	sm_pmu_uart_polling_msg(_impl(_arg));
}
#endif /* SM_PMU_UART_C_ */
