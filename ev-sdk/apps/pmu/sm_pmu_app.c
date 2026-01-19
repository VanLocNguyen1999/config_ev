//
// Created by vnbk on 24/05/2024.
//
#include "sm_logger.h"
#include "sm_pmu_app.h"
#include "sm_sv_pms.h"
#include "sm_pmu_stm.h"
#include "sm_pmu_co.c"
#include "sm_pmu_co_sdo.h"
#include "sm_pmu_bpm_handle.h"
#include "sm_pmu_flash_config.h"
#include "sm_core_sch.h"
#include "sm_ev_mc_module.h"
#include "sm_ev_pmu_module.h"
#include "sm_bsp_pmu.h"
#include "sm_ev_data.h"

#define TAG "SM_PMU_APP"

#define _impl(x) ((sm_pmu_app_t*)x)

#define	ERR_NONE					0
#define	OVER_VOLTAGE_CHARGER		04
#define	ERR_PMU_POWER_LIMIT_1		30
#define	ERR_PMU_POWER_LIMIT_2		31

static uint8_t g_software_version[4] = {0, 0, 2, 0x00};

static sm_pmu_app_t g_pmu_app_default = {
        .m_sch_task = NULL,
        .m_co = NULL,
        .m_co_interface = NULL,
        .m_bp_service = NULL,
        .m_mc_module = NULL,
        .m_charger_service = NULL,
		.m_ev_io_service = NULL,
        .m_pms_service = NULL,
        .m_pmu_bpm = NULL,
        .m_pms_controller = NULL,
        .m_sw_version = g_software_version,
        .m_driver = {
                .m_nodeid_controller = NULL,
        },
		.m_pmu_err = NULL,
};

sm_pmu_app_t* g_pmu_app = NULL;

void sm_co_if_proc(void* _arg){
    if(!_arg){
        return;
    }
    sm_co_if_process(_impl(_arg)->m_co_interface);
}
void sm_bp_service_proc(void *_arg) {
	if (!_arg) {
		return;
	}

	sm_pmu_data_t *pmu_data = sm_pmu_get_data(_impl(_arg)->m_pmu_module);
	int32_t ret = sm_sv_bp_process(_impl(_arg)->m_bp_service);
	if (ret == -1) {

		ev_err_set(_impl(_arg)->m_pmu_err, ERR_PMU_POWER_LIMIT_1);
		pmu_data->m_err_code = ERR_PMU_POWER_LIMIT_1;
		return;
	} else if (ret == -2) {

		ev_err_set(_impl(_arg)->m_pmu_err, ERR_PMU_POWER_LIMIT_2);
		pmu_data->m_err_code = ERR_PMU_POWER_LIMIT_2;
		return;
	}
	ev_err_set(_impl(_arg)->m_pmu_err, ERR_NONE);
	pmu_data->m_err_code = ERR_NONE;
}
void sm_pms_service_proc(void *_arg) {
	if (!_arg) {
		return;
	}
	sm_pmu_data_t *pmu_data = sm_pmu_get_data(_impl(_arg)->m_pmu_module);
	if (sm_sv_pms_process(_impl(_arg)->m_pms_service) < 0) {

		ev_err_set(_impl(_arg)->m_pmu_err, OVER_VOLTAGE_CHARGER);
		pmu_data->m_err_code = OVER_VOLTAGE_CHARGER;
		return;
	}

	ev_err_set(_impl(_arg)->m_pmu_err, ERR_NONE);
	pmu_data->m_err_code = ERR_NONE;
}

static void sm_ev_io_service_proc(void *_arg) {
	if (!_arg) {
		return;
	}

	sm_ev_io_sv_process(_impl(_arg)->m_ev_io_service);
//    if (!elapsed_timer_get_remain (&g_pmu_app->m_mc_module->m_connected_timeout))
//    {
//        sm_module_set_state_connect (&g_pmu_app->m_mc_module, MODULE_STATE_DISCONNECTED);
//    }
}

