/*
 * sm_pmu_co_sdo.c
 *
 *  Created on: 3 Oct 2025
 *      Author: My PC
 */
#include "sm_pmu_app.h"
#include "sm_pmu_co_sdo.h"
#include "sm_logger.h"

#include "sm_co_od_common.h"
#include "sm_ev_manu_storage.h"

#include "sm_ev_mc_module.h"
#include "ev_io_define.h"

#include "bsp_api.h"

#define TAG "SM_BPA_CO_SDO"


/********************************************SDT TOOLS ********************************************************/
//static uint8_t sm_app_store_manu_setting() {
//    LOG_INF(TAG, "Setting up from Selex SDT tool");
//    sm_pmu_app_t *app = g_pmu_app;
//    if (app->m_storage.m_manu_storage->m_proc->store(app->m_storage.m_manu_storage,
//                                                     app->m_ev_manu_cfg_buff + 9) < 0) {
//        LOG_ERR(TAG, "Could NOT store new manufacture info");
//        return CO_EXT_CONFIRM_abort;
//    }
//
//    if (sm_ev_manu_storage_validate(app->m_storage.m_manu_storage, app->m_ev_manu_cfg_buff, 9) < 0) {
//        LOG_ERR(TAG, "Could NOT store new manufacture crc");
//        return CO_EXT_CONFIRM_abort;
//    }

//    return CO_EXT_CONFIRM_success;
//}

//static uint8_t sm_app_load_ev_config_setting() {
//    LOG_INF(TAG, "Read ev config up from Selex SDT tool");
//    return CO_EXT_CONFIRM_success;
//}
//
//static uint8_t sm_app_store_ev_config_setting() {
//    LOG_INF(TAG, "Write ev config up from Selex SDT tool");
//    sm_pmu_app_t *app = g_pmu_app;

//    return CO_EXT_CONFIRM_success;
//}
//
//static uint8_t sm_app_load_ev_net_config_setting(){
//    LOG_INF(TAG, "Read ev net config up from Selex SDT tool");
//    return CO_EXT_CONFIRM_success;
//}
//
//static uint8_t sm_app_load_manu_setting(){
//    LOG_INF(TAG, "Load EV setting from Selex SDT tool");
//    return CO_EXT_CONFIRM_success;
//}
//
//
//
//
//static uint8_t sm_app_validate_manu_setting(){
//    LOG_INF(TAG, "Validate EV setting from Selex SDT tool");
//    sm_bpa_app_t* app = g_bpa_app;
//
//    if(sm_ev_manu_storage_validate(app->m_storage.m_manu_storage, (const uint8_t*)SM_EV_CONFIG_ODO_PASS_DEFAULT, 9) < 0){
//        LOG_ERR(TAG, "Could NOT store new manufacture info");
//        return CO_EXT_CONFIRM_abort;
//    }

