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
#include "sm_one_wire.h"

#define MAX_DATA_ONE_WRITE_RX		8

typedef struct {
	sm_one_wire_t* m_one_write;

	OneWireRx_Frame_t m_rx_frame;
	uint8_t m_data[MAX_DATA_ONE_WRITE_RX];
	elapsed_timer_t m_timeout;
}sm_pmu_app_t;

void sm_one_write_init     (void);
int32_t sm_pmu_app_process          (void);
extern sm_pmu_app_t* g_pmu_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_APP_H
