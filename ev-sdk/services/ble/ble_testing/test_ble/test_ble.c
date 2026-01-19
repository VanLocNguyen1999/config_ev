/*
 * test_ble.c
 *
 *  Created on: Sep 27, 2024
 *      Author: admin
 */

#include "test.h"
#include "sm_logger.h"
#include "sm_hal.h"
#include "sm_bsp_hmi.h"
#include "sm_w25qxx.h"
#include "common_utils.h"
#include "lcd_segment.h"
#include "at-modem/sm_modem.h"
#include "sm_sv_ble.h"
#include "sm_mdbt42q_ble.h"
#include "sm_ble_slave.h"
#include "sm_bp_data.h"

#define EV_ID   "RCDE0026"

static const char* TAG = "BLE_TESTING";

extern sm_hal_uart_t *g_ble_uart;
extern sm_hal_io_t* g_ble_rst;
extern sm_hal_io_t* g_ble_ind;
extern sm_hal_io_t* g_ble_wk;

extern sm_hal_i2c_t *g_lcd_i2c;
extern sm_hal_io_t *g_lcd_pow;

sm_modem_t *g_modem;
sm_sv_ble_t *g_sv_ble;
sm_host_api_t *host_api;

sm_mdbt42q_t g_ble_module;
sm_ble_slave_t *g_ble_slave;
sm_sv_ble_t *g_ble_sv;

uint8_t host_api_created;

sm_drv_lcd_seg_t* g_lcd;


static char* g_device_paired[BLE_MAX_DEVICE_PAIRED] = {
//    "RKQ1.211119.001",
    "",
    ""
};

sm_sv_ble_info_t g_ble_info = {
    .m_name = EV_ID,
    .m_device_paired = g_device_paired,
};
sm_host_sync_data_if_t host_api_if;

sm_sv_ble_event_t g_ble_event = {
    .on_device_paired = NULL,
    .on_device_unpaired = NULL,
    .on_new_device_paired = NULL,
    .on_new_ble_id = NULL,
    .on_mac_add_checked = NULL,
};

sm_host_event_t g_host_event = {
    .on_connected = NULL,
    .on_disconnected = NULL,
    .on_cmd_from_host = NULL,
};

char* get_ev_data(int32_t arg);
char* get_ev_bp_data(int32_t arg);

char ev_data[1024];
char* get_ev_data(int32_t arg) {
    LOG_DBG(TAG, "HOST GET EV DATA");
    memset(ev_data, '\0', sizeof(ev_data));
    char* j = ev_data;
    j = json_str(j, "serial_number", 1);
    j = json_double(j, "lat", 21.12313);
    j = json_double(j, "lon", 105.124925);
    j = json_int(j, "speed", 0);
    j = json_int(j, "error", 0);
    j = json_int(j, "odo", 1000);
    j = json_int(j, "bp_num", 3);
    j = json_int(j, "max_speed", 60);
    j = json_int(j, "horn", 0);
    j = json_int(j, "key", 1);
    j = json_int(j, "drive_mode", 0);

    return ev_data;
}

sm_bp_data_t g_bp[3] = {
    {
        .m_sn = "101BE1R001000",.m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
        .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
        .m_version = "v1.0.17",
        .m_assignedSn = EV_ID
    },{
        .m_sn = "101BE1R001001",.m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
    .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
    .m_version = "v1.0.17",
    .m_assignedSn = EV_ID
    },{
        .m_sn = "101BE1R001002",.m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
        .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
        .m_version = "v1.0.17",
        .m_assignedSn = EV_ID
    }
};
char bp_data_buff[1024];
char* get_ev_bp_data(int32_t arg) {
    memset(bp_data_buff, 0, 1024);
    char* p = bp_data_buff;
    sm_bp_data_t* bp_data = &g_bp[arg];
    if(bp_data && bp_data->m_sn[0] != '\0'){
//        p = json_objOpen(p, NULL);
        p = json_str(p, "sn", bp_data->m_sn);
        p = json_int(p, "cur", bp_data->m_cur);
        p = json_int(p, "vol", bp_data->m_vol);
        p = json_int(p, "cycle", bp_data->m_cycle);
        p = json_str(p, "fw_version", bp_data->m_version);
        p = json_int(p, "op_state", bp_data->m_state);
        p = json_int(p, "soc", bp_data->m_soc);
        p = json_int(p, "soh", bp_data->m_soh);
        p = json_int(p, "status", bp_data->m_status);

        p = json_objOpen(p, "assigned");
        p = json_str(p, "sn", bp_data->m_assignedSn);
        p = json_str(p, "type", "ev");
        p = json_objClose(p);

        p = json_arrOpen(p, "temps");
        for(int i = 0; i < 6; i++){
            p = json_int(p, NULL, bp_data->m_temps[i]);
        }
        p = json_arrClose(p);

        p = json_arrOpen(p, "cells_vol");
        for(int i = 0; i < 16; i++){
            p = json_int(p, NULL, bp_data->m_cellVols[i]);
        }
        p = json_arrClose(p);
//        p = json_objClose(p);
    }
    return bp_data_buff;
}