void sm_pmu_uart_process(void *_arg) {
	if (!_arg) {
		return;
	}

	  if(sm_pmu_uart_polling_msg(_impl(_arg)->m_pmu_uart, _arg) < 0){

		  return;
	  }
	  sm_pmu_uart_cmd_msg(_impl(_arg)->m_pmu_uart, _arg);
}

static void sm_pmu_system_reset(void *_arg){
    if(!_arg && _impl(_arg)->m_system_reboot_request.m_reboot){
        if(!elapsed_timer_get_remain(&_impl(_arg)->m_system_reboot_request.m_time)){
        	sm_pmu_bsp_reboot();
        }
    }
}

static void sm_pmu_regis_service_proc(void *_arg) {
	if (!_arg) {
		return;
	}
	if (sm_pmu_regis_process(_impl(_arg)->m_pmu_regis) == -2) {

		sm_pmu_regis_sync_process(_impl(_arg)->m_pmu_regis);
	}
}

static void sm_pmu_periodic_save_enery(void *_arg){
	if (!_arg) {
		return;
	}
	bool is_store = false;
	sys_energy energy_data = *sm_pmu_storage_get_sys_energy(_impl(_arg)->m_pmu_storage);
	if ((_impl(_arg)->m_est_data->m_elec_energydischarge - energy_data.elec_energyDischarge) > 1000) {
		is_store = true;
	}

	if ((_impl(_arg)->m_est_data->m_elec_energycharge - energy_data.elec_energyCharge) > 1000) {
		is_store = true;
	}

	if(is_store){
		energy_data.elec_energyCharge = _impl(_arg)->m_est_data->m_elec_energycharge ;
		energy_data.elec_energyDischarge = _impl(_arg)->m_est_data->m_elec_energydischarge;

		sm_pmu_store_sys_energy(_impl(_arg)->m_pmu_storage, energy_data);
	}
}

static void sm_pmu_protect_process(void* _arg){
	if (!_arg) {
		return;
	}
	sm_pmu_ev_protect_process(_impl(_arg)->m_pmu_protect);
}

static int32_t sm_pmu_app_storage_init(sm_pmu_app_t *_this) {

	sm_pmu_storage_t *pmu_storage = sm_pmu_storage_create();
	if (!pmu_storage)
		return -1;
	_this->m_pmu_storage = pmu_storage;
	return 0;
}

static int32_t sm_pmu_app_load_config(sm_pmu_app_t* _this){

    sm_pmu_data_t*pmu_data = sm_pmu_get_data(_this->m_pmu_module);
    int32_t ret = 0;

    ret = sm_pmu_storage_load(_this->m_pmu_storage);
    if(ret <0){

    	LOG_ERR(TAG, "Could NOT load pmu storage data,set default value");
    }
   else {

		pmu_config *pmu_config_data = NULL;
		sys_energy *energy_data = NULL;
		pmu_config_data = sm_pmu_storage_get_config(_this->m_pmu_storage);
		if(pmu_config_data){

	        pmu_data->m_block_status        = pmu_config_data->block_st;
	        pmu_data->m_lock_status         = pmu_config_data->lock_st;
	        pmu_data->m_anti_theft_status   = pmu_config_data->anti_st;

	        LOG_INF(TAG, "Load pmu_config data SUCCEED");
		}
		energy_data = sm_pmu_storage_get_sys_energy(_this->m_pmu_storage);
		if (energy_data) {
			sm_energy_data_cpy(_this->m_est_data, energy_data);
			pmu_data->m_energy_in = energy_data->elec_energyCharge;
			pmu_data->m_energy_out = energy_data->elec_energyDischarge;

			LOG_INF(TAG, "Load energy_data SUCCEED");
		}

		LOG_INF(TAG, "Load pmu storage data SUCCEED");
	}
    return 0;
}

static int32_t sm_pmu_app_protect_create(sm_pmu_app_t* _this){


	_this->m_pmu_protect = sm_pmu_ev_protect_create(_this->m_co,_this->m_pmu_storage, _this->m_mc_module,_this);
	if(!_this->m_pmu_protect) {

		return -1;
	}
    return 0;
}

