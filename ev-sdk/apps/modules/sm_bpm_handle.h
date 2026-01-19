//
// Created by vnbk on 06/02/2025.
//

#ifndef EV_SDK_SM_BPM_HANDLE_H
#define EV_SDK_SM_BPM_HANDLE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"
#include "sm_sv_bp.h"

typedef struct{
    uint8_t m_enable;
}sm_bpm_config_t;

typedef void sm_bpm_handle_t;

sm_bpm_handle_t* sm_bpm_handle_create(sm_sv_bp_t* _bp_service, sm_bpm_config_t* _config);

int32_t sm_bpm_set_config(sm_bpm_handle_t* _this, uint8_t _port, uint8_t _enable);

void sm_bpm_handle_process(void* _arg);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_BPM_HANDLE_H
