/*
 * test_charger.c
 *
 *  Created on: Feb 4, 2025
 *      Author: admin
 */

#include "test_charger.h"
#include "sm_bsp_pmu.h"
#include "sm_core_sch.h"
#include "sm_sv_bp.h"
#include "sm_bp_data.h"
#include "sm_sv_pms.h"
#include "sm_core_co.h"



#include "common_utils.h"
#include "sm_logger.h"
#include "sm_shell.h"

#define SDO_DATA_BUF_SIZE	1024

static sm_sv_bp_t* 		g_sv_bpm;
static sm_sv_pms_t*		g_sv_pms_;
static sm_sv_charger_t*	g_sv_charger_;
static sm_co_t*			g_canopen;
static sm_co_if_t*		g_co_if;
static sm_sch_t* 		g_schedule;

static uint8_t 			cmd_data_handle[SDO_DATA_BUF_SIZE];

typedef void (*node_id_ctrl)(uint8_t);
static node_id_ctrl sm_bsp_slaves_node_id_ctrl[3] = {
     sm_bsp_pmu_io_set_node_id1,
     sm_bsp_pmu_io_set_node_id2,
     sm_bsp_pmu_io_set_node_id3
};

static int32_t sm_slave_node_id_select(int32_t _id){
     if (_id < 3){
          sm_bsp_slaves_node_id_ctrl[_id](1);
     }
     return 0;
}

static int32_t sm_slave_node_id_deselect(int32_t _id){
     if (_id < 3){
         sm_bsp_slaves_node_id_ctrl[_id](0);
     }

     return 0;
}

static sm_bp_node_id_controller_t g_nodeid_controller = {
     .sm_bp_node_id_select  		= sm_slave_node_id_select,
     .sm_bp_node_id_deselect 	     = sm_slave_node_id_deselect,
 };


static void canopen_process(void* arg){
	sm_co_process(g_canopen, 1);
	return 0;
}

static void pmu_process(void* arg){
	sm_co_if_process(g_co_if);
	sm_sv_bp_process(g_sv_bpm);
//	sm_sv_pms_process(g_sv_pms_);
	sm_sv_charger_process(g_sv_charger_);
}

static void cmd_co_write(uint8_t _argc, char* arg[]);
static void cmd_co_read(uint8_t _argc, char* arg[]);

static void cmd_list_bp(uint8_t _argc, char* arg[]);
static void cmd_disable_bp(uint8_t _argc, char* arg[]);
static void cmd_enable_bp(uint8_t _argc, char* arg[]);
static void cmd_bp_state(uint8_t _argc, char* arg[]);
static void cmd_bp_read(uint8_t _argc, char* arg[]);
static void cmd_off_all_bp(uint8_t _argc, char* arg[]);

static void cmd_paused_pms(uint8_t _argc, char* arg[]);
static void cmd_force_switch_bp(uint8_t _argc, char* arg[]);
static void cmd_pms_release(uint8_t _argc, char* arg[]);
static void cmd_pms_dis_bp(uint8_t _argc, char* arg[]);
static void cmd_pms_en_bp(uint8_t _argc, char* arg[]);
static void cmd_pms_get_total_energy(uint8_t _argc, char* arg[]);
static void cmd_pms_get_instant_pw(uint8_t _argc, char* arg[]);

static void cmd_plug_charger(uint8_t _argc, char* arg[]);
static void cmd_paused_charger(uint8_t _argc, char* arg[]);
static void cmd_charger_dis_bp(uint8_t _argc, char* arg[]);
static void cmd_charger_en_bp(uint8_t _argc, char* arg[]);
static void cmd_force_charge_bp(uint8_t _argc, char* arg[]);
static void cmd_charger_release(uint8_t _argc, char* arg[]);

static void co_write_callback(SM_SDO_STATUS_t _st, int32_t tx_code, int32_t rx_code, void* arg){

}
static void co_read_callback(SM_SDO_STATUS_t _st, int32_t tx_code, int32_t rx_code, void* arg){

}

static void on_bp_cnt(int32_t _id, const char * _sn, int32_t _soc, void * _arg){
	LOG_INF(TAG, "BP %d , SN %s, SoC %d Connected", _id, _sn, _soc);
}
static void on_bp_discnt(int32_t _id, const char * _sn, void * _arg){
	LOG_INF(TAG, "BP %d , SN %s Disconnected", _id, _sn);
}
sm_sv_bp_event_cb_t g_bp_cb = {
		.on_bp_connected = on_bp_cnt,
		.on_bp_disconnected = on_bp_discnt
};