static int32_t sm_pmu_app_regis_create(sm_pmu_app_t* _this){


	_this->m_pmu_regis = sm_pmu_regis_create(_this->m_mc_module,_this->m_pmu_module
												,_this->m_pmu_storage, _this);
	if(!_this->m_pmu_regis) {

		return -1;
	}
    return 0;
}


static int32_t sm_pmu_app_err_sv_create(sm_pmu_app_t* _this){


	_this->m_pmu_err = ev_err_create();
	if(!_this->m_pmu_err) {

		return -1;
	}
    return 0;
}

static int32_t sm_pmu_co_create(sm_pmu_app_t* _this){
    if(!sm_bsp_pmu_get_can_port()){
        LOG_ERR(TAG, "CanBus could NOT created");
        return -1;
    }

    _this->m_co_interface = sm_co_if_create_default(0,
                                                    NULL,
                                                    0,
                                                    sm_bsp_pmu_get_can_port());

    /// Create CanOpen Core
    _this->m_co = sm_co_create(1, true, _this->m_co_interface);
    sm_co_set_self_version(_this->m_co, _this->m_sw_version);

    sm_pmu_co_sdo_setting();

    return 0;
}

static int32_t sm_bpm_service_create(sm_pmu_app_t* _this){
    sm_bp_node_id_controller_t* node_id_controller = sm_pmu_get_node_id_if();
    if(!node_id_controller){
        LOG_ERR(TAG, "Node ID controller could NOT initialized");
        return -1;
    }
    _this->m_driver.m_nodeid_controller = node_id_controller;
    sm_sv_bp_t* bpm = sm_sv_bp_create(SM_BP_NUMBER_DEFAULT, _this->m_co, true, _this->m_driver.m_nodeid_controller);
    if(!bpm){
        return -1;
    }

    _this->m_bp_service = bpm;

    _this->m_pmu_bpm = sm_pmu_bpm_create(bpm, NULL);
    return 0;
}

static int32_t sm_pms_service_create(sm_pmu_app_t* _this){
    if(!_this->m_bp_service){
        LOG_ERR(TAG, "BPM service is NOT initialized");
        return -1;
    }
    sys_energy energy_data = *sm_pmu_storage_get_sys_energy(_this->m_pmu_storage);
    sm_pms_ctl_t* pms_ctl = sm_pms_ctl_create(_this->m_bp_service);
    _this->m_est_data = sm_pms_est_create (_this->m_bp_service, sm_bp_retain_get (), _this->m_mc_module,
                                              _this->m_pmu_module,energy_data);
       if(!pms_ctl){
           LOG_ERR(TAG, "PMS Controller is NOT created");
           return -1;
       }
    _this->m_pms_controller = pms_ctl;
    _this->m_pms_service = sm_sv_pms_create(_this->m_pms_controller,_this->m_bp_service, _this->m_charger_service, _this->m_discharger_service, _this->m_est_data);

    return 0;
}

static int32_t sm_discharger_service_create(sm_pmu_app_t* _this){
    if(!_this->m_bp_service){
           LOG_ERR(TAG, "BPM service is NOT initialized");
           return -1;
       }

       if(!_this->m_pms_controller){
           LOG_ERR(TAG, "PMS controller is NOT initialized");
           return -1;
       }

       sm_sv_discharger_t* discharger = sm_sv_discharger_create(_this->m_pms_controller,
                                                        _this->m_bp_service);
       if(!discharger){
           LOG_ERR(TAG, "Charger Service could NOT created");
           return -1;
       }

       _this->m_discharger_service = discharger;
       sm_sv_pms_reinit(NULL, NULL, _this->m_discharger_service, NULL);
       return 0;
}

