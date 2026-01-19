//
// Created by vnbk on 30/08/2024.
//
#include <string.h>

#include "sm_sv_iot.h"
#include "sm_host_api.h"
#include "sm_sv_iot_topic_builder.h"
#include "sm_sv_iot_define.h"

#include "sm_ev_data.h"
#include "sm_bp_data.h"
#include "sm_ev_cmd.h"
#include "sm_string_t.h"

#include "sm_logger.h"
#include "sm_elapsed_timer.h"
#include "sm_sv_bp.h"
#include "tiny-json/tiny-json.h"
#include "json-maker/json-maker.h"
#include "sm_string_util.h"

#define TAG "sm_sv_iot"

enum{
    SM_IOT_INITIALIZED,
    SM_IOT_CONNECTING,
    SM_IOT_CONNECTED,
    SM_IOT_SUBSCRIBING,
    SM_IOT_SUBSCRIBED,
    SM_IOT_RUNNING
};

#define SM_IOT_INITIALIZED_TIMEOUT  5000
#define SM_IOT_CONNECTING_TIMEOUT   3000
#define SM_IOT_SUBSCRIBING_TIMEOUT  3000

#define SM_IOT_CONNECTING_RETRY         3
#define SM_IOT_SUBSCRIBING_RETRY        3
#define SM_IOT_PUBLISH_RETRY            3

#define SM_IOT_PUB_QUEUE                8

#define SM_IOT_PING_PERIOD              10

#define SM_IOT_BUFFER_DEFAULT           256

#define _impl(x) ((sm_sv_iot_impl_t*)(x))

typedef struct{
    sm_host_api_t m_base;
    const char* m_sn;

    sm_sv_iot_config_t* m_iot_config;
    const sm_sv_iot_event_t* m_iot_event;
    void* m_event_arg;

    sm_mqtt_client_t* m_mqtt_client;

    struct {
        const char* m_topic;
        void (*topic_handle)(sm_mqtt_msg_t*, void*);
    }m_sub_topics[SM_IOT_SUB_TOPIC_NUMBER];
    uint8_t m_topic_subscribed_number;

    struct{
        const char* m_topic;
        char m_payload[SM_IOT_BUFFER_DEFAULT];
    }m_pub_topics[SM_IOT_PUB_QUEUE];
    uint8_t m_queue_head;
    uint8_t m_queue_tail;

    uint8_t m_state;
    uint8_t m_retry;
    elapsed_timer_t m_timeout;

    uint8_t m_ping_count;
    uint8_t m_ping_period;

#ifdef __RTOS
    sm_mutex  m_lock;
#endif
}sm_sv_iot_impl_t;

static int32_t sm_sv_iot_init(sm_host_api_t* _this, sm_host_sync_data_if_t*, int32_t _sync_time);
static int32_t sm_sv_iot_free(sm_host_api_t* _this);
static int32_t sm_sv_iot_is_connected(sm_host_api_t* _this);
static int32_t sm_sv_iot_update(sm_host_api_t* _this, const char* _key, const char* _value);
static int32_t sm_sv_iot_updates(sm_host_api_t* _this, void* _stream);
static int32_t sm_sv_iot_push_event_to_cloud(sm_host_api_t* _this, void* _data_event);
static int32_t sm_sv_iot_process(sm_host_api_t* _this);

static void sm_mqtt_on_connected(int32_t _success, void* _arg);
static void sm_mqtt_on_disconnected(int32_t _success, void* _arg);
static void sm_mqtt_on_subscribed(int32_t _success, void* _arg);
static void sm_mqtt_on_unsubscribed(int32_t _success, void* _arg);
static void sm_mqtt_on_published(int32_t _success, void* _arg);
static void sm_mqtt_on_message(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg);

