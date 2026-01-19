//
// Created by vnbk on 19/09/2024.
//
#include "sm_dev_handle.h"
#include "sm_logger.h"
#include "sm_bpa_app.h"
#include "sm_sv_iot_define.h"
#include "sm_l76x_gps.h"

#define TAG "SM_EV_HANDLE"

void sm_dev_on_bp_connected_event(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    if(!_arg || !_sn){
        return;
    }
    sm_bpa_app_t *app = (sm_bpa_app_t *) _arg;
    sm_host_api_t* iot_service = (sm_host_api_t*)app->m_iot_service;
    if(iot_service && iot_service->m_proc->is_connected(iot_service)){
        const sm_bp_data_t* bp_data = sm_sv_bp_get_data(app->m_bp_service, _id);
        if(bp_data && bp_data->m_sn[0] != '\0'){
            iot_service->m_proc->push_event_to_host(iot_service, (void*)bp_data->m_sn);
        }
    }

    sm_auth_bp_connection_update(app->m_modules.m_auth_bp, _id, MODULE_STATE_CONNECTED);
}

void sm_dev_on_bp_disconnected_event(int32_t _id, const char* _sn, void* _arg){
    if(!_arg || !_sn){
        return;
    }
    sm_bpa_app_t *app = (sm_bpa_app_t *) _arg;
    sm_auth_bp_connection_update(app->m_modules.m_auth_bp, _id, MODULE_STATE_DISCONNECTED);
}

/************************************** GET ev ev_data **********************************/
void* sm_dev_get_manu(void*_arg) {
    sm_bpa_app_t *app = (sm_bpa_app_t *) _arg;
    return &app->m_ev_manu;
}

static char g_bpa_version[16] = {0,};
void* sm_dev_get_module_info(int32_t _type, void* _arg){
    sm_bpa_app_t *app = (sm_bpa_app_t *) _arg;
    if(!app || _type != SM_EV_MODULE_HMI){
        return NULL;
    }
    sm_ev_version_to_string((char*)app->m_sw_version, g_bpa_version);
    return (void*)g_bpa_version;
}

void* sm_dev_get_ev_config(void*_arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    return &app->m_config.m_ev_config;
}

void* sm_dev_get_bp_data(int32_t _id, void*_arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_bp_t* bp_service = (sm_sv_bp_t*)app->m_bp_service;
    if(!bp_service){
        return NULL;
    }
    return (void*)sm_sv_bp_get_data(bp_service, _id);
}

int32_t sm_dev_get_gps_data(void* _gps_coordinate, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!app->m_modules.m_gps){
        return -1;
    }
    if(!app->m_modules.m_gps->proc->data_is_valid(app->m_modules.m_gps)){
        return -1;
    }
    app->m_modules.m_gps->proc->get_coordinate(app->m_modules.m_gps, _gps_coordinate);
    return 0;
}

int32_t sm_dev_get_network_data(int32_t* _rssi, char* _simNb, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;

    if(app->m_driver.m_ec200_driver->rssi >= 90){
        return -1;
    }
    *_rssi = app->m_driver.m_ec200_driver->rssi;
    memcpy(_simNb, &app->m_driver.m_ec200_driver->IMSI, sizeof(app->m_driver.m_ec200_driver->IMSI));

    return 0;
}

int32_t sm_dev_get_optional_data(char* _data, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!app){
        return -1;
    }
    if(g_bpa_version[0] == '\0'){
        sm_ev_version_to_string((char*)app->m_sw_version, g_bpa_version);
    }
    int len = sprintf(_data, "\"%s\": %s", SM_IOT_FW_VERSION,  g_bpa_version);
    return len;
}
