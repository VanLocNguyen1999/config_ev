//
// Created by admin on 9/25/2024.
//



#ifndef BLE_S_SERVICE_H
#define BLE_S_SERVICE_H

#include "sm_types.h"
#include "sm_ble_slave.h"
#include "sm_host_api.h"

#define BLE_MAX_DEVICE_PAIRED               1
#define BLE_DEVICE_PAIRED_NAME_MAX          32

typedef sm_host_api_t sm_sv_ble_t;

typedef struct {
    void        (*on_new_device_paired)(char const*, void*);            //  RESPONSE REQUIRED
    void        (*on_new_ble_id)(uint32_t, void*);                      //  RESPONSE REQUIRED
}sm_sv_ble_event_t;

typedef struct {
    char            *m_name;
    char* 			(m_device_paired)[BLE_MAX_DEVICE_PAIRED];
}sm_sv_ble_info_t;

sm_sv_ble_t* sm_sv_ble_create(sm_ble_slave_t* _ble_slave,
                                sm_sv_ble_info_t *_ble_info,
                                sm_sv_ble_event_t *_sv_ble_event,
                                void *_arg);

const char* sm_sv_ble_get_uuid_master(sm_sv_ble_t* _this);

int32_t sm_sv_ble_reset(sm_sv_ble_t* _this);

int32_t sm_sv_ble_send_response_write_cmd(sm_sv_ble_t* _this, const char* _object, const uint8_t _state);

int32_t sm_sv_ble_send_response_read_cmd(sm_sv_ble_t* _this, char* _data);

#endif //BLE_S_SERVICE_H
