//
// Created by vnbk on 24/05/2024.
//
#include "sm_bpa_app.h"
#include "sm_bpm_handle.h"
#include "sm_ota_handle.h"
#include "sm_dev_handle.h"
#include "sm_bpa_co_sdo.h"

#include "sm_ev_cmd.h"

#include "sm_co_od_common.h"
#include "sm_sv_iot.h"

#include "sm_core_sch.h"

#include "sm_fw.h"

#include "sm_module_gps.h"
#include "sm_gps_l76x.h"

#include "sm_ex_flash_storage_if.h"
#include "sm_ev_manu_storage.h"

#include "sm_bsp_bpa.h"
#include "sm_bsp_porting.h"
#include "sm_bpa_flash_config.h"

#include "sm_logger.h"
#include "sm_l76x_gps.h"

#define TAG "SM_BPA_APP"

#define _impl(x) ((sm_bpa_app_t*)x)

static uint8_t g_software_version[4] = {0, 0, 2, 0x05};

static sm_bpa_app_t g_bpa_app_default = {
        .m_sch_task = NULL,
        .m_co = NULL,
        .m_co_interface = NULL,
        .m_bp_service = NULL,
        .m_charger_service = NULL,
        .m_pms_service = NULL,
        .m_iot_service = NULL,
        .m_ota_service = NULL,
        .m_upgrade_service = NULL,

        .m_modules = {
                .m_http_client = NULL,
                .m_mqtt_client = NULL,
                .m_net_monitor = NULL,
                .m_gps = NULL,
                .m_auth_bp = NULL,
                .m_pms_controller = NULL,
        },

        .m_bpm = NULL,

        .m_sw_version = g_software_version,

        .m_storage = {
                .m_config_storage = NULL,
                .m_manu_storage = NULL,
                .m_boot2_signature_storage = NULL,
                .m_downloaded_fw_storage = NULL,
                .m_net_config_storage = NULL,
        },

        .m_driver = {
                .m_nodeid_controller = NULL,
                .m_charger_if = NULL,
                .m_gps_driver = NULL,
                .m_ec200_driver = NULL
        },

        .m_init_flag = {
                .m_net_ready = false,
                .m_sync_time = false,
        },

        .m_reset_setting_request = false,
        .m_system_reboot_request = {
                .m_reboot = false,
        }
};

sm_bpa_app_t* g_bpa_app = NULL;

static void sm_co_if_proc(void* _arg);
static void sm_bp_service_proc(void* _arg);
static void sm_pms_service_proc(void* _arg);

static void sm_net_event_handle(int32_t _event, void* _arg);

static void sm_auth_bp_event_handle(uint8_t _type, uint8_t _port, uint8_t _level, void* _arg);

void sm_iot_event_on_bp_event(const char* bp_sn, int32_t _accepted, void* _arg);
static sm_sv_iot_event_t g_iot_event_handle = {
        .on_ev_request_upgrade = sm_ota_handle_request_upgrade,
        .on_ev_new_fw = sm_ota_handle_ev_new_fw,
        .on_ev_finish_new_fw_extract = sm_ota_handle_finish_new_fw_extract,
        .on_bp_new_fw = sm_ota_handle_bp_new_fw,
        .on_bp_request_upgrade = sm_ota_handle_request_bp_upgrade,
        .on_bp_event = sm_iot_event_on_bp_event,
        .on_cmd_ota_progress = NULL, //sm_ota_handle_cmd_progress,
};

void sm_iot_event_handle_on_connected(int32_t, void*);
void sm_iot_event_handle_on_disconnected(int32_t, void*);
int32_t sm_iot_event_handle_on_cmd_from_host(int32_t, void*, void*);
int32_t sm_iot_event_handle_on_cfg_from_host(int32_t, void*, void*);
int32_t sm_iot_event_handle_on_cfg_completed(void*);
static sm_host_event_t g_host_iot_event_handle = {
        .on_connected = sm_iot_event_handle_on_connected,
        .on_disconnected = sm_iot_event_handle_on_disconnected,
        .on_cmd_from_host = sm_iot_event_handle_on_cmd_from_host,
        .on_cfg_from_host = sm_iot_event_handle_on_cfg_from_host,
        .on_cfg_completed = sm_iot_event_handle_on_cfg_completed
};

static sm_host_sync_data_if_t g_host_sync_data = {
        .get_ev_manu = sm_dev_get_manu,
        .get_ev_module_info = sm_dev_get_module_info,
        .get_ev_data = NULL,
        .get_ev_config = sm_dev_get_ev_config,
        .get_ev_bp_data = sm_dev_get_bp_data,
        .get_network_data = sm_dev_get_network_data,
        .get_gps_data = sm_dev_get_gps_data,
        .get_optional_data = sm_dev_get_optional_data,
};