static void sm_topic_delta_handle(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_cmd_handle(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_config_handle(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_ping_handle(sm_mqtt_msg_t* _msg, void* _arg);

static void sm_topic_bp_rejected(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_bp_accepted(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_ota_request_upgrade_handle(sm_mqtt_msg_t* _msg, void* _arg);
static void sm_topic_ota_fw_info_handle(sm_mqtt_msg_t* _msg, void* _arg);

static int32_t sm_iot_publish_data(sm_sv_iot_impl_t* _this, const char* _data, const char* _topic);

static sm_host_api_proc_t g_iot_proc_default = {
    .init = sm_sv_iot_init,
    .free = sm_sv_iot_free,
    .sync_pause = sm_host_api_sync_pause,
    .sync_resume = sm_host_api_sync_resume,
    .is_connected = sm_sv_iot_is_connected,
    .reg_event_handle = sm_host_api_reg_event_handle,
    .unreg_event_handle = sm_host_api_unreg_event_handle,
    .update_data_to_host = sm_sv_iot_update,
    .update_datas_to_host = sm_sv_iot_updates,
    .push_event_to_host = sm_sv_iot_push_event_to_cloud,
    .process = sm_sv_iot_process
};

static sm_sv_iot_impl_t g_iot_service = {
    .m_base = {
        .m_proc = &g_iot_proc_default,
        .m_sync = true,
        .m_sync_time = SM_SV_IOT_SYNC_TIME_DEFAULT,
        .m_sync_if = NULL,
        .m_host_event = NULL,
        .m_host_event_arg = NULL,
    },
    .m_mqtt_client = NULL,
    .m_iot_config = NULL,
    .m_iot_event = NULL,
    .m_event_arg = NULL,
    .m_sn = NULL,
    .m_ping_count = 0,
    .m_ping_period = 0
};

sm_sv_iot_t* sm_sv_iot_create(const char* _sn,
                              sm_mqtt_client_t* _mqtt_client,
                              sm_sv_iot_config_t* _config,
                              sm_sv_iot_event_t* _iot_event, 
                              void* _arg){
    if(!_sn || !_mqtt_client){
        LOG_ERR(TAG, "Could NOT created IOT Service, missing MQTT Client");
        return NULL;
    }
    g_iot_service.m_sn = _sn;

    g_iot_service.m_iot_event = _iot_event;
    g_iot_service.m_event_arg = _arg;
    g_iot_service.m_mqtt_client = _mqtt_client;
    g_iot_service.m_iot_config = _config;

    g_iot_service.m_state = SM_IOT_INITIALIZED;
    sm_iot_topic_create_all(NULL,
                            NULL,
                            _sn);
    sm_iot_topic_ota_create_all(NULL,
                                NULL,
                                _sn);

#ifdef __RTOS
    MUTEX_INIT(g_iot_service.m_lock);
#endif
    
    return &g_iot_service.m_base;
}

sm_sv_iot_t* sm_sv_iot_createz(const char* _dev_type,
                               const char* _dev_model,
                               const char* _dev_sn,
                               sm_mqtt_client_t* _mqtt_client,
                               sm_sv_iot_config_t* _config,
                               sm_sv_iot_event_t* _iot_event,
                               void* _arg){
    if(!_dev_sn || !_mqtt_client){
        LOG_ERR(TAG, "Could NOT created IOT Service, missing MQTT Client");
        return NULL;
    }
    g_iot_service.m_sn = _dev_sn;

    g_iot_service.m_iot_event = _iot_event;
    g_iot_service.m_event_arg = _arg;
    g_iot_service.m_mqtt_client = _mqtt_client;
    g_iot_service.m_iot_config = _config;

    g_iot_service.m_state = SM_IOT_INITIALIZED;

    sm_iot_topic_create_all(_dev_type,
                            _dev_model,
                            _dev_sn);
    sm_iot_topic_ota_create_all(_dev_type,
                                _dev_model,
                                _dev_sn);

#ifdef __RTOS
    MUTEX_INIT(g_iot_service.m_lock);
#endif

    return &g_iot_service.m_base;
}

static int32_t sm_sv_iot_init(sm_host_api_t* _this, sm_host_sync_data_if_t* _sync_if, int32_t _sync_time){
    if(!_this || !_sync_if){
        return -1;
    }
    sm_host_api_init(_this, _sync_if, _sync_time);

    _impl(_this)->m_base.m_sync_time = _sync_time;

    elapsed_timer_resetz(&g_iot_service.m_timeout, SM_IOT_INITIALIZED_TIMEOUT);
    _impl(_this)->m_retry = 0;

    int index = 0;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_delta();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_delta_handle;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_cmd();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_cmd_handle;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_ota_request_upgrade();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_ota_request_upgrade_handle;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_ota_fw_info();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_ota_fw_info_handle;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_bp_rejected();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_bp_rejected;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_bp_accepted();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_bp_accepted;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_ping();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_ping_handle;
    index++;
    _impl(_this)->m_sub_topics[index].m_topic = sm_iot_get_topic_config();
    _impl(_this)->m_sub_topics[index].topic_handle = sm_topic_config_handle;

    _impl(_this)->m_topic_subscribed_number = 0;

    sm_mqtt_set_incoming_data_cb(_impl(_this)->m_mqtt_client, sm_mqtt_on_message, _this);

    return 0;
}

int32_t sm_sv_iot_free(sm_host_api_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

static int32_t sm_sv_iot_is_connected(sm_host_api_t* _this){
    if(!_this){
        return -1;
    }
    return _impl(_this)->m_state >= SM_IOT_CONNECTED;
}

static int32_t sm_sv_iot_update(sm_host_api_t* _this, const char* _key, const char* _value){
    if(!_this){
        return -1;
    }
    char buf[256];
    memset(buf, '\0', 256);
    char* p = buf;

    p = json_objOpen(p, NULL);
    p = json_objOpen(p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen(p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_str(p, SM_IOT_COMMON_SN_FIELD, _impl(_this)->m_sn);

    p = json_str(p, _key, _value);

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

#ifdef __RTOS
   ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
    memcpy(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_payload, buf, strlen(buf));
    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_topic = sm_iot_get_topic_update();
    _impl(_this)->m_queue_head++;
    if(_impl(_this)->m_queue_head >= SM_IOT_PUB_QUEUE){
        _impl(_this)->m_queue_head = 0;
    }
#ifdef __RTOS
   EXIT_CRITICAL(_impl(_this)->m_lock);
#endif
    return 0;
}

static int32_t sm_sv_iot_updates(sm_host_api_t* _this, void* _stream){
    if(!_this){
        return -1;
    }
    char buf[256];
    memset(buf, '\0', 256);
    char* p = buf;

    p = json_objOpen(p, NULL);
    p = json_objOpen(p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen(p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_str(p, SM_IOT_COMMON_SN_FIELD, _impl(_this)->m_sn);

    memcpy(p, _stream, strlen(_stream));
    p += strlen(_stream);

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

#ifdef __RTOS
   ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
    memcpy(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_payload, buf, strlen(buf));
    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_topic = sm_iot_get_topic_update();
    _impl(_this)->m_queue_head++;
    if(_impl(_this)->m_queue_head >= SM_IOT_PUB_QUEUE){
        _impl(_this)->m_queue_head = 0;
    }
#ifdef __RTOS
   EXIT_CRITICAL(_impl(_this)->m_lock);
#endif
    return 0;
}

static int32_t sm_sv_iot_build_bp_event(sm_sv_iot_impl_t* _this,  char* _buf, const char* _bp_sn){
    char* p = _buf;

    p = json_objOpen(p, NULL);
    p = json_objOpen(p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen(p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_objOpen(p, "battery");
    p = json_str(p, SM_IOT_COMMON_SN_FIELD, _bp_sn);
    p = json_objOpen(p, "connected");
    p = json_str(p, SM_IOT_COMMON_SN_FIELD, _impl(_this)->m_sn);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_DEVICE_TYPE_EV);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

    return (int32_t)(p - _buf);
}

static int32_t sm_sv_iot_build_ev_update(sm_sv_iot_impl_t* _this, char* _buf){
    char* p = _buf;
    sm_ev_data_t *ev_data = NULL;
    sm_mc_data_t *mc_data = NULL;
    sm_pmu_data_t *pmu_data = NULL;

    if(!_this->m_base.m_sync_if){
        return -1;
    }

    p = json_objOpen(p, NULL);
    p = json_objOpen(p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen(p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_str(p, SM_IOT_COMMON_SN_FIELD, _impl(_this)->m_sn);

    if(_this->m_base.m_sync_if->get_ev_data) {
        ev_data = _impl(_this)->m_base.m_sync_if->get_ev_data(_impl(_this)->m_base.m_sync_if->m_arg);

        mc_data = ev_data->m_mc_data;
        pmu_data = ev_data->m_pmu_data;

        p = json_int(p, SM_IOT_ODO_FIELD, (int32_t) ev_data->m_odo);
        p = json_int(p, SM_IOT_ERROR_FIELD, ev_data->m_err);
        p = json_int(p, SM_IOT_DRIVE_MODE_FIELD, ev_data->m_driver_mode);
        p = json_int(p, SM_IOT_RANGE_FIELD, pmu_data->m_range);
        p = json_int(p, SM_IOT_PARKING_FIELD, pmu_data->m_parking);
        p = json_int(p, SM_IOT_KEY_FIELD, pmu_data->m_key);
        p = json_int(p, SM_IOT_LOCK_MODE_FIELD, pmu_data->m_lock_status == EV_LOCK_STATE ? 1 : 0);
        p = json_int(p, SM_IOT_LOCK_STEP_FIELD, pmu_data->m_lock_status == EV_PRE_LOCK_STATE ? 1 : 0);
        p = json_int(p, SM_IOT_BLOCK_MODE_FIELD, pmu_data->m_block_status == EV_BLOCK_STATE ? 1 : 0);
        p = json_int(p, SM_IOT_BLOCK_STEP_FIELD, pmu_data->m_block_status == EV_PRE_BLOCK_STATE ? 1 : 0);
        p = json_int(p, SM_IOT_ACTIVE_MODE_FIELD, ev_data->m_active);
        p = json_int(p, SM_IOT_MOTOR_STATE_FIELD, ev_data->m_motor_active_state);
    }

    sm_gps_coordinate_t gps_data;
    char data[64];
    int32_t rssi;
    int32_t ret = -1;

    p = json_objOpen(p, SM_IOT_MODULE_HMI);

    if(_this->m_base.m_sync_if->get_network_data){
        ret = _impl(_this)->m_base.m_sync_if->get_network_data(&rssi, data, _impl(_this)->m_base.m_sync_if->m_arg);
        if(!ret){
            p = json_objOpen(p,"lte");
            p = json_int(p, "rssi", rssi);
            data[20] = '\0';
            p = json_str(p, "imsi", data);
            p = json_objClose(p);
        }
    }

    if(_this->m_base.m_sync_if->get_gps_data){
        ret = _impl(_this)->m_base.m_sync_if->get_gps_data(&gps_data, _impl(_this)->m_base.m_sync_if->m_arg);
        if(!ret) {
        	if((gps_data.lat >180.0f) || (gps_data.lat <1.0f)){
        		gps_data.lat=1.0f;
        	}

        	if((gps_data.lon >180.0f) || (gps_data.lon <1.0f)){
        		gps_data.lon=1.0f;
        	}
            p = json_objOpen(p, "gps");
            p = json_double(p, "lat", gps_data.lat);
            p = json_double(p, "lon", gps_data.lon);
            p = json_objClose(p);
        }
    }

    /// HMI Data
    if(ev_data && _this->m_base.m_sync_if->get_gps_data && _this->m_base.m_sync_if->get_network_data) {


        ret = _this->m_base.m_sync_if->get_gps_data(&gps_data, _this->m_base.m_sync_if->m_arg);
        if (!ret) {
            p = json_objOpen(p, "gps");
            p = json_double(p, "lat", gps_data.lat);
            p = json_double(p, "lon", gps_data.lon);
            p = json_objClose(p);
        }

        ret = _this->m_base.m_sync_if->get_network_data(&rssi, data, _this->m_base.m_sync_if->m_arg);
        if (!ret) {
            p = json_objOpen(p, "lte");
            p = json_int(p, "rssi", rssi);
            data[20] = '\0';
            p = json_str(p, "imsi", data);
            p = json_objClose(p);
        }

        if(_this->m_base.m_sync_if->get_optional_data) {
            ret = _this->m_base.m_sync_if->get_optional_data(data, _this->m_base.m_sync_if->m_arg);
            if (ret > 0) {
                memcpy(p, data, ret);
                p += ret;
            }
        }


    }

    p = json_objClose(p);
    /// MC data
    if(ev_data && ev_data->m_pmu_data->m_parking == EV_EXIT_PARKING) {
        p = json_int(p, SM_IOT_SPEED_FIELD, ev_data->m_speed);
        if(ev_data->m_trip >= 0){
            p = json_int(p, SM_IOT_TRIP_FIELD, ev_data->m_trip);
        }
        p = json_objOpen(p, SM_IOT_MODULE_MC);

        p = json_int(p, "motor_temp", mc_data->m_motor_temp);
        p = json_int(p, "board_temp", mc_data->m_board_temp);
        p = json_int(p, SM_IOT_STATUS_FIELD, mc_data->m_status);
        p = json_int(p, "thr_cmd", mc_data->m_thr_cmd);
        p = json_int(p, "est_tor", mc_data->m_est_tor);
        p = json_int(p, "est_dc_cur", mc_data->m_est_dc_cur);
        p = json_int(p, "allow_dc_cur", mc_data->m_allow_dc_cur);
        p = json_int(p, "revol_cnt", (int32_t) mc_data->m_revol_cnt);
        p = json_int(p, "avr_effic", mc_data->m_aver_effic);
        p = json_int(p, "enr_con_eff", mc_data->m_enr_con_effic);
        p = json_int(p, "enr_dur_trip", mc_data->m_enr_dur_trip);
        p = json_int(p, SM_IOT_MAX_SPEED_FIELD, ev_data->m_max_speed);

        p = json_objClose(p);
    }

    /// PMU data
    if(pmu_data) {
        p = json_objOpen(p, SM_IOT_MODULE_PMU);
        p = json_int(p, "abp_vol", pmu_data->m_abp_voltage);
        p = json_int(p, "all_pw_out", (int32_t) pmu_data->m_power_out);
        p = json_int(p, "all_pw_in", (int32_t) pmu_data->m_power_in);
        p = json_int(p, "pw_per_km", pmu_data->m_power_per_km);
        p = json_objClose(p);
    }

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

    return (int32_t)(p - _buf);
}

static int32_t sm_sv_iot_build_bp_update(sm_sv_iot_impl_t* _this,  char* _buf){
    char* p = _buf;

    p = json_objOpen(p, NULL);
    p = json_objOpen(p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen(p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_arrOpen(p, "batteries");

    const sm_bp_data_t* bp_data = NULL;
    for(int index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++){
        bp_data = _impl(_this)->m_base.m_sync_if->get_ev_bp_data(index, _impl(_this)->m_base.m_sync_if->m_arg);
        if(bp_data && bp_data->m_sn[0] != '\0'){
            p = json_objOpen(p, NULL);
            p = json_str(p, SM_IOT_COMMON_SN_FIELD, bp_data->m_sn);
            p = json_int(p, "slot", index);
            p = json_int(p, "cur", bp_data->m_cur);
            p = json_int(p, "vol", bp_data->m_vol);
            p = json_int(p, "cycle", bp_data->m_cycle);

            char version[16];
            memset(version, '\0', 16);
            sm_ev_version_to_string((char*)bp_data->m_version, version);

            p = json_str(p, "fw_version", version);
            p = json_int(p, "op_state", bp_data->m_state);
            p = json_int(p, "soc", bp_data->m_soc);
            p = json_int(p, "soh", bp_data->m_soh);
            p = json_int(p, SM_IOT_STATUS_FIELD, bp_data->m_status);

            p = json_objOpen(p, "assigned");
            p = json_str(p, SM_IOT_COMMON_SN_FIELD, bp_data->m_assignedSn);
            p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_DEVICE_TYPE_EV);
            p = json_objClose(p);

            p = json_arrOpen(p, "temps");
            for(int i = 0; i < 6; i++){
                p = json_int(p, NULL, bp_data->m_temps[i]);
            }
            p = json_arrClose(p);

            p = json_arrOpen(p, "cells_vol");
            for(int i = 0; i < 16; i++){
                p = json_int(p, NULL, bp_data->m_cellVols[i]);
            }
            p = json_arrClose(p);

            p = json_int(p, SM_IOT_ACTIVE_MODE_FIELD, bp_data->m_active_mode);
            p = json_int(p, "block", bp_data->m_block_mode);

            char bp_version_compare[] = {1,0,19};
            if(sm_ev_version_compare((char*)bp_data->m_version, bp_version_compare) > 0){
                p = json_int(p, "cycle_decimal", bp_data->m_cycle_decimal);
            }

            p = json_objClose(p);
        }
    }
    p = json_arrClose(p);

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

    return (int32_t)(p - _buf);
}

static int32_t sm_sv_iot_build_info_update(sm_sv_iot_impl_t *_this, char *_buf){
    char *p = _buf;

    if(!_this->m_base.m_sync_if->get_ev_module_info){
        return -1;
    }

    const char* hmi_version = _this->m_base.m_sync_if->get_ev_module_info(SM_EV_MODULE_HMI,
                                                                 _this->m_base.m_sync_if->m_arg);
    const char* mc_version = _this->m_base.m_sync_if->get_ev_module_info(SM_EV_MODULE_MC,
                                                                     _this->m_base.m_sync_if->m_arg);
    const char* pmu_version = _this->m_base.m_sync_if->get_ev_module_info(SM_EV_MODULE_PMU,
                                                                     _this->m_base.m_sync_if->m_arg);

    p = json_objOpen (p, NULL);
    p = json_objOpen (p, SM_IOT_COMMON_STATE_FIELD);
    p = json_objOpen (p, SM_IOT_COMMON_REPORTED_FIELD);
    p = json_str (p, SM_IOT_COMMON_SN_FIELD, _impl(_this)->m_sn);

    if(hmi_version){
        p = json_objOpen (p, SM_IOT_MODULE_HMI);
        p = json_str (p, SM_IOT_FW_VERSION, hmi_version);
        p = json_objClose (p);
    }

    if(mc_version){
        p = json_objOpen (p, SM_IOT_MODULE_MC);
        p = json_str (p, SM_IOT_FW_VERSION, mc_version);
        p = json_objClose (p);
    }

    if(pmu_version){
        p = json_objOpen (p, SM_IOT_MODULE_PMU);
        p = json_str (p, SM_IOT_FW_VERSION, pmu_version);
        p = json_objClose (p);
    }

    p = json_objClose (p);
    p = json_objClose (p);
    p = json_objClose (p);
    p = json_end (p);

    return (int32_t) (p - _buf);
}

static int32_t sm_sv_iot_sync_info(sm_sv_iot_impl_t* _this){
    char buf[256];
    memset(buf, 0, 256);

    int32_t payload_len = sm_sv_iot_build_info_update(_this, buf);

    if(payload_len <= 0){
        LOG_ERR(TAG, "Could NOT created SYNC Info message");
        return -1;
    }

    int32_t ret = sm_mqtt_publish(_impl(_this)->m_mqtt_client,
                                  sm_iot_get_topic_update(),
                                  buf,
                                  SM_IOT_QOS_DEFAULT,
                                  SM_IOT_RETAIN_DEFAULT,
                                  sm_mqtt_on_published,
                                  _this);
    if (ret < 0) {
        LOG_ERR(TAG, "Sync EV INFO FAILURE, reason: Could NOT publish EV INFO");
        return ret;
    }
    LOG_INF(TAG, "Sync EV INFO SUCCESS");

    return 0;
}

static int32_t sm_sv_iot_sync_data(sm_sv_iot_impl_t* _this){
    char buf[1536];
    int32_t ret = -1;
    memset(buf, 0, 1536);
    int32_t payload_len = sm_sv_iot_build_ev_update(_impl(_this), buf);

    if(payload_len > 0){
        ret = sm_mqtt_publish(_impl(_this)->m_mqtt_client,
                              sm_iot_get_topic_update(),
                              buf,
                              SM_IOT_QOS_DEFAULT,
                              SM_IOT_RETAIN_DEFAULT,
                              sm_mqtt_on_published,
                              _this);
        if (ret < 0) {
            LOG_ERR(TAG, "Sync EV Data FAILURE, reason: Could NOT publish Data");
        }else{
            LOG_DBG(TAG, "Sync EV Data SUCCESS");
        }
    }

    memset(buf, 0, sizeof(buf));
    payload_len = sm_sv_iot_build_bp_update(_impl(_this), buf);
    ret = -1;

    if(payload_len > 0){
        ret = sm_mqtt_publish(_impl(_this)->m_mqtt_client,
                              sm_iot_get_topic_bp_update(),
                              buf,
                              SM_IOT_QOS_DEFAULT,
                              SM_IOT_RETAIN_DEFAULT,
                              sm_mqtt_on_published,
                              _this);
        if(ret < 0){
            LOG_ERR(TAG, "Sync BP ev_data FAILURE, reason: Could NOT publish BP data");
            return ret;
        }
        LOG_DBG(TAG, "Sync BP ev_data SUCCESS");
    }

    return ret;
}

static void sm_sv_iot_sync_bp_event_to_cloud(sm_sv_iot_impl_t* _this){
    /// Sync Event BP when connected
    const sm_bp_data_t* bp_data = NULL;
    int32_t index = 0;
    for(index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++)
    {
        bp_data = _impl(_this)->m_base.m_sync_if->get_ev_bp_data(index,
                                                                 _impl(_this)->m_base.m_sync_if->m_arg);
        if (bp_data && bp_data->m_sn[0] != '\0') {
            sm_sv_iot_push_event_to_cloud((sm_host_api_t*)_this, (void*)bp_data->m_sn);
        }
    }
}

static int32_t sm_sv_iot_push_event_to_cloud(sm_host_api_t* _this, void* _data_event){
    char buf[256];
    if(!_this || !_data_event){
        return -1;
    }

    int32_t payload_len = sm_sv_iot_build_bp_event(_impl(_this), buf, (const char*)_data_event);
    if(!payload_len){
        LOG_ERR(TAG, "Could NOT created bp event message");
        return -1;
    }

#ifdef __RTOS
    ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
    memcpy(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_payload, buf, strlen(buf));
    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_topic = sm_iot_get_topic_bp_event();
    _impl(_this)->m_queue_head++;
    if(_impl(_this)->m_queue_head >= SM_IOT_PUB_QUEUE){
        _impl(_this)->m_queue_head = 0;
    }
#ifdef __RTOS
   EXIT_CRITICAL(_impl(_this)->m_lock);
#endif
    return 0;
}

static int32_t sm_iot_publish_data(sm_sv_iot_impl_t *_this, const char *_data, const char *_topic) {
    int32_t ret = sm_mqtt_publish(_impl(_this)->m_mqtt_client,
                                  _topic,
                                  _data,
                                  SM_IOT_QOS_DEFAULT,
                                  SM_IOT_RETAIN_DEFAULT,
                                  sm_mqtt_on_published,
                                  _this);
    if (ret < 0) {
        LOG_ERR(TAG, "Sync BP ev_data FAILURE, reason: Could NOT publish ev_data");
    }
    return ret;
}

static int32_t sm_iot_ping(sm_sv_iot_impl_t *_this){
    char buf[64];
    char* p = buf;

    memset(buf, 0, 64);

    p = json_objOpen (p, NULL);
    p = json_objClose (p);
    p = json_end (p);

    return sm_mqtt_publish(_impl(_this)->m_mqtt_client,
                                  sm_iot_get_topic_ping(),
                                  buf,
                                  SM_IOT_QOS_DEFAULT,
                                  SM_IOT_RETAIN_DEFAULT,
                                  sm_mqtt_on_published,
                                  _this);
}

static int32_t sm_sv_iot_process(sm_host_api_t* _this){
    if(!_this){
        return -1;
    }
    int32_t ret = -1;
    switch (_impl(_this)->m_state) {
        case SM_IOT_INITIALIZED:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)){
                _impl(_this)->m_state = SM_IOT_CONNECTING;
                elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_CONNECTING_TIMEOUT);
            }
            break;
        case SM_IOT_CONNECTING:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)){
                ret = sm_mqtt_connect(_impl(_this)->m_mqtt_client,
                                      _impl(_this)->m_iot_config->m_host,
                                      _impl(_this)->m_iot_config->m_port,
                                      (char*)_impl(_this)->m_iot_config->m_client_id,
                                      _impl(_this)->m_iot_config->m_username,
                                      _impl(_this)->m_iot_config->m_password,
                                      sm_mqtt_on_connected,
                                      _this);
                if(ret >= 0){
                    _impl(_this)->m_state = SM_IOT_CONNECTED;
                    break;
                }else{
                    _impl(_this)->m_retry++;
                    elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_CONNECTING_TIMEOUT);
                    if(_impl(_this)->m_retry >= SM_IOT_CONNECTING_RETRY){
                        _impl(_this)->m_state = SM_IOT_INITIALIZED;
                        elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_INITIALIZED_TIMEOUT);
                    }
                }
            }
            break;
        case SM_IOT_CONNECTED:
            LOG_INF(TAG, "MQTT Client connected");
            _impl(_this)->m_state = SM_IOT_SUBSCRIBING;
            _impl(_this)->m_retry = 0;
            elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_SUBSCRIBING_TIMEOUT);
            break;
        case SM_IOT_SUBSCRIBING:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)){
                ret = 0;
                if ( _impl(_this)->m_sub_topics[_impl(_this)->m_topic_subscribed_number].m_topic != NULL){
                    ret = sm_mqtt_subscribe(_impl(_this)->m_mqtt_client,
                        _impl(_this)->m_sub_topics[_impl(_this)->m_topic_subscribed_number].m_topic,
                        SM_IOT_QOS_DEFAULT,
                        sm_mqtt_on_subscribed,
                        _this);
                }
                if(ret >= 0){
                    _impl(_this)->m_topic_subscribed_number++;
                    if(_impl(_this)->m_topic_subscribed_number >= SM_IOT_SUB_TOPIC_NUMBER){
                        _impl(_this)->m_state = SM_IOT_SUBSCRIBED;
                    }
                }else{
                    _impl(_this)->m_retry++;
                    if(_impl(_this)->m_retry >= SM_IOT_SUBSCRIBING_RETRY){
                        sm_mqtt_disconnect(_impl(_this)->m_mqtt_client, sm_mqtt_on_disconnected, _this);
                        _impl(_this)->m_state = SM_IOT_INITIALIZED;
                        elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_INITIALIZED_TIMEOUT);
                    }
                }
            }
            break;
        case SM_IOT_SUBSCRIBED:
            LOG_ERR(TAG, "MQTT Client is subscribed all topic");
            _impl(_this)->m_state = SM_IOT_RUNNING;
            _impl(_this)->m_retry = 0;
            elapsed_timer_resetz(&_impl(_this)->m_timeout, _impl(_this)->m_base.m_sync_time*1000);

            sm_sv_iot_sync_info(_impl(_this));
            sm_sv_iot_sync_data(_impl(_this));
            sm_sv_iot_sync_bp_event_to_cloud(_impl(_this));
            break;
        case SM_IOT_RUNNING:
            if (_this->m_sync) {
                ret = sm_mqtt_client_process(_impl(_this)->m_mqtt_client);
                if (ret < 0) {
                    LOG_ERR(TAG, "MQTT Client connection is problem");
                    sm_mqtt_on_disconnected(0, _this);
                }
                if (!elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {
                    ret = sm_sv_iot_sync_data(_impl(_this));
                    if (ret < 0) {
                        _impl(_this)->m_retry++;
                        if (_impl(_this)->m_retry >= SM_IOT_PUBLISH_RETRY) {
                            sm_mqtt_on_disconnected(0, _this);
                            _impl(_this)->m_retry = 0;
                            break;
                        }
                    } else {
                        _impl(_this)->m_retry = 0;
                    }

                    elapsed_timer_reset(&_impl(_this)->m_timeout);

                    _impl(_this)->m_ping_period++;
                    if (_impl(_this)->m_ping_period >= SM_IOT_PING_PERIOD) {
                        sm_iot_ping(_impl(_this));
                        _impl(_this)->m_ping_period = 0;

                        _impl(_this)->m_ping_count++;
                        if (_impl(_this)->m_ping_count >= 2) {
                            _impl(_this)->m_ping_count = 0;
                            sm_sv_iot_reset(_this);
                            return -1;
                        }
                    }
                }
            }

            if(_impl(_this)->m_queue_head != _impl(_this)->m_queue_tail){
                sm_iot_publish_data(_impl(_this),
                                    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_tail].m_payload,
                                    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_tail].m_topic);

#ifdef __RTOS
               ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
                memset(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_tail].m_payload, '\0', SM_IOT_BUFFER_DEFAULT);
                _impl(_this)->m_pub_topics[_impl(_this)->m_queue_tail].m_topic = NULL;
                _impl(_this)->m_queue_tail++;
                if(_impl(_this)->m_queue_tail >= SM_IOT_PUB_QUEUE){
                    _impl(_this)->m_queue_tail = 0;
                }
#ifdef __RTOS
                EXIT_CRITICAL(_impl(_this)->m_lock);
#endif
            }

            break;
        default:
            LOG_ERR(TAG, "State Not Support");
            break;
    }

    return 0;
}

static void sm_mqtt_on_connected(int32_t _success, void* _arg){
    if(!_arg){
        return;
    }
    if(!_success){
        LOG_INF(TAG, "MQTT Client on connected");
    }else{
        LOG_ERR(TAG, "MQTT Client could NOT connecting to Broker");
    }

    if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_connected){
        _impl(_arg)->m_base.m_host_event->on_connected(_success, _impl(_arg)->m_base.m_host_event_arg);
    }
}

