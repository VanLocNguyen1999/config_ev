//
// Created by vnbk on 11/09/2024.
//
#include <string.h>
#include "../../../apps/hmi/sm_hmi_storage.h"
#include "sm_linux_file.h"
#include "json-maker/json-maker.h"
#include "tiny-json/tiny-json.h"
#include "sm_logger.h"

#define TAG "SM_HMI_STORAGE"

sm_sv_iot_config_t* sm_iot_load_config_iot(){
    static sm_sv_iot_config_t iot_config = {
            .m_host = SM_IOT_MQTT_HOST_DEFAULT,
            .m_port = SM_IOT_MQTT_PORT_DEFAULT,
            .m_username = SM_IOT_MQTT_USERNAME_DEFAULT,
            .m_password = SM_IOT_MQTT_PASSWORD_DEFAULT,
            .m_client_id = SM_IOT_CLIENT_ID_DEFAULT,
            .m_sync_time = SM_IOT_MQTT_SYNC_TIME_DEFAULT,
            .m_tls = 0
    };
    return &iot_config;
}
int32_t sm_iot_store_config(const sm_sv_iot_config_t* _iot_config){
    return 0;
}

int32_t sm_lte_load_config(char* _apn, char* _user, char* _pass){
    strcpy(_apn, SM_LTE_APN_DEFAULT);
    strcpy(_user, SM_LTE_USERNAME_DEFAULT);
    strcpy(_pass, SM_LTE_PASSWORD_DEFAULT);
    return 0;
}

int32_t sm_lte_store_config(const char* _apn, const char* _user, const char* _pass){
    return 0;
}

sm_module_info_t* sm_ev_load_manu_info(){
    static sm_module_info_t ev_info;
    memcpy(ev_info.m_ev_sn, SM_IOT_CLIENT_ID_DEFAULT, sizeof(ev_info.m_ev_sn));
    memcpy(ev_info.m_sn, SM_IOT_CLIENT_ID_DEFAULT, sizeof(ev_info.m_sn));
    return &ev_info;
}

int32_t sm_ev_store_manu_info(const sm_ev_manu_t* _ev_info){
    return 0;
}

//// EV operating data
int32_t sm_ev_store_opt_data(const sm_ev_opt_t* _ev_opt, void* _arg);
int32_t sm_ev_load_opt_data(sm_ev_opt_t* _ev_opt, void* _arg){
    sm_linux_file_t* file = (sm_linux_file_t*)_arg;
    char content[128];
    int len = sm_linux_file_read(file, content);

    if(len <= 0){
        sm_ev_opt_t opt_default = {
                .m_odo = 0,
                .m_max_speed = 60
        };
        sm_ev_store_opt_data(&opt_default, _arg);
        return 0;
    }

    json_t mem[10];
    const json_t* json = json_create((char*)content, mem, ARRAY_SIZE(mem));
    if(!json) {
        LOG_ERR(TAG, "CMD message FAILURE");
    }
    const json_t* odo = json_getProperty(json, "odo");
    if(!odo || JSON_INTEGER != json_getType(odo)){
        LOG_ERR(TAG, "Odo param INVALID");
        return -1;
    }
    _ev_opt->m_odo = json_getInteger(odo);

    const json_t* max_speed = json_getProperty(json, "max_speed");
    if(!max_speed || JSON_INTEGER != json_getType(max_speed)){
        LOG_ERR(TAG, "max_speed param INVALID");
        return -1;
    }
    _ev_opt->m_max_speed = json_getInteger(max_speed);

    return 0;
}
int32_t sm_ev_store_opt_data(const sm_ev_opt_t* _ev_opt, void* _arg){
    char buf[64];
    char* p = buf;

    p = json_objOpen(p, NULL);
    p = json_int(p, "odo", _ev_opt->m_odo);
    p = json_int(p, "max_speed", _ev_opt->m_max_speed);
    p = json_objClose(p);
    p = json_end(p);

    sm_linux_file_t* file = (sm_linux_file_t*)_arg;
    sm_linux_file_write(file, buf, (int32_t)strlen(buf));

    return 0;
}
int32_t sm_ev_store_odo(uint32_t _odo, void* _arg){
    sm_ev_opt_t ev_opt;
    sm_ev_load_opt_data(&ev_opt, _arg);

    ev_opt.m_odo = _odo;

    return sm_ev_store_opt_data(&ev_opt, _arg);
}
int32_t sm_ev_load_max_speed(){
    return 0;
}
int32_t sm_ev_store_max_speed(uint32_t _max_speed, void* _arg){
    return 0;
}

sm_ev_opt_storage_t g_ev_opt_storage = {
        .load = sm_ev_load_opt_data,
        .store = sm_ev_store_opt_data,
        .store_odo = sm_ev_store_odo,
        .store_max_speed = sm_ev_store_max_speed
};
sm_ev_opt_storage_t* sm_ev_get_opt_storage(){
    sm_linux_file_t* ev_opt_storage = sm_linux_file_create("ev_opt_storage.json");
    g_ev_opt_storage.m_arg = ev_opt_storage;
    return &g_ev_opt_storage;
}

/** EV Config ******/

int32_t sm_ev_load_config(sm_ev_config_t* _config, void* _arg){
    return -1;
}
int32_t sm_ev_store_config(const sm_ev_config_t* _config, void* _arg){
    return 0;
}

//static sm_ev_config_storage_t g_ev_config_storage = {
//        .store = sm_ev_store_config,
//        .load = sm_ev_load_config,
//};
//
//sm_ev_config_storage_t* sm_get_config_ev_storage(){
//    sm_linux_file_t* ev_config_storage = sm_linux_file_create("ev_config.json");
//    g_ev_config_storage.m_arg = ev_config_storage;
//    return &g_ev_config_storage;
//}

sm_ex_flash_storage_partition_t* sm_ev_get_download_fw_storage(){

}
fw_signature_storage_fn_t sm_ev_get_hmi_signature_storage(){

}