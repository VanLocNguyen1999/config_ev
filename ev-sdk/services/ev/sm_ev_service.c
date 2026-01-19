//
// Created by vnbk on 15/08/2024.
//
#include <math.h>
#include "sm_ev_service.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"
#include "sm_queue.h"
#include "sm_math.h"
#include "sm_ev_pmu_module.h"
#include "sm_ev_mc_module.h"

#include "sm_ev_opt_storage.h"
#include "sm_ev_internal_service.h"

#include "sm_co_od_common.h"

#include "sm_bsp_hmi.h"

#define TAG "SM_EV_SERVICE"

#define _impl(p)    ((sm_sv_ev_impl_t*)(p))

#define SM_EV_DATA_QUEUE_SIZE   32
#define SM_MODULE_DATA_SIZE     8

#define SM_EV_CMD_TIMEOUT   5000

#define KM_TO_M             1000

#define MC_FACTOR

#define SM_EV_LOST_CONNECTION_TIMEOUT_DEFAULT   (30*1000)

typedef struct sm_module_data{
    int32_t m_msg_id;
    uint8_t m_data[SM_MODULE_DATA_SIZE];
    uint8_t m_data_len;
}sm_ev_module_data_coming_t;

typedef struct {
    sm_co_t* m_co;
    sm_sv_bp_t* m_bp_manager;
    sm_sv_bp_event_cb_t m_bp_event_handle;

    sm_ev_config_t* m_config;
    sm_sec_storage_t* m_backup_odo_storage;
    sm_odo_storage_t* m_odo_storage;
    sm_opt_storage_t* m_opt_storage;

    sm_ev_data_t m_ev_data;
    uint32_t m_odo_latch;

    sm_ev_module_t* m_pmu_module;
    sm_pmu_event_t m_pmu_internal_handle;

    sm_ev_module_t* m_mc_module;

    sm_ev_manu_t* m_ev_manu;
    char m_hmi_version[EV_VERSION_STRING_LENGTH];

    sm_sv_ev_event_t* m_event_handle;
    void* m_event_arg;

    struct{
        uint8_t m_cmd;
        uint8_t m_in_process;
        union {
            int32_t m_common_data;
            struct {
                int32_t m_id;
                uint8_t m_data[128];
            }m_cmd_extended_data;
        }m_cmd_data;
        sm_sv_ev_on_cmd_fn_t m_cb;
        void* m_cb_arg;
        elapsed_timer_t m_timeout;
    }m_cmd;

    sm_queue_t* m_data_queue;

    sm_mutex m_lock;

    uint8_t m_hmi_reboot_request;
    elapsed_timer_t m_reboot_timeout;
}sm_sv_ev_impl_t;

static void sm_ev_process_data(sm_sv_ev_impl_t* _this, sm_ev_module_data_coming_t* _data);
static void sm_ev_module_init(sm_sv_ev_impl_t* _this);
static sm_ev_module_t* sm_ev_get_module_by_type(sm_sv_ev_impl_t* _this, uint8_t _type);

static void sm_ev_recv_data(const uint32_t _can_id, uint8_t* _data, void* _arg);
static void sm_ev_recv_sdo_confirmed(SM_SDO_STATUS_t _status, int32_t _tx_err, int32_t _rx_err, void* _arg);

static void sm_ev_on_bp_connected(int32_t _slot, const char* _sn, int32_t _soc, void* _arg);
static void sm_ev_on_bp_disconnected(int32_t _slot, const char* _sn, void* _arg);
static void sm_ev_on_bp_update_data(int32_t _slot, const sm_bp_data_t* _bp_data, void*_arg);
static void sm_ev_on_bp_cmd(int32_t _slot, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg);