static void sm_mqtt_on_disconnected(int32_t _success, void* _arg){
    if(!_arg){
        return;
    }
    if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_disconnected){
        _impl(_arg)->m_base.m_host_event->on_disconnected(_success, _impl(_arg)->m_base.m_host_event_arg);
    }

    _impl(_arg)->m_state = SM_IOT_CONNECTING;
    _impl(_arg)->m_topic_subscribed_number = 0;
    elapsed_timer_resetz(&_impl(_arg)->m_timeout, SM_IOT_CONNECTING_TIMEOUT);
    LOG_WRN(TAG, "MQTT Client on disconnected, Reconnecting to broker");
}

static void sm_mqtt_on_subscribed(int32_t _success, void* _arg){
    if(!_arg){
        return;
    }
    LOG_INF(TAG, "MQTT Client on subscribed %s at topic index: %d \n %s", !_success ? "SUCCESS":"FAILURE",
                                                                            _impl(_arg)->m_topic_subscribed_number,
                                                                            _impl(_arg)->m_sub_topics[_impl(_arg)->m_topic_subscribed_number].m_topic);
}

static void sm_mqtt_on_unsubscribed(int32_t _success, void* _arg){
    if(!_arg){
        return;
    }
    LOG_DBG(TAG, "MQTT Client on unsubscribed: %d", _success);
}

