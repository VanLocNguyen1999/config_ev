//
// Created by vnbk on 07/08/2024.
//
#include "sm_app_hmi.h"
#include "sm_logger.h"

#include "sm_ev_handle.h"
#include "sm_ev_ota.h"
#include "sm_ev_ble.h"

#include "sm_sv_iot.h"

#include "sm_ev_config.h"
#include "sm_ev_net_config.h"

#include "sm_ev_manu_storage.h"
#include "sm_ev_storage.h"

#include "sm_ev_opt_storage.h"

#include "sm_ex_flash_storage_if.h"

#include "sm_hmi_flash_config.h"

#include "sm_bsp_hmi.h"

#include "sm_w25qxx.h"

#include "gps/sm_gps_l76x.h"
#include "uc1676_lcd.h"

#include "sm_ssl_file_storage_impl.h"

#include "sm_co_od_common.h"

#include "sm_time_utils.h"
#include "sm_math.h"

#ifdef SIMULATOR_EV
#include "net/paho_mqtt_network.h"
#endif

static uint8_t g_software_version[4] = {0, 1, 2, 0x01};

#define TAG "sm_app_hmi"

#ifdef OTA_TESTING
volatile sm_storage_t* g_test_storage;

volatile ota_report_t g_ota_report;
#endif

#define SYNC_TIME_PERIOD                    (60*1000)
#define SYNC_TIME_VERIFY                    (2000)

sm_hmi_app_t g_hmi_app_default = {
        .m_sch_task = NULL,
        .m_co = NULL,
        .m_co_interface = NULL,

        .m_ev_service = NULL,
        .m_ota_service = NULL,
        .m_iot_service = NULL,
        .m_bp_manager = NULL,
        .m_hmi_service = NULL,
        .m_upgrade_service = NULL,
        .m_ble_service = NULL,
        .m_sw_version = g_software_version,

        .m_modules = {
                .m_mqtt_client = NULL,
                .m_http_client = NULL,
                .m_gps = NULL,
                .m_net_monitor = NULL,
                .m_ble_slave = NULL,
        },
        .m_inactive_mode = NULL,
        .m_auth_bp = NULL,
        .m_driver = {
            .m_ec200_driver =NULL,
            .m_ble_driver = NULL,
            .m_uc1676 = NULL,
            .m_lcd_driver = NULL,
            .m_gps_driver = NULL
        },

        .m_config.m_iot_config = NULL,
        .m_storage.m_ev_opt_storage = NULL,
        .m_storage.m_ev_config_storage = NULL,
        .m_init_flag = {
                .m_ble_ready = false,
                .m_net_ready = false
        },
        .m_co_timer = {
                .m_start_time = 0,
                .m_duration = 1
        },
};

#ifdef SIMULATOR_EV
static void sm_co_service_proc(void* _arg);
static void sm_ev_service_proc(void* _arg);
static void sm_iot_service_proc(void* _arg);
static void sm_hmi_service_proc(void* _arg);
static void sm_hmi_app_main_thread(void* _arg);
#endif

static void sm_inactive_mode_proc(void* _arg);
static void sm_display_update(void* _arg);
static void sm_gps_proc(void* _arg);
static void sm_sync_time_proc(void* _arg);

void sm_iot_event_on_bp_event(const char* bp_sn, int32_t _accepted, void* _arg);
static sm_sv_iot_event_t g_iot_event_handle = {
        .on_ev_request_upgrade = sm_ota_handle_request_upgrade,
        .on_ev_new_fw = sm_ota_handle_ev_new_fw,
        .on_ev_finish_new_fw_extract = sm_ota_handle_finish_new_fw_extract,
        .on_bp_new_fw = sm_ota_handle_bp_new_fw,
        .on_bp_request_upgrade = NULL, //sm_ota_handle_request_bp_upgrade,
        .on_bp_event = sm_iot_event_on_bp_event,
        .on_cmd_ota_progress = NULL, //sm_ota_handle_cmd_progress,
};

void sm_iot_event_handle_on_connected(int32_t, void*);
void sm_iot_event_handle_on_disconnected(int32_t, void*);
int32_t sm_event_handle_on_cmd_from_host(int32_t, void*, void*);
int32_t sm_event_handle_on_cfg_from_host(int32_t, void*, void*);
int32_t sm_event_handle_on_cfg_completed(void*);
static sm_host_event_t g_host_iot_event_handle = {
        .on_connected = sm_iot_event_handle_on_connected,
        .on_disconnected = sm_iot_event_handle_on_disconnected,
        .on_cmd_from_host = sm_event_handle_on_cmd_from_host,
        .on_cfg_from_host = sm_event_handle_on_cfg_from_host,
        .on_cfg_completed = sm_event_handle_on_cfg_completed
};

static sm_sv_ble_event_t g_ble_event_handle = {
      .on_new_device_paired = sm_ble_on_new_paired,
      .on_new_ble_id = sm_ble_on_new_ble_id,
};

void sm_ble_event_handle_on_connected(int32_t, void*);
void sm_ble_event_handle_on_disconnected(int32_t, void*);
static sm_host_event_t g_host_ble_event_handle = {
        .on_connected = sm_ble_event_handle_on_connected,
        .on_disconnected = sm_ble_event_handle_on_disconnected,
        .on_cmd_from_host = sm_event_handle_on_cmd_from_host
};

static sm_host_sync_data_if_t g_host_sync_data = {
        .get_ev_manu = sm_ev_get_manu,
        .get_ev_module_info = sm_ev_get_module_info,
        .get_ev_data = sm_ev_get_data,
        .get_ev_config = sm_ev_get_ev_config,
        .get_ev_bp_data = sm_ev_get_bp_data,
        .get_network_data = sm_ev_get_network_data,
        .get_gps_data = sm_ev_get_gps_data,
        .get_optional_data = sm_ev_get_optional_data,
};

static sm_sv_ev_event_t g_ev_event_handle = {
        .on_ev_event = sm_ev_on_event,
        .on_bp_event = sm_ev_on_bp_event
};

static sm_ota_event_t g_ota_event_handle = {
        .on_start = sm_ota_on_start,
        .on_start_module = sm_ota_on_start_module,
        .on_downloading_status = sm_ota_on_downloading_status,
        .on_upgrading_status = sm_ota_on_upgrading_status,
        .on_finished = sm_ota_on_finished
};

