//
// Created by vnbk on 10/12/2024.
//

#ifndef EV_SDK_SM_AUTH_BP_H
#define EV_SDK_SM_AUTH_BP_H

#include "sm_types.h"

#include "sm_sv_bp.h"
#include "sm_ev_data.h"
#include "sm_ev_service.h"

#define SM_AUTH_BP_SCAN_TIME_DEFAULT    500 //ms

typedef void sm_auth_bp_t;

enum {
    SM_AUTH_BP_REJECTED = 0,
    SM_AUTH_BP_ACCEPTED,
};

enum {
    SM_AUTH_BP_CONFIG_DISABLE = 0,
    SM_AUTH_BP_CONFIG_FORCE = 1,
    SM_AUTH_BP_CONFIG_WARNING = 2,
};

enum {
    SM_AUTH_BP_INVALID = 0,
    SM_AUTH_BP_INVALID_WARNING = 1,
    SM_AUTH_BP_VALID = 2
};

typedef struct {
    uint8_t m_auth_bp_offline;
    uint8_t m_auth_bp_online;

    int32_t m_scan_time;
}sm_auth_bp_config_t;

sm_auth_bp_t* sm_auth_bp_create(const char* _assigned_id,
                                const sm_auth_bp_config_t* _config,
                                sm_sv_bp_t* _bp_service,
                                sm_sv_ev_t* _ev_service);

int32_t sm_auth_bp_destroy(sm_auth_bp_t* _this);

int32_t sm_auth_bp_connection_update(sm_auth_bp_t* _this, uint8_t _port, uint8_t _status);

int32_t sm_auth_bp_update_from_cloud(sm_auth_bp_t* _this, const char* _bp_sn, uint8_t _is_rejected);

int32_t sm_auth_bp_get_status_local(sm_auth_bp_t* _this, uint8_t _port);

int32_t sm_auth_bp_get_status_cloud(sm_auth_bp_t* _this, uint8_t _port);

void sm_auth_bp_process(sm_auth_bp_t* _this);

#endif //EV_SDK_SM_AUTH_BP_H