static void sm_mqtt_on_published(int32_t _success, void* _arg){
    if(!_arg){
        return;
    }
    LOG_DBG(TAG, "MQTT Client on published: %d", _success);
}

static void sm_mqtt_on_message(sm_mqtt_client_t* _mqtt_client, sm_mqtt_msg_t* _msg, void* _arg){
    if(!_arg){
        return;
    }
    LOG_INF(TAG, "MQTT Client received message. Topic: %s, payload: %s", _msg->m_topic, _msg->m_payload);
    for(int index = 0; index < SM_IOT_SUB_TOPIC_NUMBER; index++){
        if (_impl(_arg)->m_sub_topics[index].m_topic == NULL){
            continue;
        }
        if(!strncmp(_impl(_arg)->m_sub_topics[index].m_topic, _msg->m_topic, _msg->m_topic_length)){
            _impl(_arg)->m_sub_topics[index].topic_handle(_msg, _arg);
        }
    }
}

/********************************* Topic Handle*******************************/
static void sm_topic_ping_handle(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create((char*)_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "CMD message FAILURE");
        return;
    }
    _impl(_arg)->m_ping_count = 0;
}

static void sm_topic_delta_handle(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create((char*)_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "CMD message FAILURE");
        return;
    }
    const json_t* state = json_getProperty(json, SM_IOT_COMMON_STATE_FIELD);
    if(!state || JSON_OBJ != json_getType(state)){
        LOG_ERR(TAG, "Delta message missing state field");
        return;
    }
    const json_t* cmd_obj = NULL;
    int32_t value;
    int32_t cmd;
    if((cmd_obj = json_getProperty(state, SM_IOT_CMD_EV_BLOCK)) != NULL){
        cmd = SM_EV_CMD_BLOCK_EV;
        value = json_getInteger(cmd_obj);
    }else if((cmd_obj = json_getProperty(state, SM_IOT_CMD_EV_LOCK)) != NULL){
        cmd = SM_EV_CMD_LOCK_EV;
        value = json_getInteger(cmd_obj);
    }else if((cmd_obj = json_getProperty(state, SM_IOT_CMD_BP)) != NULL){
            LOG_INF(TAG, "Write device SN to BP from server");
            const json_t* bp_cmd = json_getChild(cmd_obj);

            if(strcmp(json_getName(bp_cmd), SM_IOT_CMD_ASSIGN_BP) != 0){
                return;
            }

            const json_t* bp_obj = json_getProperty(bp_cmd, SM_IOT_COMMON_SN_FIELD);
            const json_t* dev_obj = json_getProperty(bp_cmd, SM_IOT_CMD_ASSIGNED_DEV);

            if(!bp_obj || !dev_obj){
                return;
            }

            const char* bp_sn = json_getValue(bp_obj);
            const char* dev_sn = json_getValue(dev_obj);

            if(!bp_sn){
                return;
            }

            const sm_bp_data_t* bp_data = NULL;
            for(int index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++) {
                bp_data = _impl(_arg)->m_base.m_sync_if->get_ev_bp_data(index, _impl(_arg)->m_base.m_sync_if->m_arg);
                if(!bp_data){
                    continue;
                }
                if(!strcmp(bp_data->m_sn, bp_sn)){
                    sm_cmd_extended_data_t bp_cmd_data = {
                            .m_id = index,
                            .m_data = dev_sn
                    };
                    if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
                        _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_WRITE_DEV_TO_BP,
                                                                           &bp_cmd_data,
                                                                           _impl(_arg)->m_base.m_host_event_arg);
                    }
                    return;
                }
            }

            return;
    }else{
        LOG_ERR(TAG, "Delta message ev_data INVALID");
        return;
    }

    if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
        _impl(_arg)->m_base.m_host_event->on_cmd_from_host(cmd,
                                                           &value,
                                                           _impl(_arg)->m_base.m_host_event_arg);
    }
}