static void sm_hmi_app_inactive_mode_event_handle(int32_t _event, void* _arg);
static void sm_hmi_app_net_event_handle(int32_t _event, void* _arg);
static uint8_t sm_hmi_app_store_manu_setting(void);
static uint8_t sm_hmi_app_load_manu_setting(void);
static uint8_t sm_hmi_app_reset_setting_handle(void);
static uint8_t sm_hmi_app_validate_manu_setting(void);
static uint8_t sm_hmi_app_store_ev_config_setting();
static uint8_t sm_hmi_app_load_ev_config_setting();
static uint8_t sm_hmi_app_store_ev_net_config_setting();
static uint8_t sm_hmi_app_load_ev_net_config_setting();

static void sm_sv_file_recv_callback(uint8_t event, void *arg) {
    switch (event) {
        case SM_SV_FILE_RECEIVER_START_RECV:
            LOG_INF(TAG, "even: Start recv file");
            break;
        case SM_SV_FILE_RECEIVER_START_ABORT:
            LOG_INF(TAG, "even: Recv file about internal");
            break;
        case SM_SV_FILE_RECEIVER_RECV_SUCCESS:
            LOG_INF(TAG, "even: Recv file success");
            break;
        case SM_SV_FILE_RECEIVER_RECV_FAILED:
            LOG_INF(TAG, "even:  Recv file failed");
            break;
        default:
            LOG_INF(TAG, "even: NOT SUPPORT");
    }
}