static void cmd_bp_cb(int32_t id, SM_BP_CMD cmd, int32_t succ, void *data, void *arg){
	if (succ == SM_BP_CMD_SUCCESS){
		switch (cmd){
		case BP_CMD_STANDBY:
			break;
		case BP_CMD_DISCHARGE:
			break;
		case BP_CMD_CHARGE:
			break;
		case BP_CMD_READ_SN:
			LOG_INF(TAG, "BP's serinumber: %s", (char*)data);
			break;
		case BP_CMD_READ_VERSION:
			LOG_INF(TAG, "BP's version: %s", (char*)data);
			break;
		case BP_CMD_READ_ASSIGNED_DEV:
			LOG_INF(TAG, "BP's assigned device: %s", (char*) data);
			break;
		}
	}
}

static void pms_force_bp_cb(uint8_t _id, uint8_t _err){
	LOG_WRN(TAG, "Force switch to BP %d %s", _id, _err ? "FAIL" : "SUCCESS");
}

static void on_service_paused(void* _sv){
	if (!_sv){
		return;
	}

	if (_sv == g_sv_pms_){
		LOG_INF(TAG, "PMS is paused");
	}else if (_sv == g_sv_charger_){
		LOG_INF(TAG, "CHARGER is paused");
	}
}

static void on_service_resumed(void* _sv){
	if (!_sv){
		return;
	}

	if (_sv == g_sv_pms_){
		LOG_INF(TAG, "PMS is resumed");
	}else if (_sv == g_sv_charger_){
		LOG_INF(TAG, "CHARGER is resumed");
	}
}

static int32_t g_charger_voltage = 68000;
static int32_t test_charger_get_voltage(){
	if (g_charger_voltage == 0){
		return 0;
	}else{
		return sm_bsp_pmu_adc_get_charger_vol();
	}

}
static int32_t test_charger_set_charger_fet(uint8_t _value){
	return 0;
}
sm_sv_charger_if_t g_charger_if = {
		.get_charger_vol_fn_t = test_charger_get_voltage,
		.set_charger_mostfet = test_charger_set_charger_fet,
};

static void on_charger_plugging(int32_t _is_charger_plugged, void* _arg){
	LOG_INF(TAG, "CHARGER IS %s", _is_charger_plugged ? "PLUGGED" : "UNPLUGGED");
}
sm_sv_charger_event_cb_fn_t g_charger_event_cb = {
		.on_charger_is_plugged = on_charger_plugging,
		.on_paused = on_service_paused,
		.on_resumed	= on_service_resumed,
};

sm_sv_charger_prof_t g_charger_profile = {
		.m_max_volt = 70000,
		.m_merge_diff_volt = 2000,
		.m_min_cur = 1,
		.m_min_volt = 30000,
		.m_max_temp = 40,

};

sm_sv_pms_event_cb_fn_t g_pms_event_cb = {
	.on_paused = on_service_paused,
	.on_resumed	= on_service_resumed,
};


