//
// Created by vnbk on 24/05/2024.
//

#ifndef EV_SDK_SM_PMU_APP_H
#define EV_SDK_SM_PMU_APP_H

#ifdef __cplusplus
extern "C"{
#endif
#include "sm_bsp_pmu.h"
#include "sm_types.h"
#include "sm_elapsed_timer.h"
#include "one_write_tx.h"

typedef struct {
	OneWireTx_t m_one_wtite;
}sm_pmu_app_t;

void sm_one_write_init     (void);
int32_t sm_pmu_app_process          (void);
extern sm_pmu_app_t* g_pmu_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_APP_H
