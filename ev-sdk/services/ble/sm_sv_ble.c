//
// Created by admin on 9/26/2024.
//
#include <stdio.h>
#include <string.h>

#include "sm_sv_ble.h"
#include "sm_ble_slave.h"
#include "sm_elapsed_timer.h"
#include "sm_hal_delay.h"
#include "sm_logger.h"
#include "sm_sv_ble_define.h"
#include "tiny-json/tiny-json.h"
#include "json-maker/json-maker.h"
#include "sm_ev_cmd.h"
#include "sm_string_util.h"


#define BLE_SYNC_TIMEOUT            0
#define BLE_PAIRING_TIMEOUT         (10*1000)
#define BLE_KEY_EXISTING_TIMEOUT    (60*1000)
#define BLE_DISCONNECT_TIMEOUT      500

#define BLE_LOCK_CONNECTION_TIMEOUT (15*60*1000)

#define SM_BLE_CMD_JSON_OBJECT_MAX       8
#define SM_BLE_RETRY_PAIRING_MAX        10
#define SM_BLE_KEY_RANDOM_A         287835
#define SM_BLE_KEY_RANDOM_B         166452
#define SM_BLE_KEY_RANDOM_C         139042

const static char* TAG = "SV_BLE";

enum {
    FIELD_WRITE = 'W',
    FIELD_READ = 'R',
};

enum {
    BLE_EV_FIELD   = 'E',           // Vehicle field
    BLE_BP_FIELD   = 'B',           // Battery field
    BLE_NOTI_FIELD = 'N'            // Notification field (Message, Call)
};

typedef enum {
    SM_BLE_INIT,
    SM_BLE_PAIRING,
    SM_BLE_AUTH,
    SM_BLE_CONNECTED,
    SM_BLE_DISCONNECTED,
    SM_BLE_LOCKED,
} BLE_STATE;

typedef struct {
    sm_host_api_t       m_base;
    sm_ble_slave_t      *m_ble_slave;
    sm_sv_ble_info_t    *m_info;
    BLE_STATE           m_state;
    uint32_t            m_ble_id;
    uint8_t             m_retry_pairing;
    sm_sv_ble_event_t   *m_ble_event;
    void                *m_event_arg;
    char                m_ble_master_id[BLE_DEVICE_PAIRED_NAME_MAX];
    elapsed_timer_t 	m_timeout;

} sm_sv_ble_impl_t;
#define impl(x) ((sm_sv_ble_impl_t *)(x))

static int32_t sm_sv_ble_init(sm_host_api_t*, sm_host_sync_data_if_t*, int32_t _sync_time);
static int32_t sm_sv_ble_free(sm_host_api_t* _this);
static int32_t sm_sv_ble_is_connected(sm_host_api_t*);
static int32_t sm_sv_ble_update(sm_host_api_t* _this, const char* _key, const char* _value);
static int32_t sm_sv_ble_updates(sm_host_api_t* _this, void* _stream);
static int32_t sm_sv_ble_push_event_to_host(sm_host_api_t* _this, void* _data_event);
static int32_t sm_sv_ble_process(sm_host_api_t* _this);

static uint32_t ble_gen_key();
static uint32_t rand_n(uint32_t x);

static uint32_t rand_n(uint32_t x) {
    return (SM_BLE_KEY_RANDOM_A * x + SM_BLE_KEY_RANDOM_B) % 1000000;
}
static uint32_t ble_gen_key() {
    uint32_t raw_key = 0;
    raw_key += get_tick_count()%1000000;
    while (raw_key < 100000){
        raw_key = rand_n(raw_key);
    }
    return raw_key;
}

bool ble_ev_id_is_exist(sm_host_api_t* _this, const char* _master_id) {
    for (int i=0; i < BLE_MAX_DEVICE_PAIRED; i++) {
        if (!strcmp(impl(_this)->m_info->m_device_paired[i], _master_id)) {
            return true;
        }
    }
    return false;
}

