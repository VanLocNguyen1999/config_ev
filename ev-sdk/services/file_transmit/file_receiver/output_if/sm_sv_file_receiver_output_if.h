//
// Created by vuonglk on 06/09/2024.
//

#ifndef EV_SDK_SM_SV_FILE_RECEIVER_OUTPUT_IF_H
#define EV_SDK_SM_SV_FILE_RECEIVER_OUTPUT_IF_H

#include "sm_sv_file_transmit_common.h"
#include "sm_ex_flash_storage.h"
#include "sm_ssl_file_storage.h"

typedef struct{
    int32_t (*init)(sm_memory_info_t* mem_info);
    int32_t (*free)(sm_memory_info_t* mem_info);
    int32_t (*save_frame)(sm_file_data_frame_t* info);
    int32_t (*check_valid_file)(sm_file_info_t* file_info);
    int32_t (*process)();
}sm_file_output_if_t;

sm_file_output_if_t* sm_file_receiver_get_file_output();

sm_file_output_if_t* sm_file_receiver_get_flash_output(sm_ssl_storage_proc_t* _file_storage);


#endif //EV_SDK_SM_SV_FILE_RECEIVER_OUTPUT_IF_H
