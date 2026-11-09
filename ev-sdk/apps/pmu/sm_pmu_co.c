//
// Created by vnbk on 06/02/2025.
//
#include "sm_pmu_co.h"
#include "sm_pmu_app.h"
#include "sm_utils.h"
#include "sm_ev_pmu_module.h"

extern sm_pmu_app_t* g_pmu_app;

void tpdo1_build_data_impl(uint8_t *_buffer) {

    sm_pmu_app_t *this = (sm_pmu_app_t*)g_pmu_app;
    setUint16(&_buffer[6], (uint16_t)this->speed_rpm);
}

void tpdo2_build_data_impl(uint8_t *_buffer) {
	sm_pmu_app_t *this = (sm_pmu_app_t*)g_pmu_app;
    setUint32(&_buffer[4], this->hall_position_counter);
}

void tpdo3_build_data_impl(uint8_t *_buffer) {
	(void)_buffer;
//    setUint32(&_buffer[4], 0);
}

void tpdo4_build_data_impl(uint8_t *_buffer) {
	(void)_buffer;
}
