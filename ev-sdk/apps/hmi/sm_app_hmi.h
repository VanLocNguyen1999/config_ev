//
// Created by vnbk on 07/08/2024.
//

#ifndef EV_SDK_SM_APP_HMI_H
#define EV_SDK_SM_APP_HMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_core_sch.h"
#include "sm_module_gps.h"
#include "sm_co_if.h"
#include "sm_core_co.h"

#include "sm_ev_service.h"
#include "sm_sv_bp.h"
#include "sm_sv_iot.h"
#include "sm_sv_ota.h"
#include "sm_sv_hmi.h"
#include "sm_sv_ble.h"

#include "sm_storage.h"
#include "sm_ev_sec_storage.h"
#include "sm_ev_odo_storage.h"

#include "sm_ex_flash_storage.h"

#include "sm_drv_lcd.h"
#include "sm_bsp_hmi.h"

#include "sm_ev_config.h"
#include "sm_ev_net_config.h"

#include "sm_hmi_io.h"
#include "sm_net_monitor.h"
#include "sm_ev_inactive_mode.h"
#include "sm_auth_bp.h"

#include "sm_ble_slave.h"
#include "sm_sv_ble.h"

#include "sm_sv_file_receiver.h"

#include "sm_sync_time.h"

//#define SIMULATOR_EV
#ifdef SIMULATOR_EV
#include "threading/thpool.h"

#else
#include "sm_w25qxx.h"
#include "sm_ec200.h"
#include "sm_ec200_mqtt.h"
#include "sm_ec200_mqtt.h"
#include "sm_mdbt42q_ble.h"
#include "sm_l76x_gps.h"
#endif

#ifdef OTA_TESTING
typedef struct {
  uint8_t m_total[3];
  uint8_t m_success[3];
}ota_report_t;

extern volatile sm_storage_t* g_test_storage;
extern volatile ota_report_t g_ota_report;
#endif

typedef struct {
    sm_sch_t* m_sch_task;

    sm_co_if_t* m_co_interface;
    sm_co_t* m_co;

    sm_host_api_t* m_iot_service;
    sm_sv_ev_t* m_ev_service;
    sm_sv_bp_t* m_bp_manager;
    sm_sv_upgrade_t* m_upgrade_service;

    sm_sv_ota_t* m_ota_service;
    uint8_t m_flag_sys_reset;             //special flag, check if system need to reset system after ota process

    sm_sv_hmi_t* m_hmi_service;
    sm_sv_ble_t* m_ble_service;

    sm_sv_file_receiver_t* m_file_recv_service;

    sm_ev_manu_t m_ev_manu;
    uint8_t m_ev_manu_cfg_buff[128 + 9];

    uint8_t* m_sw_version;

    struct {
        sm_gps_t* m_gps;
        sm_mqtt_client_t* m_mqtt_client;
        sm_http_client_t* m_http_client;
        sm_ble_slave_t* m_ble_slave;

        sm_hmi_io_t m_hmi_io;
        sm_net_monitor_t* m_net_monitor;
    }m_modules;

    sm_inactive_mode_t* m_inactive_mode;
    sm_auth_bp_t* m_auth_bp;

#ifndef SIMULATOR_EV
    struct {
        sm_ec200_t* m_ec200_driver;

        uc1676_t* m_uc1676;
        sm_drv_lcd_seg_t* m_lcd_driver;

        w25qxx_t m_ex_flash_driver;
        sm_l76x_t* m_gps_driver;
        sm_mdbt42q_t* m_ble_driver;
    }m_driver;
#endif

    struct {
        sm_ev_config_t m_ev_config;
        sm_ev_net_config_t m_net_config;

        sm_sv_ble_info_t m_ble_config;
        sm_sv_iot_config_t m_iot_config;
        sm_auth_bp_config_t m_auth_bp;
    }m_config;

    uint8_t m_reset_setting_request;

    struct {
        sm_sec_storage_t* m_backup_odo_storage;
        sm_odo_storage_t* m_odo_storage;
        sm_storage_t* m_ev_opt_storage;

        sm_storage_t* m_ev_config_storage;
        sm_storage_t* m_ev_manu_storage;
        sm_storage_t* m_net_config_storage;

        sm_sec_storage_t* m_boot2_signature_storage;

        sm_ex_flash_storage_t* m_ex_storage;
        sm_ex_flash_storage_partition_t* m_downloaded_fw_storage;
        sm_ex_flash_storage_partition_t* m_hmi_new_fw_storage;
    }m_storage;

    struct {
        bool m_net_ready;
        bool m_ble_ready;
        bool m_sync_time;
    }m_init_flag;

    elapsed_timer_t m_co_timer;

    sm_sync_time_t* m_sync_time;
    elapsed_timer_t m_sync_time_period;

    sm_mutex m_lock;

#ifdef SIMULATOR_EV
    threadpool m_thread_pool;
#endif
}sm_hmi_app_t;

extern sm_hmi_app_t g_hmi_app_default;

int32_t sm_hmi_app_init();

int32_t sm_hmi_app_ev_process(sm_hmi_app_t* _app);

int32_t sm_hmi_app_net_process(sm_hmi_app_t* _app);

int32_t sm_hmi_app_ble_process(sm_hmi_app_t* _app);

extern sm_hmi_app_t g_hmi_app_default;


#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_APP_HMI_H
