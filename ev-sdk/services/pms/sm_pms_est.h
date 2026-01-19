//
// Created by DELL 5425 on 5/25/2024.
//

#ifndef EV_SDK_SM_PMS_EST_H
#define EV_SDK_SM_PMS_EST_H
#include "sm_types.h"
#include "sm_ev_data.h"
#include "sm_sv_bp.h"
#include "sm_bp_retain.h"
#include "sm_ev_module.h"

typedef struct{

	int32_t one_pin;
    int32_t two_pin;
    int32_t three_pin;
}discharge_cur_config_t;

typedef struct {

    sm_ev_module_t*	    mc_module;
    sm_ev_module_t*     pmu_module;
	sm_sv_bp_t*         m_bpm;
	sm_bp_retain_t*     m_bp_reatain;

	uint32_t            m_mechanical_energy; // Wh
	uint32_t            m_elec_energydischarge; // Wh
	uint32_t            m_elec_energycharge; // Wh
	uint32_t		    m_cur_dischar_limit; // mA
	uint16_t		    m_distance;         //kmh
} est_data_t;

est_data_t* sm_pms_est_create(sm_sv_bp_t *_bp, sm_bp_retain_t *_bp_reatain,
        sm_ev_module_t* mc_module, sm_ev_module_t* pmu_module, sys_energy _energy);
int32_t pms_est_process(est_data_t *_this);
static inline int32_t sm_est_data_cpy(est_data_t* _des, const est_data_t* _src){
    if (_src == NULL) return 0;
    _des->m_mechanical_energy       = _src->m_mechanical_energy;
    _des->m_elec_energydischarge    = _src->m_elec_energydischarge;
    _des->m_elec_energycharge       = _src->m_elec_energycharge;
    _des->m_distance                = _src->m_distance;
    _des->m_cur_dischar_limit       = _src->m_cur_dischar_limit;
    return 1;
}
/**
 * @brief sm_pms_reset_data
 * @param _est_data
 * @return
 */
static inline int32_t sm_est_reset_data(est_data_t* _est_data){
    if (!_est_data) return -1;
    _est_data->m_mechanical_energy       = 0;
    _est_data->m_elec_energydischarge    = 0;
    _est_data->m_elec_energycharge       = 0;
    _est_data->m_distance                = 0;
    _est_data->m_cur_dischar_limit       = 0;
    return 0;
}
static inline int32_t sm_energy_data_cpy(est_data_t* _des, sys_energy* energy_data){
    _des->m_elec_energydischarge    = energy_data->elec_energyDischarge;
    _des->m_elec_energycharge       = energy_data->elec_energyCharge;
    return 1;
}
#endif //EV_SDK_SM_PMS_EST_H