static sm_ota_event_t g_ota_event_handle = {
        .on_start = sm_ota_on_start,
        .on_start_module = sm_ota_on_start_module,
        .on_downloading_status = sm_ota_on_downloading_status,
        .on_upgrading_status = sm_ota_on_upgrading_status,
        .on_finished_module = sm_ota_on_finished_module,
        .on_finished = sm_ota_on_finished
};

static sm_sv_bp_event_cb_t g_bp_event_handle = {
        .on_bp_connected = sm_dev_on_bp_connected_event,
        .on_bp_disconnected = sm_dev_on_bp_disconnected_event,
        .on_bp_update_data = NULL
};

/// Initializing DRIVER
static int32_t sm_bpa_driver_init(sm_bpa_app_t* _this){
    LOG_INF(TAG, "Initializing LTE driver");
    _this->m_driver.m_ec200_driver = sm_ec200_create(sm_bsp_bpa_get_lte_power(),
                                                     sm_bsp_bpa_get_lte_uart(),
                                                     NULL);
    if(!_this->m_driver.m_ec200_driver ){
        return -1;
    }

    LOG_INF(TAG, "Initializing gps L76x");
    _this->m_driver.m_gps_driver = sm_l76x_init(sm_bsp_bpa_get_gps_uart(),
                                                sm_bsp_bpa_get_gps_reset());
    _this->m_modules.m_gps = sm_gps_create(_this->m_driver.m_gps_driver);

    LOG_INF(TAG, "Initializing external flash at25xe");
    at25xe_Begin(&_this->m_driver.m_ex_flash_driver, sm_bsp_bpa_get_ext_mem(), sm_bsp_bpa_get_ext_mem_cs());
    if(at25xe_Init(&_this->m_driver.m_ex_flash_driver) == false){
        LOG_ERR(TAG, "Initializing external flash AT25XE FAILURE");
        return -1;
    }

    return 0;
}

static int32_t sm_bpa_app_storage_init(sm_bpa_app_t* _this){
    sm_storage_t* storage = NULL;
    sm_hal_flash_t* data_flash = sm_bsp_bpa_get_data_flash();

    storage = sm_ev_manu_storage_create(data_flash, SM_STORAGE_EV_INFO_FLASH_ADDR);
    _this->m_storage.m_manu_storage = storage;

    storage = sm_ev_security_storage_create(data_flash, SM_STORAGE_FW_SIGNATURE_FLASH_ADDR, sizeof(sm_boot_2_fw_setting_t));
    _this->m_storage.m_boot2_signature_storage = storage;

    storage = sm_ev_config_create_default(data_flash, SM_STORAGE_EV_CONFIG_FLASH_ADDR, SM_EV_CONFIG_SIZE_OF);
    _this->m_storage.m_config_storage = storage;

    storage = sm_ev_config_create(data_flash, SM_STORAGE_NET_CONFIG_FLASH_ADDR, SM_NET_CONFIG_SIZE_OF);
    _this->m_storage.m_net_config_storage = storage;

#ifdef OTA_TESTING
    g_test_storage = sm_ev_config_create(data_flash, SM_HMI_TEST_INFO_FLASH_ADDR, sizeof(ota_report_t));

    if(g_test_storage->m_proc->load(g_test_storage, &g_ota_report) < 0){
        memset(&g_ota_report, 0, sizeof(ota_report_t));

        LOG_ERR(TAG, "Load default test report value = 0");
        g_test_storage->m_proc->store(g_test_storage, &g_ota_report);
    }
#endif

#ifdef SIMULATOR_EV
    sm_ex_flash_proc_t* ex_flash_proc = sm_sv_ex_flash_storage_get_virtual_flash_proc(1024, 100);
#else
    sm_ex_flash_proc_t* ex_flash_proc = sm_sv_ex_flash_storage_get_at25xe_proc(&_this->m_driver.m_ex_flash_driver);
#endif
    sm_ex_flash_storage_t* ex_storage = sm_flash_storage_create_default(ex_flash_proc);
    sm_ex_flash_storage_init(ex_storage);

    _this->m_storage.m_downloaded_fw_storage = sm_ex_flash_storage_create_partition(ex_storage,
                                                                                    SM_DOWNLOADED_HEX_FILE_STORAGE_ADDR,
                                                                                    SM_DOWNLOADED_FW_STORAGE_SIZE);

    _this->m_storage.m_new_fw_storage = sm_ex_flash_storage_create_partition(ex_storage,
                                                                             SM_NEW_MAINAPP_STORAGE_ADDR,
                                                                             SM_NEW_MAINAPP_STORAGE_SIZE);

    return 0;
}

