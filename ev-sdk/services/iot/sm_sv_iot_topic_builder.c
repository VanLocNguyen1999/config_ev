//
// Created by vnbk on 04/09/2024.
//
#include "sm_sv_iot_topic_builder.h"
#include "sm_logger.h"
#include "sm_string_t.h"

#define TAG "IOT_TOPIC_BUILDER"

#define SM_IOT_TOPIC_EV_UPDATE "selex/%s/%s/shadow/update"
#define SM_IOT_TOPIC_DELTA     "selex/%s/%s/shadow/update/delta"
#define SM_IOT_TOPIC_CMD       "selex/%s/%s/command"
#define SM_IOT_TOPIC_CONFIG    "selex/%s/%s/config"
#define SM_IOT_TOPIC_PING      "selex/%s/%s/shadow/get"

#define SM_IOT_TOPIC_BP_UPDATE  "selex/%s/%s/shadow/bulk/bp/update"
#define SM_IOT_TOPIC_BP_EVENT   "selex/%s/%s/shadow/bulk/bp/event"
#define SM_IOT_TOPIC_BP_REJECTED  "selex/%s/%s/shadow/bulk/bp/event/rejected"
#define SM_IOT_TOPIC_BP_ACCEPTED  "selex/%s/%s/shadow/bulk/bp/event/accepted"
#define SM_IOT_TOPIC_BP_CMD     "selex/ev/%s/shadow/bulk/bp/command"

#define SM_IOT_TOPIC_OTA_REQUEST_UPGRADE            "ota/%s/%s/%s/ping_request"             // ota/ev/s2/NADE0001/...
#define SM_IOT_TOPIC_OTA_REQUEST_UPGRADE_RESPONSE   "ota/%s/%s/%s/ping_response"
#define SM_IOT_TOPIC_OTA_FW_INFO                    "ota/%s/%s/%s/fw_info"
#define SM_IOT_TOPIC_OTA_FW_INFO_RESPONSE           "ota/%s/%s/%s/fw_info_response"
#define SM_IOT_TOPIC_OTA_DOWNLOAD_STATUS            "ota/%s/%s/%s/download_status"
#define SM_IOT_TOPIC_OTA_UPGRADE_STATUS             "ota/%s/%s/%s/upgrading_status"

static string_t* g_topic_update = NULL;
static string_t* g_topic_bp_update = NULL;
static string_t* g_topic_delta = NULL;
static string_t* g_topic_cmd = NULL;
static string_t* g_topic_ping = NULL;
static string_t* g_topic_config = NULL;
static string_t* g_topic_bp_event = NULL;
static string_t* g_topic_bp_rejected = NULL;
static string_t* g_topic_bp_accepted = NULL;
//static string_t* g_topic_bp_cmd = NULL;

static string_t* g_topic_ev_ota_request_upgrade = NULL;
static string_t* g_topic_ev_ota_request_upgrade_response = NULL;
static string_t* g_topic_ev_ota_fw_info = NULL;
static string_t* g_topic_ev_ota_fw_info_response = NULL;
static string_t* g_topic_ev_ota_download_status = NULL;
static string_t* g_topic_ev_ota_upgrade_status = NULL;

/*
typedef struct {
    string_t* m_topic_bp_ota_request_upgrade;
    string_t* m_topic_bp_ota_request_upgrade_response;
    string_t* m_topic_bp_ota_fw_info;
    string_t* m_topic_bp_ota_fw_info_response ;
    string_t* m_topic_bp_ota_download_status;
    string_t* m_topic_bp_ota_upgrade_status;
}sm_topic_ota_bp_t;

static sm_topic_ota_bp_t g_topic_ota_bp[3] = {
        {
            .m_topic_bp_ota_request_upgrade = NULL,
            .m_topic_bp_ota_fw_info = NULL,
            .m_topic_bp_ota_fw_info_response = NULL,
            .m_topic_bp_ota_request_upgrade_response = NULL,
            .m_topic_bp_ota_upgrade_status = NULL,
            .m_topic_bp_ota_download_status = NULL,
        },
        {
                .m_topic_bp_ota_request_upgrade = NULL,
                .m_topic_bp_ota_fw_info = NULL,
                .m_topic_bp_ota_fw_info_response = NULL,
                .m_topic_bp_ota_request_upgrade_response = NULL,
                .m_topic_bp_ota_upgrade_status = NULL,
                .m_topic_bp_ota_download_status = NULL,
        },
        {
                .m_topic_bp_ota_request_upgrade = NULL,
                .m_topic_bp_ota_fw_info = NULL,
                .m_topic_bp_ota_fw_info_response = NULL,
                .m_topic_bp_ota_request_upgrade_response = NULL,
                .m_topic_bp_ota_upgrade_status = NULL,
                .m_topic_bp_ota_download_status = NULL,
        },
};
 */

