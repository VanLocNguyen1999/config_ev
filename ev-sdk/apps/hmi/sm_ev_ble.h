/*
 * sm_ev_ble.h
 *
 *  Created on: Oct 4, 2024
 *      Author: admin
 */

#ifndef SM_EV_BLE_H_
#define SM_EV_BLE_H_

#include "sm_types.h"

void sm_ble_on_paired(void* arg);

void sm_ble_on_unpaired(void *arg);

void sm_ble_on_new_paired(const char* _add, void *arg);

void sm_ble_on_new_ble_id(uint32_t id, void *arg);

void sm_ble_on_mac_checked(const char* add, uint8_t argv, void* arg);

#endif /* SM_EV_BLE_H_ */