int32_t sm_bpa_app_load_config(sm_bpa_app_t* _this){
    if(!_this){
        return -1;
    }
    /// LOAD Manufacture Info
    if(_this->m_storage.m_manu_storage->m_proc->load(_this->m_storage.m_manu_storage,
                                                        &_this->m_ev_manu) < 0){
        LOG_ERR(TAG, "Could NOT load EV Manufacture, now storage default value");
        sm_module_reset_data(&_this->m_ev_manu);

        memcpy(_this->m_ev_manu.m_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
        memcpy(_this->m_ev_manu.m_ev_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));

        _this->m_storage.m_manu_storage->m_proc->store(_this->m_storage.m_manu_storage,
                                                          _this->m_ev_manu.m_device_name);
        sm_ev_manu_storage_validate(_this->m_storage.m_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);
    }else{
        LOG_INF(TAG, "Load EV manu SUCCEED, EV manu is:");
        LOG_INF(TAG, "Ev self sn: %s", _this->m_ev_manu.m_sn);
        LOG_INF(TAG, "Ev sn: %s", _this->m_ev_manu.m_ev_sn);
    }

    /// LOAD EV configuration
    int32_t ret = _this->m_storage.m_config_storage->m_proc->load(_this->m_storage.m_config_storage,
                                                                     &_this->m_config.m_ev_config);
    if(ret == -1){
        LOG_ERR(TAG, "Could NOT load EV Configuration, now set default value");
        sm_ev_config_reset_default( &_this->m_config.m_ev_config);

        _this->m_storage.m_config_storage->m_proc->store(_this->m_storage.m_config_storage,
                                                            &_this->m_config.m_ev_config);
    }else if(ret == -2){
        sm_ev_config_t config;
        sm_ev_config_clone(&_this->m_config.m_ev_config, &config);

        if(!sm_ev_config_validate(&config)) {
            LOG_WRN(TAG, "Config Param INVALID, Re-config again");
            sm_ev_config_clone(&config, &_this->m_config.m_ev_config);
            _this->m_storage.m_config_storage->m_proc->store(_this->m_storage.m_config_storage,
                                                                &_this->m_config.m_ev_config);
        }
    }else{
        LOG_INF(TAG, "Load EV config SUCCEED");
    }

    if(!sm_ev_config_validate(&_this->m_config.m_ev_config)) {
        LOG_WRN(TAG, "Config Param INVALID, Re-config again");
        _this->m_storage.m_config_storage->m_proc->store(_this->m_storage.m_config_storage,
                                                            &_this->m_config.m_ev_config);
    }

    /// Configure BPM
//    _this->m_config.m_bpm_config[0].m_enable = !(_this->m_config.m_ev_config.m_lock_port & 0x01);
//    _this->m_config.m_bpm_config[1].m_enable = !(_this->m_config.m_ev_config.m_lock_port & 0x02);
//    _this->m_config.m_bpm_config[2].m_enable = !(_this->m_config.m_ev_config.m_lock_port & 0x04);

    /// Config Authentication BP.
    _this->m_config.m_auth_bp.m_auth_bp_online = (_this->m_config.m_ev_config.m_auth_bp >> 4) & SM_EV_CONF_AUTH_BP_OFFLINE_MASK;
    _this->m_config.m_auth_bp.m_auth_bp_offline = _this->m_config.m_ev_config.m_auth_bp & SM_EV_CONF_AUTH_BP_OFFLINE_MASK;
    _this->m_config.m_auth_bp.m_scan_time = SM_AUTH_BP_SCAN_TIME_DEFAULT;

    /// LOAD IOT configuration
    if(_this->m_storage.m_net_config_storage->m_proc->load(_this->m_storage.m_net_config_storage,
                                                           &_this->m_config.m_net_config) < 0){
        LOG_ERR(TAG, "Could NOT load EV net configuration. Setup net config default");
        sm_ev_net_config_default(&_this->m_config.m_net_config);

        _this->m_storage.m_net_config_storage->m_proc->store(_this->m_storage.m_net_config_storage,
                                                             &_this->m_config.m_net_config);
    }else{
        LOG_INF(TAG, "Load EV net config SUCCEED");
    }

    /// Config IOT
    _this->m_config.m_iot_config.m_host = (const char*)_this->m_config.m_net_config.m_host;
//    _this->m_config.m_iot_config.m_host = (const char*)"hub-uat.selex.vn";
    _this->m_config.m_iot_config.m_port = _this->m_config.m_net_config.m_port;
    _this->m_config.m_iot_config.m_username = (const char*)_this->m_config.m_net_config.m_user;
    _this->m_config.m_iot_config.m_password = (const char*)_this->m_config.m_net_config.m_pass;
    _this->m_config.m_iot_config.m_client_id = (const char*)_this->m_ev_manu.m_ev_sn;

    /// Config LTE
    sm_ec200_config_t lte_config = {
            .m_apn = (const char*)_this->m_config.m_net_config.m_lte_apn,
            .m_user = (const char*)_this->m_config.m_net_config.m_lte_user,
            .m_pass = (const char*)_this->m_config.m_net_config.m_lte_pass,
    };
    sm_ec200_set_config(_this->m_driver.m_ec200_driver, &lte_config);

    return 0;
}

