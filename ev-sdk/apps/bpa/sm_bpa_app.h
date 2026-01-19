//
// Created by vnbk on 24/05/2024.
//

#ifndef EV_SDK_SM_BPA_APP_H
#define EV_SDK_SM_BPA_APP_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_sv_bp.h"
#include "sm_sv_charger.h"
#include "sm_sv_pms.h"
#include "sm_host_api.h"
#include "sm_sv_ota.h"

#include "sm_module_gps.h"
#include "sm_l76x_gps.h"

#include "sm_core_sch.h"
#include "sm_core_co.h"

#include "sm_ev_data.h"
#include "sm_bpm_handle.h"
#include "sm_auth_bp.h"

#include "sm_ev_storage.h"
#include "sm_storage.h"
#include "sm_ev_sec_storage.h"

#include "sm_ex_flash_storage.h"

#include "sm_ev_net_config.h"
#include "sm_sv_iot_config.h"
#include "sm_net_monitor.h"

#include "sm_ec200.h"
#include "sm_mqtt_client.h"
#include "sm_http_client.h"
#include "sm_at25xe.h"

#define EV_SN_DEFAULT "BPA00000"

#define SYSTEM_REBOOT_WAITING_TIME_DEFAULT   500

typedef struct {
    sm_sch_t* m_sch_task;

    sm_co_if_t* m_co_interface;
    sm_co_t* m_co;

    sm_sv_bp_t* m_bp_service;
    sm_sv_charger_t* m_charger_service;
    sm_sv_pms_t* m_pms_service;

    sm_host_api_t* m_iot_service;
    sm_sv_ota_t* m_ota_service;
    sm_sv_upgrade_t* m_upgrade_service;

    struct {
        sm_gps_t* m_gps;
        sm_mqtt_client_t* m_mqtt_client;
        sm_http_client_t* m_http_client;
        sm_net_monitor_t* m_net_monitor;

        sm_auth_bp_t* m_auth_bp;

        sm_pms_ctl_t* m_pms_controller;
    }m_modules;

    struct {
        sm_bp_node_id_controller_t* m_nodeid_controller;
        sm_sv_charger_if_t* m_charger_if;

        sm_ec200_t* m_ec200_driver;
        at25xe_t m_ex_flash_driver;
        sm_l76x_t* m_gps_driver;
    }m_driver;

    struct {
        sm_ev_config_t m_ev_config;
        sm_ev_net_config_t m_net_config;

        sm_sv_iot_config_t m_iot_config;
        sm_bpm_config_t m_bpm_config[SM_BP_NUMBER_DEFAULT];
        sm_auth_bp_config_t m_auth_bp;
    }m_config;

    sm_bpm_handle_t* m_bpm;

    sm_ev_manu_t m_ev_manu;
    uint8_t m_ev_manu_cfg_buff[128 + 9];
    uint8_t* m_sw_version;

    struct{
        sm_storage_t* m_manu_storage;
        sm_storage_t* m_config_storage;
        sm_storage_t* m_net_config_storage;

        sm_sec_storage_t* m_boot2_signature_storage;

        sm_ex_flash_storage_partition_t* m_downloaded_fw_storage;
        sm_ex_flash_storage_partition_t* m_new_fw_storage;
    }m_storage;

    uint8_t m_reset_setting_request;
    struct {
        bool m_reboot;
        elapsed_timer_t m_time;
    }m_system_reboot_request;

    struct {
        bool m_net_ready;
        bool m_sync_time;
    }m_init_flag;

    uint8_t m_cmd_buffer_temp[128];

}sm_bpa_app_t;

sm_bpa_app_t* sm_bpa_app_create();

int32_t sm_bpa_app_init(sm_bpa_app_t* _app);

int32_t sm_bpa_app_process(sm_bpa_app_t* _app);

int32_t sm_bpa_app_iot_process(sm_bpa_app_t* _app);

void sm_bpa_system_reset(sm_bpa_app_t* _app);

extern sm_bpa_app_t* g_bpa_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_BPA_APP_H
