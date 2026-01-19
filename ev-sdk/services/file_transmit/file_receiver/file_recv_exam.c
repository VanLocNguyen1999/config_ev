//
// Created by lekhacvuong on 9/7/24.
//
#include <time.h>
#include "sm_logger.h"
#include "sm_sv_file_receiver.h"
#include "sm_core_co.h"
#include "sm_co_od_common.h"
#include "sm_elapsed_timer.h"

#define TAG "file_recv_exam"

#include "sm_ex_flash_storage_if.h"
#include "sm_ssl_file_storage_impl.h"

#define CO_TCP_SERVER_IP    "/dev/ttyUSB1"
#define CO_TCP_SERVER_PORT  115200

#include "sm_hmi_flash_config.h"

//#define CO_TCP_SERVER_IP    "127.0.0.1"
//#define CO_TCP_SERVER_PORT  5000

sm_sv_file_receiver_t *g_sv_file_recv;
sm_co_t *g_core_co;
elapsed_timer_t g_co_1ms_timer;


int64_t get_tick_count() {
    struct timespec ts;
    int64_t tick = 0U;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

void log_put(const char *_log) {
    printf("%s\n", _log);
}

void file_recv_callback(uint8_t event, void *arg) {
    switch (event) {
        case SM_SV_FILE_RECEIVER_START_RECV:
            LOG_INF(TAG, "even: Start recv file");
            break;
        case SM_SV_FILE_RECEIVER_START_ABORT:
            LOG_INF(TAG, "even: Recv file about internal");
            break;
        case SM_SV_FILE_RECEIVER_RECV_SUCCESS:
            LOG_INF(TAG, "even: Recv file success");

            uint8_t buff[2 * 1024] = {0,};

            for (int i = 0; i < SM_SSL_FILE_NUMBER; i++) {
                memset(buff, 0, 2048);
                int ret = sm_ssl_storage_get_ext_flash_instance()->read_file(i, buff, 2 * 1024);
                char name[32] = {0,};
                sprintf(name, "./ssl_file_%d.txt", i);

                FILE *file_log = fopen(name, "w");
                if (file_log) {
                    fwrite(buff, sizeof(uint8_t), 2 * 1024, file_log);
                    fclose(file_log);
                    printf("log data to file %d\n", i);
                }
            }

            break;
        case SM_SV_FILE_RECEIVER_RECV_FAILED:
            LOG_INF(TAG, "even:  Recv file failed");
            break;
    }
}

int main() {
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);

    LOG_INF(TAG, "Start test recv app");

    sm_ex_flash_proc_t *flash_proc = sm_sv_ex_flash_storage_get_virtual_flash_proc(1024, 4 * 1024);
    sm_ex_flash_storage_t *g_ex_flash = sm_flash_storage_create_default(flash_proc);
    sm_ex_flash_storage_init(g_ex_flash);
    sm_ex_flash_erase_full(g_ex_flash);

    LOG_INF(TAG, "Create virtual flash size %d", sm_ex_flash_storage_get_total_size(g_ex_flash));
    sm_ssl_storage_init_ext_flash_instance(g_ex_flash);

    sm_co_if_t *m_co_if = sm_co_if_create_default(CO_SERIAL_CANBUS_IF,
                                                  CO_TCP_SERVER_IP, CO_TCP_SERVER_PORT, NULL);

    g_core_co = sm_co_create(8, false, m_co_if);
    sm_co_disable_sync_mode(g_core_co);

    sm_ssl_storage_proc_t *ssl_proc = sm_ssl_storage_get_ext_flash_instance();

    sm_file_output_if_t *file_output = sm_file_receiver_get_flash_output(ssl_proc);

    g_sv_file_recv = sm_sv_file_receiver_create_default(g_core_co, file_output, file_recv_callback, NULL);

    elapsed_timer_resetz(&g_co_1ms_timer, 1);

    while (1) {
        if (!elapsed_timer_get_remain(&g_co_1ms_timer)) {
            elapsed_timer_reset(&g_co_1ms_timer);
            sm_co_process(g_core_co, 1);
        }
        sm_co_if_process(sm_co_get_if(g_core_co));
        sm_sv_file_receiver_process(g_sv_file_recv);
    }

}
