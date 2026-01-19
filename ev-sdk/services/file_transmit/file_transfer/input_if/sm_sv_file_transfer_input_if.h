//
// Created by vuonglk on 05/09/2024.
//

#ifndef EV_SDK_SM_SV_FILE_TRANSFER_INPUT_IF_H
#define EV_SDK_SM_SV_FILE_TRANSFER_INPUT_IF_H

#include "sm_sv_file_transmit_common.h"

typedef struct{
    int32_t (*init)();
    int32_t (*free)();
    int32_t (*get_file_info)(sm_file_info_t* info);
    int32_t (*get_file_data_frame)(sm_file_data_frame_t* frame, uint32_t id);
    int32_t (*process)();
}sm_file_input_if_t;

sm_file_input_if_t* sm_sv_file_transfer_get_file_input(const char* _path);

#endif //EV_SDK_SM_SV_FILE_TRANSFER_INPUT_IF_H
