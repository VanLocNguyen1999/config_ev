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
	uint8_t m_none1;
	uint8_t m_soc;
	uint8_t m_vol;
	uint8_t m_temp;
	uint8_t m_none2;
	uint8_t m_cs;
}sm_bp_data_t;
typedef struct {
	uint8_t m_flag;
	uint8_t m_motor_vol;
	uint8_t m_ev_purpose;
	uint8_t m_motor_pole_pairs;
	uint16_t m_motor_power;
	uint16_t m_motor_speed;
	float m_wheel_radius;
	uint8_t m_set_max_speed;
}sm_icm_init_data_t;

typedef struct {
	uint8_t m_set_reverse_mode;
	uint8_t m_set_forward_mode;
	uint8_t m_set_side_stand_mode;
	uint8_t m_set_time_enter_parking;
	uint8_t m_set_anti_thef;
	uint8_t m_side_stand_single;
	uint8_t m_set_break_mode;
	uint8_t m_set_max_speed;
}sm_icm_cycle_data_t;
typedef struct {
	sm_one_wire_t* m_one_write;
	sm_icm_init_data_t m_icm_init;
	sm_icm_cycle_data_t m_icm_cycle;
	uint32_t m_icm_init_counter;
	elapsed_timer_t m_timeout;
}sm_pmu_app_t;

void sm_one_write_init     (void);
int32_t sm_pmu_app_process          (void);
extern sm_pmu_app_t* g_pmu_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_APP_H
