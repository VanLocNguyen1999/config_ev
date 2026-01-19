//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_IOT_H
#define EV_SDK_SM_SV_IOT_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"
#include "sm_mqtt_client.h"
#include "sm_host_api.h"

#include "sm_sv_iot_config.h"
#include "sm_sv_iot_define.h"

#define SM_SV_IOT_SYNC_TIME_DEFAULT     (30) //seconds

enum {
    SM_BP_REJECTED = 0,
    SM_BP_ACCEPTED = 1
};

typedef sm_host_api_t sm_sv_iot_t;

typedef struct {
    int32_t (*on_ev_request_upgrade)(void*);
    const char* (*on_ev_new_fw)(const char*, const char*, int32_t, uint16_t, const char*, int32_t*, void*);
    void (*on_ev_finish_new_fw_extract)(int32_t, void*);
    void (*on_bp_event)(const char*, int32_t, void*);
    int32_t (*on_bp_request_upgrade)(void*);
    int32_t (*on_bp_new_fw)(void*, void*);
    int32_t (*on_cmd_ota_progress)(const char*, void*);
}sm_sv_iot_event_t;

sm_sv_iot_t* sm_sv_iot_create(const char* _sn,
                              sm_mqtt_client_t* _mqtt_client,
                              sm_sv_iot_config_t* _config,
                              sm_sv_iot_event_t* _iot_event,
                              void* _arg);

sm_sv_iot_t* sm_sv_iot_createz(const char* _dev_type,
                              const char* _dev_model,
                              const char* _dev_sn,
                              sm_mqtt_client_t* _mqtt_client,
                              sm_sv_iot_config_t* _config,
                              sm_sv_iot_event_t* _iot_event,
                              void* _arg);

int32_t sm_sv_iot_reset(sm_sv_iot_t* _this);

int32_t sm_sv_iot_notify_ota_upgrade_status(sm_sv_iot_t *_this,
                                            const char *_module,
                                            const char* _version,
                                            int32_t _err,
                                            const char *_err_msg);

int32_t sm_sv_iot_notify_ota_download_status(sm_sv_iot_t *_this,
                                             const char *_module,
                                             int32_t _total_frame,
                                             int32_t _frame_index,
                                             bool _last_frame,
                                             int32_t _err,
                                             const char *_err_msg);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_SV_IOT_H
