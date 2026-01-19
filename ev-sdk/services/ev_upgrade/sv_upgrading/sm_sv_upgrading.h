//
// Created by vuonglk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_UPGRADING_H
#define EV_SDK_SM_SV_UPGRADING_H

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "sm_elapsed_timer.h"
#include "sm_vector.h"

#include "sm_boot_master.h"
#include "sm_ex_flash_storage.h"

#include "sm_boot_impl.h"

typedef void sm_sv_boot_master_t;

typedef enum{
    SM_SV_BOOT_MASTER_INPUT_NONE = 0,
    SM_SV_BOOT_MASTER_INPUT_FILE,
    SM_SV_BOOT_MASTER_INPUT_FLASH,
    SM_SV_BOOT_MASTER_INPUT_TYPE_NUMBER
} SM_SV_BOOT_MASTER_INPUT_TYPE;

typedef enum{
    SM_SV_BOOT_MASTER_OUTPUT_NONE = 0,
    SM_SV_BOOT_MASTER_OUTPUT_CANOPEN,
    SM_SV_BOOT_MASTER_OUTPUT_FLASH,
    SM_SV_BOOT_MASTER_OUTPUT_TYPE_NUMBER
} SM_SV_BOOT_MASTER_OUTPUT_TYPE;

typedef enum{
    SM_SV_BOOT_MASTER_ERR_NONE = 0,
    SM_SV_BOOT_MASTER_ERR_INTERNAL = -1,
    SM_SV_BOOT_MASTER_ERR_TIMEOUT = -2,
    SM_SV_BOOT_MASTER_ERR_REBOOT = -3,
    SM_SV_BOOT_MASTER_ERR_DOWNLOAD = -4,
    SM_SV_BOOT_MASTER_ERR_NOT_SUPPORT = -5,
    SM_SV_BOOT_MASTER_ERR_UNKNOWN = -6
} SM_SV_BOOT_MASTER_UPGRADING_ERROR;

typedef struct sm_sv_boot_master_events_callback{
    void (*on_rebooted_dev)(uint8_t devId, uint8_t issSuccess, void* arg);
    void (*on_upgraded_dev)(uint8_t devId, uint8_t err, void* arg);
    void (*on_finish_upgrading)(void* arg);
    void* arg;
} sm_sv_boot_master_events_callback_t;


//If device not need to reboot, just leave reboot_dev_if is NULL
//If device not need to wait for reboot, just leave check_reboot_status_if is NULL

typedef struct sm_sv_boot_dev_if_fn{
    int  (*reboot_dev)(uint8_t id, uint32_t timeout, void* arg);
    bool (*check_is_dev_rebooted)(uint8_t id, void* arg);
    bool (*check_dev_upgrading_condition)(uint8_t id, void* arg);
    void* arg;
} sm_sv_boot_dev_if_fn_t;

typedef struct sm_sv_boot_request_dev_info{         ///TODO: add more member for another input or output type interface if necessary
    uint8_t m_boot_id;
    uint8_t m_network_id;
    sm_sv_boot_dev_if_fn_t* m_if;
    SM_SV_BOOT_MASTER_INPUT_TYPE m_input_type;
    SM_SV_BOOT_MASTER_OUTPUT_TYPE m_output_type;
    const char* m_file_path;                                // for input file interface
    sm_ex_flash_storage_partition_t* m_flash_part_input;    // for input flash interface
    sm_ex_flash_storage_partition_t* m_flash_part_output;   // for output flash interface

    fw_signature_storage_fn_t m_signature_storage_fn;
    void* m_signature_storage_arg;
} sm_sv_boot_request_dev_info_t;



sm_sv_boot_master_t* sm_sv_boot_master_create(sm_sv_boot_master_events_callback_t* _eventCallback);

int sm_sv_boot_master_request_upgrade(sm_sv_boot_master_t *_this, sm_sv_boot_request_dev_info_t _devInfo);

void sm_sv_boot_master_process(sm_sv_boot_master_t* _this);

int sm_sv_boot_master_destroy(sm_sv_boot_master_t* _this);

#ifdef __cplusplus
}
#endif

#endif //EV_SDK_SM_SV_UPGRADING_H
