//
// Created by vnbk on 06/02/2025.
//

#ifndef EV_SDK_SM_PMU_BPM_HANDLE_H
#define EV_SDK_SM_PMU_BPM_HANDLE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"
#include "sm_sv_bp.h"

sm_bp_node_id_controller_t* sm_pmu_get_node_id_if();

typedef struct{
    uint8_t m_enable;
}sm_pmu_bpm_config_t;

typedef void sm_pmu_bpm_t;

sm_pmu_bpm_t* sm_pmu_bpm_create(sm_sv_bp_t* _bp_service, sm_pmu_bpm_config_t* _config);

void sm_pmu_bpm_handle_process(void *_arg);
void sm_pmu_bpm_first_bp_auth(void *_arg);
void sm_pmu_bpm_handle_on_re_config_node_id(int32_t _id, SM_BP_CMD _cmd, int32_t _success, void *_data, void *_arg);
void sm_pmu_bpm_handle_on_connected(int32_t _id, const char *_sn, int32_t _soc, void *_arg);
void sm_pmu_bpm_on_disconnected(int32_t _id, const char *_sn, void *_arg);
void sm_pmu_bpm_on_update_data(int32_t _id, const sm_bp_data_t *_bp_data, void *_arg);
void sm_pmu_bpm_handle_assign_fail(int32_t _id ,void *_arg);

void sm_pmu_bpm_auth_handle(int32_t _id, SM_BP_AUTH_EVENT _event, const char *_sn, int32_t _soc, void *_arg);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_BPM_HANDLE_H
