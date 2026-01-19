//
// Created by vnbk on 10/12/2024.
//
#include "sm_auth_bp.h"
#include "sm_logger.h"

#define TAG "sm_auth_bp"

#define _impl(x)    ((sm_auth_bp_impl_t*)(x))

#define RETRY_AUTH_TIME_MS     (20000) //ms

typedef struct{
    const char* m_assigned_id;
    const sm_auth_bp_config_t* m_config;
    sm_sv_bp_t* m_bp_service;
    sm_sv_ev_t* m_ev_service;

    uint8_t m_bp_offline[SM_BP_NUMBER_DEFAULT];
    uint32_t m_bp_offline_rescan_time[SM_BP_NUMBER_DEFAULT];

    uint8_t m_bp_online[SM_BP_NUMBER_DEFAULT];

    sm_mutex m_lock;
}sm_auth_bp_impl_t;

static sm_auth_bp_impl_t g_auth_bp = {
        .m_assigned_id = NULL,
        .m_config = NULL,
        .m_bp_service = NULL,
        .m_ev_service = NULL,
        .m_bp_offline = {
                SM_AUTH_BP_ACCEPTED,
                SM_AUTH_BP_ACCEPTED,
                SM_AUTH_BP_ACCEPTED
        },
        .m_bp_offline_rescan_time = {0, 0, 0},
        .m_bp_online = {
                SM_AUTH_BP_ACCEPTED,
                SM_AUTH_BP_ACCEPTED,
                SM_AUTH_BP_ACCEPTED
        }
};

sm_auth_bp_t* sm_auth_bp_create(const char* _assigned_id,
                                const sm_auth_bp_config_t* _config,
                                sm_sv_bp_t* _bp_service,
                                sm_sv_ev_t* _ev_service){
    if(!_assigned_id || !_bp_service || !_ev_service){
        return NULL;
    }
    g_auth_bp.m_assigned_id = _assigned_id;
    g_auth_bp.m_bp_service = _bp_service;
    g_auth_bp.m_config = _config;
    g_auth_bp.m_ev_service = _ev_service;

    MUTEX_INIT(g_auth_bp.m_lock);

    return &g_auth_bp;
}

int32_t sm_auth_bp_destroy(sm_auth_bp_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_config = NULL;
    _impl(_this)->m_bp_service = NULL;
    _impl(_this)->m_ev_service = NULL;
    _impl(_this)->m_assigned_id = NULL;

    return 0;
}

int32_t sm_auth_bp_connection_update(sm_auth_bp_t* _this, uint8_t _port, uint8_t _status){
    if(!_this){
        return -1;
    }
     if(_status == MODULE_STATE_DISCONNECTED) {
         _impl(_this)->m_bp_offline[_port] = SM_AUTH_BP_ACCEPTED;
         _impl(_this)->m_bp_offline_rescan_time[_port] = 0;

         ENTER_CRITICAL(_impl(_this)->m_lock);
         _impl(_this)->m_bp_online[_port] = SM_AUTH_BP_ACCEPTED;
         EXIT_CRITICAL(_impl(_this)->m_lock);
     }else{
         _impl(_this)->m_bp_offline[_port] = SM_AUTH_BP_REJECTED;
         _impl(_this)->m_bp_offline_rescan_time[_port] = 0;
     }
    return 0;
}

int32_t sm_auth_bp_update_from_cloud(sm_auth_bp_t* _this, const char* _bp_sn, uint8_t _is_rejected){
    if(!_this || !_bp_sn){
        return -1;
    }
    sm_auth_bp_impl_t* auth_bp = _impl(_this);

    if(auth_bp->m_config->m_auth_bp_online == SM_AUTH_BP_CONFIG_DISABLE){
        return 0;
    }

    sm_sv_bp_t* bpm = auth_bp->m_bp_service;
    const sm_bp_data_t* bp_data = NULL;
    int32_t bp_id = -1;

    for(int index = 0; index < sm_sv_bp_get_number(bpm); index++){
        if(sm_sv_bp_is_connected(bpm, index)){
            bp_data = sm_sv_bp_get_data(bpm, index);
            if(strstr(_bp_sn, bp_data->m_sn) != NULL){
                bp_id = index;
                break;
            }
        }
    }

    if(bp_id >= 0){

        ENTER_CRITICAL(auth_bp->m_lock);
        auth_bp->m_bp_online[bp_id] = _is_rejected;
        EXIT_CRITICAL(auth_bp->m_lock);

        if(auth_bp->m_config->m_auth_bp_online == SM_AUTH_BP_CONFIG_WARNING){
            return 0;
        }

        if(_is_rejected == SM_AUTH_BP_REJECTED){
            sm_sv_ev_set_motor_active_condition(auth_bp->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_BP_ONLINE_INVALID);
        }else{
            sm_sv_ev_reset_motor_active_condition(auth_bp->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_BP_ONLINE_INVALID);
        }
        return 0;
    }

    return -1;
}

