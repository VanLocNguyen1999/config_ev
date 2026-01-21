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

#define _impl(x) ((sm_uart*)(x))
#define MAX_UART_CMD		4

//#define  CMD_POLL                   "POLL"
//#define  CMD_INPUT                  "INPUT"
//#define  CMD_OUTPUT                 "OUTPUT"
//
//#define  CMD_LEFT_LIGHT             "LEFT_LIGHT"
//#define  CMD_RIGH_LIGHT             "RIGH_LIGHT"
//#define  CMD_HORN                   "HORN"
//#define  CMD_EMERGENCY              "EMERGENCY"
//#define  CMD_HIGH_BEAM_LIGHT        "HIGH_BEAM"
//#define  CMD_POS_BEAM_LIGHT         "POS_BEAM"
//#define  CMD_LOW_BEAM_LIGHT         "LOW_BEAM"
//#define  CMD_CHARGER                "CHARGER"
//#define  CMD_MC_POWER               "MC_POWER"


typedef struct sm_uart sm_uart_t;
typedef void (*msg_handler_t)(const char* data, void* _arg);
typedef struct {

	uart_module_t* m_base;

    ascii_field_cb_t m_extract_frame_cb;

    sm_fifo_handle_t cmd;
    char 	m_cmd_current[MAX_ASCII_MSG_SIZE];
    int8_t m_cmd_in_process;
    int8_t m_pause_poll;

    int8_t m_force;
    UART_CFG_STATE m_state;
}sm_uart ;

static sm_uart g_sm_uart = {
		.m_base = NULL,
		.m_cmd_in_process = 0,
		.m_pause_poll = 0,
		.m_state = UART_CFG_IDLE,
		.m_force = 0
};

static void msg_cmd_read(const char* data, void* _arg);
static void msg_cmd_write(const char* data, void* _arg);

msg_handler_t msg_idle_state_handler[CMD_IDLE_NUMBER] ={

        msg_cmd_read,
        msg_cmd_write,
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
sm_pmu_uart_t* sm_pmu_uart_create_default(void){

	sm_uart *this = &g_sm_uart;
    uart_module_t * uart = (uart_module_t*) malloc(sizeof(uart_module_t));
    if (!uart)
    {
        return NULL;
    }

    this->m_base = uart;
    memset(g_sm_uart.m_cmd_current, 0, sizeof(g_sm_uart.m_cmd_current));

    sm_hal_uart_t *driver = sm_bsp_pmu_get_uart_port();
    uart_module_init(this->m_base, driver, MAX_ASCII_MSG_SIZE);
    sm_uart_cmd_queue_init(this);

    return this;
}

int32_t sm_pmu_uart_polling_msg(sm_pmu_uart_t* _this, void* _arg){

	(void)_arg;
	if(!_this || (_impl(_this)->m_pause_poll && !_impl(_this)->m_force)){

		return -1;
	}
	_impl(_this)->m_force = 0;
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;
	char _msg[300] = "";
	strcpy(_msg, "\r\n============================================\r\n"
            		 "======        TRANG THAI CHO        ========\r\n"
                     "============================================\r\n"
                     "=  Nhan nut \"1\" de doc thong so dang kiem  =\r\n"
                     "=  Nhan nut \"2\" de ghi thong so dang kiem  =\r\n"
            		 "============================================\r\n");
	uart_module_write(uart_module, _msg);
	return 0;
}

int32_t sm_uart_process(sm_pmu_uart_t* _this, void* _arg){

	(void)_arg;
	if(!_this){

		return -1;
	}
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;
	char data_recv[100] = "";
	if(uart_module_get_data(uart_module,data_recv) < 0){
	    return -1;
	}
	int32_t id = atoi(data_recv);
	switch (_impl(_this)->m_state) {
	case UART_CFG_IDLE:
		if (id > CMD_IDLE_NUMBER || id - 1 < 0)
			return -1;
		msg_idle_state_handler[id - 1](NULL, _this);
		break;
	case UART_CFG_READ:

		if (id == 0) {
			_impl(_this)->m_force = 1;
			_impl(_this)->m_state = UART_CFG_IDLE;
			sm_pmu_uart_polling_msg(_this, _arg);
		}
		break;
	case UART_CFG_WRITE:
		if (id == 0) {
			_impl(_this)->m_force = 1;
			_impl(_this)->m_state = UART_CFG_IDLE;
			sm_pmu_uart_polling_msg(_this, _arg);
		}
		break;
	default:
		break;
	}
	return 0;
}


static void msg_cmd_write(const char* data, void* _arg) {

	(void) data;
	uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
	_impl(_arg)->m_pause_poll = 1;
	char _msg[300] = "";
	_impl(_arg)->m_state = UART_CFG_WRITE;
	strcpy(_msg, "\r\n============================================\r\n"
                     "= Nhan nut \"1\" de ghi toc do toi da        =\r\n"
            		 "= Nhan nut \"2\" de ghi gioi han dong PIN    =\r\n"
                     "= Nhan nut \"3\" de ghi bao ve thap ap       =\r\n"
					 "= Nhan \"0\" de thoat                        =\r\n"
            		 "============================================\r\n");
	uart_module_write(uart_module, _msg);
}
static void msg_cmd_read(const char* data, void* _arg)  {

    (void)data;
    uart_module_t* uart_module = (uart_module_t*)_impl(_arg)->m_base;
    _impl(_arg)->m_pause_poll = 1;
    char _msg[300] = "";
    _impl(_arg)->m_state = UART_CFG_READ;

    float speed = 50.0;
    float current_limit = 49.5;
    float low_voltage = 48.5;

    snprintf(_msg, sizeof(_msg),
             "\r\n============================================\r\n"
                 "= Toc do toi da la    : %0.2f kmh           =\r\n"
                 "= Gioi han dong PIN la: %0.2f A             =\r\n"
                 "= Bao ve thap ap la   : %0.2f V             =\r\n"
                 "= Nhan \"0\" de thoat                        =\r\n"
                 "============================================\r\n",
             speed, current_limit, low_voltage);

    uart_module_write(uart_module, _msg);
}
#endif /* SM_PMU_UART_C_ */
