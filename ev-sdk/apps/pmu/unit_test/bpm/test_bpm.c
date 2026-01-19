/*
 * test_bpm.c
 *
 *  Created on: Jan 21, 2025
 *      Author: admin
 */
#include "test_bpm.h"
#include "sm_bsp_pmu.h"
#include "sm_core_sch.h"

#include "common_utils.h"

#include "sm_shell.h"

static sm_sv_bp_t* 		g_sv_bpm;
static sm_co_t*			g_canopen;
static sm_co_if_t*		g_co_if;
static sm_sch_t* 		g_schedule;

static uint8_t 			cmd_data_handle[32];

typedef void (*node_id_ctrl)(uint8_t);
static node_id_ctrl sm_bsp_slaves_node_id_ctrl[3] = {
     sm_bsp_pmu_io_set_node_id1,
     sm_bsp_pmu_io_set_node_id2,
     sm_bsp_pmu_io_set_node_id3
};

static int32_t sm_slave_node_id_select(int32_t _id){
     if (_id < 3)
          sm_bsp_slaves_node_id_ctrl[_id](1);
     return 0;
}

static int32_t sm_slave_node_id_deselect(int32_t _id){
     if (_id < 3)
          sm_bsp_slaves_node_id_ctrl[_id](0);
     return 0;
}

static sm_bp_node_id_controller_t g_nodeid_controller = {
     .sm_bp_node_id_select  		= sm_slave_node_id_select,
     .sm_bp_node_id_deselect 	     = sm_slave_node_id_deselect,
 };

static int32_t canopen_process(){
	sm_co_process(g_canopen, 1);
	return 0;
}

static void pmu_process(void* arg){
	sm_co_if_process(g_co_if);
	sm_sv_bp_process(g_sv_bpm);
}

static void cmd_disable_bp0(void* arg);
static void cmd_disable_bp1(void* arg);
static void cmd_disable_bp2(void* arg);

static void cmd_enable_bp0(void* arg);
static void cmd_enable_bp1(void* arg);
static void cmd_enable_bp2(void* arg);

static void cmd_standby(void*);
static void cmd_discharge(void*);
static void cmd_charge(void*);
static void cmd_only_dis(void*);
static void cmd_read_sn(void*);
static void cmd_read_version(void*);
static void cmd_read_assigned_dev(void*);
static void cmd_off_all_bp(void*);
static void cmd_bp_cb(int32_t id, SM_BP_CMD cmd, int32_t succ, void *data, void *arg);

void test_process(){
	sm_shell_init();

	g_co_if = sm_co_if_create_default(0, NULL, 0, sm_bsp_pmu_get_can_port());
	g_canopen = sm_co_create(1, 1, g_co_if);
	g_sv_bpm = sm_sv_bp_create(3, g_canopen, true, &g_nodeid_controller);

	g_schedule = sm_sch_create_default();

	sm_sch_start_task(g_schedule, 0, SM_SCH_REPEAT_FOREVER, pmu_process, NULL);
	sm_sch_start_task(g_schedule, 50, SM_SCH_REPEAT_FOREVER, sm_shell_cmd_process, NULL);

	sm_bsp_pmu_timer_1_set_callback(canopen_process, NULL);

	sm_shell_add_cmd("-dis bp0", 	"Disable BP 0", 	cmd_disable_bp0, NULL);
	sm_shell_add_cmd("-dis bp1", 	"Disable BP 1", 	cmd_disable_bp1, NULL);
	sm_shell_add_cmd("-dis bp2", 	"Disable BP 2", 	cmd_disable_bp2, NULL);

	sm_shell_add_cmd("-en bp0", 	"Enable BP 0", 		cmd_enable_bp0, NULL);
	sm_shell_add_cmd("-en bp1", 	"Enable BP 1", 		cmd_enable_bp1, NULL);
	sm_shell_add_cmd("-en bp2", 	"Enable BP 2", 		cmd_enable_bp2, NULL);

	sm_shell_add_cmd("-standby", 	"Set BP0 STANDBY", 			cmd_standby, NULL);
	sm_shell_add_cmd("-discharge", 	"Set BP0 DISCHARGING", 		cmd_discharge, NULL);
	sm_shell_add_cmd("-charge", 	"Set BP0 CHARGING", 		cmd_charge, NULL);
	sm_shell_add_cmd("-only dis", 	"Set BP0 ONLY DISCHARGING", cmd_only_dis, NULL);

	sm_shell_add_cmd("-read sn", 	"Read SN BP0", 			cmd_read_sn, NULL);
	sm_shell_add_cmd("-read ver", 	"Read version BP0", 	cmd_read_version, NULL);
	sm_shell_add_cmd("-read dev", 	"Read assign dev BP0", 	cmd_read_assigned_dev, NULL);
	sm_shell_add_cmd("-off all", 	"Off all BP", 			cmd_off_all_bp, NULL);

	while(1){
		sm_sch_process(g_schedule);
	}
}

static void cmd_disable_bp0(void* arg){
	sm_sv_bp_disable_bp(g_sv_bpm, 0);
	sm_bsp_pmu_io_set_node_id1(0);
}
static void cmd_disable_bp1(void* arg){
	sm_sv_bp_disable_bp(g_sv_bpm, 1);
	sm_bsp_pmu_io_set_node_id2(0);
}
static void cmd_disable_bp2(void* arg){
	sm_sv_bp_disable_bp(g_sv_bpm, 2);
	sm_bsp_pmu_io_set_node_id3(0);
}
static void cmd_enable_bp0(void* arg){
	sm_sv_bp_enable_bp(g_sv_bpm, 0);
}
static void cmd_enable_bp1(void* arg){
	sm_sv_bp_enable_bp(g_sv_bpm, 1);
}
static void cmd_enable_bp2(void* arg){
	sm_sv_bp_enable_bp(g_sv_bpm, 2);
}

static void cmd_standby(void* arg){
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_STANDBY, NULL, cmd_bp_cb, NULL);
}
static void cmd_discharge(void* arg){
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_DISCHARGE, NULL, cmd_bp_cb, NULL);
}
static void cmd_charge(void* arg){
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_CHARGE, NULL, cmd_bp_cb, NULL);
}

static void cmd_only_dis(void* arg){
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_ONLY_DISCHARGE, NULL, cmd_bp_cb, NULL);
}
static void cmd_read_sn(void* arg){
	memset(cmd_data_handle, 0, 32);
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_READ_SN, cmd_data_handle, cmd_bp_cb, NULL);
}

static void cmd_read_version(void* arg){
	memset(cmd_data_handle, 0, 32);
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_READ_VERSION, cmd_data_handle, cmd_bp_cb, NULL);
}
static void cmd_read_assigned_dev(void* arg){
	memset(cmd_data_handle, 0, 32);
	sm_sv_bp_set_cmd(g_sv_bpm, 0, BP_CMD_READ_ASSIGNED_DEV, cmd_data_handle, cmd_bp_cb, NULL);
}
static void cmd_off_all_bp(void* arg){
	sm_sv_bp_set_off_all(g_sv_bpm);
}

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