int32_t sm_auth_bp_get_status_local(sm_auth_bp_t* _this, uint8_t _port){
    if(!_this){
        return SM_AUTH_BP_INVALID;
    }

    if(_impl(_this)->m_config->m_auth_bp_offline == SM_AUTH_BP_CONFIG_DISABLE){
        return SM_AUTH_BP_VALID;
    }

    if(_impl(_this)->m_bp_offline[_port] == SM_AUTH_BP_ACCEPTED){
        return SM_AUTH_BP_VALID;
    }else{
        if(_impl(_this)->m_config->m_auth_bp_offline == SM_AUTH_BP_CONFIG_WARNING){
            return SM_AUTH_BP_INVALID_WARNING;
        }else{
            return SM_AUTH_BP_INVALID;
        }
    }
}

int32_t sm_auth_bp_get_status_cloud(sm_auth_bp_t* _this, uint8_t _port){
    if(!_this){
        return SM_AUTH_BP_INVALID;
    }

    if(_impl(_this)->m_config->m_auth_bp_online == SM_AUTH_BP_CONFIG_DISABLE){
        return SM_AUTH_BP_VALID;
    }

    if(_impl(_this)->m_bp_online[_port] == SM_AUTH_BP_ACCEPTED){
        return SM_AUTH_BP_VALID;
    }else{
        if(_impl(_this)->m_config->m_auth_bp_online == SM_AUTH_BP_CONFIG_WARNING){
            return SM_AUTH_BP_INVALID_WARNING;
        }else{
            return SM_AUTH_BP_INVALID;
        }
    }
}

void sm_auth_bp_process(sm_auth_bp_t* _this){
    sm_auth_bp_impl_t *auth_bp = _impl(_this);
    const sm_bp_data_t *bp_data = NULL;
    uint8_t index = 0;

    if (auth_bp->m_config->m_auth_bp_offline != SM_AUTH_BP_CONFIG_DISABLE) {
        for (index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++) {
            if (auth_bp->m_bp_offline[index] == SM_AUTH_BP_ACCEPTED) {
                continue;
            }

            auth_bp->m_bp_offline_rescan_time[index] += auth_bp->m_config->m_scan_time;
            if (auth_bp->m_bp_offline_rescan_time[index] >= RETRY_AUTH_TIME_MS) {
                sm_sv_bp_force_get_assigned_dev(auth_bp->m_bp_service, index);
                auth_bp->m_bp_offline_rescan_time[index] = 0;
            }

            bp_data = sm_sv_bp_get_data(auth_bp->m_bp_service, index);
            if (bp_data->m_assignedSn[0] == '\0') {
                continue;
            }

            if (memcmp(bp_data->m_assignedSn, auth_bp->m_assigned_id, strlen(auth_bp->m_assigned_id)) != 0) {
                if (auth_bp->m_config->m_auth_bp_offline == SM_AUTH_BP_CONFIG_FORCE) {
                    sm_sv_ev_set_motor_active_condition(auth_bp->m_ev_service,
                                                        SM_MOTOR_ACTIVE_CONDITION_BP_OFFLINE_INVALID);
                }
                auth_bp->m_bp_offline[index] = SM_AUTH_BP_REJECTED;
            } else {
                auth_bp->m_bp_offline[index] = SM_AUTH_BP_ACCEPTED;
                auth_bp->m_bp_offline_rescan_time[index] = 0;
            }
        }
    }

    if(auth_bp->m_config->m_auth_bp_offline == SM_AUTH_BP_CONFIG_FORCE &&
       auth_bp->m_bp_offline[0] == SM_AUTH_BP_ACCEPTED &&
       auth_bp->m_bp_offline[1] == SM_AUTH_BP_ACCEPTED &&
       auth_bp->m_bp_offline[2] == SM_AUTH_BP_ACCEPTED){
        sm_sv_ev_reset_motor_active_condition(auth_bp->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_BP_OFFLINE_INVALID);
    }

    if(auth_bp->m_config->m_auth_bp_online == SM_AUTH_BP_CONFIG_FORCE &&
       auth_bp->m_bp_online[0] == SM_AUTH_BP_ACCEPTED &&
       auth_bp->m_bp_online[1] == SM_AUTH_BP_ACCEPTED &&
       auth_bp->m_bp_online[2] == SM_AUTH_BP_ACCEPTED){
        sm_sv_ev_reset_motor_active_condition(auth_bp->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_BP_ONLINE_INVALID);
    }
}