void ble_on_connected(sm_host_api_t* _this) {
    LOG_DBG(TAG, "on ble module connected");

    if (impl(_this)->m_state == SM_BLE_INIT) {
        impl(_this)->m_state = SM_BLE_PAIRING;
        elapsed_timer_resetz(&impl(_this)->m_timeout, BLE_PAIRING_TIMEOUT);
    }
}
void ble_on_disconnect(sm_host_api_t* _this) {
    LOG_DBG(TAG, "on ble module connected");

    if (impl(_this)->m_state == SM_BLE_CONNECTED ||
        impl(_this)->m_state == SM_BLE_PAIRING ||
        impl(_this)->m_state == SM_BLE_AUTH) {
        impl(_this)->m_state = SM_BLE_DISCONNECTED;
        elapsed_timer_resetz(&impl(_this)->m_timeout, BLE_DISCONNECT_TIMEOUT);
    }
}

void ble_on_data_received(sm_host_api_t *_this, char *_data, uint32_t _len) {
    sm_sv_ble_impl_t *this = impl(_this);

    static char buff[256];
    memset(buff, 0, sizeof(buff));
    memcpy(buff, _data + 1, _len - 2); // remove '+' and '*'
    buff[_len - 2] = '\0';

    json_t mem[SM_BLE_CMD_JSON_OBJECT_MAX];
    const json_t *json = json_create((char *) buff, mem, ARRAY_SIZE(mem));
    if (!json) {
        LOG_ERR(TAG, "CMD message FAILURE");
        return;
    }
    const json_t *type_obj = json_getProperty(json, SM_SV_BLE_TYPE_FIELD);
    if (!type_obj || JSON_TEXT != json_getType(type_obj)) {
        LOG_ERR(TAG, "CMD message missing \"type\" field");
        return;
    }
    const char *type_cmd;
    type_cmd = json_getValue(type_obj);

    const json_t *field_obj = json_getProperty(json, SM_SV_BLE_FIELD_FIELD);
    if (!field_obj || JSON_TEXT != json_getType(field_obj)) {
        LOG_ERR(TAG, "CMD message missing \"field\" field");
        return;
    }
    const char *field_cmd;
    field_cmd = json_getValue(field_obj);

    const json_t *data_obj = json_getProperty(json, SM_SV_BLE_DATA_FIELD);
    if (!data_obj || JSON_OBJ != json_getType(data_obj)) {
        LOG_ERR(TAG, "CMD message missing \"data\" field");
        return;
    }

    const char *object = NULL;
    const json_t *cmd_value;
    int value_int = 0;
    const char *value_str = NULL;

    object = json_getPropertyValue(data_obj, SM_SV_BLE_OBJECT_FIELD);

    if (!object) {
        LOG_ERR(TAG, "CMD message \"data\" field is not enough data");
        return;
    }

    // Data is all valid
    if (type_cmd[0] == FIELD_WRITE) {
        // Write cmd
        cmd_value = json_getProperty(data_obj, SM_SV_BLE_VALUE_FIELD);
        if (!cmd_value) {
            LOG_ERR(TAG, "CMD message \"data\" field is not enough data");
            return;
        }
        if (field_cmd[0] == BLE_EV_FIELD) {
            if (!strcmp(object, SM_SV_BLE_OBJECT_EV_ID)) {
                LOG_DBG(TAG, "Receive CMD EV_ID");
                if (this->m_state != SM_BLE_PAIRING) {
                    sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_EV_ID, 2);
                    return;
                }
                const char *master_id = json_getPropertyValue(data_obj, SM_SV_BLE_OBJECT_UUID_FIELD);
                if (master_id) {
                    LOG_DBG(TAG, "UUID : %s", master_id);
                    memcpy(this->m_ble_master_id, master_id, strlen(master_id));
                    if (ble_ev_id_is_exist(_this, master_id)) {
                    	sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_EV_ID, 1);
                        this->m_state = SM_BLE_CONNECTED;
                        if (this->m_base.m_host_event->on_connected) {
                            this->m_base.m_host_event->on_connected(1, this->m_base.m_host_event_arg);
                        }
                        return;
                    } else {
                        sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_EV_ID, 0);
                    }
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_ID_MAKER)) {
                if (this->m_state != SM_BLE_PAIRING && this->m_state != SM_BLE_AUTH) {
                    return;
                }
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_str = json_getValue(cmd_value);
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                if (!strcmp(value_str, "new")) {
                    sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_ID_MAKER, 1);
                    this->m_ble_id = ble_gen_key();
                    if (this->m_ble_event->on_new_ble_id) {
                        this->m_ble_event->on_new_ble_id(this->m_ble_id, this->m_event_arg);
                    }
                    this->m_retry_pairing++;
                    this->m_state = SM_BLE_AUTH;
                    elapsed_timer_resetz(&this->m_timeout, BLE_KEY_EXISTING_TIMEOUT);
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_ID_MAKER, 0);
                    return;
                }
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_BLE_ID)) {
                LOG_DBG(TAG, "Receive CMD BLE_ID");
                if (this->m_state != SM_BLE_AUTH) {
                    return;
                }
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_str = json_getValue(cmd_value);
                } else {
                    LOG_ERR(TAG, "CMD message \"data\" field is not enough data");
                    sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_BLE_ID, 0);
                    return;
                }
                value_int = sm_atoi(value_str);
                if (value_int == this->m_ble_id) {
                	sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_BLE_ID, 1);
                    if (this->m_base.m_host_event->on_connected) {
                        this->m_base.m_host_event->on_connected(1, this->m_base.m_host_event_arg);
                    }
                    if (this->m_ble_event->on_new_device_paired) {
                        this->m_ble_event->on_new_device_paired(this->m_ble_master_id, this->m_event_arg);
                    }
                    this->m_state = SM_BLE_CONNECTED;
                } else {
                	sm_sv_ble_send_response_write_cmd(_this, SM_SV_BLE_OBJECT_BLE_ID, 0);
                    if (this->m_retry_pairing++ >= SM_BLE_RETRY_PAIRING_MAX) {
                        this->m_state = SM_BLE_DISCONNECTED;
                        elapsed_timer_resetz(&this->m_timeout, BLE_DISCONNECT_TIMEOUT);
                        return;
                    }
                    this->m_ble_id = 0;
                }
                return;
            }
            //-----------------CMD-ON-CONNECTING------------------//
            if (this->m_state != SM_BLE_CONNECTED) {
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_FIND_EV)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }

                LOG_DBG(TAG, "Receive CMD FIND EV");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_FIND_EV,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_DRV_MODE)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }

                LOG_DBG(TAG, "Receive CMD DRV_MODE");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_DRIVE_MODE,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_MAX_SPEED)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD SPEED");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_MAX_SPEED,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_HEAD_LIGHT)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD HEAD LIGHT");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_CONTROL_HIGH_BEAM,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_SIGNAL_LIGHT)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD SIGNAL LIGHT");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_CONTROL_SIGNAL,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_HORN)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD SIGNAL LIGHT");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_CONTROL_HORN,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_LOCK_EV)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD LOCK EV");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_LOCK_EV,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_SET_ODO)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD SET ODO");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_ODO,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_DISCONNECT)) {
                LOG_DBG(TAG, "Receive CMD DISCONNECT");
                this->m_state = SM_BLE_DISCONNECTED;
                elapsed_timer_resetz(&this->m_timeout, BLE_DISCONNECT_TIMEOUT);
                return;
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_FORGET_MASTER)) {
                if (JSON_TEXT == json_getType(cmd_value)) {
                    value_int = sm_atoi(json_getValue(cmd_value));
                } else {
                    LOG_ERR(TAG, "CMD message \"value\" field is not enough data");
                    return;
                }
                LOG_DBG(TAG, "Receive CMD FORGET ME NOW");
                if (this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host) {
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_SET_DRIVE_MODE,
                                                                &value_int,
                                                                this->m_base.m_host_event_arg);
                }
                return;
            }

            LOG_WRN(TAG, "CMD message \"object\" field is not accepted");
        } else if (field_cmd[0] == BLE_NOTI_FIELD) {
            if (this->m_state != SM_BLE_CONNECTED) {
                return;
            }
            // Notification
        }
    } else if (type_cmd[0] == FIELD_READ) {
        if (this->m_state != SM_BLE_CONNECTED) {
            return;
        }
        // Read cmd
        if (field_cmd[0] == BLE_EV_FIELD) {
            /*if (!strcmp(object, SM_SV_BLE_OBJECT_FW_VERSION)){
                if(this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host){
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_FW_VERSION,
                                                               NULL,
                                                               this->m_base.m_host_event_arg);
                }
            }
            if (!strcmp(object, SM_SV_BLE_OBJECT_SPEED_PROF)){
                if(this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host){
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_GET_SPEED_PROF,
                                                               NULL,
                                                               this->m_base.m_host_event_arg);
                }
            }
            if (!strcmp(object, "all")){
                if(this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host){
                    this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_EV_DATA,
                                                               NULL,
                                                               this->m_base.m_host_event_arg);
                }
            }*/

            return;
        }
        if (field_cmd[0] == BLE_BP_FIELD) {
            /*value_int = -1;
            value_int = atoi(&field_cmd[1]);
            if(this->m_base.m_host_event && this->m_base.m_host_event->on_cmd_from_host){
                this->m_base.m_host_event->on_cmd_from_host(SM_EV_CMD_BP_DATA,
                                                           &value_int,
                                                           this->m_base.m_host_event_arg);
            }*/
            return;
        }
    }
}

