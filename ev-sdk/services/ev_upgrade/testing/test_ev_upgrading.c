//
// Created by vuonglk on 04/09/2024.
//
#include <malloc.h>
#include <bits/time.h>
#include <time.h>
#include "stdio.h"
#include "sm_sv_ev_upgrade.h"
#include "sm_memory.h"
#include "sm_core_co.h"
#include "sm_logger.h"
#include "sm_co_od_common.h"
#include "sm_elapsed_timer.h"
#include "sm_ex_flash_storage_if.h"

#define TAG "ev_upgrade_exam"

#define CO_TCP_SERVER_IP    "192.168.1.254"
#define CO_TCP_SERVER_PORT  32000

#define BP_ID_OFFSET            5
#define MC_CANOPEN_ID           2
#define PMU_CANOPEN_ID          1
#define HMI_CANOPEN_ID          8

#define SM_DEVICE_BP_BOOT_INDEX                             0x2001
#define SN_DEVICE_BP_REBOOT_SUB_INDEX                       0x07

#define SM_DEVICE_HMI_BOOT_INDEX                            0x2801
#define SM_DEVICE_HMI_REBOOT_SUB_INDEX                      0x00

#define SM_DEVICE_PMU_BOOT_INDEX                            0x2300
#define SM_DEVICE_PMU_REBOOT_SUB_INDEX                      0x01

#define SM_DEVICE_MC_BOOT_INDEX                             0x2200
#define SM_DEVICE_MC_REBOOT_SUB_INDEX                       0x00

#define SM_DEVICE_REBOOT_TIMEOUT                            500

void log_put(const char* _log) {
    printf("%s\n", _log);

}

int64_t get_tick_count() {
    struct timespec ts;
    int32_t tick = 0U;
    clock_gettime(CLOCK_REALTIME, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

#define HEX_PATH "./mc_mainapp.hex"
static char* g_file_buffer = NULL;
static char* g_flash_output_addr = NULL;

sm_sv_upgrade_t * g_sv_ev_upgrade;

sm_co_t* g_core_co;

elapsed_timer_t g_co_1ms_timer;


void canopen_reboot_dev_callback(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void* _arg) {
    printf("Send SDO %s, tx 0x%x, rx 0x%x\n", _sdoStt==SM_SDO_ST_SUCCESS?"SUCCEED":"FAILED", _txCode, _rxCode);
}

sm_co_if_t* m_co_if;

static void test_on_dev_upgraded_callback(uint8_t _devId, int32_t _err, void *_arg) {
    LOG_INF(TAG, "Device id %d upgrade finish, err code %d", _devId, _err);
}

static void test_on_finish_upgrading_callback(void* _arg) {
    LOG_INF(TAG, "All device have been upgraded call back in main");
}

int32_t test_storage_fw_signature(const sm_fw_signature_t* signature, void* arg){
    LOG_INF(TAG, "Saved fw signature to internal flash");

    LOG_INF(TAG, "Fw version: %.*s", SM_FW_VERSION_LENGTH, signature->m_version);
    LOG_INF(TAG, "Fw address: 0x%llx", signature->m_addr);
    LOG_INF(TAG, "Fw size: %u ", signature->m_size);
    LOG_INF(TAG, "Fw CRC: %d", signature->m_crc);
    LOG_INF(TAG, "Data address offset: %d", signature->m_data_address_offset);
    LOG_INF(TAG, "Fw valid: %u", signature->m_is_valid);

    return 1;
}



int main(){

    sm_logger_init(log_put, LOG_LEVEL_DEBUG);

    FILE* hex_file = fopen(HEX_PATH, "rb");
    if(!hex_file){
        printf("Cannot open hex file\n");
        return -1;
    }

    fseek(hex_file, 0L, SEEK_END);
    int file_size = (int)ftell(hex_file);

    fseek(hex_file, 0L, SEEK_SET);

    uint32_t size_test = file_size * sizeof(char) * 2;

    g_flash_output_addr = malloc(file_size * sizeof(char) * 2);
    g_file_buffer = malloc(file_size * sizeof(char) * 2);

    memset(g_file_buffer, 0xFF, file_size * sizeof(char) * 2);

    fread(g_file_buffer, sizeof(char), file_size, hex_file);

    LOG_INF(TAG, "Virtual flash with size  %d (Kb)", (file_size * sizeof(char) * 2) / 1024);

    m_co_if = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF, CO_TCP_SERVER_IP, CO_TCP_SERVER_PORT, NULL);

    printf("CO if addr %p\n", m_co_if);

    g_core_co = sm_co_create(NODE_ID_DEFAULT, false, m_co_if);

    sm_co_disable_sync_mode(g_core_co);

    sm_sv_ev_upgrade_callback_t test_ev_upgrade_cb = {
            .m_onUpgradeDev = test_on_dev_upgraded_callback,
            .m_onFinishUpgradeProcess = test_on_finish_upgrading_callback,
            .arg = NULL
    };

    g_sv_ev_upgrade = sm_sv_ev_upgrade_create_default(g_core_co);
    sm_sv_ev_upgrade_set_callback(g_sv_ev_upgrade, &test_ev_upgrade_cb);

    elapsed_timer_resetz(&g_co_1ms_timer, 1);

    sm_ex_flash_proc_t* flash_proc = sm_sv_ex_flash_storage_get_virtual_flash_proc(1024, 2*1024);

    sm_ex_flash_storage_t* ex_flash = sm_flash_storage_create_default(flash_proc);
    sm_ex_flash_storage_init(ex_flash);
    sm_ex_flash_erase_full(ex_flash);

    sm_ex_flash_storage_write(ex_flash, 0, g_file_buffer, size_test);


    sm_ex_flash_storage_partition_t* input_part = sm_ex_flash_storage_create_partition(ex_flash, 0, 1024*1024);
    sm_ex_flash_storage_partition_t* output_part = sm_ex_flash_storage_create_partition(ex_flash, 1024*1024, 516*1024);

    sm_sv_ev_upgrade_hmi(g_sv_ev_upgrade, input_part, output_part, test_storage_fw_signature, NULL);

    while(1){
        if(!elapsed_timer_get_remain(&g_co_1ms_timer)){
            elapsed_timer_reset(&g_co_1ms_timer);
            sm_co_process(g_core_co, 1);
        }
        sm_co_if_process(sm_co_get_if(g_core_co));
        sm_sv_ev_upgrade_process(g_sv_ev_upgrade);
    }
}