int32_t sm_net_service_create(sm_bpa_app_t *_this) {
    if (!_this->m_driver.m_ec200_driver) {
        LOG_ERR(TAG, "Could NOT initialize EC200 driver");
        return -1;
    }

    _this->m_modules.m_mqtt_client = sm_mqtt_client_create(_this->m_driver.m_ec200_driver);
    _this->m_modules.m_http_client = sm_http_client_create(_this->m_driver.m_ec200_driver);

    _this->m_iot_service = sm_sv_iot_createz(SM_IOT_DEVICE_TYPE_EV, //SM_IOT_DEVICE_TYPE_ADAPTER, //SM_IOT_DEVICE_TYPE_EV,
                                             SM_IOT_MODEL_TYPE_S2, // SM_IOT_MODEL_TYPE_BPA, //SM_IOT_MODEL_TYPE_S2,
                                             _this->m_ev_manu.m_ev_sn,
                                             _this->m_modules.m_mqtt_client,
                                             &_this->m_config.m_iot_config,
                                             &g_iot_event_handle,
                                             _this);

    sm_host_api_t *host = (sm_host_api_t *) _this->m_iot_service;
    g_host_sync_data.m_arg = _this;
    host->m_proc->init(host, &g_host_sync_data, _this->m_config.m_ev_config.m_sync_time);
    sm_host_api_reg_event_handle(_this->m_iot_service,
                                 &g_host_iot_event_handle,
                                 _this);

    /// Create Net Monitor
    _this->m_modules.m_net_monitor = sm_net_monitor_create(SM_NET_MONITOR_DETECTED_TIME_DEFAULT,
                                                           sm_net_event_handle,
                                                           _this);

    return 0;
}

static int32_t sm_app_storage_fw_signature(const sm_fw_signature_t* signature, void* arg){
    (void)arg;

    sm_bpa_app_t* this = &g_bpa_app_default;

    LOG_INF(TAG, "Saved fw signature to internal flash");
    LOG_INF(TAG, "Fw address: 0x%x", signature->m_addr);
    LOG_INF(TAG, "Fw size: %u ", signature->m_size);
    LOG_INF(TAG, "Fw CRC: %d", signature->m_crc);
    LOG_INF(TAG, "Fw valid: %u", signature->m_is_valid);

    sm_boot_2_fw_setting_t fw_setting = {0,};

    sm_sec_storage_t* fw_signature_storage = this->m_storage.m_boot2_signature_storage;

    if(signature->m_addr != SM_MAINAPP_DEFAULT_ADDR){
        LOG_ERR(TAG, "BPA main-app addr 0x%x is invalid, reject firmware", signature->m_addr);
        return -1;
    }

    if(sm_ev_security_storage_load(fw_signature_storage, "selex123@", &fw_setting) < 0){
        return -1;
    }

    fw_setting.m_new_fw_flag = 2;
    fw_setting.m_size = signature->m_size;
    fw_setting.m_addr = signature->m_addr;
    fw_setting.m_crc  = signature->m_crc;

    return sm_ev_security_storage_store(fw_signature_storage, "selex123@", &fw_setting);
}

static int32_t sm_ota_service_create(sm_bpa_app_t* _this){
    if(!_this->m_modules.m_http_client){
        LOG_ERR(TAG, "Could NOT created HTTP Client");
        return -1;
    }

    _this->m_upgrade_service = sm_sv_ev_upgrade_create_default(_this->m_co);
    _this->m_ota_service = sm_sv_ota_create(_this->m_modules.m_http_client,
                                            _this->m_upgrade_service,
                                            _this->m_storage.m_downloaded_fw_storage);

    sm_sv_ota_reg_event(_this->m_ota_service, &g_ota_event_handle, _this);

    sm_sv_ota_set_fw_output_storage_if(_this->m_ota_service,
                                       _this->m_storage.m_new_fw_storage,
                                       sm_app_storage_fw_signature,
                                       _this);
    return 0;
}

/**********************************************************************************************************************/
static void sm_co_if_proc(void* _arg){
    if(!_arg || !_impl(_arg)->m_co_interface){
        return;
    }
    sm_co_if_process(_impl(_arg)->m_co_interface);
}

static void sm_bp_service_proc(void* _arg){
    if(!_arg || !_impl(_arg)->m_bp_service){
        return;
    }
    sm_sv_bp_process(_impl(_arg)->m_bp_service);
}

static void sm_pms_service_proc(void* _arg){
    if(!_arg || !_impl(_arg)->m_pms_service){
        return;
    }
    sm_pms_ctl_process(_impl(_arg)->m_modules.m_pms_controller);
    sm_sv_charger_process(_impl(_arg)->m_charger_service);
    if(!sm_sv_charger_is_charging(_impl(_arg)->m_charger_service)){
        sm_sv_pms_process(_impl(_arg)->m_pms_service);
    }
}

static void sm_ota_service_proc(void* _arg){
    if(!_arg || !_impl(_arg)->m_ota_service){
        return;
    }
    sm_sv_ota_process(_impl(_arg)->m_ota_service);
}

