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
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(g_pmu_app->m_pmu_module);

    uint8_t w = 0;

    BIT_WRITE(w, SM_PMU_KEY_BIT_IDX,pmu_data->m_key);
    BIT_WRITE(w, SM_PMU_HORN_BIT_IDX,pmu_data->m_horn);
    BIT_WRITE(w, SM_PMU_RIGHT_SIGNAL_BIT_IDX,pmu_data->m_right_signal);
    BIT_WRITE(w, SM_PMU_LEFT_SIGNAL_BIT_IDX,pmu_data->m_left_signal);
    BIT_WRITE(w, SM_PMU_DRIVE_MODE_BIT_IDX,pmu_data->m_drive_mode_signal);
    BIT_WRITE(w, SM_PMU_PARKING_BIT_IDX,pmu_data->m_parking);
    BIT_WRITE(w, SM_PMU_LOCK_PORT0_BIT_IDX,pmu_data->m_port_lock_status[0]);
    BIT_WRITE(w, SM_PMU_BRAKE_BIT_IDX,pmu_data->m_brake);
    _buffer[0] = w;

    w = 0;
    BIT_WRITE(w, SM_PMU_HIGH_BEAM_BIT_IDX,pmu_data->m_high_beam_state);
    BIT_WRITE(w, SM_PMU_LOW_BEAM_BIT_IDX,pmu_data->m_low_beam_state);

    if (sm_stm_get_current_state(this->m_pmu_stm) == PMU_STM_CHARGING) {

        BIT_WRITE(w, SM_PMU_CHARGING_BIT_IDX,1);
    }else {
    	BIT_WRITE(w, SM_PMU_CHARGING_BIT_IDX,0);
    }
    BIT_WRITE(w, SM_PMU_THEFT_PROTECT_BIT_IDX,0);

    if(!pmu_data->m_backward_mode_signal){

        BIT_WRITE(w, SM_PMU_REVERSE_BIT0_IDX,0);
        BIT_WRITE(w, SM_PMU_REVERSE_BIT1_IDX,0);
    }else if(pmu_data->m_backward_mode_signal == 1){

        BIT_WRITE(w, SM_PMU_REVERSE_BIT0_IDX,1);
        BIT_WRITE(w, SM_PMU_REVERSE_BIT1_IDX,0);
	}else {

        BIT_WRITE(w, SM_PMU_REVERSE_BIT0_IDX,0);
        BIT_WRITE(w, SM_PMU_REVERSE_BIT1_IDX,1);
	}
	BIT_WRITE(w, SM_PMU_LOCK_PORT1_BIT_IDX, pmu_data->m_port_lock_status[1]);
	BIT_WRITE(w, SM_PMU_LOCK_PORT2_BIT_IDX, pmu_data->m_port_lock_status[2]);

    _buffer[1] = w;

    setUint16(&_buffer[2], (uint16_t)pmu_data->m_range);
    setUint16(&_buffer[4], (uint16_t)(pmu_data->m_discharge_cur_lim/10));
    setUint16(&_buffer[6], (uint16_t)pmu_data->m_charge_cur_lim);
}

void tpdo2_build_data_impl(uint8_t *_buffer) {
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(g_pmu_app->m_pmu_module);
    setUint32(&_buffer[0], pmu_data->m_energy_in);
    setUint32(&_buffer[4], pmu_data->m_energy_out);
}

void tpdo3_build_data_impl(uint8_t *_buffer) {
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(g_pmu_app->m_pmu_module);
    _buffer[0] = (uint8_t)pmu_data->m_bp_checking_state[0];
    _buffer[1] = (uint8_t)pmu_data->m_bp_checking_state[1];
    _buffer[2] = (uint8_t)pmu_data->m_bp_checking_state[2];
    _buffer[3] = (uint8_t)ev_err_get(g_pmu_app->m_pmu_err);
    _buffer[4] = (uint8_t)pmu_data->m_purpose_state;
//    setUint32(&_buffer[4], 0);
}

void tpdo4_build_data_impl(uint8_t *_buffer) {
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(g_pmu_app->m_pmu_module);

    setUint16(&_buffer[0], (uint16_t)pmu_data->m_abp_voltage);
    _buffer[2] = (uint8_t)pmu_data->m_power_per_km;
    _buffer[3] = (uint8_t)pmu_data->m_block_status;
    _buffer[4] = (uint8_t)pmu_data->m_lock_status;
    _buffer[5] = (uint8_t)pmu_data->m_anti_theft_status;
    _buffer[6] = (uint8_t)pmu_data->m_inactive_mode;
    _buffer[7] = (uint8_t)pmu_data->m_uphill_mode;
}
