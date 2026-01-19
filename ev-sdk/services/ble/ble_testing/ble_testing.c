//
// Created by admin on 9/27/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sm_logger.h"
#include "sm_hal.h"
#include "tiny-json/tiny-json.h"
#include "json-maker/json-maker.h"
#include "at-modem/sm_modem.h"
#include "sm_sv_ble.h"
#include "threading/thpool.h"
#include "linux/utils/linux_serial.h"

#include "sm_mdbt42q_ble.h"
#include "sm_ble_slave.h"

#define EV_ID   "CM0005"

int32_t g_fd;
static const char* TAG = "BLE_TESTING";

int64_t get_tick_count() {
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int64_t )tick;
}

const char* ignore_log_str[10] = {
    "",
    "",
    "",
};

void log_puts(char* _str) {
    // for (int i = 0; i<10; i++) {
    //     if (ignore_log_str[i] != NULL && strstr(_str, ignore_log_str[i]) != NULL) {
    //         return;
    //     }
    // }
    printf("%s\n", _str);
}

/* GLOBAL VAR */
sm_hal_uart_t *g_modem_uart;
sm_hal_io_t* g_io_HIGH;
sm_hal_io_t* g_io_LOW;
sm_modem_t *g_modem;
sm_sv_ble_t *g_sv_ble;
sm_host_api_t *host_api;

sm_mdbt42q_t g_ble_module;
sm_ble_slave_t *g_ble_slave;
sm_sv_ble_t *g_ble_sv;

uint8_t host_api_created;

char g_device_paired[BLE_MAX_DEVICE_PAIRED][BLE_DEVICE_PAIRED_NAME_MAX] = {
    "CM0001","",""
};
sm_sv_ble_info_t g_ble_info = {
    .m_name = EV_ID,
};
sm_host_sync_data_if_t host_api_if;

sm_sv_ble_event_t g_ble_event = {
    .on_device_paired = NULL,
    .on_device_unpaired = NULL,
    .on_new_device_paired = NULL,
    .on_new_ble_id = NULL,
};

sm_host_event_t g_host_event = {
    .on_connected = NULL,
    .on_disconnected = NULL,
    .on_cmd_from_host = NULL,
};

const char* host_cmd_str[] = {
    "SM_HOST_CMD_SET_ODO",
    "SM_HOST_CMD_BP_EV_SN",
    "SM_HOST_CMD_FORGET_HOST",
    "SM_HOST_CMD_EV_DATA",
    "SM_HOST_CMD_BP_DATA",
    "SM_HOST_CMD_FW_VERSION",
    "SM_HOST_CMD_SIM_NB",
    "SM_HOST_CMD_FIND_EV",
    "SM_HOST_CMD_HEAD_LIGHT",
    "SM_HOST_CMD_SET_SIGNAL_LIGHT",
    "SM_HOST_CMD_BLOCK_EV",
    "SM_HOST_CMD_LOCK_EV",
    "SM_HOST_CMD_SET_DRIVE_MODE",
    "SM_HOST_CMD_SET_MAX_SPEED",
    "SM_HOST_CMD_SET_ACTIVE",
    "SM_HOST_CMD_SET_LOCK_PORT",
    "SM_HOST_CMD_SET_UNLOCK_PORT",
    "SM_HOST_CMD_CONFIG_VERIFY_BP_OFFLINE",
};
/* STATIC FUNCTION */
static void ble_on_dv_paired(void* arg);
static void ble_on_dv_unpaired(void* arg);
static void ble_on_new_dv_paired(const char* uuid, void* arg);
static void ble_on_new_ble_id(uint32_t ble_id, void* arg);
static void host_on_conn(int32_t _success, void* _arg);
static void host_on_disconnect(int32_t _success, void* _arg);
static void host_on_cmd(int32_t _cmd, void* _ret, void* _arg);