int32_t sm_net_init(void* _app){
    if(sm_ec200_init(_impl(_app)->m_driver.m_ec200_driver) < 0){
        return -1;
    }

    if( sm_mqtt_init(_impl(_app)->m_modules.m_mqtt_client) < 0 || sm_http_client_init(_impl(_app)->m_modules.m_http_client)){
        return -1;
    }

    _impl(_app)->m_init_flag.m_net_ready = true;

    return 0;
}

static void sm_iot_service_proc(void* _arg){
    if(!_arg || !_impl(_arg)->m_iot_service){
        return;
    }

    if(!_impl(_arg)->m_init_flag.m_net_ready && sm_net_init(_arg) < 0){
        return;
    }

    if(_impl(_arg)->m_iot_service->m_proc->process(_impl(_arg)->m_iot_service) < 0){
        _impl(_arg)->m_init_flag.m_net_ready = false;
    }
}

static void sm_gps_proc(void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!app || !app->m_modules.m_gps){
        return;
    }
    app->m_modules.m_gps->proc->process(app->m_modules.m_gps);
}

/**********************************************************************************************************************/
static int32_t sm_bpa_co_create(sm_bpa_app_t* _this){
    if(!sm_bsp_bpa_get_can_port()){
        LOG_ERR(TAG, "CanBus could NOT created");
        return -1;
    }

    _this->m_co_interface = sm_co_if_create_default(0,
                                                    NULL,
                                                    0,
                                                    sm_bsp_bpa_get_can_port());

    /// Create CanOpen Core
    _this->m_co = sm_co_create(NODE_ID_DEFAULT, true, _this->m_co_interface);
    sm_co_set_self_version(_this->m_co, _this->m_sw_version);
    memcpy(_this->m_ev_manu.m_sw_ver, _this->m_sw_version, 4);

    /// Create CanOpen SDO.
    sm_bpa_co_sdo_setting();

    return 0;
}

static int32_t sm_bpm_service_create(sm_bpa_app_t* _this){
    sm_bp_node_id_controller_t* node_id_controller = sm_can_master_get_node_id_if();
    if(!node_id_controller){
        LOG_ERR(TAG, "Node ID controller could NOT initialized");
        return -1;
    }
    _this->m_driver.m_nodeid_controller = node_id_controller;
    sm_sv_bp_t* bpm = sm_sv_bp_create(2, _this->m_co, true, _this->m_driver.m_nodeid_controller);
    if(!bpm){
        return -1;
    }

    _this->m_bp_service = bpm;
    sm_sv_bp_reg_event(_this->m_bp_service, &g_bp_event_handle, _this);

    _this->m_bpm = sm_bpm_handle_create(bpm, NULL);

    _this->m_modules.m_auth_bp = sm_auth_bp_create(_this->m_ev_manu.m_ev_sn,
                                                   &_this->m_config.m_auth_bp,
                                                   _this->m_bp_service,
                                                   sm_auth_bp_event_handle,
                                                   _this);

    return 0;
}

static int32_t sm_pms_service_create(sm_bpa_app_t* _this){
    if(!_this->m_bp_service){
        LOG_ERR(TAG, "BPM service is NOT initialized");
        return -1;
    }

    sm_pms_ctl_t* pms_ctl = sm_pms_ctl_create(_this->m_bp_service);
    if(!pms_ctl){
        LOG_ERR(TAG, "PMS Controller is NOT created");
        return -1;
    }
    _this->m_modules.m_pms_controller = pms_ctl;

    _this->m_pms_service = sm_sv_pms_create(_this->m_modules.m_pms_controller, _this->m_bp_service, NULL);

    return 0;
}

static int32_t sm_charger_service_create(sm_bpa_app_t* _this){
    if(!_this->m_bp_service){
        LOG_ERR(TAG, "BPM service is NOT initialized");
        return -1;
    }

    if(!_this->m_modules.m_pms_controller){
        LOG_ERR(TAG, "PMS controller is NOT initialized");
        return -1;
    }

    _this->m_driver.m_charger_if = sm_get_charger_if();

    sm_sv_charger_t* charger = sm_sv_charger_create(_this->m_driver.m_charger_if,
                                                     _this->m_modules.m_pms_controller,
                                                     NULL,
                                                     _this->m_bp_service);
    if(!charger){
        LOG_ERR(TAG, "Charger Service could NOT created");
        return -1;
    }

    _this->m_charger_service = charger;

    /// TODO: Set callback charger Service if need

    return 0;
}

static int32_t sm_sch_task_create(sm_bpa_app_t* _this){
    _this->m_sch_task = sm_sch_create_default();

    if(_this->m_co_interface){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_co_if_proc, _this);
    }

    if(_this->m_bp_service){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_bp_service_proc, _this);
    }

    if(_this->m_pms_service){
        sm_sch_start_task(_this->m_sch_task, 500, SM_SCH_REPEAT_FOREVER, sm_pms_service_proc, _this);
    }

