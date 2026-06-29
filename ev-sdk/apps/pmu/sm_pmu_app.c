//
// Created by vnbk on 24/05/2024.
//

#include "sm_pmu_app.h"
#include "hal_data.h"

#define TAG "SM_PMU_APP"

#define _impl(x) ((sm_pmu_app_t*)x)

#define	ERR_NONE					0
#define	OVER_VOLTAGE_CHARGER		04
#define	ERR_PMU_POWER_LIMIT_1		30
#define	ERR_PMU_POWER_LIMIT_2		31

#define	TIME_5_MIU		3*60*1000



sm_pmu_app_t* g_pmu_app = NULL;

static sm_pmu_app_t g_pmu_app_default;
//void sm_led_init(){
//	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
//	g_pmu_app = &g_pmu_app_default;
//	led_reset();
//}



void sm_one_write_init(){
	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	sm_hal_io_t* port = sm_bsp_get_one_write_tx();
	OneWireTx_Init(&pmu_app->m_one_wtite, port);
	g_pmu_app = &g_pmu_app_default;
}

int32_t sm_pmu_app_process(void){

	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	uint8_t data = 30;
	return pmu_app->m_one_wtite.send(&pmu_app->m_one_wtite,&data,1);
}
