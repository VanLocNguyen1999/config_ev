/*
 * sm_ev_odo_storage.h
 *
 *  Created on: Oct 25, 2024
 *      Author: vuonglk
 */

#ifndef SM_EV_ODO_STORAGE_H_
#define SM_EV_ODO_STORAGE_H_
#include "sm_hal_flash.h"

typedef void sm_odo_storage_t;

sm_odo_storage_t* sm_ev_odo_storage_create_default(sm_hal_flash_t* _flash, uint32_t _address);

int32_t sm_ev_odo_storage_load(sm_odo_storage_t* _this, uint32_t* _odo_storage);

int32_t sm_ev_odo_storage_store(sm_odo_storage_t* _this, uint32_t _odo);

int32_t sm_ev_odo_clear(sm_odo_storage_t* _this);

#endif /* SM_EV_ODO_STORAGE_H_ */