const char* host_cmd_str[] = {
    "SM_HOST_CMD_SET_ODO",
    "SM_HOST_CMD_BP_EV_SN",
    "SM_HOST_CMD_FORGET_HOST",
    "SM_HOST_CMD_EV_DATA",
    "SM_HOST_CMD_BP_DATA",
    "SM_HOST_CMD_FW_VERSION",
    "SM_HOST_CMD_SIM_NB",
    "SM_HOST_CMD_FIND_EV",
    "SM_HOST_CMD_HEAD_LIGHT",
    "SM_HOST_CMD_SET_SIGNAL_LIGHT",
    "SM_HOST_CMD_SET_HORN",
    "SM_HOST_CMD_BLOCK_EV",
    "SM_HOST_CMD_LOCK_EV",
    "SM_HOST_CMD_SET_DRIVE_MODE",
    "SM_HOST_CMD_SET_MAX_SPEED",
    "SM_HOST_CMD_SET_ACTIVE",
    "SM_HOST_CMD_SET_LOCK_PORT",
    "SM_HOST_CMD_SET_UNLOCK_PORT",
    "SM_HOST_CMD_CONFIG_VERIFY_BP_OFFLINE",
};

static void ble_on_dv_paired(void* arg);
static void ble_on_dv_unpaired(void* arg);
static void ble_on_new_dv_paired(const char* uuid, void* arg);
static void ble_on_new_ble_id(uint32_t ble_id, void* arg);
static void ble_on_mac_add_checked(const char* mac_add, uint8_t status, void* arg);

static void host_on_conn(int32_t _success, void* _arg);
static void host_on_disconnect(int32_t _success, void* _arg);
static void host_on_cmd(int32_t _cmd, void* _ret, void* _arg);
void init(){
    sm_bsp_hmi_init();

    g_lcd = sm_drv_lcd_seg_create(g_lcd_i2c, g_lcd_pow);
    sm_drv_lcd_seg_power_on(g_lcd);
    sm_drv_lcd_seg_power_off(g_lcd);

    sm_mdbt42q_init(&g_ble_module, g_ble_uart, g_ble_rst, g_ble_ind, g_ble_wk);
    g_ble_slave = sm_ble_sl_init(&g_ble_module, NULL);

    g_ble_event.on_new_ble_id = ble_on_new_ble_id;
    g_ble_event.on_device_paired = ble_on_dv_paired;
    g_ble_event.on_device_unpaired = ble_on_dv_unpaired;
    g_ble_event.on_new_device_paired = ble_on_new_dv_paired;
    g_ble_event.on_mac_add_checked = ble_on_mac_add_checked;

    memcpy(g_ble_info.m_device_paired, g_device_paired,sizeof(g_device_paired));
    g_ble_sv = sm_sv_ble_create(g_ble_slave, &g_ble_info, &g_ble_event, NULL);
    host_api = (sm_host_api_t*)g_ble_sv;
    host_api->m_proc->init(host_api, &host_api_if, 5);

    g_host_event.on_connected = host_on_conn;
    g_host_event.on_disconnected = host_on_disconnect;
    g_host_event.on_cmd_from_host = host_on_cmd;
    sm_host_api_reg_event_handle(host_api, &g_host_event, host_api);
    host_api_created = 1;
    LOG_DBG(TAG, "READY");
}

int test_ble_main(){
    init();

    while(1){
        if (host_api_created) {
            host_api->m_proc->process(host_api);
//            sm_hal_delay_ms(10);
       }
    }
    return 0;
}


static void ble_on_dv_paired(void* arg) {
    LOG_DBG(TAG, "Device paired");
    sm_sv_ble_send_response_write_cmd( g_ble_sv, "ev_id", 1);
}
static void ble_on_dv_unpaired(void* arg) {
    LOG_DBG(TAG, "Device unpaired");
}
static void ble_on_new_dv_paired(const char* uuid, void* arg) {
    LOG_DBG(TAG, "New Device paired %s", uuid);
    sm_sv_ble_send_response_write_cmd( g_ble_sv, "ble_id", 1);
}
static void ble_on_new_ble_id(uint32_t ble_id, void* arg) {
    LOG_DBG(TAG, "New BLE ID %d", ble_id);
    sm_drv_lcd_seg_set_odo_value(g_lcd, ble_id, 1);
//    sm_sv_ble_send_response_write_cmd( g_ble_sv, "id_maker", (ble_id) ? 1 : 0);
}

