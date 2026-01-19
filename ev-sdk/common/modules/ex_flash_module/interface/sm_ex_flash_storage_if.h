//
// Created by vuonglk on 18/09/2024.
//

#ifndef EV_SDK_SM_EX_FLASH_STORAGE_IF_H
#define EV_SDK_SM_EX_FLASH_STORAGE_IF_H

#include "sm_ex_flash_storage.h"
#include "sm_w25qxx.h"
#include "sm_at25xe.h"

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_virtual_flash_proc();

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_w25q_proc(w25qxx_t* _w25q);

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_at25xe_proc(at25xe_t* _at25xe);


#endif //EV_SDK_SM_EX_FLASH_STORAGE_IF_H