static int32_t sm_hmi_app_storage_fw_signature(const sm_fw_signature_t* signature, void* arg){
    (void)arg;

    sm_hmi_app_t* this = &g_hmi_app_default;

    LOG_INF(TAG, "Saved fw signature to internal flash");
    LOG_INF(TAG, "Fw address: 0x%x", signature->m_addr);
    LOG_INF(TAG, "Fw size: %u ", signature->m_size);
    LOG_INF(TAG, "Fw CRC: %d", signature->m_crc);
    LOG_INF(TAG, "Fw valid: %u", signature->m_is_valid);

    sm_boot_2_fw_setting_t fw_setting = {0,};

    sm_sec_storage_t* fw_signature_storage = this->m_storage.m_boot2_signature_storage;

    if(signature->m_addr != SM_HMI_DEFAULT_MAINAPP_ADDR){
        LOG_ERR(TAG, "HMI mainapp addr 0x%x is invalid, reject firmware", signature->m_addr);
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

int32_t sm_hmi_driver_init(sm_hmi_app_t* _this){
    LOG_INF(TAG, "Initialized LTE driver");
    _this->m_driver.m_ec200_driver = sm_ec200_create(sm_bsp_hmi_get_lte_power(),
                                                     sm_bsp_hmi_get_lte_uart(),
                                                     NULL);

    LOG_INF(TAG, "Initialized HMI driver (UC1766)");
    _this->m_driver.m_uc1676 = uc1676_create(sm_bsp_hmi_get_lcd_i2c(),
                                             sm_bsp_hmi_get_lcd_power());
    sm_drv_lcd_seg_t* lcd_driver = sm_drv_lcd_seg_create(_this->m_driver.m_uc1676);
    if(!lcd_driver){
        LOG_ERR(TAG, "Could NOT created LCD Driver: UC1766");
    }
    _this->m_driver.m_lcd_driver = lcd_driver;

    LOG_INF(TAG, "Initializing external flash w25qxx");
    w25qxx_Begin(&_this->m_driver.m_ex_flash_driver, sm_bsp_hmi_get_ext_mem(), sm_bsp_hmi_get_ext_mem_cs());
    w25qxx_Init(&_this->m_driver.m_ex_flash_driver);

    LOG_INF(TAG, "Initializing gps L76x");
    _this->m_driver.m_gps_driver = sm_l76x_init(sm_bsp_hmi_get_gps_uart(),
                                                sm_bsp_hmi_get_gps_reset());
    _this->m_modules.m_gps = sm_gps_create(_this->m_driver.m_gps_driver);

    LOG_INF(TAG, "Initializing BLE mdbt42q");
    _this->m_driver.m_ble_driver = sm_mdbt42q_create(sm_bsp_hmi_get_ble_uart(),
                                                     sm_bsp_hmi_get_ble_reset(),
                                                     sm_bsp_hmi_get_ble_indicator(),
                                                     sm_bsp_hmi_get_ble_wakeup());

    LOG_INF(TAG, "Initializing HMI IO");
    _this->m_modules.m_hmi_io.m_left_signal = sm_bsp_hmi_get_left_signal();
    _this->m_modules.m_hmi_io.m_right_signal = sm_bsp_hmi_get_right_signal();
    _this->m_modules.m_hmi_io.m_input_voltage = sm_bsp_hmi_get_input_vol();
    sm_hmi_io_set_signal(&_this->m_modules.m_hmi_io, SM_HAL_IO_OFF);
    sm_hmi_io_get_input_vol(&_this->m_modules.m_hmi_io);

    return 0;
}
int32_t sm_hmi_app_storage_init(sm_hmi_app_t* _this){
    sm_storage_t* storage = NULL;
    sm_hal_flash_t* data_flash = sm_bsp_hmi_get_data_flash();

    storage = sm_ev_manu_storage_create(data_flash, SM_STORAGE_EV_INFO_FLASH_ADDR);
    _this->m_storage.m_ev_manu_storage = storage;

    storage = sm_ev_security_storage_create_default(data_flash, SM_STORAGE_EV_BACKUP_ODO_FLASH_ADDR, sizeof(int32_t));
    _this->m_storage.m_backup_odo_storage = storage;

    storage = sm_ev_odo_storage_create_default(data_flash, SM_STORAGE_EV_ODO_FLASH_ADDR);
    _this->m_storage.m_odo_storage = storage;

    storage = sm_ev_security_storage_create(data_flash, SM_HMI_FW_SIGNATURE_FLASH_ADDR, sizeof(sm_boot_2_fw_setting_t));
    _this->m_storage.m_boot2_signature_storage = storage;

    storage = sm_ev_opt_storage_create_default(data_flash, SM_STORAGE_EV_OPT_FLASH_ADDR);
    _this->m_storage.m_ev_opt_storage = storage;

    storage = sm_ev_config_create_default(data_flash, SM_STORAGE_EV_CONFIG_FLASH_ADDR, SM_EV_CONFIG_SIZE_OF);
    _this->m_storage.m_ev_config_storage = storage;

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
    sm_ex_flash_proc_t* ex_flash_proc = sm_sv_ex_flash_storage_get_w25q_proc(&g_hmi_app_default.m_driver.m_ex_flash_driver);
#endif
    sm_ex_flash_storage_t* ex_storage = sm_flash_storage_create_default(ex_flash_proc);
    sm_ex_flash_storage_init(ex_storage);

    sm_ssl_storage_init_ext_flash_instance(ex_storage);

    _this->m_storage.m_downloaded_fw_storage = sm_ex_flash_storage_create_partition(ex_storage, SM_HMI_HEX_FILE_STORAGE_ADDR, SM_FW_DOWNLOADED_STORAGE_SIZE);

    _this->m_storage.m_hmi_new_fw_storage = sm_ex_flash_storage_create_partition(ex_storage, SM_HMI_MAINAPP_FW_STORAGE_ADDR, SM_FW_NEW_HMI_STORAGE_SIZE);

    return 0;
}

static const char* EV_SN_DEFAULT = "CM005";
static char BLE_UUID[32] = {'\0',};

int32_t sm_hmi_app_load_config(sm_hmi_app_t* _this){
    /// LOAD Manufacture Info
    if(_this->m_storage.m_ev_manu_storage->m_proc->load(_this->m_storage.m_ev_manu_storage,
                                                        &_this->m_ev_manu) < 0){
        LOG_ERR(TAG, "Could NOT load EV Manufacture, now storage default value");
        sm_module_reset_data(&_this->m_ev_manu);

        memcpy(_this->m_ev_manu.m_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
        memcpy(_this->m_ev_manu.m_ev_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));

        _this->m_storage.m_ev_manu_storage->m_proc->store(_this->m_storage.m_ev_manu_storage,
                                                          _this->m_ev_manu.m_device_name);
        sm_ev_manu_storage_validate(_this->m_storage.m_ev_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);
    }else{
        LOG_INF(TAG, "Load EV manu SUCCEED, EV manu is:");
        LOG_INF(TAG, "Ev self sn: %s", _this->m_ev_manu.m_sn);
        LOG_INF(TAG, "Ev sn: %s", _this->m_ev_manu.m_ev_sn);
    }

//#define TEST
#ifdef TEST
        memset(_this->m_ev_manu.m_sn, 0, 32);
        memset(_this->m_ev_manu.m_ev_sn, 0, 32);

        memcpy(_this->m_ev_manu.m_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
        memcpy(_this->m_ev_manu.m_ev_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
#endif

    if(_this->m_ev_manu.m_registration != SM_MODULE_INFO_REGISTRATION && _this->m_ev_manu.m_registration != SM_MODULE_INFO_PRODUCTION){
        _this->m_ev_manu.m_registration = SM_MODULE_INFO_PRODUCTION;
        _this->m_storage.m_ev_manu_storage->m_proc->store(_this->m_storage.m_ev_manu_storage,
                                                                  _this->m_ev_manu.m_device_name);
         sm_ev_manu_storage_validate(_this->m_storage.m_ev_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);
    }

    if(_this->m_ev_manu.m_wheel_radius < SM_EV_CONFIG_WHEEL_RADIUS_MIN ||
        _this->m_ev_manu.m_wheel_radius > SM_EV_CONFIG_WHEEL_RADIUS_MAX) {
        _this->m_ev_manu.m_wheel_radius = SM_EV_S2_CONFIG_WHEEL_RADIUS_DEFAULT;
        _this->m_storage.m_ev_manu_storage->m_proc->store(_this->m_storage.m_ev_manu_storage,
                                                          _this->m_ev_manu.m_device_name);
        sm_ev_manu_storage_validate(_this->m_storage.m_ev_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);
    }

    /// LOAD EV configuration
    int32_t ret = _this->m_storage.m_ev_config_storage->m_proc->load(_this->m_storage.m_ev_config_storage,
                                                                     &_this->m_config.m_ev_config);
    if(ret == -1){
        LOG_ERR(TAG, "Could NOT load EV Configuration, now set default value");
        sm_ev_config_reset_default( &_this->m_config.m_ev_config);

        _this->m_storage.m_ev_config_storage->m_proc->store(_this->m_storage.m_ev_config_storage,
                                                            &_this->m_config.m_ev_config);
    }else if(ret == -2){
        sm_ev_config_t config;
        sm_ev_config_clone(&_this->m_config.m_ev_config, &config);

        if(!sm_ev_config_validate(&config)) {
            LOG_WRN(TAG, "Config Param INVALID, Re-config again");
            sm_ev_config_clone(&config, &_this->m_config.m_ev_config);
            _this->m_storage.m_ev_config_storage->m_proc->store(_this->m_storage.m_ev_config_storage,
                                                                &_this->m_config.m_ev_config);
        }
    }else{
        LOG_INF(TAG, "Load EV config SUCCEED");
    }

    if(!sm_ev_config_validate(&_this->m_config.m_ev_config)) {
        LOG_WRN(TAG, "Config Param INVALID, Re-config again");
        _this->m_storage.m_ev_config_storage->m_proc->store(_this->m_storage.m_ev_config_storage,
                                                            &_this->m_config.m_ev_config);
    }

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


    _this->m_config.m_iot_config.m_host = (const char*)_this->m_config.m_net_config.m_host;
//    _this->m_config.m_iot_config.m_host = (const char*)SM_IOT_MQTT_HOST_DEFAULT;
    _this->m_config.m_iot_config.m_port = _this->m_config.m_net_config.m_port;
    _this->m_config.m_iot_config.m_username = (const char*)_this->m_config.m_net_config.m_user;
    _this->m_config.m_iot_config.m_password = (const char*)_this->m_config.m_net_config.m_pass;
    _this->m_config.m_iot_config.m_client_id = (const char*)_this->m_ev_manu.m_ev_sn;

    /// Config BLE
    _this->m_config.m_ble_config.m_device_paired[0] = BLE_UUID;
    sm_ev_opt_load_ble_uuid(g_hmi_app_default.m_storage.m_ev_opt_storage,  (uint8_t*)BLE_UUID);
    _this->m_config.m_ble_config.m_name = _this->m_ev_manu.m_ev_sn;

    /// Config LTE
    sm_ec200_config_t lte_config = {
            .m_apn = (const char*)_this->m_config.m_net_config.m_lte_apn,
            .m_user = (const char*)_this->m_config.m_net_config.m_lte_user,
            .m_pass = (const char*)_this->m_config.m_net_config.m_lte_pass,
    };
    sm_ec200_set_config(_this->m_driver.m_ec200_driver, &lte_config);

    return 0;
}

int32_t sm_net_service_create(sm_hmi_app_t *_this) {
    if (!_this->m_driver.m_ec200_driver) {
        LOG_ERR(TAG, "Could NOT initialize EC200 driver");
        return -1;
    }

#ifdef SIMULATOR_EV
    mqtt_network_if_t* paho_mqtt_if = mqtt_network_create();
    _this->m_modules.m_mqtt_client = sm_mqtt_init(paho_mqtt_if);
    _this->m_modules.m_http_client = sm_http_client_create(NULL); //CURL HTTP
#else
    _this->m_modules.m_mqtt_client = sm_mqtt_client_create(_this->m_driver.m_ec200_driver);
    _this->m_modules.m_http_client = sm_http_client_create(_this->m_driver.m_ec200_driver);
#endif

    _this->m_iot_service = sm_sv_iot_create(_this->m_ev_manu.m_ev_sn,
                                            _this->m_modules.m_mqtt_client,
                                            &_this->m_config.m_iot_config,
                                            &g_iot_event_handle,
                                            _this);

    sm_host_api_t *host = (sm_host_api_t *) _this->m_iot_service;
    g_host_sync_data.m_arg = _this;
    host->m_proc->init(host, &g_host_sync_data, _this->m_config.m_ev_config.m_sync_time);
    sm_host_api_reg_event_handle(_this->m_iot_service,
                                 &g_host_iot_event_handle,
                                 &g_hmi_app_default);

    /// Create Net Monitor
    _this->m_modules.m_net_monitor = sm_net_monitor_create(SM_NET_MONITOR_DETECTED_TIME_DEFAULT,
                                                           sm_hmi_app_net_event_handle,
                                                           _this);

    return 0;
}

int32_t sm_ble_service_create(sm_hmi_app_t* _this){
    if(!_this->m_driver.m_ble_driver){
        LOG_ERR(TAG, "Could NOT initialized BLE driver");
        return -1;
    }
    _this->m_modules.m_ble_slave = sm_ble_sl_create(_this->m_driver.m_ble_driver);
    _this->m_ble_service = sm_sv_ble_create(_this->m_modules.m_ble_slave,
                                                       &_this->m_config.m_ble_config,
                                                       &g_ble_event_handle,
                                                       _this);
    sm_host_api_reg_event_handle(g_hmi_app_default.m_ble_service,
                                 &g_host_ble_event_handle,
                                 _this);

    return 0;
}

int32_t sm_ev_service_create(sm_hmi_app_t* _this){
    if(!sm_bsp_hmi_get_can_port()){
        LOG_ERR(TAG, "CanBus could NOT created");
        return -1;
    }
#ifdef SIMULATOR_EV
    _this->m_co_interface = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF,
                                                               CO_HOST_DEFAULT,
                                                               CO_PORT_DEFAULT,
                                                               _this);
#else
    _this->m_co_interface = sm_co_if_create_default(0,
                                                    NULL,
                                                    0,
                                                    sm_bsp_hmi_get_can_port());
#endif

    /// Create CanOpen Core
    _this->m_co = sm_co_create(8, false, _this->m_co_interface);
    sm_co_set_self_version(_this->m_co, _this->m_sw_version);
    memcpy(_this->m_ev_manu.m_sw_ver, _this->m_sw_version, 4);
    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_WRITE_INFO_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_store_manu_setting,
                                &_this->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_VALIDATE_CRC_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_validate_manu_setting,
                                &_this->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_RESET_SETTING_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_reset_setting_handle,
                                &_this->m_reset_setting_request);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_INFO_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_load_manu_setting,
                                &_this->m_ev_manu);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_LTE_SIMNB_SUB_INDEX,
                                NULL,
                                &_this->m_driver.m_ec200_driver->simNb);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_CONFIG_WRITE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_store_ev_config_setting,
                                &_this->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_CONFIG_READ_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_load_ev_config_setting,
                                &_this->m_config.m_ev_config);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_NET_CONFIG_WRITE_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_store_ev_net_config_setting,
                                &_this->m_ev_manu_cfg_buff);

    sm_co_sdo_server_set_handle(_this->m_co,
                                SDO_EV_CONFIG_INDEX,
                                SDO_EV_NET_CONFIG_READ_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_hmi_app_load_ev_net_config_setting,
                                &_this->m_config.m_net_config);


    /// Create BP service
    _this->m_bp_manager = sm_sv_bp_create(SM_SV_BP_NUMBER_DEFAULT,
                                          _this->m_co,
                                          false,
                                          NULL);

    /// Create EV service
    _this->m_ev_service = sm_sv_ev_create(&_this->m_ev_manu,
                                          _this->m_co,
                                          _this->m_bp_manager,
                                          &_this->m_config.m_ev_config,
                                          _this->m_storage.m_backup_odo_storage,
                                          _this->m_storage.m_odo_storage,
                                          _this->m_storage.m_ev_opt_storage);

    sm_sv_ev_reg_event(_this->m_ev_service, &g_ev_event_handle, _this);

    return 0;
}