static sm_sv_charger_event_cb_fn_t charger_event_cb = {
		.on_charger_is_plugged = sm_ioc_io_on_charger,
		.on_err = sm_ioc_io_err_charger
};
static int32_t sm_charger_service_create(sm_pmu_app_t *_this) {
	if (!_this->m_bp_service) {
		LOG_ERR(TAG, "BPM service is NOT initialized");
		return -1;
	}

	if (!_this->m_pms_controller) {
		LOG_ERR(TAG, "PMS controller is NOT initialized");
		return -1;
	}

	sm_sv_charger_t *charger = sm_sv_charger_create(NULL,
			_this->m_pms_controller,
			NULL, _this->m_bp_service);

	if (!charger) {
		LOG_ERR(TAG, "Charger Service could NOT created");
		return -1;
	}

	_this->m_charger_service = charger;
	sm_sv_pms_reinit(NULL, NULL, NULL, _this->m_charger_service);
	sm_sv_charger_reg_event(charger, &charger_event_cb, _this);
	return 0;
}
static int32_t sm_uart_service_create(sm_pmu_app_t* _this){

    sm_pmu_uart_t * pmu_uart = sm_pmu_uart_create_default();
    if (!pmu_uart)
    {
        return -1;
    }

    _this->m_pmu_uart = pmu_uart;
    return 0;
}

static int32_t sm_ev_io_service_create(sm_pmu_app_t* _this){

	sm_sv_io_t* pmu_io = sm_sv_io_create(sm_pmu_get_io_if());
    if(!pmu_io){
        LOG_ERR(TAG, "Could NOT create PMU IO service. Missing PMU IO interface");
        return -1;
    }
	sm_sv_ev_io_t* ev_io = sm_ev_io_sv_create(pmu_io,_this);
    if(!ev_io){
        LOG_ERR(TAG, "Could NOT create EV IO service. Missing EV IO interface");
        return -1;
    }

    _this->m_ev_io_service = ev_io;
    for(uint8_t index = 0; index < SM_SV_IO_EVENT_NUMBER; index++){
        sm_sv_io_reg_event(_this->m_ev_io_service->m_pmu_io_service, index, sm_pmu_io_changed, _this);
    }
    return 0;
}
static int32_t sm_ev_module_create(sm_pmu_app_t* _this){
    _this->m_mc_module = sm_mc_create(&g_pmu_app_default, _this->m_co);
    _this->m_mc_module->m_proc->reset_data(_this->m_mc_module);
    sm_co_if_reg_recv_callback(_this->m_co_interface, sm_mc_co_received_data, _this->m_mc_module);

    _this->m_pmu_module = sm_pmu_create(&g_pmu_app_default, _this->m_co, NULL, NULL);
    _this->m_pmu_module->m_proc->reset_data(_this->m_pmu_module);

    return 0;
}
#if 0
static void pms_enable_test(void* _arg){
    sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)_arg;

    static uint8_t port = 0;
    static uint8_t enable = 1;

    if(enable){
        sm_sv_pms_disable_bp(pmu_app->m_pms_service, port);
    }else{
        sm_sv_pms_enable_bp(pmu_app->m_pms_service, port);
        port++;
        if(port >= 3){
            port = 0;
        }
    }

    enable = !enable;
}

static void pms_force_test(void* _arg){
    sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)_arg;

    static uint8_t port = 0;
    static uint8_t force = 1;

    if(force){
        LOG_WRN(TAG, "Force discharging BP 0");
        sm_sv_pms_force_discharging_bp(pmu_app->m_pms_service, port, NULL, NULL);
    }else{
        LOG_WRN(TAG, "Release force discharging BP 0");
        sm_sv_pms_release_bp(pmu_app->m_pms_service);
    }

    force = !force;
}
#endif

static int32_t sm_sch_task_create(sm_pmu_app_t* _this){
    _this->m_sch_task = sm_sch_create_default();

//    if(_this->m_co_interface){
//        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_co_if_proc, _this);
//    }

    if(_this->m_bp_service){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_bp_service_proc, _this);
    }

    if(_this->m_pmu_bpm){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_pmu_bpm_handle_process, _this);
    }

    if(_this->m_pms_service){
         sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_pms_service_proc, _this);
     }

    if(_this->m_ev_io_service){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_ev_io_service_proc, _this);
    }

    if(_this->m_pmu_storage){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_pmu_periodic_save_enery, _this);
    }

    if(_this->m_pmu_protect){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_pmu_protect_process, _this);
    }

    if(_this->m_pmu_regis){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_pmu_regis_service_proc, _this);
    }

	if (_this->m_pmu_stm) {
		sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER,
				sm_pmu_app_stm_proc, _this);
	}

	sm_sch_start_task(_this->m_sch_task, 0,SM_SCH_REPEAT_FOREVER, sm_pmu_system_reset, _this);
    return 0;
}

