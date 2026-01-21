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
        .m_co_interface = NULL

};

sm_pmu_app_t* g_pmu_app = NULL;

void sm_co_if_proc(void* _arg){
    if(!_arg){
        return;
    }
    sm_co_if_process(_impl(_arg)->m_co_interface);
}

uint32_t uart_ctr = 0;
void sm_pmu_uart_process(void *_arg) {
	if (!_arg) {
		return;
	}
	uart_ctr++;
	if (uart_ctr > 10000) {
		sm_pmu_uart_polling_msg(_impl(_arg)->m_pmu_uart, _arg);
		uart_ctr = 0;
	}

	sm_uart_process(_impl(_arg)->m_pmu_uart, _arg);
}



static int32_t sm_pmu_app_storage_init(sm_pmu_app_t *_this) {

	sm_pmu_storage_t *pmu_storage = sm_pmu_storage_create();
	if (!pmu_storage)
		return -1;
	return 0;
}

static int32_t sm_pmu_app_load_config(sm_pmu_app_t* _this){


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
    sm_pmu_co_sdo_setting();

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


static int32_t sm_sch_task_create(sm_pmu_app_t* _this){
    _this->m_sch_task = sm_sch_create_default();

//    if(_this->m_co_interface){
//        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_co_if_proc, _this);
//    }

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

    if(sm_pmu_app_load_config(pmu_app) < 0){
        LOG_ERR(TAG, "PMU is loaded the configuration FAILURE, please check flash memory again");
        return NULL;
    }

    if(sm_uart_service_create(pmu_app) < 0){

        LOG_ERR(TAG, "...");
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
    return 0;
}

int32_t sm_pmu_app_process(sm_pmu_app_t* _app){
    sm_pmu_app_t* pmu_app = _app;

    if(pmu_app){
        sm_sch_process(pmu_app->m_sch_task);
    }
    return 0;
}
