/*
 * sm_ev_ble.c
 *
 *  Created on: Oct 4, 2024
 *      Author: admin
 */
#include "sm_ev_ble.h"
#include "sm_sv_ble.h"
#include "sm_app_hmi.h"
#include "sm_ev_opt_storage.h"
#include "sm_logger.h"

void sm_ble_on_new_paired(const char* _add, void *_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ble_t *ble = app->m_ble_service;
    /// TODO: Dangerous: Conflict thread BLE and EV.
    sm_ev_opt_store_ble_uuid(app->m_storage.m_ev_opt_storage, _add, strlen(_add));
    memcpy(app->m_config.m_ble_config.m_device_paired[0], _add, 32);
}

void sm_ble_on_new_ble_id(uint32_t _id, void *_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_hmi_t * display = app->m_hmi_service;
    /// TODO: Dangerous: Conflict thread BLE and EV.
    sm_sv_hmi_set_ble_key(display, _id, SM_DRV_LCD_SHOW);
}