int32_t sm_ota_service_create(sm_hmi_app_t* _this){
    if(!_this->m_modules.m_http_client){
        LOG_ERR(TAG, "Could NOT created HTTP Client");
        return -1;
    }

    _this->m_flag_sys_reset = false;
    _this->m_upgrade_service = sm_sv_ev_upgrade_create_default(_this->m_co);
    _this->m_ota_service = sm_sv_ota_create(_this->m_modules.m_http_client,
                                            _this->m_upgrade_service,
                                            _this->m_ev_service,
                                            _this->m_storage.m_downloaded_fw_storage);

    sm_sv_ota_reg_event(_this->m_ota_service, &g_ota_event_handle, _this);

    sm_sv_ota_set_fw_output_storage_if(_this->m_ota_service,
                                       _this->m_storage.m_hmi_new_fw_storage,
                                       sm_hmi_app_storage_fw_signature,
                                       _this);
    return 0;
}

int32_t sm_file_recv_service_create(sm_hmi_app_t* _this){
    if(!_this->m_co){
        return -1;
    }

    sm_ssl_storage_proc_t *ssl_proc = sm_ssl_storage_get_ext_flash_instance();

    sm_file_output_if_t *file_output = sm_file_receiver_get_flash_output(ssl_proc);

    _this->m_file_recv_service = sm_sv_file_receiver_create_default(_this->m_co, file_output, sm_sv_file_recv_callback, _this);

    return 0;
}

int32_t sm_hmi_service_create(sm_hmi_app_t* _this){
    if(!_this->m_driver.m_lcd_driver){
        LOG_ERR(TAG, "Could NOT create LCD driver");
        return -1;
    }
    _this->m_hmi_service = sm_sv_hmi_create(_this->m_driver.m_lcd_driver, _this->m_sch_task);
    sm_sv_hmi_power_off(_this->m_hmi_service);
    return 0;
}

