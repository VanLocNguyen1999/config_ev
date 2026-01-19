//
// Created by admin on 9/25/2024.
//
#include <string.h>
#include "sm_ble_slave.h"
#include "sm_mdbt42q_ble.h"
#include "sm_elapsed_timer.h"
#include "sm_string_util.h"

#define BLE_DATA_MAX_LEN 512

#define BlE_STATE_CHANGED_TIMEOUT_MS        100
#define BlE_READ_CMD_TIMEOUT_MS             500

#define impl(x)     ((sm_ble_slave_impl_t*)(x))

typedef struct {
    sm_mdbt42q_t        *m_module;
    SM_BLE_MODULE_ST    m_state;
    on_event_cb_fn      m_event_cb;
    void                *m_arg;
    elapsed_timer_t     m_common_timeout;
} sm_ble_slave_impl_t;

static void ble_connected_handle(sm_ble_slave_t* _this);
static void ble_disconnected_handle(sm_ble_slave_t* _this);
static bool ble_rx_data_is_valid(const char* buff);

static sm_ble_slave_impl_t g_ble_impl = {
        .m_module = NULL,
        .m_state = SM_BLE_ST_IDLE,
        .m_event_cb = NULL,
        .m_arg = NULL,
};

sm_ble_slave_t* sm_ble_sl_create(void *_bt_module){
    if (!_bt_module) {
        return NULL;
    }
    sm_ble_slave_impl_t* _this = &g_ble_impl;
    _this->m_module = _bt_module;

    return _this;
}

int32_t sm_ble_sl_init(sm_ble_slave_t* _this, char* _device_name) {
    if (!_this) {
        return -1;
    }

    if(sm_mdbt42q_init(impl(_this)->m_module) < 0) {
        return -1;
    }

    sm_mdbt42q_set_name(impl(_this)->m_module, _device_name);

    impl(_this)->m_state = SM_BLE_ST_IDLE;
    elapsed_timer_resetz(&impl(_this)->m_common_timeout, BlE_STATE_CHANGED_TIMEOUT_MS);

    return 0;
}

int32_t sm_ble_sl_set_event_cb(sm_ble_slave_t* _this, on_event_cb_fn _cb, void *_arg) {
    if (!_this || !_cb) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    this->m_event_cb = _cb;
    this->m_arg = _arg;
    return 0;
}

int32_t sm_ble_sl_advertising(sm_ble_slave_t* _this) {
    if (!_this) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    return sm_mdbt42q_start_advertising(this->m_module);
}

int32_t sm_ble_sl_disconnect(sm_ble_slave_t* _this) {
    if (!_this) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    return sm_mdbt42q_hard_reset(this->m_module);
}

int32_t sm_ble_sl_send_msg(sm_ble_slave_t* _this, char* _msg, int32_t _len) {
    if (!_this) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    return sm_mdbt42q_write_data(this->m_module, _msg, _len);
}

int32_t sm_ble_sl_rename(sm_ble_slave_t* _this, char* _new_name) {
    if (!_this) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    return sm_mdbt42q_set_name(this->m_module, _new_name);
}

int32_t sm_ble_sl_process(sm_ble_slave_t* _this) {
    if (!_this) {
        return -1;
    }
    sm_ble_slave_impl_t* this = impl(_this);
    if (sm_mdbt42q_is_connected(this->m_module)) {
        ble_connected_handle(_this);
    }else {
        ble_disconnected_handle(_this);
    }
    switch (this->m_state) {
        case SM_BLE_ST_IDLE:
            if (!elapsed_timer_get_remain(&this->m_common_timeout)) {
                if (sm_mdbt42q_start_advertising(this->m_module) == 0) {
                    this->m_state = SM_BLE_ST_ADVERTISING;
                }else {
                    this->m_state = SM_BLE_ST_IDLE;
                }
                elapsed_timer_reset(&this->m_common_timeout);
            }
            break;
        case SM_BLE_ST_CONNECTED:
            if (!elapsed_timer_get_remain(&this->m_common_timeout)) {
            	char buff[BLE_DATA_MAX_LEN];
                memset(buff, 0, BLE_DATA_MAX_LEN);
                if (sm_mdbt42q_read_data(this->m_module, buff, BLE_DATA_MAX_LEN) > 0) {
                    if (ble_rx_data_is_valid(buff)) {
                        if (this->m_event_cb) {
                        	char event_data[BLE_DATA_MAX_LEN];
                            memset(event_data, 0, BLE_DATA_MAX_LEN);
                            shortest_substring(buff,
                                        		"+{", "}*",
                                                event_data,
                                                BLE_DATA_MAX_LEN);
                            this->m_event_cb(SM_BLE_EVENT_DATA, event_data, this->m_arg);
                        }
                    }
                }
                elapsed_timer_resetz(&this->m_common_timeout, BlE_READ_CMD_TIMEOUT_MS);
            }
            break;
        case SM_BLE_ST_ADVERTISING:
        case SM_BLE_ST_DISCONNECTED:
        case SM_BLE_ST_SLEEP:
        default:
            break;
    }
    return 0;
}

static void ble_connected_handle(sm_ble_slave_t* _this) {
    sm_ble_slave_impl_t* this = impl(_this);
    switch (this->m_state) {
    case SM_BLE_ST_IDLE:
    case SM_BLE_ST_ADVERTISING:
    case SM_BLE_ST_DISCONNECTED:
        this->m_state = SM_BLE_ST_CONNECTED;
        if (this->m_event_cb) {
            this->m_event_cb(SM_BLE_EVENT_CONNECTED, NULL, this->m_arg);
        }
        break;
    case SM_BLE_ST_CONNECTED:
        default:
        break;
    }
}

static void ble_disconnected_handle(sm_ble_slave_t* _this) {
    sm_ble_slave_impl_t* this = impl(_this);
    switch (this->m_state) {
    case SM_BLE_ST_IDLE:
    case SM_BLE_ST_ADVERTISING:
    case SM_BLE_ST_DISCONNECTED:
        break;
    case SM_BLE_ST_CONNECTED:
        this->m_state = SM_BLE_ST_DISCONNECTED;
        if (this->m_event_cb) {
            this->m_event_cb(SM_BLE_EVENT_DISCONNECTED, NULL, this->m_arg);
        }
        break;
    default:
        break;
    }
}

static bool ble_rx_data_is_valid(const char* buff) {
    const char* data = buff;
    char* p_start = NULL, *p_end = NULL;
    p_start = strstr(data, "+{");
    if (p_start == NULL) return false;
    p_end = strstr(p_start, "}*");
    if (p_end == NULL) return false;
    return true;
}