/*    if(_this->m_iot_service){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_iot_service_proc, _this);
    }*/

 /*   if(_this->m_ota_service){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_ota_service_proc, _this);
    }*/

    if(_this->m_bpm){
        sm_sch_start_task(_this->m_sch_task, 0, SM_SCH_REPEAT_FOREVER, sm_bpm_handle_process, _this->m_bpm);
    }

    sm_sch_start_task(_this->m_sch_task,
                      5000,
                      SM_SCH_REPEAT_FOREVER,
                      sm_gps_proc,
                      _this);

    sm_sch_start_task(_this->m_sch_task,
                      0,
                      SM_SCH_REPEAT_FOREVER,
                      sm_net_monitor_process,
                      _this->m_modules.m_net_monitor);

    sm_sch_start_task(_this->m_sch_task,
                      0,
                      SM_SCH_REPEAT_FOREVER,
                      (sm_sch_task_fn_t)sm_bpa_system_reset,
                      _this);

    sm_sch_start_task(_this->m_sch_task,
                      500,
                      SM_SCH_REPEAT_FOREVER,
                      sm_auth_bp_process,
                      _this->m_modules.m_auth_bp);

    return 0;
}

sm_bpa_app_t* sm_bpa_app_create(){
    LOG_INF(TAG, "BPA Application start initializing........");
    g_bpa_app = &g_bpa_app_default;

    LOG_INF(TAG, "Initializing BPA Driver.............");
    if(sm_bpa_driver_init(g_bpa_app) < 0){
        LOG_ERR(TAG, "Initialized BPA Driver FAILURE");
    }

    sm_bpa_app_storage_init(g_bpa_app);

    if(sm_bpa_app_load_config(g_bpa_app) < 0){
        LOG_ERR(TAG, "PMU is loaded the configuration FAILURE, please check flash memory again");
        return NULL;
    }

    if(sm_bpa_co_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "CanOpen PMU initialized FAILURE");
        return NULL;
    }

    if(sm_bpm_service_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "BPM service initialized FAILURE");
        return NULL;
    }

    if(sm_pms_service_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "PMS service initialized FAILURE");
        return NULL;
    }

    if(sm_charger_service_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "Charger service initialized FAILURE");
        return NULL;
    }

    if(sm_net_service_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "NET service initialized FAILURE");
        return NULL;
    }

    if(sm_ota_service_create(g_bpa_app) < 0){
        LOG_ERR(TAG, "OTA Service initialized FAILURE");
        return NULL;
    }

    sm_sch_task_create(g_bpa_app);

    LOG_INF(TAG, "BPA Application start SUCCESS !!!");
    return g_bpa_app;
}

int32_t sm_bpa_app_init(sm_bpa_app_t* _app){
    if(!_app){
        return -1;
    }
    return 0;
}

int32_t sm_bpa_app_process(sm_bpa_app_t* _app){
    sm_bpa_app_t* bpa_app = _app;

    if(bpa_app){
        sm_sch_process(bpa_app->m_sch_task);
    }
    return 0;
}

int32_t sm_bpa_app_iot_process(sm_bpa_app_t* _arg){
    sm_iot_service_proc(_arg);
    sm_ota_service_proc(_arg);
    return 0;
}

void sm_bpa_system_reset(sm_bpa_app_t* _app){
    if(_app && _app->m_system_reboot_request.m_reboot){
        if(!elapsed_timer_get_remain(&_app->m_system_reboot_request.m_time)){
            sm_bsp_system_reset();
        }
    }
}


/******************************** Authentication BP EVENT HANDLE **********************************************/
static void sm_auth_bp_event_handle(uint8_t _type, uint8_t _port, uint8_t _level, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!app){
        return;
    }
    LOG_DBG(TAG, "Auth BP EVENT: type %s, port %d, level: %d", _type==SM_AUTH_BP_OFFLINE_TYPE ? "OFFLINE" : "ONLINE",
                                                                _port,
                                                                _level);

    /// TODO: Handle BP COULD NOT assign to Device. Can set OFF BP or BLOCK BP.
    if(_level == SM_AUTH_BP_INVALID_WARNING){
        sm_sv_bp_set_cmd(app->m_bp_service, _port, BP_CMD_STANDBY, NULL, NULL, NULL);
    }else if(_level == SM_AUTH_BP_INVALID){
        sm_sv_bp_set_cmd(app->m_bp_service, _port, BP_CMD_STANDBY, NULL, NULL, NULL);
        sm_sv_bp_set_cmd(app->m_bp_service, _port, BP_CMD_SET_BLOCK, NULL, NULL, NULL);
    }
}

/******************************** NET EVENT HANDLE **********************************************/
static void sm_net_event_handle(int32_t _event, void* _arg){
//    LOG_DBG(TAG, "Event from NET Monitor event: %d", _event);
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;

    if(_event == NET_RESET_CONNECTION){
        LOG_WRN(TAG, "Reset Network");
        app->m_init_flag.m_net_ready = false;
    }
}

