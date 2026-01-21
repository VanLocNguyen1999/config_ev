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

    sm_pmu_uart_t* m_pmu_uart;
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