void init() {
    sm_logger_init(log_puts, LOG_LEVEL_DEBUG);

    g_modem_uart = sm_hal_uart_init(&g_fd, 9600, 0, 0);
    g_io_HIGH = sm_hal_io_init(0);

    sm_mdbt42q_init(&g_ble_module, g_modem_uart, g_io_LOW, g_io_HIGH, g_io_LOW);
    g_ble_slave = sm_ble_sl_init(&g_ble_module, NULL);

    g_ble_event.on_new_ble_id = ble_on_new_ble_id;
    g_ble_event.on_device_paired = ble_on_dv_paired;
    g_ble_event.on_device_unpaired = ble_on_dv_unpaired;
    g_ble_event.on_new_device_paired = ble_on_new_dv_paired;

    memcpy(g_ble_info.m_device_paired, g_device_paired,sizeof(g_device_paired));
    g_ble_sv = sm_sv_ble_create(g_ble_slave, &g_ble_info, &g_ble_event, NULL);
    host_api = (sm_host_api_t*)g_ble_sv;
    host_api->m_proc->init(host_api, &host_api_if, 5);

    g_host_event.on_connected = host_on_conn;
    g_host_event.on_disconnected = host_on_disconnect;
    g_host_event.on_cmd_from_host = host_on_cmd;
    sm_host_api_reg_event_handle(host_api, &g_host_event, host_api);
    host_api_created = 1;
    LOG_DBG(TAG, "READY");
}

static void ble_on_dv_paired(void* arg) {
    LOG_DBG(TAG, "Device paired");
}
static void ble_on_dv_unpaired(void* arg) {
    LOG_DBG(TAG, "Device unpaired");
}
static void ble_on_new_dv_paired(const char* uuid, void* arg) {
    LOG_DBG(TAG, "New Device paired %s", uuid);
}
static void ble_on_new_ble_id(uint32_t ble_id, void* arg) {
    LOG_DBG(TAG, "New BLE ID %d", ble_id);
}

static void host_on_conn(int32_t _success, void* _arg) {
    if (_success)
        LOG_DBG(TAG, "Connected to host");
    else
        LOG_ERR(TAG, "Connection to host failed");
}
static void host_on_disconnect(int32_t _success, void* _arg) {
    if (_success)
        LOG_DBG(TAG, "Disconnected from host");
    else
        LOG_ERR(TAG, "Disconnection to host failed");
}
static void host_on_cmd(int32_t _cmd, void* _ret, void* _arg) {
    LOG_INF(TAG, "HOST ON CMD: %s - %d", host_cmd_str[_cmd], *(int32_t*)_ret);
    // global_test_flag.m_recv_cmd.m_value = _cmd;
}
/* THREADING */
threadpool pthpool;

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        static uint8_t data[129] = {0,};
        memset(data, '\0',129);
        int32_t len = serial_recv_bytes(g_fd, data, 128);
        if (len >0) {
            for(int i=0; i<len; i++) {
                sm_hal_uart_rx_irq(_this, data[i]);
            }
        }
    }
}

void mcu(void *_arg) {
    while (1) {
        uart_process(g_modem_uart, _arg);
        sm_hal_delay_ms(1);
    }
}

void app(void* _arg) {
    while(1) {
        if (host_api_created) {
            host_api->m_proc->process(host_api);
            sm_hal_delay_ms(100);
        }
    }
}
void main() {
    pthpool = thpool_init(4);
    thpool_add_work(pthpool, (void*)mcu, NULL);
    thpool_add_work(pthpool, (void*)app, NULL);
    init();
    char ch;
    while(1) {
        sm_hal_delay_ms(10);
        scanf("%c", &ch);
        while(ch == '\n')
            scanf("%c", &ch);
        switch (ch) {
        case 'z':
            if (sm_hal_io_get_value(g_io_HIGH))
                sm_hal_io_set_value(g_io_HIGH, 0);
            else
                sm_hal_io_set_value(g_io_LOW, 1);
            break;
        }
    }
}