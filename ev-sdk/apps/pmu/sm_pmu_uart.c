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

const char *UART_CMD[CMD_NUMBER] = {
          "POLL",
          "INPUT",
          "OUTPUT",
          "VOLT_CHARGER",
          "HORN",
          "LEFT_LIGHT",
          "RIGH_LIGHT",
          "EMERGENCY",
          "HIGH_BEAM",
          "POS_BEAM",
          "LOW_BEAM",
          "CHARGER",
          "MC_POWER",
          "RESPONSE"
};

typedef struct sm_uart sm_uart_t;
typedef void (*msg_handler_t)(const char* data, void* _arg);
typedef struct {

	uart_module_t* m_base;

    ascii_field_cb_t m_extract_frame_cb;

    sm_fifo_handle_t cmd;
    char 	m_cmd_current[MAX_ASCII_MSG_SIZE];
    int8_t m_cmd_in_process;
}sm_uart ;

static sm_uart g_sm_uart = {
		.m_base = NULL,
		.m_cmd_in_process = 0,
};
static void sm_pmu_uart_extract_frame(const char* type, const char* data, void* _arg);

static void msg_cmd_poll_fn(const char* data, void* _arg);
static void msg_cmd_input_fn(const char* data, void* _arg);
static void msg_cmd_output_fn(const char* data, void* _arg);
static void msg_cmd_volt_charger_fn(const char* data, void* _arg);
static void msg_cmd_left_light_control(const char* data, void* _arg);
static void msg_cmd_righ_light_control(const char* data, void* _arg);
static void msg_cmd_horn_control(const char* data, void* _arg);
static void msg_cmd_emergency_control(const char* data, void* _arg);
static void msg_cmd_high_beam_light_control(const char* data, void* _arg);
static void msg_cmd_pos_beam_light_control(const char* data, void* _arg);
static void msg_cmd_low_beam_light_control(const char* data, void* _arg);
static void msg_cmd_charger_control(const char* data, void* _arg);
static void msg_cmd_mc_power_control(const char* data, void* _arg);
static void msg_cmd_response_fn(const char* data, void* _arg);

msg_handler_t msg_handler_pro[CMD_NUMBER] ={

        msg_cmd_poll_fn,
        msg_cmd_input_fn,
        msg_cmd_output_fn,
		msg_cmd_volt_charger_fn,

        msg_cmd_left_light_control,
        msg_cmd_righ_light_control,
        msg_cmd_horn_control,
        msg_cmd_emergency_control,
        msg_cmd_high_beam_light_control,
        msg_cmd_pos_beam_light_control,
        msg_cmd_low_beam_light_control,

        msg_cmd_charger_control,
        msg_cmd_mc_power_control,

        msg_cmd_response_fn
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
    this->m_extract_frame_cb = sm_pmu_uart_extract_frame;
    memset(g_sm_uart.m_cmd_current, 0, sizeof(g_sm_uart.m_cmd_current));

    sm_hal_uart_t *driver = sm_bsp_pmu_get_uart_port();
    uart_module_init(this->m_base, driver, MAX_ASCII_MSG_SIZE);
    sm_uart_cmd_queue_init(this);

    return this;
}

static int32_t sm_pmu_uart_send_recv_handle(sm_pmu_uart_t* _this, char* _msg ,void* _arg){

	if(!_this){

		return -1;
	}
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;

	sm_uart *this = &g_sm_uart;
	char recv_msg[MAX_ASCII_MSG_SIZE] = "";
	if(uart_module_send_recv(uart_module, _msg, recv_msg, 100) < 0){

		return -1;
	}

	char frame[MAX_ASCII_MSG_SIZE] = "";
	if(ascii_msg_parse_frame(recv_msg, frame, MAX_ASCII_MSG_SIZE) < 0){

		return -2 ;
	}

	if(ascii_msg_iterate_fields(frame, this->m_extract_frame_cb, _arg) < 0){

		return -3;
	}

	return 0;
}

static void sm_pmu_uart_extract_frame(const char *type, const char *data, void* _arg) {

    uint8_t numbs = CMD_NUMBER;
    for (size_t i = 0; i < numbs;i++) {
        if (strcmp(type, UART_CMD[i]) == 0) {
            msg_handler_pro[i](data,_arg);
            return;
        }
    }
}

int32_t sm_pmu_uart_polling_msg(sm_pmu_uart_t* _this, void* _arg){

	if(!_this){

		return -1;
	}
	char polling_msg[MAX_ASCII_MSG_SIZE] = "";
	ascii_msg_builder_frame(polling_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_POLL], "0");

	if(sm_pmu_uart_send_recv_handle(_this, polling_msg, _arg) < 0){

		return -2;
	}
	return 0;
}

int32_t sm_pmu_uart_cmd_msg(sm_pmu_uart_t* _this, void* _arg){

	(void)_arg;
	if(!_this){

		return -1;
	}
	uart_module_t* uart_module = (uart_module_t*)_impl(_this)->m_base;

	if(uart_module_is_busy(uart_module) < 0) return -2;
	char frame[MAX_ASCII_MSG_SIZE] = "";

	if(g_sm_uart.m_cmd_in_process){
		return -3;
	}

	if(!(sm_cmd_pop(&g_sm_uart, frame))){
		return -4;
	}
	g_sm_uart.m_cmd_in_process = 1;
	strcpy(g_sm_uart.m_cmd_current, frame);
	if(sm_pmu_uart_send_recv_handle(_this, frame, _arg) < 0){

		sm_cmd_push(&g_sm_uart, g_sm_uart.m_cmd_current);
		g_sm_uart.m_cmd_in_process = 0;
		return -5;
	}
	return 0;
}


static void msg_cmd_input_fn(const char* data, void* _arg) {

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	ioc_data_input_t input = {0};
	for (int i = 0; i < (uint8_t)strlen(data); i++) {
		input.arr[i] = (uint8_t)(data[i] - '0');
	}
	app->m_ev_io_service->m_ioc_input = input.st;
}
static void msg_cmd_output_fn(const char* data, void* _arg)  {

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	ioc_data_output_t output = {0};
	for (int i = 0; i < (uint8_t)strlen(data); i++) {
		output.arr[i] = (uint8_t)(data[i] - '0');
	}
	app->m_ev_io_service->m_ioc_output = output.st;
}
static void msg_cmd_volt_charger_fn(const char* data, void* _arg)  {

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	int32_t vol = atoi(data);
	sm_sv_charger_set_volt(app->m_charger_service, vol);
}
static void msg_cmd_poll_fn(const char* data, void* _arg)  {

	(void)data;
	(void)_arg;
}

static void msg_cmd_left_light_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_righ_light_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_horn_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_emergency_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_high_beam_light_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_pos_beam_light_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_low_beam_light_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_charger_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_mc_power_control(const char* data, void* _arg) {

    (void)data;
    (void)_arg;
}
static void msg_cmd_response_fn(const char* data, void* _arg) {

    (void)_arg;
    (void)data;
    memset(g_sm_uart.m_cmd_current, 0, sizeof(g_sm_uart.m_cmd_current));
    g_sm_uart.m_cmd_in_process = 0;
}
#endif /* SM_PMU_UART_C_ */