void ble_slave_event_cb(int32_t event, void *data, void *arg) {
    switch (event) {
        case SM_BLE_EVENT_CONNECTED:
            ble_on_connected(arg);
            break;
        case SM_BLE_EVENT_DISCONNECTED:
            ble_on_disconnect(arg);
            break;
        case SM_BLE_EVENT_DATA:
            ble_on_data_received(arg, (char*)data, strlen((char*)data));
            break;
        default:
            break;
     }
}

static sm_host_api_proc_t g_ble_proc_default = {
    .init = sm_sv_ble_init,
    .free = sm_sv_ble_free,
    .sync_pause = sm_host_api_sync_pause,
    .sync_resume = sm_host_api_sync_resume,
    .is_connected = sm_sv_ble_is_connected,
    .reg_event_handle = sm_host_api_reg_event_handle,
    .unreg_event_handle = sm_host_api_unreg_event_handle,
    .update_data_to_host = sm_sv_ble_update,
    .update_datas_to_host = sm_sv_ble_updates,
    .push_event_to_host = sm_sv_ble_push_event_to_host,
    .process = sm_sv_ble_process
};

static sm_sv_ble_impl_t g_ble_service = {
    .m_base = {
        .m_proc = &g_ble_proc_default,
        .m_sync = false,
        .m_sync_time = 0,
        .m_sync_if = NULL,
        .m_host_event = NULL,
        .m_host_event_arg = NULL,
    },
};