static void sm_topic_module_cmd_handle(sm_sv_iot_impl_t* _this, const json_t* _cmd){
    const json_t* module = json_getChild(_cmd);
    const char* module_id = json_getName(module);

    const json_t* sub_cmd = json_getChild(module);
    if(!sub_cmd){
        return;
    }
    const char* cmd_string = json_getName(sub_cmd);

    if(!strncmp(cmd_string, SM_IOT_CMD_MODULE_REBOOT, strlen(SM_IOT_CMD_MODULE_REBOOT))){
        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            int32_t value = sm_atoi(module_id);
            _impl(_this)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_REBOOT_MODULE,
                                                               &value,
                                                               _this->m_base.m_host_event_arg);
        }
    }else if(!strncmp(cmd_string, SM_IOT_CMD_MODULE_VERSION, strlen(SM_IOT_CMD_MODULE_VERSION))){
        sm_sv_iot_sync_info(_this);
    }
}

static void sm_topic_port_cmd_handle(sm_sv_iot_impl_t* _this, const json_t* _cmd){
    const json_t* port = json_getChild(_cmd);
    const char* port_id = json_getName(port);

    const json_t* sub_cmd = json_getChild(port);
    if(!sub_cmd){
        return;
    }

    const char* cmd_string = json_getName(sub_cmd);
    int32_t value = json_getInteger(sub_cmd);

    sm_cmd_extended_data_t cmd_data = {
            .m_id = sm_atoi(port_id),
            .m_data = (const char*)&value
    };

    int32_t port_cmd = SM_EV_CMD_NUMBER;

    if(!strncmp(cmd_string, SM_IOT_CMD_PORT_LOCK, strlen(SM_IOT_CMD_PORT_LOCK))){
        port_cmd = SM_EV_CMD_PORT_LOCK;
    }else if(!strncmp(cmd_string, SM_IOT_CMD_PORT_ENABLE, strlen(SM_IOT_CMD_PORT_ENABLE))){
        port_cmd = SM_EV_CMD_PORT_ENABLE;
    }else if(!strncmp(cmd_string, SM_IOT_CMD_PORT_FORCE, strlen(SM_IOT_CMD_PORT_FORCE))){
        port_cmd = SM_EV_CMD_PORT_FORCE;
    }

    if(port_cmd < SM_EV_CMD_NUMBER && _this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
        _this->m_base.m_host_event->on_cmd_from_host(port_cmd,
                                                     &cmd_data,
                                                     _this->m_base.m_host_event_arg);
    }
}

static void sm_topic_bp_cmd_handle(sm_sv_iot_impl_t* _this, const json_t* _cmd){
    const json_t* slot = json_getChild(_cmd);
    const char* slot_id = json_getName(slot);

    const json_t* sub_cmd = json_getChild(slot);

    if(!sub_cmd){
        return;
    }

    const char* cmd_string = json_getName(sub_cmd);

    if(!strncmp(cmd_string, SM_IOT_CMD_ASSIGN_BP, strlen(cmd_string))){
        sm_cmd_extended_data_t bp_cmd_data = {
                .m_id = sm_atoi(slot_id),
                .m_data = json_getValue(json_getChild(sub_cmd))
        };
        LOG_INF(TAG, "Write device SN to BP from server");

        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            _this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_WRITE_DEV_TO_BP,
                                                         &bp_cmd_data,
                                                         _this->m_base.m_host_event_arg);
        }
        return;
    }

    if(!strncmp(cmd_string, SM_IOT_CMD_UPGRADE_BP, strlen(cmd_string))){
        const json_t* version = json_getProperty(sub_cmd, SM_IOT_VERSION_FIELD);
        const json_t* size = json_getProperty(sub_cmd, SM_IOT_FW_SIZE_FIELD);
        const json_t* crc = json_getProperty(sub_cmd, SM_IOT_FW_CRC_FIELD);
        const json_t* link = json_getProperty(sub_cmd, SM_IOT_FW_LINK_FIELD);

        sm_cmd_upgrade_bp_data_t bp_cmd_data = {
                .m_slot = sm_atoi(slot_id),
                .m_new_version = json_getValue(version),
                .m_size = json_getInteger(size),
                .m_crc = json_getInteger(crc),
                .m_link = json_getValue(link)
        };
        LOG_INF(TAG, "Upgrade bp request from server");

        if(_this->m_iot_event && _this->m_iot_event->on_bp_new_fw){
            _this->m_iot_event->on_bp_new_fw(&bp_cmd_data,
                                             _this->m_base.m_host_event_arg);
        }
        return;
    }

    if(!strncmp(cmd_string, "op_state", strlen(cmd_string))){
        uint8_t bp_state = json_getInteger(sub_cmd);
        sm_cmd_extended_data_t bp_cmd_data = {
                .m_id = sm_atoi(slot_id),
                .m_data = (const char*)&bp_state
        };
        LOG_INF(TAG, "Set BP state from Server");

        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            _this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_STATE_BP,
                                                         &bp_cmd_data,
                                                         _this->m_base.m_host_event_arg);
        }
        return;
    }

    if(!strncmp(cmd_string, SM_IOT_CMD_EV_BLOCK, strlen(cmd_string))){
        uint8_t block_value = json_getInteger(sub_cmd);
        sm_cmd_extended_data_t bp_cmd_data = {
                .m_id = sm_atoi(slot_id),
                .m_data = (const char*)&block_value
        };
        LOG_INF(TAG, "Set BLOCK from Server");

        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            _this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_BLOCK_BP,
                                                         &bp_cmd_data,
                                                         _this->m_base.m_host_event_arg);
        }
        return;
    }

    if(!strncmp(cmd_string, SM_IOT_ACTIVE_MODE_FIELD, strlen(cmd_string))){
        uint8_t active_value = json_getInteger(sub_cmd);
        sm_cmd_extended_data_t bp_cmd_data = {
                .m_id = sm_atoi(slot_id),
                .m_data = (const char*)&active_value
        };
        LOG_INF(TAG, "Set ACTIVE BP from Server");

        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            _this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_ACTIVE_BP,
                                                         &bp_cmd_data,
                                                         _this->m_base.m_host_event_arg);
        }
        return;
    }

    if(!strncmp(cmd_string, "cycle", strlen(cmd_string))){
        uint16_t value = json_getInteger(sub_cmd);
        sm_cmd_extended_data_t bp_cmd_data = {
                .m_id = sm_atoi(slot_id),
                .m_data = (const char*)&value
        };
        LOG_INF(TAG, "Set Cycle BP from Server");

        if(_this->m_base.m_host_event && _this->m_base.m_host_event->on_cmd_from_host){
            _this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_CYCLE_BP,
                                                         &bp_cmd_data,
                                                         _this->m_base.m_host_event_arg);
        }
        return;
    }
}

