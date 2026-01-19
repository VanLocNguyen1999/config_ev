/*
 * ioc_io_handle.c
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */
#include "sm_pmu_app.h"
#include "ioc_io_handle.h"
#include "sm_pmu_stm.h"
#include "ev_io_define.h"

typedef int8_t (*sm_ioc_io_event_handle_fn_t)(uint8_t, void*);

static char cmd_msg[MAX_ASCII_MSG_SIZE] = "";
static char data_farm[MAX_ASCII_MSG_SIZE] = "";
static int8_t sm_ioc_io_set_horn(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE_BLINK;
//    if(app->m_ev_io_service->m_ioc_output.m_horn == cmd) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_HORN], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static int8_t sm_ioc_io_set_left_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE_BLINK;
//    if(app->m_ev_io_service->m_ioc_output.m_left_light == cmd) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_LEFT_LIGHT], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static int8_t sm_ioc_io_set_right_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE_BLINK;
//    if(app->m_ev_io_service->m_ioc_output.m_right_light == cmd) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_RIGH_LIGHT], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}

static int8_t sm_ioc_io_set_emergency_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd = _value;
//    if(app->m_ev_io_service->m_ioc_output.m_emergency == cmd) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_EMERGENCY], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}

static int8_t sm_ioc_io_set_high_beam_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;
//    if(app->m_ev_io_service->m_ioc_output.m_high_beam_light == _value) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", _value);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_HIGH_BEAM_LIGHT], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static int8_t sm_ioc_io_set_pos_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;
//    if(app->m_ev_io_service->m_ioc_output.m_pos_light == _value) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", _value);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_POS_BEAM_LIGHT], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static int8_t sm_ioc_io_set_low_beam_light(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;
//    if(app->m_ev_io_service->m_ioc_output.m_low_beam_light == _value) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", _value);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_LOW_BEAM_LIGHT], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}

static int8_t sm_ioc_io_set_lock_mode(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_DEFAULT;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_MC_POWER], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}

static int8_t sm_ioc_io_ctrl_key_mode(uint8_t _value, void* _arg){

    (void)_arg;
    (void)_value;

    return -1;
}

static int8_t sm_ioc_io_find_vehicle_mode(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE_BLINK;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_EMERGENCY], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}

static int8_t sm_ioc_io_set_block_mode(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;
    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_NORMAL : OP_STATE_DEFAULT;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_MC_POWER], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static int8_t sm_ioc_io_set_anti_theft_mode(uint8_t _value, void* _arg){
    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    if(!app) return -1;

    uint8_t mc_power_cmd = 0;
    uint8_t cmd = 0;
    mc_power_cmd =  (_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE;
    cmd          =  (_value == 0) ? OP_STATE_NORMAL : OP_STATE_FORCE_BLINK;
    cmd_msg[0] = '\0';

    data_farm[0] = '\0';
    sprintf(data_farm, "%d", mc_power_cmd);
    if(ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_MC_POWER], data_farm) < 0) return -2;

    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    if(ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_HORN], data_farm) < 0) return -2;

    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    if(ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_EMERGENCY], data_farm) < 0) return -2;

    sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    return 0;
}
static int8_t sm_ioc_io_set_mc_power(uint8_t _value, void* _arg){

	sm_pmu_app_t *app = (sm_pmu_app_t*) _arg;
	uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();

    if(!app) return -1;

	if (pmu_stm == PMU_STM_IDLE || pmu_stm == PMU_STM_RUNNING
			|| pmu_stm == PMU_STM_POWER_LIMIT || pmu_stm == PMU_STM_UPGRADING
			|| (pmu_stm == PMU_STM_STARTING
					&& sm_pmu_is_regis_process(app->m_pmu_regis) >= 0)){

	    uint8_t cmd = 0;
	    cmd =  _value;//(_value == 0) ? OP_STATE_DEFAULT : _value;
	    if(app->m_ev_io_service->m_ioc_output.m_mc_power == cmd) return 1;
	    cmd_msg[0] = '\0';
	    data_farm[0] = '\0';
	    sprintf(data_farm, "%d", cmd);
	    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_MC_POWER], data_farm);
	    if(ret < 0){
	        return -2;
	    }else {

	        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
	    }
	    return 0;
	}

	return -2;
}
static int8_t sm_ioc_io_set_charger(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    uint8_t cmd = 0;
    cmd =  _value;//(_value == 0) ? OP_STATE_DEFAULT : OP_STATE_FORCE;
    if(app->m_ev_io_service->m_ioc_output.m_charger == cmd) return 1;
    cmd_msg[0] = '\0';
    data_farm[0] = '\0';
    sprintf(data_farm, "%d", cmd);
    int8_t ret = (int8_t)ascii_msg_builder_frame(cmd_msg, MAX_ASCII_MSG_SIZE, UART_CMD[CMD_CHARGER], data_farm);
    if(ret < 0){
        return -2;
    }else {

        sm_cmd_push(&app->m_pmu_uart, cmd_msg);
    }
    return 0;
}
static sm_ioc_io_event_handle_fn_t g_ioc_io_event_handle[SM_IOC_IO_EVENT_NUMBER] = {

        sm_ioc_io_set_horn,
        sm_ioc_io_set_left_light,
        sm_ioc_io_set_right_light,
        sm_ioc_io_set_emergency_light,

        sm_ioc_io_set_high_beam_light,
        sm_ioc_io_set_pos_light,
        sm_ioc_io_set_low_beam_light,

        sm_ioc_io_find_vehicle_mode,
        sm_ioc_io_ctrl_key_mode,
        sm_ioc_io_set_lock_mode,
        sm_ioc_io_set_block_mode,
        sm_ioc_io_set_anti_theft_mode,

        sm_ioc_io_set_mc_power,
        sm_ioc_io_set_charger,
};

int8_t sm_ioc_io_set_event( uint8_t _cmd,uint8_t _value, void *_arg) {

    int8_t ret = g_ioc_io_event_handle[_cmd](_value,_arg);
    return ret;
}

void sm_ioc_io_on_charger(uint8_t value, void *_arg) {

	if (sm_ioc_io_set_charger(value, _arg) >= 0) {

		uint8_t event =
				value ? PMU_STM_EVENT_PLUG_CHARGER : PMU_STM_EVENT_UNPLUG_CHARGER;
		sm_stm_set_event(event);
	}
}
void sm_ioc_io_err_charger(int32_t value, void *_arg){

	if (sm_ioc_io_set_charger((uint8_t)value, _arg) >= 0) {

		uint8_t event =
				value ? PMU_STM_EVENT_PLUG_CHARGER : PMU_STM_EVENT_UNPLUG_CHARGER;
		sm_stm_set_event(event);
	}
}