int32_t sm_hmi_bp_auth_create(sm_hmi_app_t* _this){
    sm_auth_bp_t* auth_bp = sm_auth_bp_create(_this->m_ev_manu.m_ev_sn,
                                         &_this->m_config.m_auth_bp,
                                         _this->m_bp_manager,
                                         _this->m_ev_service);
    if(!auth_bp){
        LOG_ERR(TAG, "Create BP authentication module FAILURE");
        return -1;
    }

    _this->m_auth_bp = auth_bp;

    return 0;
}

void sm_task_sch_create(sm_hmi_app_t* _this){
    _this->m_sch_task = sm_sch_create_default();

    sm_sch_start_task(_this->m_sch_task,
                      5000,
                      SM_SCH_REPEAT_FOREVER,
                      sm_gps_proc,
                      _this);

    sm_sch_start_task(_this->m_sch_task,
                      5000,
                      SM_SCH_REPEAT_FOREVER,
                      sm_display_update,
                      _this);

    sm_sch_start_task(_this->m_sch_task,
                      SYNC_TIME_VERIFY,
                      SM_SCH_REPEAT_FOREVER,
                      sm_sync_time_proc,
                      _this);

    sm_sch_start_task(_this->m_sch_task,
                      0,
                      SM_SCH_REPEAT_FOREVER,
                      sm_net_monitor_process,
                      _this->m_modules.m_net_monitor);

    /// Check INACTIVE MODE ENABLE.
    if(_this->m_config.m_ev_config.m_inactive_mode_config.m_enable){
        sm_sch_start_task(_this->m_sch_task,
                          10000,
                          SM_SCH_REPEAT_FOREVER,
                          sm_inactive_mode_proc,
                          &g_hmi_app_default);
    }

    /// Check Auth BP
    sm_sch_start_task(_this->m_sch_task,
                          500,
                          SM_SCH_REPEAT_FOREVER,
                          sm_auth_bp_process,
                          g_hmi_app_default.m_auth_bp);
}

int32_t sm_hmi_app_init(){
    LOG_INF(TAG, "Start initialized HMI APP....................................................");

    LOG_INF(TAG, "Initializing driver.....");
    sm_hmi_driver_init(&g_hmi_app_default);

    LOG_INF(TAG, "Create EV storage partition");
    sm_hmi_app_storage_init(&g_hmi_app_default);

    LOG_INF(TAG, "Loading configuration.......");
    sm_hmi_app_load_config(&g_hmi_app_default);

    LOG_INF(TAG, "Creating services .......");

    /// Create EV Service
    if(sm_ev_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Create EV service FAILURE");
        return -1;
    }

    /// Create NET services
    if(sm_net_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Create NET services FAILURE");
        return -1;
    }

    /// Create OTA Service
    if(sm_ota_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Create OTA service FAILURE");
        return -1;
    }

    /// Create BLE service
    if(sm_ble_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Create BLE service FAILURE");
        return -1;
    }

    if(sm_file_recv_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Create FILE RECV service FAILURE");
        return -1;
    }

    /// Create Sync TIME
    g_hmi_app_default.m_sync_time = sm_sync_time_create(g_hmi_app_default.m_modules.m_gps,
                                                        sm_bsp_hmi_get_rtc(),
                                                        SM_TIME_ZONE_DEFAULT);
    elapsed_timer_resetz(&g_hmi_app_default.m_sync_time_period, SYNC_TIME_VERIFY);

    /// Create Auth BP
    if(sm_hmi_bp_auth_create(&g_hmi_app_default) < 0){
        return -1;
    }

    /// Create scheduler tasks
    sm_task_sch_create(&g_hmi_app_default);

    /// Create HMI service
    if(sm_hmi_service_create(&g_hmi_app_default) < 0){
        LOG_ERR(TAG, "Crate HMI service FAILURE");
        return -1;
    }

    /// Create INACTIVE MODE
    g_hmi_app_default.m_inactive_mode = sm_inactive_mode_create(g_hmi_app_default.m_ev_service,
                                                                g_hmi_app_default.m_storage.m_ev_opt_storage,
                                                                &g_hmi_app_default.m_config.m_ev_config.m_inactive_mode_config,
                                                                sm_hmi_app_inactive_mode_event_handle,
                                                                &g_hmi_app_default);

    /// Reset CO timer
    elapsed_timer_resetz(&g_hmi_app_default.m_co_timer, 1);

#ifdef __RTOS
    MUTEX_INIT(g_hmi_app_default.m_lock);
#endif

#ifdef SIMULATOR_EV
    g_hmi_app_default.m_thread_pool = thpool_init(4);

//    thpool_add_work(g_hmi_app_default.m_thread_pool, sm_hmi_app_ev_process, g_hmi_app_default.m_ev_service);
//    thpool_add_work(g_hmi_app_default.m_thread_pool, sm_hmi_app_net_process, g_hmi_app_default.m_co);
    thpool_add_work(g_hmi_app_default.m_thread_pool, sm_hmi_app_ble_process, g_hmi_app_default.m_iot_service);
    thpool_add_work(g_hmi_app_default.m_thread_pool, sm_hmi_service_proc, g_hmi_app_default.m_hmi_service);

    thpool_add_work(g_hmi_app_default.m_thread_pool, sm_hmi_app_main_thread, &g_hmi_app_default);
#endif
    return 0;
}

int32_t sm_hmi_app_ev_process(sm_hmi_app_t *_app) {
    sm_co_if_process(_app->m_co_interface);
    if(!elapsed_timer_get_remain(&_app->m_co_timer)) {
        sm_co_process(_app->m_co, 1);
        elapsed_timer_reset(&_app->m_co_timer);
    }

    sm_sv_ota_process(_app->m_ota_service);
    sm_sv_bp_process(_app->m_bp_manager);

    if(sm_sv_ota_get_status(_app->m_ota_service) == SM_OTA_STT_IDLE){
        sm_sv_hmi_process(_app->m_hmi_service);
        sm_sv_ev_process(_app->m_ev_service);
        if(_app->m_file_recv_service){
            sm_sv_file_receiver_process(_app->m_file_recv_service);
        }
        sm_sch_process(_app->m_sch_task);
    }
    return 0;
}

int32_t sm_net_init(sm_hmi_app_t* _app){
    if(sm_ec200_init(_app->m_driver.m_ec200_driver) < 0){
        return -1;
    }

    if( sm_mqtt_init(_app->m_modules.m_mqtt_client) < 0 || sm_http_client_init(_app->m_modules.m_http_client)){
        return -1;
    }

    _app->m_init_flag.m_net_ready = true;

    return 0;
}

