//
// Created by lekhacvuong on 9/7/24.
//
#include <time.h>
#include "sm_logger.h"
#include "sm_sv_file_transfer.h"
#include "sm_core_co.h"
#include "sm_co_od_common.h"
#include "sm_elapsed_timer.h"

#define TAG "file_tran_exam"

#define CO_TCP_SERVER_IP    "/dev/ttyUSB0"
#define CO_TCP_SERVER_PORT  115200
#include "sm_hmi_flash_config.h"

//#define CO_TCP_SERVER_IP    "127.0.0.1"
//#define CO_TCP_SERVER_PORT  5000

sm_sv_file_transfer_t * g_sv_file_trans;
sm_co_t* g_core_co;
elapsed_timer_t g_co_1ms_timer;

#define FILE_CA_CERT        "./data.txt"
#define FILE_CLIENT_CERT    "./data2.txt"
#define FILE_CLIENT_KEY     "./data3.txt"


int64_t get_tick_count() {
    struct timespec ts;
    int64_t tick = 0U;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

void log_put(const char* _log) {
    printf("%s\n", _log);
}


void file_transfer_callback(uint8_t success, int32_t id, void* arg){
    LOG_INF(TAG, "Transfer file to dev id %d %s", id, success?"SUCCESS":"FAILED");
}

int main(){
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);

    LOG_INF(TAG, "Start test trans app");

    sm_co_if_t* m_co_if = sm_co_if_create_default(CO_SERIAL_CANBUS_IF,
                                                  CO_TCP_SERVER_IP, CO_TCP_SERVER_PORT, NULL);

    g_core_co = sm_co_create(NODE_ID_DEFAULT, false, m_co_if);
    sm_co_disable_sync_mode(g_core_co);

    g_sv_file_trans = sm_sv_file_transfer_create(g_core_co, file_transfer_callback, NULL);

    sm_sv_file_transfer_add_request(g_sv_file_trans,
                                    8,
                                    SM_HMI_SSL_CA_CERT_STORAGE_ADDR,
                                    SM_MEMORY_TYPE_EXTERNAL,
                                    SM_MEMORY_EXT_TXT_FILE,
                                    FILE_CA_CERT);

    sm_sv_file_transfer_add_request(g_sv_file_trans,
                                    8,
                                    SM_HMI_SSL_CLIENT_CERT_STORAGE_ADDR,
                                    SM_MEMORY_TYPE_EXTERNAL,
                                    SM_MEMORY_EXT_TXT_FILE,
                                    FILE_CLIENT_CERT);

    sm_sv_file_transfer_add_request(g_sv_file_trans,
                                    8,
                                    SM_HMI_SSL_CLIENT_KEY_STORAGE_ADDR,
                                    SM_MEMORY_TYPE_EXTERNAL,
                                    SM_MEMORY_EXT_TXT_FILE,
                                    FILE_CLIENT_KEY);

    elapsed_timer_resetz(&g_co_1ms_timer, 1);

    while(1){
        if(!elapsed_timer_get_remain(&g_co_1ms_timer)){
            elapsed_timer_reset(&g_co_1ms_timer);
            sm_co_process(g_core_co, 1);
        }
        sm_co_if_process(sm_co_get_if(g_core_co));
        sm_sv_file_transfer_process(g_sv_file_trans);
    }
}