static void sm_topic_cmd_handle(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "CMD message FAILURE");
        return;
    }
    const json_t* state = json_getProperty(json, SM_IOT_COMMON_STATE_FIELD);
    if(!state || JSON_OBJ != json_getType(state)){
        LOG_ERR(TAG, "Delta message missing state field");
        return;
    }

    const json_t* cmd = json_getProperty(state, SM_IOT_CMD_DRIVE_MODE);
    int32_t value;
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set Driver mode from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_DRIVE_MODE,
                                                               &value, // mode
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    cmd = json_getProperty(state, SM_IOT_CMD_FIND_EV);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Find ev from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_FIND_EV,
                                                               &value,
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    cmd = json_getProperty(state, SM_IOT_CMD_MAX_SPEED);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set Max speed from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_MAX_SPEED,
                                                               &value, // max_speed
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    cmd = json_getProperty(state, SM_IOT_CMD_LOCK_PORT);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set LOCK BP Port from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_LOCK_PORT,
                                                               &value, // port
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    cmd = json_getProperty(state, SM_IOT_CMD_UNLOCK_PORT);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set UNBLOCK BP Port from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_UNLOCK_PORT,
                                                               &value, // port
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    /// Config verify BP Offline
    cmd = json_getProperty(state, SM_IOT_CONFIG_VERIFY_BP_OFFLINE);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set config verify BP offline from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_CONFIG_VERIFY_BP_OFFLINE,
                                                               &value,
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    /// Set Active EV
    cmd = json_getProperty(state, SM_IOT_CMD_EV_ACTIVE);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "Set Active EV from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
           value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_INACTIVE_EV,
                                                              &value,
                                                              _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
     }

    /// Reboot Module
    cmd = json_getProperty(state, SM_IOT_CMD_REBOOT_MODULE);
    if(cmd && JSON_INTEGER == json_getType(cmd)){
        LOG_INF(TAG, "REBOOT module from server");
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            value = json_getInteger(cmd);
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_REBOOT_MODULE,
                                                               &value, // module
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    /// Set ODO
    cmd = json_getProperty(state, SM_IOT_CMD_SET_ODO);
    if(cmd && JSON_OBJ == json_getType(cmd)){
        LOG_INF(TAG, "Set ODO from server");
        const json_t* odo = json_getProperty(cmd, "value");
        const json_t* pass = json_getProperty(cmd, "pass");

        sm_cmd_extended_data_t odo_data = {
                .m_id = json_getInteger(odo),
                .m_data = json_getValue(pass)
        };
        if(_impl(_arg)->m_base.m_host_event && _impl(_arg)->m_base.m_host_event->on_cmd_from_host){
            _impl(_arg)->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_ODO,
                                                               &odo_data,
                                                               _impl(_arg)->m_base.m_host_event_arg);
        }
        return;
    }

    /// CMD Module
    cmd = json_getProperty(state, SM_IOT_CMD_MODULE);
    if(cmd && JSON_OBJ == json_getType(cmd)){
        LOG_INF(TAG, "Module CMD from server");
        sm_topic_module_cmd_handle(_impl(_arg), cmd);
        return;
    }

    /// CMD PORT
    cmd = json_getProperty(state, SM_IOT_CMD_PORT);
    if(cmd && JSON_OBJ == json_getType(cmd)){
        LOG_INF(TAG, "PORT CMD from server");
        sm_topic_port_cmd_handle(_impl(_arg), cmd);
        return;
    }

    /// CMD BP
    cmd = json_getProperty(state, SM_IOT_CMD_BP);
    if(cmd){
        sm_topic_bp_cmd_handle(_impl(_arg), cmd);
    }
}

static void sm_iot_load_config_handle(sm_sv_iot_impl_t* _this){
    if(!_this->m_base.m_sync_if->get_ev_config){
        return;
    }
    char buf[512];
    char* p = buf;

    memset(buf, 0, 512);

    sm_ev_config_t* config = _this->m_base.m_sync_if->get_ev_config(_this->m_base.m_sync_if->m_arg);

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_LOAD_CONFIG_RESPONSE);
    p = json_objOpen(p, SM_IOT_DATA_FIELD);

    p = json_objOpen(p, SM_IOT_CONF_AUTH_BP);
    p = json_int(p, SM_IOT_CONF_AUTH_BP_OFFLINE, config->m_auth_bp & SM_EV_CONF_AUTH_BP_OFFLINE_MASK);
    p = json_int(p, SM_IOT_CONF_AUTH_BP_ONLINE, (config->m_auth_bp >> 4) & SM_EV_CONF_AUTH_BP_OFFLINE_MASK);
    p = json_objClose(p);

    p = json_bool(p, SM_IOT_CONF_UPHILL_MODE, config->m_uphill_mode);
    p = json_bool(p, SM_IOT_CONF_LOCK_PORT, config->m_lock_port);

    p = json_objOpen(p, SM_IOT_CONF_INACTIVE_MODE);
    p = json_bool(p, "enable", config->m_inactive_mode_config.m_enable);
    p = json_int(p, SM_IOT_CONF_INACTIVE_MODE_KM_WARNING, config->m_inactive_mode_config.m_km_warning);
    p = json_int(p, SM_IOT_CONF_INACTIVE_MODE_KM_STOP, config->m_inactive_mode_config.m_km_force_stop);
    p = json_objClose(p);

    p = json_int(p, SM_IOT_CONF_KM_ODO_STORED, config->m_km_store_odo);
    p = json_str(p, SM_IOT_CONF_ODO_PASS, (const char*)config->m_odo_pass);
    p = json_int(p, SM_IOT_CONF_SYNC_TIME, config->m_sync_time);

    p = json_objOpen(p, SM_IOT_CONF_AUTH_MODULE);
    p = json_int(p, SM_IOT_CONF_AUTH_MODULE_LEVEL, config->m_auth_module.m_level);
    p = json_int(p, SM_IOT_CONF_AUTH_MODULE_DETECTED_TIME, config->m_auth_module.m_detected_time);
    p = json_objClose(p);

    sm_ev_manu_t* ev_manu = _this->m_base.m_sync_if->get_ev_manu(_this->m_base.m_sync_if->m_arg);
    p = json_double(p, SM_IOT_CONF_WHEEL_RADIUS, ev_manu->m_wheel_radius);

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

    sm_iot_publish_data(_this, buf, sm_iot_get_topic_config());
}

static void sm_topic_config_handle_response(sm_sv_iot_impl_t* _this, int32_t _ret){
    char buf[128];
    char* p = buf;

    memset(buf, 0, 128);

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_SET_CONFIG_RESPONSE);
    p = json_objOpen(p, SM_IOT_DATA_FIELD);
    p = json_int(p, SM_IOT_ERROR_FIELD, _ret);
    p = json_str(p, SM_IOT_ERROR_MSG_FIELD, "");

    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);

    sm_iot_publish_data(_this, buf, sm_iot_get_topic_config());
}

