//
// Created by vnbk on 24/05/2024.
//

#ifndef EV_SDK_SM_PMU_APP_H
#define EV_SDK_SM_PMU_APP_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_sv_bp.h"
#include "sm_sv_charger.h"
#include "sm_sv_io.h"
#include "sm_sv_pms.h"

#include "sm_stm.h"
#include "sm_core_sch.h"
#include "sm_core_co.h"

#include "sm_ev_module.h"

#include "sm_pmu_stm.h"
#include "sm_pmu_bpm_handle.h"
#include "ev_io_handle.h"

#include "sm_pmu_uart.h"
#include "sm_ev_storage.h"
#include "sm_pmu_storage.h"
#include "sm_pmu_ev_protect.h"
#include "sm_pmu_regis_handle.h"

#include "ev_error_handle.h"


typedef struct {
    sm_sch_t* m_sch_task;

    sm_co_if_t* m_co_interface;
    sm_co_t* m_co;

    sm_sv_bp_t* m_bp_service;
    sm_ev_module_t* m_mc_module;
    sm_ev_module_t* m_pmu_module;
    sm_sv_charger_t* m_charger_service;
    sm_sv_discharger_t* m_discharger_service;
    sm_sv_pms_t* m_pms_service;


    sm_pmu_uart_t* m_pmu_uart;


    sm_stm_t* m_pmu_stm;
    struct {
        sm_bp_node_id_controller_t* m_nodeid_controller;
    }m_driver;

    sm_pmu_bpm_t* m_pmu_bpm;
    sm_pms_ctl_t* m_pms_controller;
    est_data_t*   m_est_data;

    sm_sv_ev_io_t* m_ev_io_service;

    sm_ev_manu_t m_ev_manu;
    uint8_t m_ev_manu_cfg_buff[128 + 9];
    uint8_t* m_sw_version;

    elapsed_timer_t stm_timeout;
    struct {
        bool m_reboot;
        elapsed_timer_t m_time;
    }m_system_reboot_request;

    sm_pmu_storage_t* m_pmu_storage;
    sm_pmu_ev_protect_t* m_pmu_protect;
    sm_pmu_regis_t* m_pmu_regis;

    ev_error_t*	m_pmu_err;

}sm_pmu_app_t;

sm_pmu_app_t* sm_pmu_app_create     ();

int32_t sm_pmu_app_init             (sm_pmu_app_t* _app);

int32_t sm_pmu_app_process          (sm_pmu_app_t* _app);
void sm_co_if_proc                  (void* _arg);
void sm_bp_service_proc             (void* _arg);
void sm_pms_service_proc     		(void* _arg);
void sm_pmu_uart_process			(void *_arg);
int32_t sm_pmu_assign_process		(void* _arg);
extern sm_pmu_app_t* g_pmu_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_APP_H