sm_sv_ble_t* sm_sv_ble_create(sm_ble_slave_t* _ble_slave,
                                sm_sv_ble_info_t *_ble_info,
                                sm_sv_ble_event_t *_sv_ble_event,
                                void *_arg) {
    if (!_ble_slave || !_ble_info) {
        return NULL;
    }

    g_ble_service.m_ble_slave = _ble_slave;

    g_ble_service.m_info = _ble_info;
    g_ble_service.m_ble_event = _sv_ble_event;
    g_ble_service.m_event_arg = _arg;
    g_ble_service.m_retry_pairing = 0;
    g_ble_service.m_state = SM_BLE_INIT;

    elapsed_timer_resetz(&g_ble_service.m_timeout, BLE_SYNC_TIMEOUT);
    sm_ble_sl_set_event_cb(_ble_slave, ble_slave_event_cb, &g_ble_service);

    return (sm_sv_ble_t*) &g_ble_service;
}

int32_t sm_sv_ble_reset(sm_sv_ble_t* _this) {
    return 0;
}

const char* sm_sv_ble_get_uuid_master(sm_sv_ble_t* _this){
    if(!_this || impl(_this)->m_state != SM_BLE_CONNECTED) {
        return NULL;
    }

    return impl(_this)->m_ble_master_id;
}

int32_t sm_sv_ble_send_response_write_cmd(sm_sv_ble_t* _this, const char* _object, const uint8_t _state) {
    char buff[128];
    memset(buff, 0 ,128);
    char* p = buff;
    p = json_objOpen(p, NULL);
    p = json_str(p, "type", "W");
    p = json_str(p, "field", "E");
    p = json_objOpen(p, "data");
    p = json_str(p, "object", _object);
    p = json_int(p, "state", _state);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);
    LOG_DBG(TAG, "Send to host: %s", buff);
    return sm_ble_sl_send_msg(impl(_this)->m_ble_slave, buff, p-buff);
}