static void sm_topic_config_handle(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "CONFIG message is INVALID");
        return;
    }

    const json_t *type = json_getProperty(json, SM_IOT_TYPE_FIELD);
    if (!type || JSON_TEXT != json_getType(type)){
        LOG_WRN(TAG, "CONFIG message is missed type field");
        return;
    }

    const char* type_s = json_getValue(type);
    if (!strcmp(type_s, SM_IOT_LOAD_CONFIG_RESPONSE) || !strcmp(type_s, SM_IOT_SET_CONFIG_RESPONSE)){
        return;
    }

    /// Handle LOAD config
    if (!strcmp(type_s, SM_IOT_LOAD_CONFIG_REQUEST)){
        sm_iot_load_config_handle(_impl(_arg));
        return;
    }

    if (!strcmp(type_s, SM_IOT_SET_CONFIG_REQUEST)){
        const json_t *data = json_getProperty(json, SM_IOT_DATA_FIELD);
        if (!data || JSON_OBJ != json_getType(data)){
            LOG_WRN(TAG, "CONFIG message is missed data field");
            return;
        }
        if (!_impl(_arg)->m_base.m_host_event ||
                !_impl(_arg)->m_base.m_host_event->on_cfg_from_host ||
                !_impl(_arg)->m_base.m_host_event->on_cfg_completed){
            return;
        }

        const json_t* config_obj = json_getChild(data);
        for(; config_obj != NULL; config_obj = json_getSibling(config_obj)){
            const char* config_name = json_getName(config_obj);

            if(!strcmp(config_name, SM_IOT_CONF_AUTH_BP)){
                LOG_DBG(TAG, "Handle configure BP authentication mode");
                uint8_t offline = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_AUTH_BP_OFFLINE));
                uint8_t value = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_AUTH_BP_ONLINE));

                value = (value << 4) & SM_EV_CONF_AUTH_BP_ONLINE_MASK;
                value |= (offline & SM_EV_CONF_AUTH_BP_OFFLINE_MASK);

                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_AUTH_BP,
                                                                   &value,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_UPHILL_MODE)){
                LOG_DBG(TAG, "Handle uphill mode configuration");
                int32_t enable = json_getBoolean(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_UPHILL_MODE,
                                                                   &enable,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_LOCK_PORT)){
                LOG_DBG(TAG, "Handle LOCK PORT configuration");
                int32_t enable = json_getBoolean(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_LOCK_PORT,
                                                                   &enable,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_INACTIVE_MODE)){
                LOG_DBG(TAG, "Handle INACTIVE MODE configuration");
                sm_ev_inactive_mode_config_t inactive_config = {
                        .m_enable = json_getBoolean(json_getProperty(config_obj, "enable")),
                        .m_km_warning = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_INACTIVE_MODE_KM_WARNING)),
                        .m_km_force_stop = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_INACTIVE_MODE_KM_STOP)),
                        .m_save_time = 0,
                };
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_INACTIVE_MODE,
                                                                   &inactive_config,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_WHEEL_RADIUS)){
                LOG_DBG(TAG, "Handle WHEEL RADIUS configuration");
                float wheel_radios = (float)json_getReal(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_WHEEL_RADIUS,
                                                                   &wheel_radios,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_KM_ODO_STORED)){
                LOG_DBG(TAG, "Handle ODO configuration");
                int32_t km_stored = json_getInteger(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_KM_ODO_STORED,
                                                                   &km_stored,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_ODO_PASS)){
                LOG_DBG(TAG, "Handle ODO PASS configuration");
                const char* odo_pass = json_getValue(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_ODO_PASS,
                                                                   (void*)odo_pass,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_SYNC_TIME)){
                LOG_DBG(TAG, "Handle SYNC TIME configuration");
                int32_t sync_time = json_getInteger(config_obj);
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_SYNC_TIME,
                                                                   &sync_time,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }else if(!strcmp(config_name, SM_IOT_CONF_AUTH_MODULE)){
                LOG_DBG(TAG, "Handle Auth module configuration");
                sm_auth_module_config_t auth_config = {
                        .m_level = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_AUTH_MODULE_LEVEL)),
                        .m_detected_time = json_getInteger(json_getProperty(config_obj, SM_IOT_CONF_AUTH_MODULE_DETECTED_TIME))
                };
                _impl(_arg)->m_base.m_host_event->on_cfg_from_host(SM_EV_CONF_AUTH_MODULE,
                                                                   &auth_config,
                                                                   _impl(_arg)->m_base.m_host_event_arg);
            }
        }

        int32_t ret = _impl(_arg)->m_base.m_host_event->on_cfg_completed(_impl(_arg)->m_base.m_host_event_arg);
        sm_topic_config_handle_response(_impl(_arg), ret);
    }
}

static void sm_topic_bp_rejected(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "BP rejected message FAILURE");
        return;
    }
    const json_t* state = json_getProperty(json, SM_IOT_COMMON_STATE_FIELD);
    if(!state || JSON_OBJ != json_getType(state)){
        LOG_ERR(TAG, "BP rejected message missing state field");
        return;
    }
    const json_t* reported = json_getProperty(state, SM_IOT_COMMON_REPORTED_FIELD);
    if(!reported || JSON_OBJ != json_getType(reported)){
        LOG_ERR(TAG, "BP rejected message missing reported field");
        return;
    }
    const json_t* battery = json_getProperty(reported, "battery");
    if(!battery || JSON_OBJ != json_getType(battery)){
        LOG_ERR(TAG, "BP rejected message missing battery field");
        return;
    }
    const json_t* bp_state = json_getProperty(battery, SM_IOT_COMMON_STATE_FIELD);
    if(!bp_state || JSON_INTEGER != json_getType(bp_state)){
        LOG_ERR(TAG, "BP rejected message missing BP state field");
        return;
    }
    uint8_t bp_state_value = json_getInteger(bp_state);
    if(bp_state_value != SM_BP_REJECTED){
        LOG_WRN(TAG, "BP State in BP rejected message is FAILURE");
        return;
    }

    const json_t* bp_sn = json_getProperty(battery, SM_IOT_COMMON_SN_FIELD);
    if(!bp_sn || JSON_TEXT != json_getType(bp_sn)){
        LOG_ERR(TAG, "BP rejected message missing BP SN field");
        return;
    }

    if(_impl(_arg)->m_iot_event && _impl(_arg)->m_iot_event->on_bp_event){
        _impl(_arg)->m_iot_event->on_bp_event(json_getValue(bp_sn),
                                              SM_BP_REJECTED,
                                              _impl(_arg)->m_event_arg);
    }
}

static void sm_topic_bp_accepted(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "BP accepted message FAILURE");
        return;
    }
    const json_t* state = json_getProperty(json, SM_IOT_COMMON_STATE_FIELD);
    if(!state || JSON_OBJ != json_getType(state)){
        LOG_ERR(TAG, "BP accepted message missing state field");
        return;
    }
    const json_t* reported = json_getProperty(state, SM_IOT_COMMON_REPORTED_FIELD);
    if(!reported || JSON_OBJ != json_getType(reported)){
        LOG_ERR(TAG, "BP accepted message missing reported field");
        return;
    }
    const json_t* battery = json_getProperty(reported, "battery");
    if(!battery || JSON_OBJ != json_getType(battery)){
        LOG_ERR(TAG, "BP accepted message missing battery field");
        return;
    }

    const json_t* bp_sn = json_getProperty(battery, SM_IOT_COMMON_SN_FIELD);
    if(!bp_sn || JSON_TEXT != json_getType(bp_sn)){
        LOG_ERR(TAG, "BP rejected message missing BP SN field");
        return;
    }

    if(_impl(_arg)->m_iot_event && _impl(_arg)->m_iot_event->on_bp_event){
        _impl(_arg)->m_iot_event->on_bp_event(json_getValue(bp_sn),
                                              SM_BP_ACCEPTED,
                                              _impl(_arg)->m_event_arg);
    }
}

static int32_t sm_sv_iot_build_ota_ping_response(char* _buf,
                                                 const char* _msg_id,
                                                 const char* _new_version,
                                                 bool _is_busy){
    char* p = _buf;

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_MSG_ID_FIELD, _msg_id);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_OTA_TYPE_PING_RESPONSE);

    p = json_objOpen(p, SM_IOT_DATA_FIELD);
    p = json_bool(p, SM_IOT_BUSY_FIELD, _is_busy);
    p = json_str(p, SM_IOT_VERSION_FIELD, _new_version);
    p = json_objClose(p);

    p = json_objClose(p);
    p = json_end(p);

    return (int32_t)(p - _buf);
}

static void sm_topic_ota_request_upgrade_handle(sm_mqtt_msg_t* _msg, void* _arg){
    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "OTA request upgrade message FAILURE. Format invalid");
        return;
    }

    const json_t* msg_id = json_getProperty(json, SM_IOT_MSG_ID_FIELD);
    if(!msg_id || JSON_TEXT != json_getType(msg_id)){
        LOG_ERR(TAG, "Missing message id on request upgrade");
        return;
    }

    const json_t* data_obj = json_getProperty(json, SM_IOT_DATA_FIELD);
    if(!data_obj || JSON_OBJ != json_getType(data_obj)){
        LOG_ERR(TAG, "Missing ev_data field on request upgrade");
        return;
    }

    const json_t* new_version_obj = json_getProperty(data_obj, SM_IOT_FW_NEW_VERSION_FIELD);
    if(!new_version_obj || JSON_TEXT != json_getType(new_version_obj)){
        LOG_ERR(TAG, "Missing new_version field on request upgrade");
        return;
    }

    bool is_busy = false;
    char buf[128];
    memset(buf, '\0', 128);

    if(_impl(_arg)->m_iot_event && _impl(_arg)->m_iot_event->on_ev_request_upgrade){
        is_busy = _impl(_arg)->m_iot_event->on_ev_request_upgrade(_impl(_arg)->m_event_arg);
    }else{
        is_busy = true;
    }

    int32_t len = sm_sv_iot_build_ota_ping_response(buf,
                                                    json_getValue(msg_id),
                                                    json_getValue(new_version_obj),
                                                    is_busy);
    if(len > 0){
        LOG_DBG(TAG, "OTA Ping response. EV is %s", is_busy ? "BUSY" : "FREE");
        sm_iot_publish_data(_impl(_arg),
                            buf,
                            sm_iot_get_topic_ota_request_upgrade_response());
    }
}