/************************************** HANDLE HOST API **********************************/
void sm_iot_event_handle_on_connected(int32_t _success, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!_success){
        LOG_INF(TAG, "Event from IOT system: IOT Connection is established");
        sm_net_monitor_update_state(app->m_modules.m_net_monitor, NET_RECOVERING);
    }
}
void sm_iot_event_handle_on_disconnected(int32_t _success, void* _arg){
    LOG_WRN(TAG, "Event from IOT system: IOT Connection is disconnected");
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_net_monitor_update_state(app->m_modules.m_net_monitor, NET_LOSING);
}

static void sm_ev_on_bp_cmd(int32_t _slot, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg){
    (void*)_data;
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;

    LOG_DBG(TAG, "On BP %d CMD: %s", _slot, _success == SM_BP_CMD_SUCCESS ? "SUCCESS" : "FAILURE");

    if(_success == SM_BP_CMD_SUCCESS &&  _cmd == BP_CMD_WRITE_ASSIGNED_DEV){
        sm_sv_bp_reset(app->m_bp_service, _slot);
    }
}

int32_t sm_iot_event_handle_on_cmd_from_host(int32_t _cmd, void* _cmd_data, void* _arg){
    if(!_arg || _cmd >= SM_EV_CMD_NUMBER){
        LOG_WRN(TAG, "NOT support CMD from HOST");
        return -1;
    }
    LOG_INF(TAG, "Event from IOT system: Command from HOST: %d", _cmd);
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_bp_t* bp_service = app->m_bp_service;

    if(!bp_service) {
        return -1;
    }

    switch (_cmd) {
        case SM_EV_CMD_REBOOT_MODULE:{
            uint8_t module_type = *(uint8_t*)_cmd_data;
            if(module_type > SM_EV_MODULE_BP){
                uint8_t slot = module_type - BP_NODE_ID_OFFSET;
                LOG_WRN(TAG, "Reboot BP %d immediately", slot);
                return sm_sv_bp_set_cmd(bp_service,
                                        slot,
                                        BP_CMD_REBOOT,
                                        NULL,
                                        NULL,
                                        NULL);
            }else{
                app->m_system_reboot_request.m_reboot = true;
                elapsed_timer_resetz(&app->m_system_reboot_request.m_time, 500);

                LOG_WRN(TAG, "BPA reboot after 500 mini-seconds");
            }
            break;
        }
        case SM_EV_CMD_PORT_ENABLE:
        case SM_EV_CMD_PORT_LOCK:
        case SM_EV_CMD_PORT_FORCE:{
            sm_cmd_extended_data_t* data = (sm_cmd_extended_data_t*)_cmd_data;
            uint8_t port = data->m_id;
            uint8_t value = data->m_data[0];
            if (port >= SM_BP_NUMBER_DEFAULT) {
                LOG_ERR(TAG, "Port ID INVALID");
                return -1;
            }

            if(_cmd == SM_EV_CMD_PORT_ENABLE){
                LOG_WRN(TAG, "%s port %d", !value?"ENABLE":"DISABLE", port);
                sm_bpm_set_config(app->m_bpm, port, value);
                /// TODO: Store config to flash.
            }else if(_cmd == SM_EV_CMD_PORT_LOCK){
                LOG_WRN(TAG, "%s port %d in PMS", !value?"ENABLE":"DISABLE", port);
                if(!sm_sv_charger_is_charging(app->m_charger_service)){
                    sm_sv_pms_enable_port(app->m_pms_service, port, !value);
                }else{
                    sm_sv_charger_enable_port(app->m_charger_service, port, !value);
                }
            }else{
                LOG_WRN(TAG, "%s port %d", !value?"RELEASE":"FORCE", port);
                uint8_t charging = sm_sv_charger_is_charging(app->m_charger_service);
                if(!value){
                    if(charging){
                        sm_sv_charger_release_bp(app->m_charger_service);
                    }else{
                        sm_sv_pms_release_bp(app->m_pms_service);
                    }
                }else{
                    if(charging){
                        sm_sv_charger_force_bp(app->m_charger_service, port, NULL, NULL);
                    }else{
                        sm_sv_pms_force_discharging_bp(app->m_pms_service, port, NULL, NULL);
                    }
                }
            }
            break;
        }
        case SM_EV_CMD_SET_STATE_BP: {
            sm_cmd_extended_data_t* data = (sm_cmd_extended_data_t*)_cmd_data;
            uint8_t port = data->m_id;
            uint8_t value = data->m_data[0];
            if(port > SM_BP_NUMBER_DEFAULT){
                LOG_ERR(TAG, "Port ID INVALID");
                return -1;
            }

            if(value == BP_STATE_STANDBY){
                if(port == SM_BP_NUMBER_DEFAULT){
                    LOG_WRN(TAG, "System is POWER OFF when All bp are OFF");
                    sm_sv_bp_set_off_all(bp_service);
                }else {
                    sm_sv_bp_set_cmd(bp_service,
                                     port,
                                     BP_CMD_STANDBY,
                                     NULL,
                                     NULL,
                                     NULL);
                }
            }
            break;
        }
        case SM_EV_CMD_SET_CYCLE_BP: {
            sm_cmd_extended_data_t* data = (sm_cmd_extended_data_t*)_cmd_data;
            uint8_t port = data->m_id;
            if (port >= SM_BP_NUMBER_DEFAULT) {
                LOG_ERR(TAG, "Port ID INVALID");
                return -1;
            }

            app->m_cmd_buffer_temp[0] = data->m_data[1];
            app->m_cmd_buffer_temp[1] = data->m_data[0]; /// Write to BP is Little Endian, So must be reverse

            sm_sv_bp_set_cmd(bp_service,
                             port,
                             BP_CMD_SET_CYCLE,
                             (void*)(app->m_cmd_buffer_temp),
                             NULL,
                             NULL);
            break;
        }
        case SM_EV_CMD_SET_ACTIVE_BP:
        case SM_EV_CMD_SET_BLOCK_BP:{
            sm_cmd_extended_data_t* data = (sm_cmd_extended_data_t*)_cmd_data;
            uint8_t port = data->m_id;

            if(port >= SM_BP_NUMBER_DEFAULT){
                LOG_ERR(TAG, "Port ID INVALID");
                return -1;
            }

            uint8_t bp_cmd;
            if(_cmd == SM_EV_CMD_SET_BLOCK_BP){
                bp_cmd = BP_CMD_SET_BLOCK;
            }else{
                bp_cmd = BP_CMD_SET_ACTIVE;
            }

            LOG_WRN(TAG, "Set BP %d is %s", port, bp_cmd==BP_CMD_SET_BLOCK ? "BLOCK" : "ACTIVE");

            app->m_cmd_buffer_temp[0] = data->m_data[0];
            sm_sv_bp_set_cmd(bp_service,
                             port,
                             bp_cmd,
                             (void*)(app->m_cmd_buffer_temp),
                             NULL,
                             NULL);
            break;
        }
        case SM_EV_CMD_WRITE_DEV_TO_BP:{
            sm_cmd_extended_data_t* data = (sm_cmd_extended_data_t*)_cmd_data;
            uint8_t port = data->m_id;

            if(!data->m_data || port >= SM_BP_NUMBER_DEFAULT){
                LOG_ERR(TAG, "Port ID INVALID");
                return -1;
            }
            uint8_t data_len = strlen(data->m_data);
            memcpy(app->m_cmd_buffer_temp, data->m_data, data_len);
            app->m_cmd_buffer_temp[data_len] = '\0';

            sm_sv_bp_set_cmd(bp_service,
                             port,
                             BP_CMD_WRITE_ASSIGNED_DEV,
                             (void*)app->m_cmd_buffer_temp,
                             sm_ev_on_bp_cmd,
                             app);
            break;
        }
        default:
            LOG_ERR(TAG, "Command NOT Support");
            break;
    }

    return 0;
}

