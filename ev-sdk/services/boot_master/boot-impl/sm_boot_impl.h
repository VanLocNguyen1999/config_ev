//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_SM_BOOT_IMPL_H
#define BSS_SDK_SM_BOOT_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_boot_slave.h"
#include "sm_host.h"

#include "sm_ex_flash_storage.h"

/**
 * @brief
 * @param void*     modbus_rtu_master
 * @param uint16    slave_address
 * @param uint16    start_address
 * @param uint16    quantity register
 * @param const uint16 registers
 * @return 0 - SUCCESS
 *         < 0 - FAILURE
 */
typedef int32_t (*modbus_rtu_write_multiple_registers_fn_t)(void*, uint16_t, uint16_t, uint16_t, const uint16_t*);
typedef int32_t (*modbus_rtu_read_holding_registers_fn_t)(void*, uint16_t, uint16_t, uint16_t, uint16_t*);

typedef int32_t (*fw_signature_storage_fn_t)(const sm_fw_signature_t* signature, void* arg);

sm_boot_output_if_t* sm_get_co_boot_output();
sm_boot_output_if_t* sm_get_host_sync_boot_output(sm_host_t* _host,
                                                  void* _arg);
sm_boot_output_if_t* sm_get_modbus_rtu_boot_output(const sm_fw_signature_t* _fw_info,
                                                   void* _mb_master,
                                                   modbus_rtu_write_multiple_registers_fn_t _mb_write_fn,
                                                   modbus_rtu_read_holding_registers_fn_t _mb_read_fn);

sm_boot_output_if_t* sm_get_flash_boot_output(sm_ex_flash_storage_partition_t* _flash_out,
                                              fw_signature_storage_fn_t _fn,
                                              void* _arg);

sm_boot_input_if_t* sm_get_file_boot_input(const char* file_path);

sm_boot_input_if_t* sm_get_file_boot_input_new(const char* file_path);

sm_boot_input_if_t* sm_get_flash_boot_input(sm_ex_flash_storage_partition_t* _flash_in);

#ifdef __cplusplus
};
#endif

#endif //BSS_SDK_SM_BOOT_IMPL_H