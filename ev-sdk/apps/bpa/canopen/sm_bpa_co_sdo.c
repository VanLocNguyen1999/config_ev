//
// Created by vnbk on 15/03/2025.
//
#include "sm_bpa_app.h"
#include "sm_bpa_co_sdo.h"
#include "sm_logger.h"

#include "sm_co_od_common.h"
#include "sm_ev_manu_storage.h"

#define TAG "SM_BPA_CO_SDO"

/********************************************SDT TOOLS ********************************************************/
static uint8_t sm_app_store_manu_setting() {
    LOG_INF(TAG, "Setting up from Selex SDT tool");
    sm_bpa_app_t *app = g_bpa_app;
    if (app->m_storage.m_manu_storage->m_proc->store(app->m_storage.m_manu_storage,
                                                     app->m_ev_manu_cfg_buff + 9) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return CO_EXT_CONFIRM_abort;
    }

    if (sm_ev_manu_storage_validate(app->m_storage.m_manu_storage, app->m_ev_manu_cfg_buff, 9) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture crc");
        return CO_EXT_CONFIRM_abort;
    }

    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_app_load_ev_config_setting() {
    LOG_INF(TAG, "Read ev config up from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_app_store_ev_config_setting() {
    LOG_INF(TAG, "Write ev config up from Selex SDT tool");
    sm_bpa_app_t *app = g_bpa_app;

    memcpy(&app->m_config.m_ev_config, app->m_ev_manu_cfg_buff, sizeof(app->m_config.m_ev_config));

    if (app->m_storage.m_config_storage->m_proc->store(app->m_storage.m_config_storage,
                                                       &app->m_config.m_ev_config) < 0) {
        LOG_ERR(TAG, "Could NOT store ev config");
        return CO_EXT_CONFIRM_abort;
    }

    app->m_system_reboot_request.m_reboot = true;
    elapsed_timer_resetz(&app->m_system_reboot_request.m_time, SYSTEM_REBOOT_WAITING_TIME_DEFAULT);

    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_app_load_ev_net_config_setting(){
    LOG_INF(TAG, "Read ev net config up from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_app_store_ev_net_config_setting(){
    LOG_INF(TAG, "Write ev config up from Selex SDT tool");
    sm_bpa_app_t* app = g_bpa_app;

    memcpy(&app->m_config.m_net_config, app->m_ev_manu_cfg_buff, sizeof(app->m_config.m_net_config));

    if(app->m_storage.m_net_config_storage->m_proc->store(app->m_storage.m_net_config_storage, &app->m_config.m_net_config) < 0){
        LOG_ERR(TAG, "Could NOT store ev net config");
        return CO_EXT_CONFIRM_abort;
    }

    app->m_system_reboot_request.m_reboot = true;
    elapsed_timer_resetz(&app->m_system_reboot_request.m_time, SYSTEM_REBOOT_WAITING_TIME_DEFAULT);

    return CO_EXT_CONFIRM_success;
}


static uint8_t sm_app_load_manu_setting(){
    LOG_INF(TAG, "Load EV setting from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_app_reset_setting_handle(){
    LOG_INF(TAG, "Request reset setting from Selex SDT tool");
    sm_bpa_app_t* app = g_bpa_app;

    if(app->m_reset_setting_request == 1){

        sm_module_reset_data(&app->m_ev_manu);

        memcpy(app->m_ev_manu.m_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
        memcpy(app->m_ev_manu.m_ev_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));

        app->m_storage.m_manu_storage->m_proc->store(app->m_storage.m_manu_storage,
                                                     app->m_ev_manu.m_device_name);

        sm_ev_manu_storage_validate(app->m_storage.m_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);

        app->m_storage.m_config_storage->m_proc->clear(app->m_storage.m_config_storage);
        app->m_storage.m_net_config_storage->m_proc->clear(app->m_storage.m_net_config_storage);

        app->m_system_reboot_request.m_reboot = true;
        elapsed_timer_resetz(&app->m_system_reboot_request.m_time, SYSTEM_REBOOT_WAITING_TIME_DEFAULT);

        return CO_EXT_CONFIRM_success;
    }
    return CO_EXT_CONFIRM_abort;
}


static uint8_t sm_app_validate_manu_setting(){
    LOG_INF(TAG, "Validate EV setting from Selex SDT tool");
    sm_bpa_app_t* app = g_bpa_app;

    if(sm_ev_manu_storage_validate(app->m_storage.m_manu_storage, (const uint8_t*)SM_EV_CONFIG_ODO_PASS_DEFAULT, 9) < 0){
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return CO_EXT_CONFIRM_abort;
    }

    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_ev_require_reboot(){
    LOG_INF(TAG, "Require hmi reboot");
    sm_bpa_app_t* app = g_bpa_app;

    app->m_system_reboot_request.m_reboot = true;
    elapsed_timer_resetz(&app->m_system_reboot_request.m_time, 200);

    return CO_EXT_CONFIRM_success;
}

int32_t sm_bpa_co_sdo_setting(){
    LOG_DBG(TAG, "CanOpen SDO BPA setting");
    sm_bpa_app_t* _bpa = g_bpa_app;

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_WRITE_INFO_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_store_manu_setting,
                                &_bpa->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_VALIDATE_CRC_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_validate_manu_setting,
                                &_bpa->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_RESET_SETTING_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_reset_setting_handle,
                                &_bpa->m_reset_setting_request);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_INFO_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_load_manu_setting,
                                &_bpa->m_ev_manu);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_LTE_SIMNB_SUB_INDEX,
                                NULL,
                                &_bpa->m_driver.m_ec200_driver->simNb);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_CONFIG_WRITE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_store_ev_config_setting,
                                &_bpa->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_CONFIG_READ_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_load_ev_config_setting,
                                &_bpa->m_config.m_ev_config);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_NET_CONFIG_WRITE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_store_ev_net_config_setting,
                                &_bpa->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_NET_CONFIG_READ_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_app_load_ev_net_config_setting,
                                &_bpa->m_config.m_net_config);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_EV_SN_SUB_INDEX,
                                NULL,
                                &_bpa->m_ev_manu.m_ev_sn);

    sm_co_sdo_server_set_handle(_bpa->m_co,
                                SDO_EV_REBOOT_INDEX,
                                SDO_EV_REBOOT_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_ev_require_reboot,
                                &_bpa->m_system_reboot_request.m_reboot);

    return 0;
}