int32_t sm_iot_event_handle_on_cfg_from_host(int32_t _type, void* _data, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_ev_config_t* ev_config = &app->m_config.m_ev_config;
    if(_type == SM_EV_CONF_AUTH_BP){
        ev_config->m_auth_bp = *(uint8_t*)_data;
    }if(_type == SM_EV_CONF_LOCK_PORT){
        ev_config->m_lock_port = *(uint8_t*)_data;
    }
    return 0;
}

int32_t sm_iot_event_handle_on_cfg_completed(void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;

    if (app->m_storage.m_manu_storage->m_proc->store(app->m_storage.m_manu_storage, app->m_ev_manu.m_device_name) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return -1;
    }

    if (sm_ev_manu_storage_validate(app->m_storage.m_manu_storage,
                                    (const uint8_t*)DEVICE_KEY_DEFAULT, DEVICE_KEY_LENGTH) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture crc");
        return -1;
    }

    if (app->m_storage.m_config_storage->m_proc->store(app->m_storage.m_config_storage,
                                                          &app->m_config.m_ev_config) < 0) {
        LOG_ERR(TAG, "Could NOT store ev config");
        return -1;
    }

    app->m_system_reboot_request.m_reboot = true;
    elapsed_timer_resetz(&app->m_system_reboot_request.m_time, SYSTEM_REBOOT_WAITING_TIME_DEFAULT);

    return 0;
}

void sm_iot_event_on_bp_event(const char* bp_sn, int32_t _accepted, void* _arg){
    LOG_DBG(TAG, "Event BP %s from IOT system: %s", bp_sn, _accepted == SM_BP_ACCEPTED ? "ACCEPTED" : "REJECTED");
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_auth_bp_update_from_cloud(app->m_modules.m_auth_bp, bp_sn, _accepted);
}