int32_t sm_hmi_app_net_process(sm_hmi_app_t* _app){
    if(!_app->m_init_flag.m_net_ready && sm_net_init(_app) < 0){
        delayMs(2000);
        return -1;
    }

    if(_app->m_iot_service){
        if(_app->m_iot_service->m_proc->process(_app->m_iot_service) < 0){
            _app->m_init_flag.m_net_ready = false;
        }
    }

    if(_app->m_iot_service->m_proc->is_connected(_app->m_iot_service)){
        if(!elapsed_timer_get_remain(&_app->m_sync_time_period)){
            sys_datetime_t p_time;
            if (sm_ec200_get_time_network(_app->m_driver.m_ec200_driver, &p_time) >= 0){

#ifdef __RTOS
    ENTER_CRITICAL(_app->m_lock);
#endif
                sm_sync_time_set_time(_app->m_sync_time, &p_time, true);

#ifdef __RTOS
    EXIT_CRITICAL(_app->m_lock);
#endif
            }
            sm_ec200_get_signal(_app->m_driver.m_ec200_driver);
            elapsed_timer_resetz(&_app->m_sync_time_period, SYNC_TIME_PERIOD);
        }
    }

    return 0;
}

int32_t sm_hmi_app_ble_process(sm_hmi_app_t* _app){
    if(!_app->m_init_flag.m_ble_ready && sm_ble_sl_init(_app->m_modules.m_ble_slave,
                                                        _app->m_ev_manu.m_ev_sn) < 0){
        delayMs(2000);
        return -1;
    }else{
        _app->m_init_flag.m_ble_ready = true;
    }

    if( _app->m_ble_service){
        _app->m_ble_service->m_proc->process(_app->m_ble_service);
    }

    return 0;
}

#ifdef SIMULATOR_EV
static void sm_ev_service_proc(void* _arg){
    sm_sv_ev_t* service = (sm_sv_ev_t*)_arg;
    while (1){
        sm_sv_ev_process(service);
    }
}

static void sm_co_service_proc(void* _arg){
    sm_co_t* service = (sm_co_t*)_arg;
    elapsed_timer_t co_timer = {
            .m_duration = 1,
            .m_start_time = (int32_t)get_tick_count(),
    };
    while (1){
        if(!elapsed_timer_get_remain(&co_timer)){
            sm_co_process(service, 1);
            elapsed_timer_reset(&co_timer);
        }
    }
}

static void sm_iot_service_proc(void* _arg){
    sm_host_api_t* iot_service = (sm_host_api_t*)_arg;
    iot_service->m_proc->init(iot_service, NULL, 0);
    while (1){
        iot_service->m_proc->process(iot_service);
    }
}

static void sm_hmi_service_proc(void* _arg){
    sm_sv_hmi_t* hmi_service = (sm_sv_hmi_t*)_arg;
    sm_sv_hmi_init_default(hmi_service);
    while (1){
        sm_sv_hmi_process(hmi_service);
    }
}

static void sm_hmi_app_main_thread(void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    elapsed_timer_t co_timer = {
            .m_duration = 1,
            .m_start_time = (int32_t)get_tick_count(),
    };
    while (1) {
        sm_co_if_process(app->m_co_interface);
        if (!elapsed_timer_get_remain(&co_timer)) {
            sm_co_process(app->m_co, 1);
            elapsed_timer_reset(&co_timer);
        }
        sm_sv_ev_process(app->m_ev_service);
    }
}

#endif

static void sm_sync_time_proc(void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sys_datetime_t datetime;

    ENTER_CRITICAL(app->m_lock);
    if(sm_sync_time_get_time(app->m_sync_time, &datetime) < 0){
        EXIT_CRITICAL(app->m_lock);
        return;
    }
    EXIT_CRITICAL(app->m_lock);

    if(!app->m_init_flag.m_sync_time){
        app->m_init_flag.m_sync_time = true;
        sm_sch_cancel_taskz(app->m_sch_task, sm_sync_time_proc);
        sm_sch_start_task(app->m_sch_task,
                            SYNC_TIME_PERIOD,
                            SM_SCH_REPEAT_FOREVER,
                            sm_sync_time_proc,
                            _arg);
    }

    sm_sv_hmi_set_time(app->m_hmi_service, datetime.hour, datetime.min);
}

static void sm_inactive_mode_proc(void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_inactive_mode_process(app->m_inactive_mode);
}

static void sm_gps_proc(void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    app->m_modules.m_gps->proc->process(app->m_modules.m_gps);
}

static void sm_display_update(void *_arg) {
    sm_hmi_app_t *app = (sm_hmi_app_t *) _arg;
    sm_sv_bp_t *bp_service = app->m_bp_manager;
    const sm_bp_data_t *bp_data = NULL;
    const sm_ev_data_t* ev_data = sm_sv_ev_get_data(app->m_ev_service);

    if(ev_data->m_pmu_data->m_key == EV_KEY_OFF){
        return;
    }

    for (uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++) {
        if (!sm_sv_bp_is_connected(bp_service, index)) {
            continue;
        }
        bp_data = sm_sv_bp_get_data(bp_service, index);
        if (sm_auth_bp_get_status_local(app->m_auth_bp, index) != SM_AUTH_BP_VALID ||
                sm_auth_bp_get_status_cloud(app->m_auth_bp, index) != SM_AUTH_BP_VALID ||
                bp_data->m_state == BP_STATE_FAULT ||
                ev_data->m_pmu_data->m_bp_checking_state[index] == EV_BP_INVALID ||
                ev_data->m_pmu_data->m_port_lock_status[index] == EV_LOCK_PORT) {
            if(bp_data->m_state == BP_STATE_FAULT){
                sm_sv_hmi_blink_bp(app->m_hmi_service,
                                   index,
                                   bp_data->m_soc,
                                   bp_data->m_temps[0],
                                   2*SM_SV_HMI_BLINK_DURATION_DEFAULT);
            }else{
                sm_sv_hmi_blink_bp(app->m_hmi_service,
                                   index,
                                   bp_data->m_soc,
                                   bp_data->m_temps[0],
                                   SM_SV_HMI_BLINK_DURATION_DEFAULT);
            }
        } else {
            sm_sv_hmi_set_bp(app->m_hmi_service,
                             index,
                             bp_data->m_soc,
                             bp_data->m_temps[0],
                             SM_DRV_LCD_SHOW);
        }
    }

    if(ev_data->m_pmu_data->m_parking == EV_EXIT_PARKING){
        sm_sv_hmi_set_mc_temp(app->m_hmi_service, ev_data->m_mc_data->m_motor_temp, SM_DRV_LCD_SHOW);
    }
}