static void ble_on_mac_add_checked(const char* mac_add, uint8_t status, void* arg){
    switch (status){
        case 0:
            LOG_DBG(TAG, "Mac add is new");
            break;
        case 1:
            LOG_DBG(TAG, "Mac add is paired");
            break;
        case 2:
            LOG_DBG(TAG, "Mac add is invalid");
            break;
    }
    sm_sv_ble_send_response_write_cmd( g_ble_sv, "ev_id", status);
}

static void host_on_conn(int32_t _success, void* _arg) {
    if (_success){
        LOG_DBG(TAG, "Connected to host");
        sm_drv_lcd_seg_set_ble_icon(g_lcd, 1);
    }
    else
        LOG_ERR(TAG, "Connection to host failed");
}
static void host_on_disconnect(int32_t _success, void* _arg) {
    if (_success){
        LOG_DBG(TAG, "Disconnected from host");
        sm_drv_lcd_seg_set_ble_icon(g_lcd, 0);
        sm_drv_lcd_seg_set_odo_value(g_lcd, 0, 1);
    }
    else
        LOG_ERR(TAG, "Disconnection to host failed");
}
static void host_on_cmd(int32_t _cmd, void* _ret, void* _arg) {
    LOG_INF(TAG, "HOST ON CMD: %s - %d", host_cmd_str[_cmd], *(int32_t*)_ret);
    switch(_cmd){
        case SM_HOST_CMD_SET_ODO :

		break;
        case SM_HOST_CMD_BP_EV_SN :

		break;
        case SM_HOST_CMD_FORGET_HOST :

		break;
        case SM_HOST_CMD_EV_DATA :
            sm_sv_ble_send_response_read_cmd(g_ble_sv, get_ev_data(*(int32_t*)_ret));
		break;
        case SM_HOST_CMD_BP_DATA :{
            sm_sv_ble_send_response_read_cmd(g_ble_sv, get_ev_bp_data(*(int32_t*)_ret));
        }

		break;
        case SM_HOST_CMD_FW_VERSION :

		break;
        case SM_HOST_CMD_SIM_NB :

		break;
        case SM_HOST_CMD_FIND_EV :
            sm_drv_lcd_seg_set_left_signal(g_lcd, 1);
            sm_drv_lcd_seg_set_right_signal(g_lcd, 1);
            sm_bsp_hmi_blink_signal_light();
            sm_drv_lcd_seg_set_left_signal(g_lcd, 0);
            sm_drv_lcd_seg_set_right_signal(g_lcd, 0);
		break;
        case SM_HOST_CMD_HEAD_LIGHT :
            if (!strcmp((char*)_ret, "1")){
                sm_drv_lcd_seg_set_phase_light(g_lcd, 1);
            }else{
                sm_drv_lcd_seg_set_phase_light(g_lcd, 0);
            }
		break;
        case SM_HOST_CMD_SET_SIGNAL_LIGHT :
            sm_drv_lcd_seg_set_left_signal(g_lcd, 1);
            sm_drv_lcd_seg_set_right_signal(g_lcd, 1);
            sm_bsp_hmi_blink_signal_light();
            sm_drv_lcd_seg_set_left_signal(g_lcd, 0);
            sm_drv_lcd_seg_set_right_signal(g_lcd, 0);
		break;
        case SM_HOST_CMD_BLOCK_EV :

		break;
        case SM_HOST_CMD_LOCK_EV :

		break;
        case SM_HOST_CMD_SET_DRIVE_MODE :
            if (!strcmp((char*)_ret, "1")) {
                sm_drv_lcd_seg_set_eco_mode(g_lcd, 0);
                sm_drv_lcd_seg_set_eco_sport(g_lcd, 1);
            }else {
                sm_drv_lcd_seg_set_eco_mode(g_lcd, 1);
                sm_drv_lcd_seg_set_eco_sport(g_lcd, 0);
            }

		break;
        case SM_HOST_CMD_SET_MAX_SPEED :
			sm_drv_lcd_seg_set_speed(g_lcd, atoi((char*)_ret), 1);
		break;
        case SM_HOST_CMD_SET_ACTIVE :

		break;
        case SM_HOST_CMD_SET_LOCK_PORT :

		break;
        case SM_HOST_CMD_SET_UNLOCK_PORT :

		break;
        case SM_HOST_CMD_CONFIG_VERIFY_BP_OFFLINE :

		break;
    }
    // global_test_flag.m_recv_cmd.m_value = _cmd;
}