//    return CO_EXT_CONFIRM_success;
//}
//static uint8_t sm_ev_require_reboot(){
//    LOG_INF(TAG, "Require hmi reboot");
//    sm_pmu_app_t* app = g_pmu_app;
//
//    app->m_system_reboot_request.m_reboot = true;
//    elapsed_timer_resetz(&app->m_system_reboot_request.m_time, 200);
//
//    return CO_EXT_CONFIRM_success;
//}
static uint8_t shadow_data_0x2002_00 = 0;
static uint8_t fin_vehicle = 0;
static uint8_t sm_pmu_find_vehicle(void){
    LOG_INF(TAG, "FIND_VEHICLE_SUB_INDEX");
//    g_pmu_app->m_pmu_sdo_ctrl.fin_vehicle = shadow_data_0x2002_00;
    if(fin_vehicle == shadow_data_0x2002_00){

    	return CO_EXT_CONFIRM_success;
    }

    uint8_t m_key_state = g_pmu_app->m_ev_io_service->m_key.state;
    uint8_t cmd = (shadow_data_0x2002_00) ? OP_STATE_FORCE_BLINK : OP_STATE_NORMAL;
    uint8_t cmd_12v = (shadow_data_0x2002_00) ? OP_STATE_FORCE : OP_STATE_NORMAL;

	if (m_key_state != ACTIVE) {

		sm_pmu_io_set_event(SM_PMU_IO_EVENT_12V_CHANGED, cmd_12v, g_pmu_app);
	}

	if (sm_ioc_io_set_event(SM_IOC_IO_EVENT_FIND_VEHICLE_MODE_CHANGED, cmd,g_pmu_app) >= 0) {
	    fin_vehicle = shadow_data_0x2002_00;
		return CO_EXT_CONFIRM_success;
	}
    return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2002_01 = 0;
static uint8_t sm_pmu_key_ctrl(void){

    return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2002_02 = 0;
static uint8_t m_horn_ctrl = 0;
static uint8_t sm_pmu_horn_ctrl(void) {
	LOG_INF(TAG, "ctrl ev block mode");

	if (m_horn_ctrl == shadow_data_0x2002_02) {

		return CO_EXT_CONFIRM_success;
	}

	uint8_t m_key_state = g_pmu_app->m_ev_io_service->m_key.state;
	uint8_t cmd = (shadow_data_0x2002_02) ? OP_STATE_FORCE_BLINK : OP_STATE_NORMAL;
	uint8_t cmd_12v = (shadow_data_0x2002_02) ? OP_STATE_FORCE : OP_STATE_NORMAL;

	if (m_key_state != ACTIVE) {

		sm_pmu_io_set_event(SM_PMU_IO_EVENT_12V_CHANGED, cmd_12v, g_pmu_app);
	}
	if (sm_ioc_io_set_event(SM_IOC_IO_EVENT_HORN_CHANGED, cmd, g_pmu_app) >= 0) {

		m_horn_ctrl = shadow_data_0x2002_02;
		return CO_EXT_CONFIRM_success;
	}
	return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2002_03[5] = {};
static uint8_t sm_pmu_ev_block(void){
    LOG_INF(TAG, "ctrl ev block mode");

    pmu_config pmu_config_data = *(sm_pmu_storage_get_config(g_pmu_app->m_pmu_storage));
    if(pmu_config_data.block_st == shadow_data_0x2002_03[0]){
    	memset(shadow_data_0x2002_03,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }

    pmu_config_data.block_st = shadow_data_0x2002_03[0];
//    sm_pmu_ev_set_block_resquet(g_pmu_app->m_pmu_protect, shadow_data_0x2002_03[0]);
    if(sm_pmu_store_config(g_pmu_app->m_pmu_storage, pmu_config_data) >= 0){
    	memset(shadow_data_0x2002_03,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }
    return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2002_04[5] = {};
static uint8_t sm_pmu_ev_lock(void){
    LOG_INF(TAG, "ctrl ev lock mode");
    pmu_config pmu_config_data = *(sm_pmu_storage_get_config(g_pmu_app->m_pmu_storage));
    if(pmu_config_data.lock_st == shadow_data_0x2002_04[0]){

		memset(shadow_data_0x2002_04,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }

    pmu_config_data.lock_st = shadow_data_0x2002_04[0];
//    sm_pmu_ev_set_lock_resquet(g_pmu_app->m_pmu_protect, shadow_data_0x2002_04[0]);
    if(sm_pmu_store_config(g_pmu_app->m_pmu_storage, pmu_config_data) >= 0){
    	memset(shadow_data_0x2002_04,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }
    return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2002_05[5] = {};
static uint8_t sm_pmu_anti_theft_mode(void){
    LOG_INF(TAG, "ctrl anti theft mode");
	pmu_config pmu_config_data = *(sm_pmu_storage_get_config(g_pmu_app->m_pmu_storage));
    if(pmu_config_data.anti_st == shadow_data_0x2002_05[0]){
    	memset(shadow_data_0x2002_05,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }

    pmu_config_data.anti_st = shadow_data_0x2002_05[0];
//    sm_pmu_ev_set_anthi_theft_resquet(g_pmu_app->m_pmu_protect, shadow_data_0x2002_05[0]);
    if(sm_pmu_store_config(g_pmu_app->m_pmu_storage, pmu_config_data) >= 0){
    	memset(shadow_data_0x2002_05,0x00,5);
    	return CO_EXT_CONFIRM_success;
    }
    return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2200_00[5] = {};
static uint8_t mc_time_wakeup = 0;

static uint8_t sm_pmu_req_upgrade_mode(void){
	 LOG_INF(TAG, "upgrade_mode");
	uint8_t data_state = shadow_data_0x2200_00[0];
	sm_mc_data_t*mc_data = sm_mc_get_data(g_pmu_app->m_mc_module);
	if(data_state == 0){

		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,
				OP_STATE_NORMAL, g_pmu_app);
		sm_stm_set_event(PMU_STM_EVENT_ENTER_IDLE);
		sm_co_enable_sync_mode(g_pmu_app->m_co);
		sm_co_set_node_id(g_pmu_app->m_co,PMU_NODE_ID_DEFAULT);
		return CO_EXT_CONFIRM_success;
	}
	/* upgrade PMU*/
	if(data_state == 1){
		if(mc_data->m_speed_rpm > 1){
			return CO_EXT_CONFIRM_abort;
		}
//		selex_bs_app.is_request_upgrade = 1;
		sm_stm_set_event(PMU_STM_EVENT_ENTER_UPGRADING_MODE);
		sm_co_disable_sync_mode(g_pmu_app->m_co);
		return CO_EXT_CONFIRM_success;
	}
	/* upgrade MC*/
	if(data_state == 2){
		if(mc_data->m_speed_rpm > 1){

			uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();
			if(pmu_stm == PMU_STM_SLEEP){
				sm_stm_set_event(PMU_STM_EVENT_ENTER_IDLE);
			}
			return CO_EXT_CONFIRM_abort;
		}
		sm_stm_set_event(PMU_STM_EVENT_ENTER_UPGRADING_MODE);
		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, OP_STATE_NORMAL,g_pmu_app);
		uint32_t mc_power_st =  g_pmu_app->m_ev_io_service->m_ioc_output.m_mc_power;
		if(mc_time_wakeup++ >= 100 && mc_power_st == 0){

			sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, OP_STATE_FORCE,g_pmu_app);
			mc_time_wakeup = 0;
			sm_co_disable_sync_mode(g_pmu_app->m_co);

			return CO_EXT_CONFIRM_success;
		}
		return CO_EXT_CONFIRM_processing;
	}
	/* upgrade BP*/
	if(		data_state == 5||
			data_state == 6||
			data_state == 7){
		if(mc_data->m_speed_rpm > 1){
			return CO_EXT_CONFIRM_abort;
		}
//		bs_app_remove_working_port(&selex_bs_app, data_state - 4);
		sm_stm_set_event(PMU_STM_EVENT_ENTER_UPGRADING_MODE);
		sm_co_disable_sync_mode(g_pmu_app->m_co);
		return CO_EXT_CONFIRM_success;
	}
	/* upgrade HMI*/
	if(data_state == 8){
		return CO_EXT_CONFIRM_success;

	}
	return CO_EXT_CONFIRM_abort;
}

static uint8_t shadow_data_0x2300_01 = 0;
static uint8_t sm_pmu_co_sdo_reboot_request(void) {
	if(shadow_data_0x2300_01 != 1 ) return CO_EXT_CONFIRM_abort;
	shadow_data_0x2300_01 = 0;

	sm_pmu_app_t* app = g_pmu_app;
	app->m_system_reboot_request.m_reboot = true;
	elapsed_timer_resetz(&app->m_system_reboot_request.m_time, 200);
	return CO_EXT_CONFIRM_success;
}

int32_t sm_pmu_co_sdo_setting(){
    LOG_DBG(TAG, "CanOpen SDO BPA setting");
    sm_pmu_app_t* _pmu = g_pmu_app;

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_FIND_VEHICLE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_find_vehicle,
                                &shadow_data_0x2002_00);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_LOCK_VEHICLE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_key_ctrl,
                                &shadow_data_0x2002_01);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_HORN_CTRL_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_horn_ctrl,
                                &shadow_data_0x2002_02);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_EV_BLOCK_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_ev_block,
                                shadow_data_0x2002_03);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_EV_LOCK_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_ev_lock,
                                shadow_data_0x2002_04);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_CTRL_INDEX,
								SDO_PMU_EV_ANTI_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_anti_theft_mode,
                                shadow_data_0x2002_05);

     sm_co_sdo_server_set_handle(_pmu->m_co,
    		 	 	 	 	 	SDO_PMU_REQ_EV_UPGRADE_INDEX,
								SDO_PMU_REQ_EV_UPGRADE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_req_upgrade_mode,
                                shadow_data_0x2200_00);

    sm_co_sdo_server_set_handle(_pmu->m_co,
    							SDO_PMU_REBOOT_INDEX,
								SDO_PMU_FW_REQ_UPDATE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_pmu_co_sdo_reboot_request,
                                &shadow_data_0x2300_01);



    return 0;
}



