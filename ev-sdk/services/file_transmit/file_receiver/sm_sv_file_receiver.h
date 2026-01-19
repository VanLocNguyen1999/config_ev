//
// Created by vuonglk on 05/09/2024.
//

#ifndef EV_SDK_SM_SV_FILE_RECEIVER_H
#define EV_SDK_SM_SV_FILE_RECEIVER_H

#include "sm_sv_file_receiver_output_if.h"
#include "sm_core_co.h"

typedef void sm_sv_file_receiver_t;

typedef enum{
    SM_SV_FILE_RECEIVER_START_RECV = 0,
    SM_SV_FILE_RECEIVER_START_ABORT,
    SM_SV_FILE_RECEIVER_RECV_SUCCESS,
    SM_SV_FILE_RECEIVER_RECV_FAILED
}SM_SV_FILE_RECEIVER_EVENTS;

typedef void(*file_receiver_event)(uint8_t event, void* arg);

sm_sv_file_receiver_t* sm_sv_file_receiver_create_default(sm_co_t* _co_core, sm_file_output_if_t* _output_if,
                                                          file_receiver_event _cb, void* _arg);

int32_t sm_sv_file_receiver_process(sm_sv_file_receiver_t* _this);

#endif //EV_SDK_SM_SV_FILE_RECEIVER_H
