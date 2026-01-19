//
// Created by vnbk on 25/09/2024.
//

#ifndef EV_SDK_SM_LINUX_FILE_H
#define EV_SDK_SM_LINUX_FILE_H

#include "sm_types.h"

typedef void sm_linux_file_t;

sm_linux_file_t* sm_linux_file_create(const char* _name);
int32_t sm_linux_file_destroy(sm_linux_file_t* _this);
int32_t sm_linux_file_erase(sm_linux_file_t* _this);
int32_t sm_linux_file_write(sm_linux_file_t* _this, const char* _content, int32_t _len);
int32_t sm_linux_file_read(sm_linux_file_t* _this, char* _content);

#endif //EV_SDK_SM_LINUX_FILE_H