const char* sm_iot_get_topic_update(){
    return g_topic_update->buffer;
}
const char* sm_iot_get_topic_bp_update(){
    return g_topic_bp_update->buffer;
}
const char* sm_iot_get_topic_delta(){
    return g_topic_delta->buffer;
}
const char* sm_iot_get_topic_cmd(){
    return g_topic_cmd->buffer;
}

const char* sm_iot_get_topic_ping(){
    return g_topic_ping->buffer;
}

const char* sm_iot_get_topic_config(){
    return g_topic_config->buffer;
}

const char* sm_iot_get_topic_bp_event(){
    return g_topic_bp_event->buffer;
}

const char* sm_iot_get_topic_bp_rejected(){
    return g_topic_bp_rejected->buffer;
}

const char* sm_iot_get_topic_bp_accepted(){
    return g_topic_bp_accepted->buffer;
}

/*const char* sm_iot_get_topic_bp_cmd(){
    return g_topic_bp_cmd->buffer;
}*/

const char* sm_iot_get_topic_ota_request_upgrade(){
    return g_topic_ev_ota_request_upgrade->buffer;
}
const char* sm_iot_get_topic_ota_request_upgrade_response(){
    return g_topic_ev_ota_request_upgrade_response->buffer;
}
const char* sm_iot_get_topic_ota_fw_info(){
    return g_topic_ev_ota_fw_info->buffer;
}
const char* sm_iot_get_topic_ota_fw_info_response(){
    return g_topic_ev_ota_fw_info_response->buffer;
}
const char* sm_iot_get_topic_ota_download_status(){
    return g_topic_ev_ota_download_status->buffer;
}
const char* sm_iot_get_topic_ota_upgrade_status(){
    return g_topic_ev_ota_upgrade_status->buffer;
}

void sm_iot_topic_create_all(const char* _dev_type, const char* _model, const char *_dev_sn){
    char buffer[128] = {'\0',};
    const char* dev_type = NULL;
    if(!_dev_type){
        dev_type = SM_IOT_DEVICE_TYPE_EV;
    }else{
    	dev_type = _dev_type;
    }

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_EV_UPDATE, dev_type, _dev_sn);
    g_topic_update = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_BP_UPDATE, dev_type, _dev_sn);
    g_topic_bp_update = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_DELTA, dev_type,  _dev_sn);
    g_topic_delta = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_CMD, dev_type,  _dev_sn);
    g_topic_cmd = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_BP_EVENT, dev_type,  _dev_sn);
    g_topic_bp_event = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_BP_REJECTED, dev_type, _dev_sn);
    g_topic_bp_rejected = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_BP_ACCEPTED, dev_type, _dev_sn);
    g_topic_bp_accepted = string_createz(buffer);

/*    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_BP_CMD, dev_type, _dev_sn);
    g_topic_bp_cmd = string_createz(buffer);*/

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_PING, dev_type, _dev_sn);
    g_topic_ping = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer,SM_IOT_TOPIC_CONFIG, dev_type,  _dev_sn);
    g_topic_config = string_createz(buffer);
}

void sm_iot_topic_ota_create_all(const char* _dev_type, const char* _model, const char* _sn) {
    char buffer[128] = {'\0',};
    const char* dev_type = NULL;
    const char* dev_model = NULL;
    if (_sn == NULL){
        return;
    }
    if(_dev_type == NULL){
        dev_type = SM_IOT_DEVICE_TYPE_EV;
    }else{
    	dev_type = _dev_type;
    }

    if(_model == NULL){
        dev_model = SM_IOT_MODEL_TYPE_S2;
    }else{
    	dev_model = _model;
    }

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_REQUEST_UPGRADE, dev_type, dev_model, _sn);
    g_topic_ev_ota_request_upgrade = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_REQUEST_UPGRADE_RESPONSE, dev_type, dev_model, _sn);
    g_topic_ev_ota_request_upgrade_response = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_FW_INFO, dev_type, dev_model, _sn);
    g_topic_ev_ota_fw_info = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_FW_INFO_RESPONSE, dev_type, dev_model, _sn);
    g_topic_ev_ota_fw_info_response = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_UPGRADE_STATUS, dev_type, dev_model, _sn);
    g_topic_ev_ota_upgrade_status = string_createz(buffer);

    memset(buffer, '\0', 128);
    sprintf(buffer, SM_IOT_TOPIC_OTA_DOWNLOAD_STATUS, dev_type, dev_model, _sn);
    g_topic_ev_ota_download_status = string_createz(buffer);
}
