//
// Created by vuonglk on 05/09/2024.
//

#ifndef EV_SDK_SM_SV_FILE_TRANSFER_H
#define EV_SDK_SM_SV_FILE_TRANSFER_H

#include "sm_core_co.h"
#include "sm_sv_file_transfer_input_if.h"

typedef void sm_sv_file_transfer_t;

typedef void(*finish_transfer_cb)(uint8_t success, int32_t id, void* arg);

sm_sv_file_transfer_t* sm_sv_file_transfer_create(sm_co_t* _co_core, finish_transfer_cb _cb, void* _arg);

int32_t sm_sv_file_transfer_add_request(sm_sv_file_transfer_t* _this,
                                        int32_t _id,
                                        uint64_t _start_addr,
                                        uint8_t _memory_type,
                                        uint8_t _memory_extension,
                                        const char* _path);

int32_t sm_sv_file_transfer_process(sm_sv_file_transfer_t* _this);

#endif //EV_SDK_SM_SV_FILE_TRANSFER_H