void test_process(){
	sm_hal_delay_ms(1000);

	sm_shell_init();

	g_co_if = sm_co_if_create_default(0, NULL, 0, sm_bsp_pmu_get_can_port());

	g_canopen = sm_co_create(1, 1, g_co_if);

	g_sv_bpm = sm_sv_bp_create(3, g_canopen, true, &g_nodeid_controller);
	sm_sv_bp_reg_event(g_sv_bpm, &g_bp_cb, NULL);

	g_sv_pms_ = sm_sv_pms_create(g_sv_bpm);
	sm_sv_pms_reg_event(g_sv_pms_, &g_pms_event_cb, g_sv_pms_);

	g_sv_charger_ = sm_sv_charger_create(&g_charger_if, g_sv_bpm);
	sm_sv_charger_reg_event(g_sv_charger_, &g_charger_event_cb, g_sv_charger_);
	sm_sv_charger_set_profile(g_sv_charger_, &g_charger_profile);




	g_schedule = sm_sch_create_default();

	sm_sch_start_task(g_schedule, 0, SM_SCH_REPEAT_FOREVER, pmu_process, NULL);
	sm_sch_start_task(g_schedule, 50, SM_SCH_REPEAT_FOREVER, sm_shell_cmd_process, NULL);

	sm_shell_add_cmd("-co_w", 			"Send CO SDO write.     Ex: -co_w [rx_id] [index_hex] [sub_id_hex] [data_size] [data]", cmd_co_write, 5);
	sm_shell_add_cmd("-co_r", 			"Send CO SDO read.      Ex: -co_r [rx_id] [index_hex] [sub_id_hex] [data_size]", cmd_co_read, 4);

	sm_shell_add_cmd("-bp_ls",			"Show BP data.          Ex: -bp_list [0, 1, 2, all]", cmd_list_bp, 1);
	sm_shell_add_cmd("-bp_dis", 		"Disable BP.            Ex: -bp_dis [0, 1, 2]", cmd_disable_bp, 1);
	sm_shell_add_cmd("-bp_en", 			"Enable BP.             Ex: -bp_en [0 ,1, 2]", cmd_enable_bp, 1);
	sm_shell_add_cmd("-bp_st", 			"Set BP state.          Ex: -bp_st [0, 1, 2] [stb, disch, charge, only_dis]", cmd_bp_state, 	2);
	sm_shell_add_cmd("-bp_r", 			"Read bp info.          Ex: -bp_r [0, 1, 2] [sn, ver, dev]", cmd_bp_read, 2);
	sm_shell_add_cmd("-bp_off", 		"Off all BP", 	cmd_off_all_bp, 		0);

//	sm_shell_add_cmd("-pms_pause", 		"Pause PMS service.     Ex: -pms_pause [0, 1]", cmd_paused_pms, 1);
//	sm_shell_add_cmd("-pms_dis", 		"PMS disable bp.        Ex: -pms_dis [0, 1, 2]", cmd_pms_dis_bp, 1);
//	sm_shell_add_cmd("-pms_en", 		"PMS enable bp.         Ex: -pms_pause [0, 1, 2]", cmd_pms_en_bp, 1);
//	sm_shell_add_cmd("-pms_force", 		"PMS force .            Ex: -pms_force [0, 1, 2]", cmd_force_switch_bp, 1);
//	sm_shell_add_cmd("-pms_rl", 		"PMS release.           Ex: -pms_rl", cmd_pms_release, 0);

	sm_shell_add_cmd("-charger_plug", 	"Plug/Unplug Charger.   Ex: -charger_plug  [0, 1]", cmd_plug_charger, 1);
	sm_shell_add_cmd("-charger_pause", 	"Pause Charger service. Ex: -charger_pause [0, 1]", cmd_paused_charger, 1);
	sm_shell_add_cmd("-charger_dis", 	"Charger disable bp.    Ex: -charger_dis   [0, 1, 2]", cmd_charger_dis_bp, 1);
	sm_shell_add_cmd("-charger_en", 	"Charger enable bp.     Ex: -charger_pause [0, 1, 2]", cmd_charger_en_bp, 1);
	sm_shell_add_cmd("-charger_force", 	"Charger force.         Ex: -charger_force [0, 1, 2]", cmd_force_charge_bp, 1);
	sm_shell_add_cmd("-charger_rl", 	"Charger release.       Ex: -charger_rl", cmd_charger_release, 0);

	sm_bsp_pmu_timer_1_set_callback(canopen_process, NULL);
//	sm_sv_pms_resume(g_sv_pms_);
	sm_sv_charger_resume(g_sv_charger_);

	while(1){
		sm_sch_process(g_schedule);
	}
}

static void cmd_co_write(uint8_t _argc, char* arg[]){
	long int index, sub_id;
	index = strtol(arg[1], NULL, 16);
	sub_id = strtol(arg[2], NULL, 16);
	int32_t rx_id, data_size;
	rx_id = atoi(arg[0]);
	data_size = atoi(arg[3]);

	memset(cmd_data_handle, 0, SDO_DATA_BUF_SIZE);
	memcpy(cmd_data_handle, arg[4], data_size);
	sm_co_sdo_client_send_now(g_canopen,
							index,
							sub_id,
							rx_id,
							cmd_data_handle,
							data_size,
							2000,
							co_write_callback, NULL);
}

static void cmd_co_read(uint8_t _argc, char* arg[]){

	long int index, sub_id;
	index = strtol(arg[1], NULL, 16);
	sub_id = strtol(arg[2], NULL, 16);
	int32_t rx_id, data_size;
	rx_id = atoi(arg[0]);
	data_size = atoi(arg[3]);
	memset(cmd_data_handle, 0, SDO_DATA_BUF_SIZE);
	sm_co_sdo_client_receive_now(g_canopen,
							index,
							sub_id,
							rx_id,
							cmd_data_handle,
							data_size,
							2000,
							co_read_callback, NULL);
}

static void cmd_list_bp(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	sm_bp_data_t *bp_data = NULL;
	if (!strcmp(arg[0], "all")){
		for (int i = 0; i<3; i++){
			bp_data = sm_sv_bp_get_data(g_sv_bpm, i);
			LOG_INF(TAG, "Bp %d , state %d, sn %s, soc %d, vol %d",
								i, bp_data->m_state, bp_data->m_sn, bp_data->m_soc, bp_data->m_vol);
		}
	}else{
		bp_id = atoi(arg[0]);
		bp_data = sm_sv_bp_get_data(g_sv_bpm, bp_id);
		LOG_INF(TAG, "Bp %d , state %d, sn %s, soc %d, vol %d",
				bp_id, bp_data->m_state, bp_data->m_sn, bp_data->m_soc, bp_data->m_vol);
	}
}