static int32_t sm_sv_ev_ctl_left_signal(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_ctl_right_signal(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_ctl_high_beam(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_ctl_low_beam(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_ctl_horn(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_find_ev(sm_sv_ev_t* _this);
static int32_t sm_sv_ev_lock_ev(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_block_ev(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_anti_theft_ev(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_set_lock_port(sm_sv_ev_t* _this, uint8_t _port, uint8_t _value);
static int32_t sm_sv_ev_set_enable_port(sm_sv_ev_t* _this, uint8_t _port, uint8_t _value);
static int32_t sm_sv_ev_config_verify_bp_offline(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_set_drive_mode(sm_sv_ev_t* _this, uint8_t _value);
static int32_t sm_sv_ev_set_max_speed(sm_sv_ev_t* _this, uint8_t _value);

static void sm_pmu_on_drive_mode_signal(uint8_t _value, void* _arg);
static void sm_pmu_on_backward_mode_signal(uint8_t _value, void* _arg);
static void sm_pmu_on_port_stated_changed(uint8_t, uint8_t, void* _arg);
static void sm_pmu_on_connection_changed(uint8_t, uint8_t, void*, void*);
static void sm_pmu_on_err(uint8_t _err, void* _arg);

static void sm_mc_on_connection_changed(uint8_t, uint8_t, void*, void*);

static sm_sv_ev_impl_t g_ev_service = {
        .m_co = NULL,
        .m_bp_manager = NULL,
        .m_event_handle = NULL,
        .m_event_arg = NULL,
        .m_ev_data = {
                .m_active = EV_ACTIVE_STATE,
                .m_motor_active_state = SM_MOTOR_ACTIVE_CONDITION_EXIT_PACKING,
                .m_trip = EV_DATA_TRIP_DEFAULT,
                .m_speed = 0,
                .m_driver_mode = EV_DATA_DRIVE_MODE_DEFAULT,

        },
        
        .m_cmd = {
                .m_cmd = SM_EV_CMD_NUMBER,
                .m_in_process = 0,
                .m_cb = NULL,
                .m_cb_arg = NULL,
                .m_timeout = {
                        .m_duration = SM_EV_CMD_TIMEOUT,
                }
        },
        .m_bp_event_handle = {
                .on_bp_connected = sm_ev_on_bp_connected,
                .on_bp_disconnected = sm_ev_on_bp_disconnected,
                .on_bp_update_data = sm_ev_on_bp_update_data
        },
        .m_pmu_internal_handle = {
                .on_drive_mode_signal = sm_pmu_on_drive_mode_signal,
                .on_backward_mode_signal = sm_pmu_on_backward_mode_signal,
                .on_port_stated_changed = sm_pmu_on_port_stated_changed,
                .on_err = sm_pmu_on_err
        },
        .m_config = NULL,
        .m_opt_storage = NULL,
        .m_ev_manu = NULL,
        .m_mc_module = NULL,
        .m_pmu_module = NULL,
};

static uint8_t sm_ev_require_config_odo(){
    LOG_INF(TAG, "Require configure ODO");
    sm_sv_ev_set_odo(&g_ev_service, g_ev_service.m_ev_data.m_odo, (char*)g_ev_service.m_config->m_odo_pass);
    return CO_EXT_CONFIRM_success;
}

static uint8_t sm_ev_require_reboot(){
    LOG_INF(TAG, "Require hmi reboot");

    sm_sv_ev_reboot_module(&g_ev_service, SM_EV_MODULE_HMI, 200);

    return CO_EXT_CONFIRM_success;
}

//void tpdo1_build_data_impl(uint8_t *_buffer){
//    _buffer[0] = (uint8_t)g_ev_service.m_ev_data.m_active;
//
//    if(g_ev_service.m_ev_data.m_pmu_data->m_key != EV_KEY_ON || g_ev_service.m_ev_data.m_pmu_data->m_parking != EV_EXIT_PARKING){
//        _buffer[1] = EV_MOTOR_INACTIVE_STATE;
//    }else{
//        if(g_ev_service.m_ev_data.m_motor_active_state == 0){
//            _buffer[1] = EV_MOTOR_ACTIVE_STATE;
//        }else{
//            _buffer[1] = EV_MOTOR_INACTIVE_STATE;
//        }
//    }
//}

static void sm_ev_load_data(sm_sv_ev_impl_t* _this){
    if(_this->m_backup_odo_storage->m_proc->init){
        _this->m_backup_odo_storage->m_proc->init(_this->m_opt_storage);
    }

    if(sm_ev_odo_storage_load(_this->m_odo_storage, &_this->m_ev_data.m_odo) < 0){
        LOG_WRN(TAG, "New odo is not init yet, load old odo");
        if(sm_ev_security_storage_load(_this->m_backup_odo_storage, SM_EV_CONFIG_ODO_PASS_DEFAULT, &_this->m_ev_data.m_odo) < 0){
            LOG_ERR(TAG, "Could NOT load old ODO. Please check again");
        }else{
            LOG_INF(TAG, "Load old ODO SUCCEED, ODO value is %d", _this->m_ev_data.m_odo);
        }
        if(sm_ev_odo_storage_store(_this->m_odo_storage, _this->m_ev_data.m_odo) < 0){
            LOG_ERR(TAG, "Could NOT store new ODO. Please check again");
        }
    }

    _this->m_odo_latch = _this->m_ev_data.m_odo;

    _this->m_ev_data.m_max_speed = (uint8_t)sm_ev_opt_load_max_speed(_this->m_opt_storage);
    if(_this->m_ev_data.m_max_speed < EV_MIN_SPEED || _this->m_ev_data.m_max_speed > EV_MAX_SPEED){
        LOG_ERR(TAG, "max speed %d is invalid, change it to default value %d", _this->m_ev_data.m_max_speed, EV_MAX_SPEED_DEFAULT);
        _this->m_ev_data.m_max_speed = EV_MAX_SPEED_DEFAULT;
        sm_ev_opt_store_max_speed(_this->m_opt_storage, EV_MAX_SPEED_DEFAULT);
    }
  
    _this->m_ev_data.m_driver_mode = EV_SPORT_MODE_1;
/*    uint8_t driver_mode = sm_ev_opt_load_drive_mode(_this->m_opt_storage);

    if(driver_mode >= EV_MC_UP_HILL_MODE){
        LOG_ERR(TAG, "driver mode %d is invalid, change it to default value %d", driver_mode, EV_MC_SPORT_MODE_1);
        _this->m_ev_data.m_driver_mode = EV_MC_SPORT_MODE_1;
        sm_ev_opt_store_drive_mode(_this->m_opt_storage, _this->m_ev_data.m_driver_mode);
    }*/
}

static void sm_ev_module_init(sm_sv_ev_impl_t* _this){
    _this->m_pmu_module->m_sync_info = false;
    _this->m_pmu_module->m_proc->init(_this->m_pmu_module);

    _this->m_mc_module->m_sync_info = false;
    _this->m_mc_module->m_proc->init(_this->m_mc_module);

    _this->m_ev_data.m_pmu_data = _this->m_pmu_module->m_proc->get_data(_this->m_pmu_module);
    _this->m_ev_data.m_mc_data = _this->m_mc_module->m_proc->get_data(_this->m_mc_module);
}

static sm_ev_module_t* sm_ev_get_module_by_type(sm_sv_ev_impl_t* _this, uint8_t _type){
    if(_type == SM_EV_MODULE_MC){
        return _this->m_mc_module;
    }else if(_type == SM_EV_MODULE_PMU){
        return _this->m_pmu_module;
    }else{
        return NULL;
    }
}

sm_sv_ev_t* sm_sv_ev_create(sm_ev_manu_t* _ev_manu,
                            sm_co_t* _co,
                            sm_sv_bp_t* _bp_manager,
                            sm_ev_config_t * _ev_config,
                            sm_sec_storage_t* _backup_odo_storage,
                            sm_odo_storage_t* _odo_storage,
                            sm_storage_t* _opt_storage){
    if(!_ev_manu || !_co || !_bp_manager || !_opt_storage || !_ev_config || !_backup_odo_storage || !_odo_storage){
        LOG_ERR(TAG, "Created EV Service FAILURE, INVALID parameters");
        return NULL;
    }
    g_ev_service.m_ev_manu = _ev_manu;
    memset(g_ev_service.m_hmi_version, '\0', EV_VERSION_STRING_LENGTH);
    sm_ev_version_to_string(_ev_manu->m_sw_ver, g_ev_service.m_hmi_version);

    g_ev_service.m_co = _co;
    sm_co_if_reg_recv_callback(sm_co_get_if(_co), sm_ev_recv_data, &g_ev_service);

    sm_co_sdo_server_set_handle(_co,
                                SDO_EV_CONFIG_MANU_INDEX,
                                SDO_EV_CONFIG_MANU_READ_EV_SN_SUB_INDEX,
                                NULL,
                               _ev_manu->m_ev_sn);

    sm_co_sdo_server_set_handle(_co,
                                SDO_EV_CONFIG_ODO_INDEX,
                                SDO_EV_CONFIG_ODO_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_ev_require_config_odo,
                                &g_ev_service.m_ev_data.m_odo);

    static uint8_t temp_reboot_request;
    sm_co_sdo_server_set_handle(_co,
                                SDO_EV_REBOOT_INDEX,
                                SDO_EV_REBOOT_SUB_INDEX,
                                (sm_co_sdo_server_confirm_fn_t)sm_ev_require_reboot,
                                &temp_reboot_request);

    g_ev_service.m_bp_manager = _bp_manager;
    sm_sv_bp_reg_event(g_ev_service.m_bp_manager, &g_ev_service.m_bp_event_handle, &g_ev_service);

    g_ev_service.m_config = _ev_config;
    g_ev_service.m_opt_storage = _opt_storage;
    g_ev_service.m_backup_odo_storage = _backup_odo_storage;
    g_ev_service.m_odo_storage = _odo_storage;
    
    g_ev_service.m_data_queue = sm_queue_create(SM_EV_DATA_QUEUE_SIZE, sizeof(sm_ev_module_data_coming_t));

    /*** Created PMU module ****/
    g_ev_service.m_pmu_module = sm_pmu_create(&g_ev_service, _co, &g_ev_service.m_pmu_internal_handle, &g_ev_service);
    if(!g_ev_service.m_pmu_module){
        LOG_ERR(TAG, "Could NOT create PMU");
        return NULL;
    }
    sm_module_reg_event_callback(g_ev_service.m_pmu_module, sm_pmu_on_connection_changed, &g_ev_service);

    /*** Created MC module ****/
    g_ev_service.m_mc_module = sm_mc_create(&g_ev_service, _co);
    if(!g_ev_service.m_mc_module){
        LOG_ERR(TAG, "Could NOT create MC");
        return NULL;
    }
    sm_module_reg_event_callback(g_ev_service.m_mc_module, sm_mc_on_connection_changed, &g_ev_service);

    /// init module
    sm_ev_module_init(&g_ev_service);

    /// load ev config
    sm_ev_load_data(&g_ev_service);
  
    g_ev_service.m_hmi_reboot_request = false;

    MUTEX_INIT(g_ev_service.m_lock);

    return &g_ev_service;
}

int32_t sm_sv_ev_destroy(sm_sv_ev_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_co = NULL;
    _impl(_this)->m_bp_manager = NULL;
    _impl(_this)->m_event_arg = NULL;
    _impl(_this)->m_event_handle = NULL;

    return 0;
}

int32_t sm_sv_ev_get_module_type_by_name(sm_sv_ev_t* _this, const char* _name){
    if(!_this){
        return -1;
    }
    if(!strcmp(_name, _impl(_this)->m_mc_module->m_proc->get_name(_impl(_this)->m_mc_module))){
        return SM_EV_MODULE_MC;
    }else if(!strcmp(_name, _impl(_this)->m_pmu_module->m_proc->get_name(_impl(_this)->m_pmu_module))){
        return SM_EV_MODULE_PMU;
    }else{
        return SM_EV_MODULE_HMI;
    }
}

const sm_module_info_t* sm_sv_ev_get_module_info(sm_sv_ev_t* _this, int32_t _module){
    if(!_this){
        return NULL;
    }
    if(SM_EV_MODULE_PMU == _module  && _impl(_this)->m_mc_module->m_version[0] != '\0'){
        return &_impl(_this)->m_pmu_module->m_info;
    }

    if(SM_EV_MODULE_MC == _module  && _impl(_this)->m_mc_module->m_version[0] != '\0'){
        return &_impl(_this)->m_mc_module->m_info;
    }

    if(SM_EV_MODULE_HMI == _module  && _impl(_this)->m_hmi_version[0] != '\0'){
        return _impl(_this)->m_ev_manu;
    }

    return NULL;
}

int32_t sm_sv_ev_reset_module_info(sm_sv_ev_t* _this, int32_t _module){
    if(!_this){
        return -1;
    }
    if(SM_EV_MODULE_PMU == _module ){
        _impl(_this)->m_pmu_module->m_proc->reset_data(_impl(_this)->m_pmu_module);
        return 0;
    }

    if(SM_EV_MODULE_MC == _module){
        _impl(_this)->m_mc_module->m_proc->reset_data(_impl(_this)->m_mc_module);
        return 0;
    }

    return -1;
}

const char* sm_sv_ev_get_module_version(sm_sv_ev_t* _this, int32_t _module){
    if(!_this){
        return NULL;
    }

    if(SM_EV_MODULE_PMU == _module && _impl(_this)->m_pmu_module->m_version[0] != '\0'){
        return _impl(_this)->m_pmu_module->m_version;
    }

    if(SM_EV_MODULE_MC == _module && _impl(_this)->m_mc_module->m_version[0] != '\0'){
        return _impl(_this)->m_mc_module->m_version;
    }

    if(SM_EV_MODULE_HMI == _module && _impl(_this)->m_hmi_version[0] != '\0'){
        return _impl(_this)->m_hmi_version;
    }

    return NULL;
}

int32_t sm_ev_check_module_pre_download_condition_by_name(sm_sv_ev_t* _this, const char* _name){
    if(!_this){
        return -1;
    }
    if(!strcmp(_name, _impl(_this)->m_pmu_module->m_proc->get_name(_impl(_this)->m_pmu_module))) {
        sm_sv_bp_t* bp_manager = _impl(_this)->m_bp_manager;
        for(int id = 0; id < SM_BP_NUMBER_DEFAULT; id++){
            const sm_bp_data_t* bp = sm_sv_bp_get_data(bp_manager, id);
            if(!bp){
                continue;
            }
            if(bp->m_cur >= EV_BP_LOWEST_CHARGING_CUR){
                LOG_INF(TAG, "Start ota pmu because ev is charging");
                return 0;
            }
            if(bp->m_state == BP_STATE_FORCE_DISCHARGE){
                LOG_INF(TAG, "Start ota pmu because have a bp force discharge");
                return 0;
            }
            if(sm_sv_bp_check_pre_force_discharging_condition(bp_manager, id) >= 0){
                LOG_INF(TAG, "Start ota pmu because have bp can force dis");
                return 0;
            }
        }
        LOG_WRN(TAG, "Not ota pmu, dont have any thing!");
        return -1;
    }
    return 0;
}

int32_t sm_ev_check_module_pre_upgrade_condition_by_type(sm_sv_ev_t* _this, int32_t _module){
    if(!_this){
        return -1;
    }
    if(_module == SM_EV_MODULE_PMU) {
        sm_sv_bp_t* bp_manager = _impl(_this)->m_bp_manager;
        for(int id = 0; id < SM_BP_NUMBER_DEFAULT; id++){
            const sm_bp_data_t* bp = sm_sv_bp_get_data(bp_manager, id);
            if(!bp){
                continue;
            }
            if(bp->m_cur >= EV_BP_LOWEST_CHARGING_CUR){
                LOG_INF(TAG, "Start ota pmu because ev is charging");
                return 0;
            }
            if(bp->m_state == BP_STATE_FORCE_DISCHARGE){
                LOG_INF(TAG, "Start ota pmu because have a bp force discharge");
                return 0;
            }
        }

        LOG_WRN(TAG, "Not ota pmu, dont have any thing!");
        return -1;
    }
    return 0;
}


const char* sm_sv_ev_get_module_version_by_name(sm_sv_ev_t* _this, const char* _name){
    if(!_this){
        return NULL;
    }
    if(!strcmp(_name, _impl(_this)->m_mc_module->m_proc->get_name(_impl(_this)->m_mc_module))){
        return _impl(_this)->m_mc_module->m_version;
    }else if(!strcmp(_name, _impl(_this)->m_pmu_module->m_proc->get_name(_impl(_this)->m_pmu_module))){
        return _impl(_this)->m_pmu_module->m_version;
    }else if(!strcmp(_name, sm_ev_convert_module_to_string(SM_EV_MODULE_HMI))){
        return _impl(_this)->m_hmi_version;
    }else{
        return NULL;
    }
}

sm_sv_bp_t* sm_sv_ev_get_bp_sv(sm_sv_ev_t* _this){
    if(!_this) {
        return NULL;
    }
    return _impl(_this)->m_bp_manager;
}

const sm_ev_data_t* sm_sv_ev_get_data(sm_sv_ev_t* _this){
    if(!_this) {
        return NULL;
    }
    return &_impl(_this)->m_ev_data;
}


int32_t sm_sv_ev_reboot_module(sm_sv_ev_t* _this, int32_t _module, uint32_t _delay_time){
    if(!_this) {
        return -1;
    }
    if(_module == SM_EV_MODULE_MC){
        _impl(_this)->m_mc_module->m_proc->reboot(_impl(_this)->m_mc_module);
    }else if(_module == SM_EV_MODULE_PMU){
        _impl(_this)->m_mc_module->m_proc->reboot(_impl(_this)->m_pmu_module);
    }else if(_module == SM_EV_MODULE_HMI){
        elapsed_timer_resetz(&_impl(_this)->m_reboot_timeout, (int32_t)_delay_time);
        _impl(_this)->m_hmi_reboot_request = true;
    }else{
        LOG_ERR(TAG, "NOT support module");
        return -1;
    }
    return 0;
}

int32_t sm_sv_ev_reboot_bp(sm_sv_ev_t* _this, int32_t _slot){
    if(!_this) {
        return -1;
    }
    if(sm_sv_bp_is_connected(_impl(_this)->m_bp_manager, _slot)){
        return sm_sv_bp_set_cmd(_impl(_this)->m_bp_manager,
                                _slot,
                                BP_CMD_REBOOT,
                                NULL,
                                sm_ev_on_bp_cmd,
                                NULL);
    }
    return -1;
}

int32_t sm_sv_ev_set_motor_active_condition(sm_sv_ev_t* _this, uint8_t _condition){
    if(!_this){
        return -1;
    }

    ENTER_CRITICAL(_impl(_this)->m_lock);
    _impl(_this)->m_ev_data.m_motor_active_state |= _condition;
    EXIT_CRITICAL(_impl(_this)->m_lock);

    return 0;
}

int32_t sm_sv_ev_reset_motor_active_condition(sm_sv_ev_t* _this, uint8_t _condition){
    if(!_this){
        return -1;
    }

    ENTER_CRITICAL(_impl(_this)->m_lock);
    _impl(_this)->m_ev_data.m_motor_active_state &= (~_condition);
    EXIT_CRITICAL(_impl(_this)->m_lock);

    return 0;
}

int32_t sm_sv_ev_set_odo(sm_sv_ev_t* _this, uint32_t _odo, const char* _pass){

	(void)_pass;
    if(!_this){
        return -1;
    }
    /// TODO: verify PASS. Currently bypass.
    ENTER_CRITICAL(_impl(_this)->m_lock);

    if(_odo == _impl(_this)->m_ev_data.m_odo){
        return 0;
    }

    if(sm_ev_odo_storage_store(_impl(_this)->m_odo_storage, _odo) < 0){
        LOG_ERR(TAG, "Could NOT set ODO");
        EXIT_CRITICAL(_impl(_this)->m_lock);
        return -1;
    }

    _impl(_this)->m_ev_data.m_odo = _odo;
    if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_ev_event) {
        _impl(_this)->m_event_handle->on_ev_event(SM_EV_EVENT_ODO_CHANGED,
                                               (int32_t)_impl(_this)->m_ev_data.m_odo,
                                               _impl(_this)->m_event_arg);
    }

    EXIT_CRITICAL(_impl(_this)->m_lock);

    return 0;
}

int32_t sm_sv_ev_reg_event(sm_sv_ev_t* _this, sm_sv_ev_event_t* _event_handle, void* _arg){
    if(!_this || !_event_handle){
        return -1;
    }
    _impl(_this)->m_event_handle = _event_handle;
    _impl(_this)->m_event_arg = _arg;
    return 0;
}

static void sm_sv_ev_reset_cmd(sm_sv_ev_impl_t* _this){
    _this->m_cmd.m_cmd = SM_EV_CMD_NUMBER;
    _this->m_cmd.m_in_process = 0;
    _this->m_cmd.m_cb = NULL;
    memset(&_this->m_cmd.m_cmd_data, '\0', sizeof(_this->m_cmd.m_cmd_data));
    _this->m_cmd.m_cb_arg = NULL;
    elapsed_timer_resetz(&_impl(_this)->m_cmd.m_timeout, SM_EV_CMD_TIMEOUT);
}

static int32_t sm_sv_ev_cmd_handle(sm_sv_ev_impl_t* _this){
    if(_this->m_cmd.m_cmd >= SM_EV_CMD_NUMBER){
        return -2;
    }

    if(_this->m_cmd.m_in_process){
        if(!elapsed_timer_get_remain(&_this->m_cmd.m_timeout)){
            if(_this->m_cmd.m_cb) {
                _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                  _this->m_cmd.m_cmd,
                                  &_this->m_cmd.m_cmd_data,
                                  _this->m_cmd.m_cb_arg);
            }
            sm_sv_ev_reset_cmd(_this);
        }
        return -1;
    }

    _this->m_cmd.m_in_process = 1;

    switch (_this->m_cmd.m_cmd) {
        case SM_EV_CMD_CONTROL_LEFT_SIGNAL:
            sm_sv_ev_ctl_left_signal(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_CONTROL_RIGHT_SIGNAL:
            sm_sv_ev_ctl_right_signal(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_CONTROL_HIGH_BEAM:
            sm_sv_ev_ctl_high_beam(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_CONTROL_LOW_BEAM:
            sm_sv_ev_ctl_low_beam(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_CONTROL_HORN:
            sm_sv_ev_ctl_horn(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_SET_DRIVE_MODE:
            sm_sv_ev_set_drive_mode(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_SET_MAX_SPEED: {
            int32_t max_speed = _this->m_cmd.m_cmd_data.m_common_data;
            int32_t ret = 0;
            if(_this->m_ev_data.m_max_speed != max_speed){
                ret = sm_ev_opt_store_max_speed(_this->m_opt_storage, max_speed);
                if(!ret){
                    _this->m_ev_data.m_max_speed = (uint8_t)max_speed;
                    sm_sv_ev_set_max_speed(_this, (uint8_t)_this->m_ev_data.m_max_speed);
                }
            }

            if(_this->m_cmd.m_cb){
                _this->m_cmd.m_cb(ret,
                                  SM_EV_CMD_INACTIVE_EV,
                                  &max_speed,
                                  _this->m_cmd.m_cb_arg);
            }

            sm_sv_ev_reset_cmd(_this);
            break;
        }
        case SM_EV_CMD_FIND_EV:
            sm_sv_ev_find_ev(_this);
            break;
        case SM_EV_CMD_LOCK_EV:
            sm_sv_ev_lock_ev (_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            if (_this->m_cmd.m_cmd_data.m_common_data == 1){
                sm_sv_ev_set_motor_active_condition (_this, SM_MOTOR_ACTIVE_CONDITION_EV_LOCKING);
            }else{
                sm_sv_ev_reset_motor_active_condition (_this, SM_MOTOR_ACTIVE_CONDITION_EV_LOCKING);
            }
            break;
        case SM_EV_CMD_BLOCK_EV:
            sm_sv_ev_block_ev(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);

            if(_this->m_cmd.m_cmd_data.m_common_data == 1){
                sm_sv_ev_set_motor_active_condition(_this, SM_MOTOR_ACTIVE_CONDITION_EV_LOCKING);
            }else{
                sm_sv_ev_reset_motor_active_condition(_this, SM_MOTOR_ACTIVE_CONDITION_EV_LOCKING);
            }
            break;
        case SM_EV_CMD_ANTI_THEFT_EV:
            sm_sv_ev_anti_theft_ev(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_INACTIVE_EV: {
            uint8_t value = (uint8_t)_this->m_cmd.m_cmd_data.m_common_data;

            if(value != _this->m_ev_data.m_active){
                _this->m_ev_data.m_active = value;
            }

            if(value){
                if(_this->m_ev_data.m_err != EV_ERR_BLOCK_STATE) {
                    _this->m_ev_data.m_err = EV_ERR_NONE;
                    if (_this->m_event_handle && _this->m_event_handle->on_ev_event) {
                        _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                           _this->m_ev_data.m_err,
                                                           _this->m_event_arg);
                    }
                }
            }else{
                if(_this->m_ev_data.m_err != EV_ERR_BLOCK_STATE &&
                   _this->m_ev_data.m_err != EV_ERR_LOST_CAN_NETWORK &&
                   _this->m_ev_data.m_err != EV_ERR_LOST_INTERNET) {
                    _this->m_ev_data.m_err = EV_ERR_LOST_INTERNET;
                    if (_this->m_event_handle && _this->m_event_handle->on_ev_event) {
                        _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                           _this->m_ev_data.m_err,
                                                           _this->m_event_arg);
                    }
                }
            }

            if(_this->m_cmd.m_cb){
                _this->m_cmd.m_cb(SM_EV_CMD_SUCCESS,
                                  SM_EV_CMD_INACTIVE_EV,
                                  &value,
                                  _this->m_cmd.m_cb_arg);
            }

            sm_sv_ev_reset_cmd(_impl(_this));
            break;
        }
        case SM_EV_CMD_SET_LOCK_PORT:
        case SM_EV_CMD_SET_UNLOCK_PORT:{
            uint8_t port = (uint8_t)_this->m_cmd.m_cmd_data.m_common_data;
            if(port >= SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      _this->m_cmd.m_cmd,
                                      &port,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                break;
            }
            uint8_t value = (_this->m_cmd.m_cmd == SM_EV_CMD_SET_LOCK_PORT) ? EV_CMD_LOCK_PORT : EV_CMD_UNLOCK_PORT;
            sm_sv_ev_set_lock_port(_this, port, value);
            break;
        }
        case SM_EV_CMD_CONFIG_VERIFY_BP_OFFLINE:
            sm_sv_ev_config_verify_bp_offline(_this, (uint8_t)_this->m_cmd.m_cmd_data.m_common_data);
            break;
        case SM_EV_CMD_REBOOT_MODULE: {
            uint8_t module_type = (uint8_t)_this->m_cmd.m_cmd_data.m_common_data;
            sm_ev_module_t* module = sm_ev_get_module_by_type(_this, module_type);
            if(module && module->m_proc->reboot){
                module->m_proc->reboot(module);
            }else{
                if(module_type == SM_EV_MODULE_HMI){
                    sm_ev_require_reboot();
                }else{
                    sm_sv_ev_reboot_bp(_this, module_type - 5);
                }
            }
            sm_sv_ev_reset_cmd(_impl(_this));
            break;
        }
        case SM_EV_CMD_SET_ODO: {
            int32_t ret = sm_sv_ev_set_odo(_this,
            								(uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id,
                                           (const char*)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data);
            if(_this->m_cmd.m_cb){
                _this->m_cmd.m_cb(ret,
                                  SM_EV_CMD_SET_ODO,
                                  &_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id,
                                  _this->m_cmd.m_cb_arg);
            }

            sm_sv_ev_reset_cmd(_impl(_this));
            break;
        }
        case SM_EV_CMD_PORT_ENABLE:
		uint8_t port_enable =
				(uint8_t) _this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
		uint8_t value_enable =
				(uint8_t) _this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data[0];
		if (port_enable >= SM_BP_NUMBER_DEFAULT) {
			if (_this->m_cmd.m_cb) {
				_this->m_cmd.m_cb(SM_EV_CMD_FAILURE, _this->m_cmd.m_cmd, &port_enable,
						_this->m_cmd.m_cb_arg);
			}
			sm_sv_ev_reset_cmd(_impl(_this));
			break;
		}

		sm_sv_ev_set_enable_port(_this, port_enable, value_enable);

		break;
        case SM_EV_CMD_PORT_LOCK:
            uint8_t port_lock = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
            uint8_t value_lock = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data[0];

            if(port_lock >= SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      _this->m_cmd.m_cmd,
                                      &port_lock,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                break;
            }
            sm_sv_ev_set_lock_port(_this, port_lock, value_lock + 1);
            break;
        case SM_EV_CMD_WRITE_DEV_TO_BP:
            uint8_t slot_write = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
            const char* assigned_dev = (const char*)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data;

            if(!assigned_dev || slot_write >= SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      SM_EV_CMD_WRITE_DEV_TO_BP,
                                      &slot_write,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                return -1;
            }

            sm_sv_bp_set_cmd(_impl(_this)->m_bp_manager,
                             slot_write,
                             BP_CMD_WRITE_ASSIGNED_DEV,
                             (void*)assigned_dev,
                             sm_ev_on_bp_cmd,
                             _this);
            break;
        case SM_EV_CMD_SET_STATE_BP:{
            uint8_t slot_set_state = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
            uint8_t state = _this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data[0];
            if(slot_set_state > SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      SM_EV_CMD_SET_STATE_BP,
                                      &slot_set_state,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                return -1;
            }

            if(state == BP_STATE_STANDBY){
                if(slot_set_state == SM_BP_NUMBER_DEFAULT){
                    sm_sv_bp_set_off_all(_impl(_this)->m_bp_manager);
                    sm_sv_ev_reset_cmd(_impl(_this));
                }else {
                    sm_sv_bp_set_cmd(_impl(_this)->m_bp_manager,
                                     slot_set_state,
                                     BP_CMD_STANDBY,
                                     NULL,
                                     sm_ev_on_bp_cmd,
                                     _this);
                }
            }
            break;
        }
        case SM_EV_CMD_SET_BLOCK_BP:
        case SM_EV_CMD_SET_ACTIVE_BP:
            uint8_t slot_active = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
            uint8_t* value = _this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data;
            uint8_t bp_cmd;
            if(_this->m_cmd.m_cmd == SM_EV_CMD_SET_BLOCK_BP){
                bp_cmd = BP_CMD_SET_BLOCK;
            }else{
                bp_cmd = BP_CMD_SET_ACTIVE;
            }
            if(slot_active >= SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      bp_cmd,
                                      &slot_active,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                return -1;
            }

            sm_sv_bp_set_cmd(_impl(_this)->m_bp_manager,
                             slot_active,
                             bp_cmd,
                             (void*)value,
                             sm_ev_on_bp_cmd,
                             _this);
            break;
        case SM_EV_CMD_SET_CYCLE_BP:
            uint8_t slot_set_cycle = (uint8_t)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_id;
            uint8_t* value_set_cycle = (uint8_t*)_this->m_cmd.m_cmd_data.m_cmd_extended_data.m_data;
            if(slot_set_cycle >= SM_BP_NUMBER_DEFAULT){
                if(_this->m_cmd.m_cb){
                    _this->m_cmd.m_cb(SM_EV_CMD_FAILURE,
                                      SM_EV_CMD_SET_CYCLE_BP,
                                      &slot_set_cycle,
                                      _this->m_cmd.m_cb_arg);
                }
                sm_sv_ev_reset_cmd(_impl(_this));
                return -1;
            }
            sm_sv_bp_set_cmd(_impl(_this)->m_bp_manager,
                             slot_set_cycle,
                             BP_CMD_SET_CYCLE,
                             (void*)value_set_cycle,
                             sm_ev_on_bp_cmd,
                             _this);
            break;
        default:
            LOG_ERR(TAG, "Command NOT Support");
            sm_sv_ev_reset_cmd(_impl(_this));
            break;
    }

    return 0;
}

int32_t sm_sv_ev_set_cmd(sm_sv_ev_t* _this, uint8_t _cmd, void* _value, sm_sv_ev_on_cmd_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }

    ENTER_CRITICAL(_impl(_this)->m_lock);
    if(_impl(_this)->m_cmd.m_cmd < SM_EV_CMD_NUMBER){
        LOG_ERR(TAG, "Ev Service is Busy");
        EXIT_CRITICAL(_impl(_this)->m_lock);
        return -2;
    }

    _impl(_this)->m_cmd.m_cmd = _cmd;
    _impl(_this)->m_cmd.m_cb = _cb;
    _impl(_this)->m_cmd.m_cb_arg = _arg;
    elapsed_timer_resetz(&_impl(_this)->m_cmd.m_timeout, SM_EV_CMD_TIMEOUT);

    int32_t* common_data = (int32_t*)_value;
    sm_cmd_extended_data_t* extend_data = NULL;
    switch (_cmd) {
        case SM_EV_CMD_SET_ODO:
        case SM_EV_CMD_WRITE_DEV_TO_BP:
            extend_data = (sm_cmd_extended_data_t*)_value;
            _impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_id = extend_data->m_id;
            memcpy(_impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_data,
                   extend_data->m_data,
                   strlen(extend_data->m_data));
            break;
        case SM_EV_CMD_PORT_LOCK:
        case SM_EV_CMD_PORT_ENABLE:
        case SM_EV_CMD_SET_STATE_BP:
        case SM_EV_CMD_SET_BLOCK_BP:
        case SM_EV_CMD_SET_ACTIVE_BP:
            extend_data = (sm_cmd_extended_data_t*)_value;
            _impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_id = extend_data->m_id;
            _impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_data[0] = *(uint8_t*)extend_data->m_data;
            break;
        case SM_EV_CMD_SET_CYCLE_BP:
            extend_data = (sm_cmd_extended_data_t*)_value;
            _impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_id = extend_data->m_id;
            memcpy(_impl(_this)->m_cmd.m_cmd_data.m_cmd_extended_data.m_data,
                   extend_data->m_data,
                   2);
            break;
        default:
            _impl(_this)->m_cmd.m_cmd_data.m_common_data = *common_data;
    }
    EXIT_CRITICAL(_impl(_this)->m_lock);

    return 0;
}

const sm_bp_data_t* sm_sv_ev_get_bp_data(sm_sv_ev_t* _this, uint8_t _slot){
    if(!_this || !_impl(_this)->m_bp_manager){
        return NULL;
    }

    if(sm_sv_bp_is_connected(_impl(_this)->m_bp_manager, _slot)){
        return sm_sv_bp_get_data(_impl(_this)->m_bp_manager, _slot);
    }
    return NULL;
}

int32_t sm_ev_check_bp_general_ota_condition(sm_sv_ev_t* _this, uint8_t _slot, const char* _new_ver){
    if(!_this){
        return -1;
    }
    sm_sv_ev_impl_t* this = _this;
    sm_sv_bp_t* bp_manager = this->m_bp_manager;

    if(sm_sv_bp_is_connected(bp_manager, _slot) != true){
        return -1;
    }

    if(sm_ev_check_bp_version_different(_this, _slot, _new_ver) < 0){
        return -1;
    }

    for(int id = 0; id < SM_BP_NUMBER_DEFAULT; id++){
        const sm_bp_data_t* bp = sm_sv_bp_get_data(bp_manager, id);
        if(!bp){
            continue;
        }

        // check all bp
        if(bp->m_cur >= EV_BP_LOWEST_CHARGING_CUR){
            LOG_INF(TAG, "Start ota bp %d because ev is charging", _slot);
            return 0;
        }

        if(id == _slot){
            continue;
        }

        // check other bp
        if(bp->m_state == BP_STATE_FORCE_DISCHARGE){
            LOG_INF(TAG, "Start ota bp %d because have other bp force discharge with cur %d", _slot, bp->m_cur);
            return 0;
        }
        if(bp->m_state == BP_STATE_DISCHARGING){
            LOG_INF(TAG, "Start ota bp %d because have other bp discharge with cur %d", _slot, bp->m_cur);
            return 0;
        }
    }
    return -1;
}

int32_t sm_ev_check_bp_version_different(sm_sv_ev_t* _this, uint8_t _slot, const char* _version){
    if(!_this){
        return -1;
    }
    sm_sv_ev_impl_t* this = _this;
    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(this, _slot);
    char bp_version[16];
    sm_ev_version_to_string((char*)bp_data->m_version, bp_version);
    if(!strcmp(bp_version, _version)){
        return -1;
    }
    return 0;
}

static void sm_sv_ev_sync_module_process(sm_sv_ev_impl_t *_this) {
    if(_impl(_this)->m_config->m_auth_module.m_level == SM_EV_CONFIG_AUTH_MODULE_DISABLE){
        return;
    }

    if (_this->m_mc_module->m_connection_state == MODULE_STATE_DISCONNECTED
        && !elapsed_timer_get_remain(&_this->m_mc_module->m_connected_timeout)) {

        if (_this->m_ev_data.m_pmu_data->m_parking == EV_EXIT_PARKING) {
            if(_this->m_ev_data.m_err != EV_ERR_LOST_CAN_NETWORK){
                _this->m_ev_data.m_err = EV_ERR_LOST_CAN_NETWORK;
                if (_this->m_event_handle && _this->m_event_handle->on_ev_event) {
                    _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                       _this->m_ev_data.m_err,
                                                       _this->m_event_arg);
                }
            }

            if(_this->m_config->m_auth_module.m_level == SM_EV_CONFIG_AUTH_MODULE_LEVEL_2){
                sm_sv_bp_set_off_all(_impl(_this)->m_bp_manager);
            }
        }

        elapsed_timer_resetz(&_this->m_mc_module->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
    }


    if (_this->m_pmu_module->m_connection_state == MODULE_STATE_DISCONNECTED &&
        !elapsed_timer_get_remain(&_this->m_pmu_module->m_connected_timeout)) {
        if(_this->m_ev_data.m_err != EV_ERR_LOST_CAN_NETWORK){
            _this->m_ev_data.m_err = EV_ERR_LOST_CAN_NETWORK;
            if (_this->m_event_handle && _this->m_event_handle->on_ev_event) {
                _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                   _this->m_ev_data.m_err,
                                                   _this->m_event_arg);
            }
        }

        if(_this->m_config->m_auth_module.m_level == SM_EV_CONFIG_AUTH_MODULE_LEVEL_2){
            sm_sv_bp_set_off_all(_impl(_this)->m_bp_manager);
        }

        elapsed_timer_resetz(&_this->m_pmu_module->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
    }
}

int32_t sm_sv_ev_process(sm_sv_ev_t* _this){
    if(!_this){
        return -1;
    }

    sm_module_process(_impl(_this)->m_pmu_module);
    sm_module_process(_impl(_this)->m_mc_module);
//    sm_sv_bp_process(_impl(_this)->m_bp_manager);

    if(sm_queue_get_size(_impl(_this)->m_data_queue)){
#ifdef MUTILPLE_THREAD
        ENTER_CRITICAL(m_lock);
#endif
        sm_ev_module_data_coming_t data;
        sm_queue_pop(_impl(_this)->m_data_queue, &data);

#ifdef MUTILPLE_THREAD
        EXIT_CRITICAL(m_lock);
#endif
        sm_ev_process_data(_this, &data);
    }

    sm_sv_ev_cmd_handle(_impl(_this));

    int32_t odo_diff = _impl(_this)->m_ev_data.m_odo -_impl(_this)->m_odo_latch;
    if (_impl(_this)->m_ev_data.m_pmu_data->m_key == EV_KEY_ON && odo_diff >= (_impl(_this)->m_config->m_km_store_odo * KM_TO_M)){
        if(!sm_ev_odo_storage_store(_impl(_this)->m_odo_storage, _impl(_this)->m_ev_data.m_odo)){
            _impl(_this)->m_odo_latch = _impl(_this)->m_ev_data.m_odo ;
        }
    }

    if(_impl(_this)->m_hmi_reboot_request && !elapsed_timer_get_remain(&_impl(_this)->m_reboot_timeout)){
        if(sm_ev_odo_storage_store(_impl(_this)->m_odo_storage, _impl(_this)->m_ev_data.m_odo) < 0){
            LOG_ERR(TAG, "Could NOT set ODO when key reboot requested");
        }
        sm_bsp_hmi_system_reset();
    }

    sm_sv_ev_sync_module_process(_impl(_this));

    return 0;
}

/***************************************** BP Handler ****************************************/
static void sm_ev_on_bp_connected(int32_t _slot, const char* _sn, int32_t _soc, void* _arg){
    (void)_sn;
    (void)_soc;
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(!_this){
        return;
    }
    LOG_ERR(TAG, "BP %d connected: %s, SOC: %d", _slot, _sn, _soc);
    if(_this->m_event_handle && _this->m_event_handle->on_bp_event){
        _this->m_event_handle->on_bp_event(_slot,
                                           SM_BP_EVENT_CONNECTED,
                                           _soc,
                                           _this->m_event_arg);
    }
}

static void sm_ev_on_bp_disconnected(int32_t _slot, const char* _sn, void* _arg){
    (void)_sn;
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(!_this){
        return;
    }
    LOG_INF(TAG, "BP %d disconnected: %s", _slot, _sn);
    if(_this->m_event_handle){
        _this->m_event_handle->on_bp_event(_slot,
                                           SM_BP_EVENT_DISCONNECTED,
                                           0,
                                           _this->m_event_arg);
    }
}

static void sm_ev_on_bp_update_data(int32_t _slot, const sm_bp_data_t* _bp_data, void*_arg){
    (void)_arg;
    (void)_slot;
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(!_this){
        return;
    }

    if(_bp_data->m_state == BP_STATE_FAULT && _bp_data->m_status == SM_ERR_BP_LIMIT_SOFT_START){
        _this->m_ev_data.m_err = EV_ERR_BP_LIMIT_SOFT_START;
        if(_this->m_event_handle && _this->m_event_handle->on_ev_event){
            _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                               _this->m_ev_data.m_err,
                                               _this->m_event_arg);
        }
    }

    if(_bp_data->m_state != BP_STATE_FAULT && _this->m_ev_data.m_err == EV_ERR_BP_LIMIT_SOFT_START){
        _this->m_ev_data.m_err = EV_ERR_NONE;
        if(_this->m_event_handle && _this->m_event_handle->on_ev_event){
            _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                               _this->m_ev_data.m_err,
                                               _this->m_event_arg);
        }
    }
}

static void sm_ev_on_bp_cmd(int32_t _slot, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg){

	(void)_data;
    LOG_DBG(TAG, "On BP %d CMD: %s", _slot, _success == SM_BP_CMD_SUCCESS ? "SUCCESS" : "FAILURE");
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(_this->m_cmd.m_cb){
        _this->m_cmd.m_cb(_success, _this->m_cmd.m_cmd, &_this->m_cmd.m_cmd_data, _this->m_cmd.m_cb_arg);
    }

    if(_success == SM_BP_CMD_SUCCESS && _this->m_cmd.m_cmd == SM_EV_CMD_WRITE_DEV_TO_BP && _cmd == BP_CMD_WRITE_ASSIGNED_DEV){
        sm_sv_bp_reset(_this->m_bp_manager, _slot);
    }

    sm_sv_ev_reset_cmd(_this);
}

/***************************************** BP Handler END ****************************************/

static void sm_ev_process_data(sm_sv_ev_impl_t* _this, sm_ev_module_data_coming_t* _data){
    uint8_t node_id = (uint8_t)(_data->m_msg_id & 0x7F);
    sm_ev_module_t* module = NULL;

    if(_impl(_this)->m_pmu_module->m_id == node_id){
        module = _impl(_this)->m_pmu_module;
    }else if(_impl(_this)->m_mc_module->m_id == node_id){
        module = _impl(_this)->m_mc_module;
    }else{
        return;
    }

    module->m_proc->handle_data(module, _data->m_msg_id, _data->m_data, _data->m_data_len);

    if(module->m_connection_state == MODULE_STATE_DISCONNECTED){
        module->m_connection_state = MODULE_STATE_CONNECTED;
        if(module->m_callback_fn){
            module->m_callback_fn(module->m_id, MODULE_EVENT_CONNECTED, NULL, module->m_arg);
        }
    }
    elapsed_timer_resetz(&module->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
}

static void sm_ev_recv_data(const uint32_t _can_id, uint8_t* _data, void* _arg){
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(!_this){
        return;
    }
    sm_ev_module_data_coming_t pdo = {
            .m_msg_id = (int32_t)_can_id,
    };
    memcpy(pdo.m_data, _data, 8);

#ifdef MUTILPLE_THREAD
    ENTER_CRITICAL(m_lock);
#endif

    sm_queue_push(_this->m_data_queue, &pdo);

#ifdef MUTILPLE_THREAD
    EXIT_CRITICAL(m_lock);
#endif
}

static int32_t sm_sv_ev_ctl_left_signal(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_left_signal(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_ctl_right_signal(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_right_signal(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_ctl_high_beam(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_high_beam(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_ctl_low_beam(sm_sv_ev_t* _this, uint8_t  _value){
    return sm_pmu_ctl_low_beam(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_ctl_horn(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_horn(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_find_ev(sm_sv_ev_t* _this){
    return sm_pmu_ctl_find_ev(_impl(_this)->m_pmu_module, 1, sm_ev_recv_sdo_confirmed, _this) &&
            sm_pmu_ctl_horn(_impl(_this)->m_pmu_module, 1, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_lock_ev(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_lock_ev(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_block_ev(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_block_ev(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_anti_theft_ev(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_ctl_anti_theft_ev(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_set_lock_port(sm_sv_ev_t* _this, uint8_t _port, uint8_t _value){
    return sm_pmu_set_lock_port(_impl(_this)->m_pmu_module, _port, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_set_enable_port(sm_sv_ev_t* _this, uint8_t _port, uint8_t _value){
    return sm_pmu_set_enable_port(_impl(_this)->m_pmu_module, _port, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_config_verify_bp_offline(sm_sv_ev_t* _this, uint8_t _value){
    return sm_pmu_config_verify_bp_offline(_impl(_this)->m_pmu_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_set_drive_mode(sm_sv_ev_t* _this, uint8_t  _value){
    if(_impl(_this)->m_ev_data.m_pmu_data->m_parking != EV_EXIT_PARKING){
        return -1;
    }
    return sm_mc_set_drive_mode(_impl(_this)->m_mc_module, _value, sm_ev_recv_sdo_confirmed, _this);
}

static int32_t sm_sv_ev_set_max_speed(sm_sv_ev_t* _this, uint8_t  _value){
    if(!_this || _impl(_this)->m_ev_data.m_pmu_data->m_parking != EV_EXIT_PARKING){
        return -1;
    }
    uint8_t value =  (uint8_t)(_value * (0.27777/_impl(_this)->m_ev_manu->m_wheel_radius));
    return sm_mc_set_max_speed(_impl(_this)->m_mc_module, value, sm_ev_recv_sdo_confirmed, _this);
}

static void sm_ev_recv_sdo_confirmed(SM_SDO_STATUS_t _status, int32_t _tx_err, int32_t _rx_err, void* _arg){
    (void)_tx_err;
    (void)_rx_err;
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;

    LOG_DBG(TAG, "EV receive SDO confirmed %d: %s. tx_err: 0x%x, rx_err: 0x%x",_this->m_cmd.m_cmd, _status==SM_SDO_ST_SUCCESS ? "SUCCESS" : "FAILURE",_tx_err, _rx_err);

    if(!_this){
        return;
    }
    int32_t result = _status == SM_SDO_ST_SUCCESS ? (SM_EV_CMD_SUCCESS) : (SM_EV_CMD_FAILURE);

    if(_this->m_cmd.m_cb){
        _this->m_cmd.m_cb(result, _this->m_cmd.m_cmd, &_this->m_cmd.m_cmd_data, _this->m_cmd.m_cb_arg);
    }

    if(_this->m_cmd.m_cmd == SM_EV_CMD_SET_MAX_SPEED && result == SM_EV_CMD_FAILURE){
        sm_sv_ev_set_max_speed(_this, _this->m_ev_data.m_max_speed);
    }

    sm_sv_ev_reset_cmd(_this);
}

/**************************** PMU Handler Event ************************************/
static void sm_pmu_on_drive_mode_signal(uint8_t _value, void* _arg){
    sm_sv_ev_impl_t* ev_service = (sm_sv_ev_impl_t*)_arg;
    if(!ev_service){
        LOG_ERR(TAG, "Missing param");
        return;
    }
    if(ev_service->m_ev_data.m_pmu_data->m_key == EV_KEY_OFF){
        return;
    }
    if(ev_service->m_ev_data.m_pmu_data->m_parking == EV_ENTER_PARKING){
        return;
    }
    switch (_value) {
        case EV_ECO_MODE_SIGNAL:
            LOG_DBG(TAG, "Set ECO mode from mode BUTTON");
            sm_mc_set_drive_mode(ev_service->m_mc_module,
                                 EV_ECO_MODE_1,
                                 sm_ev_recv_sdo_confirmed,
                                 ev_service);
            break;
        case EV_SPORT_MODE_SIGNAL:
            LOG_DBG(TAG, "Set SPORT mode from mode BUTTON");
            sm_mc_set_drive_mode(ev_service->m_mc_module,
                                 EV_SPORT_MODE_1,
                                 sm_ev_recv_sdo_confirmed,
                                 ev_service);
            break;
        case EV_UPHILL_MODE_SIGNAL:
            LOG_DBG(TAG, "Set Uphill mode from mode BUTTON");
            sm_mc_set_drive_mode(ev_service->m_mc_module,
                                 EV_UP_HILL_MODE,
                                 sm_ev_recv_sdo_confirmed,
                                 ev_service);
            break;
        default:
            LOG_WRN(TAG, "Drive MODE signal NOT Support");
            break;
    }
}
static void sm_pmu_on_backward_mode_signal(uint8_t _value, void* _arg){
    (void)_value;
    (void)_arg;
}
static void sm_pmu_on_port_stated_changed(uint8_t _port, uint8_t _value, void* _arg) {
    if(!_arg){
        return;
    }
    if(!_impl(_arg)->m_event_handle || !_impl(_arg)->m_event_handle->on_bp_event){
        return;
    }

    _impl(_arg)->m_event_handle->on_bp_event(_port,
                                             SM_BP_EVENT_LOCK_PORT_CHANGED,
                                             _value,
                                             _impl(_arg)->m_event_arg);
}

static void sm_pmu_on_connection_changed(uint8_t _id, uint8_t _event, void* _data, void* _arg){
    (void)_id;
    (void)_data;
    LOG_ERR(TAG, "PMU is %s", _event == MODULE_EVENT_CONNECTED ? "Connected" : "Disconnected");

    sm_sv_ev_impl_t* this = _impl(_arg);
    sm_ev_module_t* pmu = (sm_ev_module_t*)this->m_pmu_module;
    if(_event == MODULE_EVENT_DISCONNECTED){
        sm_module_reset_data(&pmu->m_info);
        elapsed_timer_resetz(&pmu->m_connected_timeout, this->m_config->m_auth_module.m_detected_time);
    }else{
        if(this->m_ev_data.m_err == EV_ERR_LOST_CAN_NETWORK){
            this->m_ev_data.m_err = EV_ERR_NONE;
            if(this->m_event_handle && this->m_event_handle->on_ev_event){
                this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                  this->m_ev_data.m_err,
                                                  this->m_event_arg);
            }
        }
    }
}

static void sm_pmu_on_err(uint8_t _err, void* _arg){
    sm_sv_ev_impl_t* _this = (sm_sv_ev_impl_t*)_arg;
    if(_err){
        _this->m_ev_data.m_err = SM_EV_ERR_OFFSET_PMU + _err;
    }else{
        _this->m_ev_data.m_err = EV_ERR_NONE;
    }
    if(_this->m_event_handle && _this->m_event_handle->on_ev_event) {
        _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                           _impl(_this)->m_ev_data.m_err,
                                           _impl(_this)->m_event_arg);
    }
}
/**************************** END ************************************/

/**************************** MC Handler Event ************************************/
static void sm_mc_on_connection_changed(uint8_t _id, uint8_t _event, void *_data, void *_arg) {
    (void) _id;
    (void) _data;
    LOG_ERR(TAG, "MC is %s", _event == MODULE_EVENT_CONNECTED ? "Connected" : "Disconnected");
    sm_sv_ev_impl_t *this = _impl(_arg);
    sm_ev_module_t *mc = (sm_ev_module_t *) this->m_mc_module;

    if (_event == MODULE_EVENT_DISCONNECTED && this->m_ev_data.m_pmu_data->m_parking == EV_EXIT_PARKING) {
        elapsed_timer_resetz(&mc->m_connected_timeout, this->m_config->m_auth_module.m_detected_time);
    }

    if (_event == MODULE_EVENT_CONNECTED && this->m_ev_data.m_err == EV_ERR_LOST_CAN_NETWORK) {
        this->m_ev_data.m_err = EV_ERR_NONE;
        if (this->m_event_handle && this->m_event_handle->on_ev_event) {
            this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                              this->m_ev_data.m_err,
                                              this->m_event_arg);
        }
    }
}
/**************************** END ************************************/

static void sm_ev_service_reset_mc_data(sm_sv_ev_impl_t* _this){
    _this->m_ev_data.m_trip = EV_DATA_TRIP_DEFAULT;
    _this->m_ev_data.m_driver_mode = EV_DATA_DRIVE_MODE_DEFAULT;
    _this->m_ev_data.m_speed = EV_DATA_SPEED_DEFAULT;

    _this->m_mc_module->m_proc->reset_data(_this->m_mc_module);
    elapsed_timer_resetz(&_this->m_mc_module->m_connected_timeout,
                         SM_EV_LOST_CONNECTION_TIMEOUT_DEFAULT);

    if(_this->m_event_handle) {
        _impl(_this)->m_event_handle->on_ev_event(SM_EV_EVENT_TRIP_CHANGED,
                                                 0,
                                                 _impl(_this)->m_event_arg);
    }
}

static void sm_ev_service_event_handle_internal(sm_sv_ev_impl_t* _this, int32_t _event, int32_t _value){
    switch (_event) {
        case SM_EV_EVENT_KEY_CHANGED:
            if (_value == EV_KEY_OFF) {
                sm_ev_odo_storage_store(_impl(_this)->m_odo_storage, _impl(_this)->m_ev_data.m_odo);
            } else {
                _this->m_odo_latch = _this->m_ev_data.m_odo;
            }

            break;
        case SM_EV_EVENT_DRIVE_MODE_CHANGED: {
                uint8_t driver_mode = EV_ECO_MODE_1;
                if (_value >= EV_MC_HAFT_ECO_MODE_1 && _value <= EV_MC_ECO_MODE_3) {
                    driver_mode = EV_ECO_MODE_1;
                } else if (_value >= EV_MC_HAFT_SPORT_MODE_2 && _value <= EV_MC_SPORT_MODE_1) {
                    driver_mode = EV_SPORT_MODE_1;
                } else if (_value >= EV_MC_HAFT_UP_HILL_MODE && _value <= EV_MC_UP_HILL_MODE) {
                    driver_mode = EV_UP_HILL_MODE;
                } else {
                    break;
                }
                if (driver_mode != _this->m_ev_data.m_driver_mode && _this->m_event_handle) {
                    _this->m_ev_data.m_driver_mode = driver_mode;
                    _impl(_this)->m_event_handle->on_ev_event(SM_EV_EVENT_DRIVE_MODE_CHANGED,
                                                              _this->m_ev_data.m_driver_mode,
                                                              _impl(_this)->m_event_arg);

                    break;
                }
            }
            break;
        case SM_EV_EVENT_SPEED_CHANGED:
            _this->m_ev_data.m_speed =  (int16_t)round(0.12f*PI_NUMBER*(_this->m_ev_manu->m_wheel_radius)*(float)_value);
            if(_this->m_ev_data.m_mc_data->m_reverse_state == EV_REVERSE_STATE_ACTIVE_REVERSE){
            	_this->m_ev_data.m_speed = -(_this->m_ev_data.m_speed);
            }
            break;
        case SM_EV_EVENT_TRIP_CHANGED: {
            int32_t new_trip = (int32_t)round((0.00694f*PI_NUMBER*2*(_this->m_ev_manu->m_wheel_radius)*(float)_value));
            if(_this->m_ev_data.m_trip == -1){
                _this->m_ev_data.m_trip = new_trip;

                if(_this->m_event_handle) {
                    _impl(_this)->m_event_handle->on_ev_event(_event,
                                                              _this->m_ev_data.m_trip,
                                                              _impl(_this)->m_event_arg);
                }
                return;
            }
            int32_t trip_step = new_trip - _this->m_ev_data.m_trip;
            if(trip_step > 0){
                _this->m_ev_data.m_odo += (uint32_t)trip_step;
                if(_this->m_event_handle){
                    _this->m_event_handle->on_ev_event(SM_EV_EVENT_ODO_CHANGED,
                                          (int32_t)_this->m_ev_data.m_odo,
                                          _this->m_event_arg);
                }

                _this->m_ev_data.m_trip = new_trip;
                _impl(_this)->m_event_handle->on_ev_event(_event,
                                                          _impl(_this)->m_ev_data.m_trip,
                                                          _impl(_this)->m_event_arg);
            }
            break;
        }
        case SM_EV_EVENT_ERROR_CHANGED:
            _this->m_ev_data.m_err = _value;
            break;
        case SM_EV_EVENT_LOCK_MODE_CHANGED:
        case SM_EV_EVENT_BLOCK_MODE_CHANGED:
        case SM_EV_EVENT_ANTI_THEFT_MODE_CHANGED:
            if(_value == EV_LOCK_STATE){
                _this->m_ev_data.m_err = EV_ERR_BLOCK_STATE;
            }else{
                _this->m_ev_data.m_err = EV_ERR_NONE;
            }
            _impl(_this)->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                      _impl(_this)->m_ev_data.m_err,
                                                      _impl(_this)->m_event_arg);
            break;
        case SM_EV_EVENT_INACTIVE_MODE_CHANGED:{
            if(!_value){
                if(sm_pmu_auth_support(_this->m_pmu_module) == EV_PMU_AUTH_SUPPORT){
                    _this->m_ev_data.m_err = EV_ERR_LOST_CAN_NETWORK;
                }
            }else{
                _this->m_ev_data.m_err = EV_ERR_NONE;
            }
            _impl(_this)->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                      _impl(_this)->m_ev_data.m_err,
                                                      _impl(_this)->m_event_arg);
            break;
        }
        case SM_EV_EVENT_PARKING_CHANGED:
            if(_value == EV_ENTER_PARKING){
                _this->m_ev_data.m_motor_active_state |= SM_MOTOR_ACTIVE_CONDITION_EXIT_PACKING;

                if(_this->m_ev_data.m_err == EV_ERR_LOST_CAN_NETWORK){
                    _this->m_ev_data.m_err = EV_ERR_NONE;
                    if (_this->m_event_handle && _this->m_event_handle->on_ev_event) {
                        _this->m_event_handle->on_ev_event(SM_EV_EVENT_ERROR_CHANGED,
                                                           _this->m_ev_data.m_err,
                                                           _this->m_event_arg);
                    }
                }
            }else{
                _this->m_ev_data.m_motor_active_state &= (uint8_t)(~SM_MOTOR_ACTIVE_CONDITION_EXIT_PACKING);

                sm_sv_ev_set_max_speed(_this, _this->m_ev_data.m_max_speed);
            }
            sm_ev_service_reset_mc_data(_this);

            break;
        default:
            break;
    }
}

int32_t sm_ev_service_notify_ev_event(sm_sv_ev_t* _this, int32_t _event, int32_t _value){
    if(!_this){
        return -1;
    }
    
    sm_ev_service_event_handle_internal(_impl(_this), _event, _value);

    if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_ev_event){
        if(_event == SM_EV_EVENT_SPEED_CHANGED){
            _impl(_this)->m_event_handle->on_ev_event(_event,
                                         _impl(_this)->m_ev_data.m_speed,
                                         _impl(_this)->m_event_arg);
        }else if(_event == SM_EV_EVENT_ERROR_CHANGED){
            _impl(_this)->m_event_handle->on_ev_event(_event,
                                                      _impl(_this)->m_ev_data.m_err,
                                                      _impl(_this)->m_event_arg);
        }else if(_event == SM_EV_EVENT_TRIP_CHANGED || _event == SM_EV_EVENT_DRIVE_MODE_CHANGED){
        }else{
            _impl(_this)->m_event_handle->on_ev_event(_event,
                                         _value,
                                         _impl(_this)->m_event_arg);
        }
    }
    return 0;
}

int32_t sm_ev_service_notify_bp_event(sm_sv_ev_t* _this, int32_t _port, int32_t _event, int32_t _value){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_bp_event){
        _impl(_this)->m_event_handle->on_bp_event(_port,
                                                  _event,
                                                  _value,
                                                  _impl(_this)->m_event_arg);

        return 0;
    }
    return -1;
}