/************************************** HANDLE HOST API **********************************/
void sm_iot_event_handle_on_connected(int32_t _success, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    if(!_success){
        LOG_INF(TAG, "Event from IOT system: IOT Connection is established");
        sm_net_monitor_update_state(app->m_modules.m_net_monitor, NET_RECOVERING);
    }
}
void sm_iot_event_handle_on_disconnected(int32_t _success, void* _arg){
    LOG_WRN(TAG, "Event from IOT system: IOT Connection is disconnected");
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_net_monitor_update_state(app->m_modules.m_net_monitor, NET_LOSING);
}

/*static void sm_ev_on_cmd(int32_t _is_success, uint8_t _cmd, void* _data, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    if(!app){
        return;
    }

    if(_cmd == SM_EV_CMD_BLOCK_EV || _cmd == SM_EV_CMD_LOCK_EV){
        sm_sv_hmi_blink_warning_icon(app->m_hmi_service, EV_ERR_BLOCK_STATE);
    }
}*/

int32_t sm_event_handle_on_cmd_from_host(int32_t _cmd, void* _cmd_data, void* _arg){
    if(_cmd >= SM_EV_CMD_NUMBER){
        LOG_WRN(TAG, "NOT support CMD from HOST");
        return -1;
    }
   // LOG_INF(TAG, "Event from IOT system: Command from HOST: %d", _cmd);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ev_t* ev_service = app->m_ev_service;

    if(ev_service) {
        sm_sv_ev_set_cmd(ev_service, _cmd, _cmd_data, NULL, NULL);
    }

    uint8_t value = *(uint8_t*)_cmd_data;
    if(_cmd == SM_EV_CMD_CONTROL_SIGNAL){
        if(value){
            sm_sch_start_task(app->m_sch_task, 1000, SM_SCH_REPEAT_FOREVER, (sm_sch_task_fn_t)sm_hmi_io_blink_signal, &app->m_modules.m_hmi_io);
        }else{
            sm_sch_cancel_taskz(app->m_sch_task, (sm_sch_task_fn_t)sm_hmi_io_blink_signal);
            sm_hmi_io_set_signal(&app->m_modules.m_hmi_io, value);
        }
    }
    if(_cmd == SM_EV_CMD_INACTIVE_EV && value){
        sm_inactive_mode_reset(app->m_inactive_mode);
    }
    return 0;
}

int32_t sm_event_handle_on_cfg_from_host(int32_t _type, void* _data, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_ev_config_t* ev_config = &app->m_config.m_ev_config;
    sm_ev_manu_t* ev_manu = &app->m_ev_manu;
    switch (_type) {
        case SM_EV_CONF_AUTH_BP:
            ev_config->m_auth_bp = *(uint8_t*)_data;
            break;
        case SM_EV_CONF_UPHILL_MODE:
            ev_config->m_uphill_mode = *(uint8_t*)_data;
            break;
        case SM_EV_CONF_LOCK_PORT:
            ev_config->m_lock_port = *(uint8_t*)_data;
            break;
        case SM_EV_CONF_INACTIVE_MODE: {
            sm_ev_inactive_mode_config_t* inactive_mode = (sm_ev_inactive_mode_config_t*)_data;
            ev_config->m_inactive_mode_config.m_enable = inactive_mode->m_enable;
            ev_config->m_inactive_mode_config.m_save_time = inactive_mode->m_save_time;
            ev_config->m_inactive_mode_config.m_km_warning = inactive_mode->m_km_warning;
            ev_config->m_inactive_mode_config.m_km_force_stop = inactive_mode->m_km_force_stop;
            break;
        }
        case SM_EV_CONF_WHEEL_RADIUS:
            ev_manu->m_wheel_radius = *(float*)_data;
            break;
        case SM_EV_CONF_KM_ODO_STORED:
            ev_config->m_km_store_odo = *(uint8_t*)_data;
            break;
        case SM_EV_CONF_ODO_PASS:
            memcpy(ev_config->m_odo_pass, _data, SM_EV_CONFIG_ODO_PASS_LENGTH);
            ev_config->m_odo_pass[SM_EV_CONFIG_ODO_PASS_LENGTH - 1] = '\0';
            break;
        case SM_EV_CONF_SYNC_TIME:
            ev_config->m_sync_time = *(uint8_t*)_data;
            break;
        case SM_EV_CONF_AUTH_MODULE: {
            sm_auth_module_config_t *config = (sm_auth_module_config_t *) _data;
            ev_config->m_auth_module.m_level = config->m_level;
            ev_config->m_auth_module.m_detected_time = config->m_detected_time;
            break;
        }
        default:
            break;
    }
    return 0;
}

int32_t sm_event_handle_on_cfg_completed(void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;

    if (app->m_storage.m_ev_manu_storage->m_proc->store(app->m_storage.m_ev_manu_storage, app->m_ev_manu.m_device_name) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return -1;
    }

    if (sm_ev_manu_storage_validate(app->m_storage.m_ev_manu_storage,
                                    (const uint8_t*)DEVICE_KEY_DEFAULT, DEVICE_KEY_LENGTH) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture crc");
        return -1;
    }

    if (app->m_storage.m_ev_config_storage->m_proc->store(app->m_storage.m_ev_config_storage,
                                                          &app->m_config.m_ev_config) < 0) {
        LOG_ERR(TAG, "Could NOT store ev config");
        return -1;
    }

    sm_sv_ev_reboot_module(app->m_ev_service, SM_EV_MODULE_HMI, 1000);

    return 0;
}

void sm_iot_event_on_bp_event(const char* bp_sn, int32_t _accepted, void* _arg){
    LOG_DBG(TAG, "Event BP %s from IOT system: %s", bp_sn, _accepted == SM_BP_ACCEPTED ? "ACCEPTED" : "REJECTED");
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_auth_bp_update_from_cloud(app->m_auth_bp, bp_sn, _accepted);
}

/******************************************* BLE Handle ******************************************************/
void sm_ble_event_handle_on_connected(int32_t _success, void* _arg){
    LOG_DBG(TAG, "BLE on connected : %d", _success);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    /// TODO: Dangerous: Conflict thread BLE and EV.
    sm_sv_hmi_set_ble_icon(app->m_hmi_service, SM_DRV_LCD_SHOW);

    const sm_ev_data_t* ev_data = sm_sv_ev_get_data(app->m_ev_service);
    sm_sv_hmi_clear_ble_key(app->m_hmi_service);
    sm_sv_hmi_set_odo(app->m_hmi_service, ev_data->m_odo, SM_DRV_LCD_SHOW);
}
void sm_ble_event_handle_on_disconnected(int32_t _success, void* _arg){
    LOG_DBG(TAG, "BLE on disconnected : %d", _success);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    /// TODO: Dangerous: Conflict thread BLE and EV.

    sm_sch_cancel_taskz(app->m_sch_task, (sm_sch_task_fn_t)sm_hmi_io_blink_signal);

    sm_sv_hmi_set_ble_icon(app->m_hmi_service, SM_DRV_LCD_HIDE);
    sm_hmi_io_set_signal(&app->m_modules.m_hmi_io, 0);
}