static int32_t sm_sv_iot_build_ota_fw_info_response(char *_buf,
                                                    const char *_msg_id,
                                                    const char **_module_name,
                                                    const char **_new_version,
                                                    const char **_current_version,
                                                    const int32_t *_bypass) {
    char *p = _buf;

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_MSG_ID_FIELD, _msg_id);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_OTA_TYPE_FW_INFO_RESPONSE);

    p = json_objOpen(p, SM_IOT_DATA_FIELD);
    p = json_arrOpen(p, SM_IOT_FW_MODULES_FILED);

    for (int index = 0; index < 3; index++) {
        if(_module_name[index] == NULL){
            continue;
        }
        p = json_objOpen(p, NULL);
        p = json_str(p, SM_IOT_FW_MODULE_FIELD, _module_name[index]);
        p = json_str(p, SM_IOT_FW_NEW_VERSION_FIELD, _new_version[index]);
        if(_current_version[index] != NULL){
            p = json_str(p, SM_IOT_FW_OLD_VERSION_FIELD, _current_version[index]);
        }
        p = json_bool(p, "bypass", _bypass[index]);
        p = json_objClose(p);
    }

    p = json_arrClose(p);
    p = json_objClose(p);

    p = json_objClose(p);
    p = json_end(p);

    return (int32_t) (p - _buf);
}

static void sm_topic_ota_fw_info_handle(sm_mqtt_msg_t* _msg, void* _arg){
    if(!_impl(_arg)->m_iot_event || !_impl(_arg)->m_iot_event->on_ev_new_fw){
        LOG_ERR(TAG, "No OTA firmware info handle");
        return;
    }

    json_t mem[SM_IOT_CMD_MAX];
    const json_t* json = json_create(_msg->m_payload, mem, ARRAY_SIZE(mem));
    if(!json){
        LOG_ERR(TAG, "OTA request upgrade message FAILURE. Format invalid");
        return;
    }

    const json_t* msg_id = json_getProperty(json, SM_IOT_MSG_ID_FIELD);
    if(!msg_id || JSON_TEXT != json_getType(msg_id)){
        LOG_ERR(TAG, "Missing message id on request upgrade");
        return;
    }

    const json_t* data_obj = json_getProperty(json, SM_IOT_DATA_FIELD);
    if(!data_obj || JSON_OBJ != json_getType(data_obj)){
        LOG_ERR(TAG, "Missing ev_data field on request upgrade");
        return;
    }

    const json_t* modules_obj = json_getProperty(data_obj, SM_IOT_FW_MODULES_FILED);
    if(!modules_obj || JSON_ARRAY != json_getType(modules_obj)){
        LOG_ERR(TAG, "Missing modules_obj field on request upgrade");
        return;
    }

    const json_t* module;
    int32_t bypass[3] = {true, true, true};
    const char* current_version[3] = {NULL, NULL, NULL};
    const char* new_version[3] = {NULL, NULL, NULL};
    const char* module_name[3] = {NULL, NULL, NULL};
    int32_t index = 0;

    for(module = json_getChild(modules_obj); module != NULL; module = json_getSibling(module)){
        if(JSON_OBJ == json_getType(module)){
            new_version[index] =  json_getPropertyValue(module, SM_IOT_FW_NEW_VERSION_FIELD),
            module_name[index] = json_getPropertyValue(module, SM_IOT_FW_MODULE_FIELD);
            current_version[index] = _impl(_arg)->m_iot_event->on_ev_new_fw(module_name[index],
                                                                            new_version[index],
                                                                            json_getInteger(json_getProperty(module, SM_IOT_FW_SIZE_FIELD)),
                                                                            json_getInteger(json_getProperty(module, SM_IOT_FW_CRC_FIELD)),
                                                                            json_getPropertyValue(module, SM_IOT_FW_LINK_FIELD),
                                                                            &bypass[index],
                                                                            _impl(_arg)->m_event_arg);

            index++;
        }
    }

    char buffer[640];
    memset(buffer, '\0', 640);
    int32_t len = sm_sv_iot_build_ota_fw_info_response(buffer,
                                                       json_getValue(msg_id),
                                                       module_name,
                                                       new_version,
                                                       current_version,
                                                       bypass);

    if(len > 0){
        LOG_DBG(TAG, "OTA FW info response");
        sm_iot_publish_data(_impl(_arg),
                            buffer,
                            sm_iot_get_topic_ota_fw_info_response());
    }

    if(_impl(_arg)->m_iot_event->on_ev_finish_new_fw_extract){
        _impl(_arg)->m_iot_event->on_ev_finish_new_fw_extract(index, _impl(_arg)->m_event_arg);
    }


}
/********************************* END ******************************************/

int32_t sm_sv_iot_reset(sm_sv_iot_t* _this){
    if (!_this) {
        return -1;
    }
    for (int i = 0; i < SM_IOT_SUB_TOPIC_NUMBER; ++i) {
        sm_mqtt_unsubscribes(_impl(_this)->m_mqtt_client,
                             _impl(_this)->m_sub_topics[i].m_topic,
                             sm_mqtt_on_unsubscribed,
                             _this);
    }
    sm_mqtt_disconnect(_impl(_this)->m_mqtt_client, sm_mqtt_on_disconnected, _this);
    _impl(_this)->m_state = SM_IOT_INITIALIZED;
    elapsed_timer_resetz(&_impl(_this)->m_timeout, SM_IOT_INITIALIZED_TIMEOUT);

    return 0;
}

int32_t sm_sv_iot_notify_ota_download_status(sm_sv_iot_t *_this,
                                             const char *_module,
                                             int32_t _total_frame,
                                             int32_t _frame_index,
                                             bool _last_frame,
                                             int32_t _err,
                                             const char *_err_msg) {
    if (!_this) {
        return -1;
    }
    char buf[256];
    memset(buf, '\0', 256);
    char* p = buf;

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_MSG_ID_FIELD, SM_IOT_MSG_ID_DEFAULT);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_OTA_TYPE_DOWNLOAD_STATUS);

    p = json_objOpen(p, SM_IOT_DATA_FIELD);
    p = json_str(p, SM_IOT_FW_MODULE_FIELD, _module);
    p = json_int(p, SM_IOT_ERROR_FIELD, _err);
    p = json_str(p, SM_IOT_ERROR_MSG_FIELD, _err_msg);
    p = json_int(p, SM_IOT_FW_TOTAL_FRAME_FIELD, _total_frame);
    p = json_int(p, SM_IOT_FW_DOWNLOADED_FRAME_FIELD, _frame_index);
    p = json_bool(p, SM_IOT_FW_DOWNLOADED_IS_DONE_FIELD, _last_frame == 0 ? false : true);
    p = json_objClose(p);

    p = json_objClose(p);
    p = json_end(p);

#ifdef __RTOS
   ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
    memcpy(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_payload, buf, strlen(buf));
    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_topic = sm_iot_get_topic_ota_download_status();
    _impl(_this)->m_queue_head++;
    if(_impl(_this)->m_queue_head >= SM_IOT_PUB_QUEUE){
        _impl(_this)->m_queue_head = 0;
    }
#ifdef __RTOS
   EXIT_CRITICAL(_impl(_this)->m_lock);
#endif

    return 0;
}

int32_t sm_sv_iot_notify_ota_upgrade_status(sm_sv_iot_t *_this,
                                            const char *_module,
                                            const char* _version,
                                            int32_t _err,
                                            const char *_err_msg) {
    if (!_this) {
        return -1;
    }
    char buf[256];
    char* p = buf;
    memset(buf, '\0', 256);

    p = json_objOpen(p, NULL);
    p = json_str(p, SM_IOT_MSG_ID_FIELD, SM_IOT_MSG_ID_DEFAULT);
    p = json_str(p, SM_IOT_TYPE_FIELD, SM_IOT_OTA_TYPE_UPGRADE_STATUS);

    p = json_objOpen(p, SM_IOT_DATA_FIELD);
    p = json_str(p, SM_IOT_FW_MODULE_FIELD, _module);
    p = json_int(p, SM_IOT_ERROR_FIELD, _err);
    p = json_str(p, SM_IOT_ERROR_MSG_FIELD, _err_msg);
    p = json_str(p, SM_IOT_VERSION_FIELD, _version);
    p = json_objClose(p);

    p = json_objClose(p);
    p = json_end(p);

#ifdef __RTOS
   ENTER_CRITICAL(_impl(_this)->m_lock);
#endif
    memcpy(_impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_payload, buf, strlen(buf));
    _impl(_this)->m_pub_topics[_impl(_this)->m_queue_head].m_topic = sm_iot_get_topic_ota_upgrade_status();
    _impl(_this)->m_queue_head++;
    if(_impl(_this)->m_queue_head >= SM_IOT_PUB_QUEUE){
        _impl(_this)->m_queue_head = 0;
    }
#ifdef __RTOS
   EXIT_CRITICAL(_impl(_this)->m_lock);
#endif

    return 0;
}