int32_t sm_sv_ble_send_response_read_cmd(sm_sv_ble_t* _this, char* _data) {
    char buff[128];
    memset(buff, 0 ,128);
    char* p = buff;

    p = json_objOpen(p, NULL);
    p = json_str(p, "type", "R");
    p = json_str(p, "field", "E");
    p = json_objOpen(p, "data");
    memcpy(p, _data, strlen(_data));
    p += strlen(_data);
    p = json_objClose(p);
    p = json_objClose(p);
    p = json_end(p);
    LOG_DBG(TAG, "Send to host: %s", buff);
    return sm_ble_sl_send_msg(impl(_this)->m_ble_slave, buff, p-buff);
}

static int32_t sm_sv_ble_init(sm_host_api_t* _this, sm_host_sync_data_if_t* _sync_if, int32_t _sync_time) {
    if(!_this || !_sync_if) {
        return -1;
    }
    sm_sv_ble_impl_t *this = impl(_this);
    sm_host_api_init(_this, _sync_if, _sync_time);

    // TODO: Get Paired device MAC Address

    this->m_state = SM_BLE_INIT;
    return 0;
}

static int32_t sm_sv_ble_free(sm_host_api_t* _this) {
    if(!_this){
        return -1;
    }
    return 0;
}

static int32_t sm_sv_ble_is_connected(sm_host_api_t* _this) {
    if(!_this){
        return -1;
    }
    return (impl(_this)->m_state == SM_BLE_CONNECTED);
}

static int32_t sm_sv_ble_update(sm_host_api_t* _this, const char* _key, const char* _value) {
    if(!_this){
        return -1;
    }

    return 0;
}

static int32_t sm_sv_ble_updates(sm_host_api_t* _this, void* _data) {
    if(!_this){
        return -1;
    }

    return 0;
}

static int32_t sm_sv_ble_push_event_to_host(sm_host_api_t* _this, void* _data_event) {
    if(!_this){
        return -1;
    }

    return 0;
}

static int32_t sm_sv_ble_process(sm_host_api_t* _this) {
    if(!_this){
        return -1;
    }
    sm_sv_ble_impl_t *this = impl(_this);
    sm_ble_sl_process(this->m_ble_slave);
    switch (this->m_state) {
        case SM_BLE_INIT:
            elapsed_timer_resetz(&this->m_timeout, BLE_PAIRING_TIMEOUT);
            break;
        case SM_BLE_PAIRING:
            if (!elapsed_timer_get_remain(&this->m_timeout)) {
                LOG_WRN(TAG, "Didn't receive any pairing request");
                this->m_state = SM_BLE_DISCONNECTED;
                elapsed_timer_resetz(&this->m_timeout, BLE_DISCONNECT_TIMEOUT);
            }
            break;
        case SM_BLE_AUTH:
            if (!elapsed_timer_get_remain(&this->m_timeout)) {
                this->m_ble_id = 0;
                this->m_state = SM_BLE_PAIRING;
                elapsed_timer_resetz(&this->m_timeout, BLE_PAIRING_TIMEOUT);
            }
            if (this->m_retry_pairing >= SM_BLE_RETRY_PAIRING_MAX) {
               this->m_state = SM_BLE_DISCONNECTED;
               elapsed_timer_resetz(&this->m_timeout, BLE_DISCONNECT_TIMEOUT);
            }
            break;
        case SM_BLE_CONNECTED:
            break;
        case SM_BLE_DISCONNECTED:
            if (!elapsed_timer_get_remain(&this->m_timeout)) {
                memset(this->m_ble_master_id, 0, sizeof(this->m_ble_master_id));
                this->m_ble_id = 0;
                sm_ble_sl_disconnect(this->m_ble_slave);
                LOG_DBG(TAG, "BLE disconnecting !");

                if (this->m_base.m_host_event->on_disconnected) {
                    this->m_base.m_host_event->on_disconnected(1, this->m_base.m_host_event_arg);
                }
                if (this->m_retry_pairing >= SM_BLE_RETRY_PAIRING_MAX){
                    this->m_state = SM_BLE_LOCKED;
                    elapsed_timer_resetz(&this->m_timeout, BLE_LOCK_CONNECTION_TIMEOUT);
                }else {
                    this->m_state = SM_BLE_INIT;
                    elapsed_timer_resetz(&this->m_timeout, 0);
                }
            }
            break;
        case SM_BLE_LOCKED:
            if (!elapsed_timer_get_remain(&this->m_timeout)) {
                this->m_state = SM_BLE_INIT;
                this->m_retry_pairing = 0;
            }
            break;
    }
    return 0;
}