static void sm_hmi_app_inactive_mode_event_handle(int32_t _event, void* _arg){
    LOG_DBG(TAG, "Event from INACTIVE mode: %d", _event);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;

    if(_event == SM_EV_INACTIVE_WARNING){
        sm_sv_hmi_set_warning_icon(app->m_hmi_service, EV_ERR_LOST_INTERNET, SM_DRV_LCD_SHOW);
        sm_sv_hmi_blink_warning_icon(app->m_hmi_service, SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else if(_event == SM_EV_INACTIVE_EXECUTED){
        uint8_t inactive = EV_INACTIVE_STATE;
        sm_sv_ev_set_cmd(app->m_ev_service, SM_EV_CMD_INACTIVE_EV, &inactive, NULL, NULL);
        sm_sv_ev_set_motor_active_condition(app->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_OFFLINE);
        sm_sv_hmi_set_warning_icon(app->m_hmi_service, EV_ERR_LOST_INTERNET, SM_DRV_LCD_SHOW);
    } else{
        uint8_t active = EV_ACTIVE_STATE;
        sm_sv_ev_set_cmd(app->m_ev_service, SM_EV_CMD_INACTIVE_EV, &active, NULL, NULL);
        sm_sv_ev_reset_motor_active_condition(app->m_ev_service, SM_MOTOR_ACTIVE_CONDITION_OFFLINE);
    }
}

static void sm_hmi_app_net_event_handle(int32_t _event, void* _arg){
    LOG_DBG(TAG, "Event from NET Monitor event: %d", _event);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;

    if(_event == NET_RESET_CONNECTION){
        app->m_init_flag.m_net_ready = false;
    }

    if(app->m_config.m_ev_config.m_inactive_mode_config.m_enable){
        sm_inactive_mode_update_net_state(app->m_inactive_mode, _event);
    }
}

/********************************************SDT TOOLS ********************************************************/

static uint8_t sm_hmi_app_store_manu_setting() {
    LOG_INF(TAG, "Setting up from Selex SDT tool");
    sm_hmi_app_t *app = &g_hmi_app_default;
    if (app->m_storage.m_ev_manu_storage->m_proc->store(app->m_storage.m_ev_manu_storage,
                                                        app->m_ev_manu_cfg_buff + 9) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return CO_EXT_CONFIRM_abort;
    }

    if (sm_ev_manu_storage_validate(app->m_storage.m_ev_manu_storage, app->m_ev_manu_cfg_buff, 9) < 0) {
        LOG_ERR(TAG, "Could NOT store new manufacture crc");
        return CO_EXT_CONFIRM_abort;
    }

    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_hmi_app_load_ev_config_setting() {
    LOG_INF(TAG, "Read ev config up from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_hmi_app_store_ev_config_setting() {
    LOG_INF(TAG, "Write ev config up from Selex SDT tool");
    sm_hmi_app_t *app = &g_hmi_app_default;

    memcpy(&app->m_config.m_ev_config, app->m_ev_manu_cfg_buff, sizeof(app->m_config.m_ev_config));

    if (app->m_storage.m_ev_config_storage->m_proc->store(app->m_storage.m_ev_config_storage,
                                                          &app->m_config.m_ev_config) < 0) {
        LOG_ERR(TAG, "Could NOT store ev config");
        return CO_EXT_CONFIRM_abort;
    }

    sm_sv_ev_reboot_module(app->m_ev_service, SM_EV_MODULE_HMI, 200);

    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_hmi_app_load_ev_net_config_setting(){
    LOG_INF(TAG, "Read ev net config up from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_hmi_app_store_ev_net_config_setting(){
LOG_INF(TAG, "Write ev config up from Selex SDT tool");
    sm_hmi_app_t* app = &g_hmi_app_default;

    memcpy(&app->m_config.m_net_config, app->m_ev_manu_cfg_buff, sizeof(app->m_config.m_net_config));

    if(app->m_storage.m_net_config_storage->m_proc->store(app->m_storage.m_net_config_storage, &app->m_config.m_net_config) < 0){
        LOG_ERR(TAG, "Could NOT store ev net config");
        return CO_EXT_CONFIRM_abort;
    }

    sm_sv_ev_reboot_module(app->m_ev_service, SM_EV_MODULE_HMI, 200);

    return CO_EXT_CONFIRM_success;
}


static uint8_t sm_hmi_app_load_manu_setting(){
    LOG_INF(TAG, "Load EV setting from Selex SDT tool");
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_hmi_app_reset_setting_handle(){
    LOG_INF(TAG, "Request reset setting from Selex SDT tool");
    sm_hmi_app_t* app = &g_hmi_app_default;

    if(app->m_reset_setting_request == 1){

        sm_module_reset_data(&app->m_ev_manu);

        memcpy(app->m_ev_manu.m_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));
        memcpy(app->m_ev_manu.m_ev_sn, EV_SN_DEFAULT, strlen(EV_SN_DEFAULT));

        app->m_storage.m_ev_manu_storage->m_proc->store(app->m_storage.m_ev_manu_storage,
                                                        app->m_ev_manu.m_device_name);

        sm_ev_manu_storage_validate(app->m_storage.m_ev_manu_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, 9);

        app->m_storage.m_ev_config_storage->m_proc->clear(app->m_storage.m_ev_config_storage);
        app->m_storage.m_net_config_storage->m_proc->clear(app->m_storage.m_net_config_storage);
        sm_ev_opt_clear_storage(app->m_storage.m_ev_opt_storage);
        sm_sv_ev_reboot_module(app->m_ev_service, SM_EV_MODULE_HMI, 1000);
        return CO_EXT_CONFIRM_success;
    }
    return CO_EXT_CONFIRM_abort;
}


static uint8_t sm_hmi_app_validate_manu_setting(){
    LOG_INF(TAG, "Validate EV setting from Selex SDT tool");
    sm_hmi_app_t* app = &g_hmi_app_default;

    if(sm_ev_manu_storage_validate(app->m_storage.m_ev_manu_storage, (const uint8_t*)SM_EV_CONFIG_ODO_PASS_DEFAULT, 9) < 0){
        LOG_ERR(TAG, "Could NOT store new manufacture info");
        return CO_EXT_CONFIRM_abort;
    }

    return CO_EXT_CONFIRM_success;
}