static void cmd_disable_bp(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	bp_id = atoi(arg[0]);
	sm_sv_bp_disable_bp(g_sv_bpm, bp_id);
	sm_sv_pms_disable_bp(g_sv_pms_, bp_id);
	sm_slave_node_id_deselect(bp_id);
}

static void cmd_enable_bp(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	bp_id = atoi(arg[0]);
	sm_sv_bp_enable_bp(g_sv_bpm, bp_id);
	sm_sv_pms_enable_bp(g_sv_pms_, bp_id);
	sm_sv_charger_enable_bp(g_sv_charger_, bp_id);
}

static void cmd_bp_state(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	bp_id = atoi(arg[0]);
	if (!strcmp(arg[1], "stb")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_STANDBY, NULL, cmd_bp_cb, NULL);
	}else if (!strcmp(arg[1], "disch")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_DISCHARGE, NULL, cmd_bp_cb, NULL);
	}else if (!strcmp(arg[1], "charge")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_CHARGE, NULL, cmd_bp_cb, NULL);
	}else if (!strcmp(arg[1], "only_dis")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_ONLY_DISCHARGE, NULL, cmd_bp_cb, NULL);
	}
}

static void cmd_bp_read(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	bp_id = atoi(arg[0]);

	memset(cmd_data_handle, 0, SDO_DATA_BUF_SIZE);

	if (!strcmp(arg[1], "sn")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_READ_SN, cmd_data_handle, cmd_bp_cb, NULL);
	}else if (!strcmp(arg[1], "ver")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_READ_VERSION, cmd_data_handle, cmd_bp_cb, NULL);
	}else if (!strcmp(arg[1], "dev")){
		sm_sv_bp_set_cmd(g_sv_bpm, bp_id, BP_CMD_READ_ASSIGNED_DEV, cmd_data_handle, cmd_bp_cb, NULL);
	}
}

static void cmd_off_all_bp(uint8_t _argc, char* arg[]){
	sm_sv_bp_set_off_all(g_sv_bpm);
}

static void cmd_paused_pms(uint8_t _argc, char* arg[]){
	if (atoi(arg[0])){
		sm_sv_pms_pause(g_sv_pms_);
	}else{
		sm_sv_pms_resume(g_sv_pms_);
	}
}

static void cmd_force_switch_bp(uint8_t _argc, char* arg[]){
	sm_sv_pms_force_discharging_bp(g_sv_pms_, atoi(arg[0]), pms_force_bp_cb, NULL);
}

static void cmd_pms_dis_bp(uint8_t _argc, char* arg[]){
	sm_sv_pms_disable_bp(g_sv_pms_, atoi(arg[0]));
}

static void cmd_pms_en_bp(uint8_t _argc, char* arg[]){
	int32_t bp_id = 0;
	bp_id = atoi(arg[0]);
	sm_sv_pms_enable_bp(g_sv_pms_, bp_id);
}

static void cmd_pms_release(uint8_t _argc, char* arg[]){
	sm_sv_pms_release_bp(g_sv_pms_);
}

static void cmd_pms_get_total_energy(uint8_t _argc, char* arg[]){

}

static void cmd_pms_get_instant_pw(uint8_t _argc, char* arg[]){

}

static void cmd_plug_charger(uint8_t _argc, char* arg[]){
	g_charger_voltage = atoi(arg[0]) ? 68000 : 0;
}

static void cmd_paused_charger(uint8_t _argc, char* arg[]){
	if (atoi(arg[0])){
		sm_sv_charger_pause(g_sv_charger_);
	}else{
		sm_sv_charger_resume(g_sv_charger_);
	}
}

static void cmd_charger_dis_bp(uint8_t _argc, char* arg[]){
	sm_sv_charger_disable_bp(g_sv_charger_, atoi(arg[0]));
}

static void cmd_charger_en_bp(uint8_t _argc, char* arg[]){
	sm_sv_charger_enable_bp(g_sv_charger_, atoi(arg[0]));
}

static void cmd_force_charge_bp(uint8_t _argc, char* arg[]){
	sm_sv_charger_force_bp(g_sv_charger_, atoi(arg[0]), NULL, NULL);
}

static void cmd_charger_release(uint8_t _argc, char* arg[]){
	sm_sv_charger_release_bp(g_sv_charger_);
}
