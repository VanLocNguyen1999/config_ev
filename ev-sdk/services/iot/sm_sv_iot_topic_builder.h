#ifndef EV_SDK_SM_SV_IOT_TOPIC_BUILDER_H
#define EV_SDK_SM_SV_IOT_TOPIC_BUILDER_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_sv_iot_define.h"

#define SM_IOT_QOS_DEFAULT      1
#define SM_IOT_RETAIN_DEFAULT   false

#define SM_IOT_SUB_TOPIC_NUMBER    10

void sm_iot_topic_create_all(const char* _dev_type, const char* _model, const char *_sn);
void sm_iot_topic_ota_create_all(const char* _dev_type, const char* _model, const char* _sn);

const char* sm_iot_get_topic_update();
const char* sm_iot_get_topic_delta();
const char* sm_iot_get_topic_cmd();
const char* sm_iot_get_topic_ping();
const char* sm_iot_get_topic_config();

const char* sm_iot_get_topic_bp_update();
const char* sm_iot_get_topic_bp_event();
const char* sm_iot_get_topic_bp_rejected();
const char* sm_iot_get_topic_bp_accepted();
//const char* sm_iot_get_topic_bp_cmd();

const char* sm_iot_get_topic_ota_request_upgrade();
const char* sm_iot_get_topic_ota_request_upgrade_response();
const char* sm_iot_get_topic_ota_fw_info();
const char* sm_iot_get_topic_ota_fw_info_response();
const char* sm_iot_get_topic_ota_download_status();
const char* sm_iot_get_topic_ota_upgrade_status();

#ifdef __cplusplus
};
#endif

#endif
