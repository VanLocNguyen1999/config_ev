//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_EV_STORAGE_H
#define EV_SDK_SM_EV_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_storage.h"
#include "sm_hal_flash.h"

sm_storage_t* sm_ev_config_create_default(sm_hal_flash_t* _data_flash,
                                          uint32_t _start_address,
                                          uint32_t _config_size);

sm_storage_t* sm_ev_config_create(sm_hal_flash_t* _data_flash,
                                  uint32_t _start_address,
                                  uint32_t _config_size);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_STORAGE_H