sm_pmu_app_t* sm_pmu_app_create(){
    LOG_INF(TAG, "PMU Application start initializing........");
    sm_pmu_app_t* pmu_app = &g_pmu_app_default;

    g_pmu_app = &g_pmu_app_default;
    sm_pmu_app_storage_init(pmu_app);

    if(sm_pmu_co_create(pmu_app) < 0){
        LOG_ERR(TAG, "CanOpen PMU initialized FAILURE");
        return NULL;
    }
    /// Create EV Service
    if(sm_ev_module_create(pmu_app) < 0){
        LOG_ERR(TAG, "Create EV service FAILURE");
        return NULL;
    }
    if(sm_pmu_app_load_config(pmu_app) < 0){
        LOG_ERR(TAG, "PMU is loaded the configuration FAILURE, please check flash memory again");
        return NULL;
    }

    if(sm_pmu_app_protect_create(pmu_app) < 0){
        LOG_ERR(TAG, "Create ev protect service FAILURE");
        return NULL;
    }

    if(sm_bpm_service_create(pmu_app) < 0){
        LOG_ERR(TAG, "BPM service initialized FAILURE");
        return NULL;
    }

    if(sm_pms_service_create(pmu_app) < 0){
        LOG_ERR(TAG, "PMS service initialized FAILURE");
        return NULL;
    }

    if(sm_charger_service_create(pmu_app) < 0){
        LOG_ERR(TAG, "Charger service initialized FAILURE");
        return NULL;
    }

    if(sm_discharger_service_create(pmu_app) < 0){
        LOG_ERR(TAG, "Charger service initialized FAILURE");
        return NULL;
    }
    if(sm_uart_service_create(pmu_app) < 0){

        LOG_ERR(TAG, "...");
        return NULL;
    }

	if (sm_ev_io_service_create(pmu_app) < 0) {

		LOG_ERR(TAG, "...");
		return NULL;
	}
	if (sm_pmu_app_regis_create(pmu_app) < 0){

        LOG_ERR(TAG, "...");
        return NULL;
    }
	if (sm_pmu_app_err_sv_create(pmu_app) < 0){

        LOG_ERR(TAG, "...");
        return NULL;
    }
	pmu_app->m_pmu_stm = sm_stm_create_default(PMU_STM_NUMBER, PMU_EVENT_NUMBER);
    if(sm_pmu_app_stm_init(pmu_app) < 0){
        LOG_ERR(TAG, "PMU configure state machine FAILURE, please check again");
        return NULL;
    }

    sm_sch_task_create(pmu_app);

    LOG_INF(TAG, "PMU Application start SUCCESS !!!");
    return pmu_app;
}
int32_t sm_pmu_app_init(sm_pmu_app_t* _app){
    if(!_app){
        return -1;
    }
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(_app->m_pmu_module);
    sm_pmu_data_reset(pmu_data);
    return 0;
}

int32_t sm_pmu_app_process(sm_pmu_app_t* _app){
    sm_pmu_app_t* pmu_app = _app;

    if(pmu_app){
        sm_sch_process(pmu_app->m_sch_task);
    }
    return 0;
}

int32_t sm_pmu_assign_process(void *_arg) {

	sm_pmu_app_t *pmu_app = (sm_pmu_app_t*) _arg;
	if (!pmu_app->m_bp_service || !pmu_app->m_pmu_bpm)
		return -1;
	sm_bp_service_proc(pmu_app);
	sm_pmu_bpm_handle_process(pmu_app);
	return 0;
}
