//
// Created by admin on 9/25/2024.
//

#ifndef SM_BLE_SLAVE_H
#define SM_BLE_SLAVE_H

#include <stdint.h>
typedef enum {
    SM_BLE_ST_IDLE,
    SM_BLE_ST_ADVERTISING,
    SM_BLE_ST_CONNECTED,
    SM_BLE_ST_DISCONNECTED,
    SM_BLE_ST_SLEEP,
    SM_BLE_ST_NUM
}SM_BLE_MODULE_ST;

typedef enum {
    SM_BLE_EVENT_CONNECTED,
    SM_BLE_EVENT_DISCONNECTED,
    SM_BLE_EVENT_DATA,
    SM_BLE_EVENT_NUM,
}SM_BLE_MODULE_EVENT;
typedef void sm_ble_slave_t;

typedef void (*on_event_cb_fn)(int32_t event, void *data, void *arg);

sm_ble_slave_t* sm_ble_sl_create(void *_bt_module);
int32_t sm_ble_sl_init(sm_ble_slave_t *_this, char *name);
int32_t sm_ble_sl_set_event_cb(sm_ble_slave_t* , on_event_cb_fn _cb, void *_arg);
int32_t sm_ble_sl_advertising(sm_ble_slave_t* );
int32_t sm_ble_sl_disconnect(sm_ble_slave_t*);
int32_t sm_ble_sl_send_msg(sm_ble_slave_t* , char*,  int32_t _len);
int32_t sm_ble_sl_rename(sm_ble_slave_t* , char*);
int32_t sm_ble_sl_process(sm_ble_slave_t* );

#endif //SM_BLE_SLAVE_H
