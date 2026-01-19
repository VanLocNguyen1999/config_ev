//
// Created by vnbk on 12/06/2023.
//
#include "sm_ev_pmu_module.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"
#include "sm_ev_cmd.h"
#include "sm_co_od_common.h"

#include "sm_ev_internal_service.h"

#define TAG "sm_ev_pmu_module"

#define _impl(p)   ((sm_pmu_module_t*)(p))

typedef struct sm_pmu_module{
    sm_ev_module_t m_base;
    sm_co_t * m_co;

    void* m_owner;

    int32_t m_auth_support;

    sm_pmu_data_t m_pmu_data;
    uint8_t m_cmd_temp[128];

    sm_pmu_event_t* m_pmu_event_handle;
    void* m_pmu_event_arg;
}sm_pmu_module_t;

static int32_t sm_pmu_init(sm_ev_module_t*);
static int32_t sm_pmu_free(sm_ev_module_t*);
static int32_t sm_pmu_reboot(sm_ev_module_t*);
static void    sm_pmu_reset_data(sm_ev_module_t*);
static const char* sm_pmu_get_name(sm_ev_module_t* _this);
static int32_t sm_pmu_process(sm_ev_module_t*);
static int32_t sm_pmu_read_info(sm_ev_module_t*);
static int32_t sm_module_handle_data(sm_ev_module_t* _this, int32_t _msg_id, const uint8_t* _data, int32_t _data_len);

static void sm_pmu_co_sdo_cb(SM_SDO_STATUS_t, int32_t, int32_t, void*);

static sm_module_proc_t g_module_proc = {
    .init = sm_pmu_init,
    .free = sm_pmu_free,
    .reboot = sm_pmu_reboot,
    .get_name = sm_pmu_get_name,
    .reset_data = sm_pmu_reset_data,
    .get_data = sm_pmu_get_data,
    .handle_data = sm_module_handle_data,
    .read_info = sm_pmu_read_info,
    .process = sm_pmu_process,
};

static sm_pmu_module_t g_pmu_module = {
    .m_base.m_proc = &g_module_proc,
    .m_base.m_id = SM_MODULE_ID_DEFAULT,
    .m_base.m_callback_fn = NULL,
    .m_base.m_arg = NULL,
    .m_auth_support = -1
};

static int32_t sm_pmu_init(sm_ev_module_t* _this){
    sm_pmu_reset_data(_this);
    return 0;
}

static int32_t sm_pmu_free(sm_ev_module_t* _this){
    _this->m_callback_fn = NULL;
    _this->m_arg = NULL;
    return 0;
}

static const char* sm_pmu_get_name(sm_ev_module_t* _this){

	(void)_this;
    return "pmu";
}

static void sm_pmu_reset_data(sm_ev_module_t* _this){

    _impl(_this)->m_pmu_data.m_key = EV_DATA_KEY_DEFAULT;
    _impl(_this)->m_pmu_data.m_brake = EV_DATA_BRAKE_DEFAULT;
    _impl(_this)->m_pmu_data.m_parking = EV_DATA_PACKING_DEFAULT;
    _impl(_this)->m_pmu_data.m_horn = EV_DATA_HORN_DEFAULT;
    _impl(_this)->m_pmu_data.m_low_beam_state = EV_DATA_LOW_BEAM_DEFAULT;
    _impl(_this)->m_pmu_data.m_high_beam_state = EV_DATA_HIGH_BEAM_DEFAULT;
    _impl(_this)->m_pmu_data.m_left_signal = EV_DATA_LEFT_SIGNAL_DEFAULT;
    _impl(_this)->m_pmu_data.m_right_signal = EV_DATA_RIGHT_SIGNAL_DEFAULT;
    _impl(_this)->m_pmu_data.m_energy_in = EV_DATA_POWER_IN_DEFAULT;
    _impl(_this)->m_pmu_data.m_energy_out = EV_DATA_POWER_OUT_DEFAULT;
    _impl(_this)->m_pmu_data.m_abp_voltage = EV_DATA_ABP_VOL_DEFAULT;
    _impl(_this)->m_pmu_data.m_power_per_km = EV_DATA_POWER_PER_KM_DEFAULT;
    _impl(_this)->m_pmu_data.m_range = EV_DATA_RANGE_DEFAULT;
    _impl(_this)->m_pmu_data.m_discharge_cur_lim = EV_DATA_DISCHAR_CUR_LIM_DEFAULT;
    _impl(_this)->m_pmu_data.m_charge_cur_lim = EV_DATA_CHAR_CUR_LIM_DEFAULT;

    _impl(_this)->m_pmu_data.m_lock_status = EV_UNLOCK_STATE;
    _impl(_this)->m_pmu_data.m_block_status = EV_UNBLOCK_STATE;
    _impl(_this)->m_pmu_data.m_anti_theft_status = EV_UNBLOCK_STATE;
    _impl(_this)->m_pmu_data.m_inactive_mode = EV_ACTIVE_STATE;

    _impl(_this)->m_auth_support = -1;
    _this->m_sync_info = false;
    sm_module_reset_data(&_this->m_info);
    memset(_this->m_version, '\0', sizeof(_this->m_version));
    elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
}

void*  sm_pmu_get_data(sm_ev_module_t* _this){
    return &_impl(_this)->m_pmu_data;
}

static int32_t sm_pmu_reboot(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    LOG_WRN(TAG, "Reboot PMU command. Are you sure!!!!");

    _impl(_this)->m_cmd_temp[0] = 0;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_REBOOT_INDEX,
                                 SDO_PMU_REBOOT_SUB_INDEX,
                                 _this->m_id,
                                 _impl(_this)->m_cmd_temp,
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 NULL,
                                 NULL);
}

static int32_t sm_pmu_process(sm_ev_module_t* _this){
//    return sm_module_process(_this);
	(void)_this;
	return 0;
}

static void sm_pmu_co_sdo_cb(SM_SDO_STATUS_t _status,  int32_t _tx_err, int32_t _rx_err, void* _arg){

	(void) _tx_err;
	(void) _rx_err;
    sm_ev_module_t* _this = (sm_ev_module_t*)_arg;
    LOG_ERR(TAG, "PMU read info FAILURE. tx_err: 0x%2x, rx_err: 0x%2x", _tx_err, _rx_err);
    if(_status == SM_SDO_ST_ABORT){
        _this->m_sync_info = false;
        return;
    }
    memcpy(&_this->m_info, _impl(_this)->m_cmd_temp, EV_INFO_SIZE);
}

static void sm_pmu_co_sdo_version_cb(SM_SDO_STATUS_t _status,  int32_t _tx_err, int32_t _rx_err, void* _arg){

	(void) _tx_err;
	(void) _rx_err;
    sm_ev_module_t* _this = (sm_ev_module_t*)_arg;
    LOG_WRN(TAG, "PMU read info FAILURE. tx_err: 0x%2x, rx_err: 0x%2x", _tx_err, _rx_err);
    if(_status == SM_SDO_ST_ABORT){
        _this->m_sync_info = false;
        return;
    }
    memcpy(_this->m_info.m_sw_ver, _impl(_this)->m_cmd_temp, 4);

    if(_this->m_info.m_sw_ver[0] == 0 && _this->m_info.m_sw_ver[1] && _this->m_info.m_sw_ver[2]){
          LOG_WRN(TAG, "PMU version is INVALID, try again");
          _this->m_sync_info = false;
          return;
      }

    sm_ev_version_to_string(_this->m_info.m_sw_ver, _this->m_version);
    LOG_INF(TAG, "%s version: %s", _this->m_proc->get_name(_this), _this->m_version);

    if(strstr(_this->m_version, EV_PMU_VER_AUTH_SUPPORT) != NULL){
        _impl(_this)->m_auth_support = EV_PMU_AUTH_SUPPORT;
    }else{
        _impl(_this)->m_auth_support = EV_PMU_AUTH_NOT_SUPPORT;
    }
}

static int32_t sm_module_handle_data(sm_ev_module_t* _this, int32_t _msg_id, const uint8_t* _data, int32_t _data_len){
    (void)_data_len;
    (void)_this;
    (void)_msg_id;
    (void)_data;
//    switch ((uint32_t)_msg_id & 0xFFFFFF80) {
//        case CO_CAN_ID_TPDO_1:
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_drive_mode_signal, CO_READ_BIT(_data[0], 4))){
//                _impl(_this)->m_pmu_data.m_drive_mode_signal = CO_READ_BIT(_data[0], 4);
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_drive_mode_signal) {
//                    _impl(_this)->m_pmu_event_handle->on_drive_mode_signal(CO_READ_BIT(_data[0], 4),
//                                                                           _impl(_this)->m_pmu_event_arg);
//                }
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_DRIVE_MODE_SIGNAL_CHANGED,
//                                           _impl(_this)->m_pmu_data.m_drive_mode_signal);
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_range, CO_getUint16(_data + 2))){
//                _impl(_this)->m_pmu_data.m_range =  CO_getUint16(_data + 2);
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_RANGE_CHANGED,
//                                           _impl(_this)->m_pmu_data.m_range);
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_backward_mode_signal, CO_READ_BIT(_data[1], 4))){
//                _impl(_this)->m_pmu_data.m_backward_mode_signal =  CO_READ_BIT(_data[1], 4);
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_backward_mode_signal) {
//                    _impl(_this)->m_pmu_event_handle->on_backward_mode_signal(CO_READ_BIT(_data[1], 4),
//                                                                              _impl(_this)->m_pmu_event_arg);
//                }
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_port_lock_status[0], CO_READ_BIT(_data[0], 6))){
//                _impl(_this)->m_pmu_data.m_port_lock_status[0] =  CO_READ_BIT(_data[0], 6);
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_port_stated_changed) {
//                    _impl(_this)->m_pmu_event_handle->on_port_stated_changed(0,
//                                                                             CO_READ_BIT(_data[0], 6),
//                                                                             _impl(_this)->m_pmu_event_arg);
//                }
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_port_lock_status[1], CO_READ_BIT(_data[1], 6))){
//                _impl(_this)->m_pmu_data.m_port_lock_status[1] =  CO_READ_BIT(_data[1], 6);
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_port_stated_changed) {
//                    _impl(_this)->m_pmu_event_handle->on_port_stated_changed(1,
//                                                                             CO_READ_BIT(_data[1], 6),
//                                                                             _impl(_this)->m_pmu_event_arg);
//                }
//
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_port_lock_status[2], CO_READ_BIT(_data[1], 7))){
//                _impl(_this)->m_pmu_data.m_port_lock_status[2] =  CO_READ_BIT(_data[1], 7);
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_port_stated_changed) {
//                    _impl(_this)->m_pmu_event_handle->on_port_stated_changed(2,
//                                                                             CO_READ_BIT(_data[1], 7),
//                                                                             _impl(_this)->m_pmu_event_arg);
//                }
//            }
//
//
//            _impl(_this)->m_pmu_data.m_bp_mode = CO_READ_BIT(_data[1], 2);
//
//            _impl(_this)->m_pmu_data.m_discharge_cur_lim = 10 * CO_getUint16(_data + 4);
//            _impl(_this)->m_pmu_data.m_charge_cur_lim = 10 * CO_getUint16(_data + 4);
//            break;
//        case CO_CAN_ID_TPDO_2:
//            _impl(_this)->m_pmu_data.m_energy_in =  CO_getUint32(_data);
//            _impl(_this)->m_pmu_data.m_energy_out =  CO_getUint32(_data + 4);
//            break;
//        case CO_CAN_ID_TPDO_3: {
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_err_code, (_data[3]))){
//                _impl(_this)->m_pmu_data.m_err_code = _data[3];
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_err) {
//                    _impl(_this)->m_pmu_event_handle->on_err(_data[3],
//                                                             _impl(_this)->m_pmu_event_arg);
//                }
//            }
//
//            for (uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++) {
//                if (SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_bp_checking_state[index], _data[index])) {
//                    _impl(_this)->m_pmu_data.m_bp_checking_state[index] = _data[index];
//                    sm_ev_service_notify_bp_event(_impl(_this)->m_owner,
//                                                  index,
//                                                  SM_BP_EVENT_VALIDATE_CHANGED,
//                                                 _impl(_this)->m_pmu_data.m_bp_checking_state[index]);
//                }
//            }
//            break;
//        }
//        case CO_CAN_ID_TPDO_4:
//            _impl(_this)->m_pmu_data.m_power_per_km = _data[2];
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_uphill_mode, _data[7])){
//                _impl(_this)->m_pmu_data.m_uphill_mode = _data[7];
//                if(_impl(_this)->m_pmu_event_handle && _impl(_this)->m_pmu_event_handle->on_drive_mode_signal) {
//                    _impl(_this)->m_pmu_event_handle->on_drive_mode_signal(EV_UPHILL_MODE_SIGNAL,
//                                                                           _impl(_this)->m_pmu_event_arg);
//                }
//                if(_data[7] == 1){
//                    sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                               SM_EV_EVENT_DRIVE_MODE_SIGNAL_CHANGED,
//                                               EV_UPHILL_MODE_SIGNAL);
//                }
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_lock_status, _data[4])){
//                _impl(_this)->m_pmu_data.m_lock_status = _data[4];
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner, SM_EV_EVENT_LOCK_MODE_CHANGED, _data[4]);
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_block_status, _data[3])){
//                _impl(_this)->m_pmu_data.m_block_status = _data[3];
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner, SM_EV_EVENT_BLOCK_MODE_CHANGED, _data[3]);
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_anti_theft_status, _data[5])){
//                _impl(_this)->m_pmu_data.m_anti_theft_status = _data[5];
////                sm_ev_service_notify_ev_event(_impl(_this)->m_owner, SM_EV_EVENT_ANTI_THEFT_MODE_CHANGED, _data[5]);
//            }
//
//            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_pmu_data.m_inactive_mode, _data[6])){
//                _impl(_this)->m_pmu_data.m_inactive_mode = _data[6];
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner, SM_EV_EVENT_INACTIVE_MODE_CHANGED, _data[6]);
//            }
//
//            break;
//        default:
//            break;
//    }

    return 0;
}

static int32_t sm_pmu_read_info(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }

    sm_co_sdo_client_receive(_impl(_this)->m_co,
                             SDO_PMU_INFO_INDEX,
                             SDO_PMU_INFO_SUB_INDEX,
                             _this->m_id,
                             (void *) _impl(_this)->m_cmd_temp,
                             EV_INFO_SIZE,
                             SDO_TIMEOUT_DEFAULT,
                             sm_pmu_co_sdo_cb,
                             _this);

    return sm_co_sdo_client_receive(_impl(_this)->m_co,
                                    SDO_VERSION_INDEX,
                                    SDO_VERSION_SUB_INDEX,
                                    _this->m_id,
                                    (void *) _impl(_this)->m_cmd_temp,
                                    EV_INFO_SIZE,
                                    SDO_TIMEOUT_DEFAULT,
                                    sm_pmu_co_sdo_version_cb,
                                    _this);
}

sm_ev_module_t* sm_pmu_create(void* _owner, sm_co_t* _co, sm_pmu_event_t* _event_handle, void* _arg){
    if(!_owner || !_co){
        LOG_ERR(TAG,"Could NOT created PMU module, INVALID Params");
        return NULL;
    }
    g_pmu_module.m_base.m_id = PMU_NODE_ID_DEFAULT;
    g_pmu_module.m_base.m_connection_state = MODULE_STATE_DISCONNECTED;

    g_pmu_module.m_co = _co;
    g_pmu_module.m_owner = _owner;

    g_pmu_module.m_pmu_event_handle = _event_handle;
    g_pmu_module.m_pmu_event_arg = _arg;

    LOG_INF(TAG, "Create PMU module SUCCESS");

    return &g_pmu_module.m_base;
}

int32_t sm_pmu_destroy(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    _this->m_proc->free(_this);
    return 0;
}

int32_t sm_pmu_auth_support(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    return _impl(_this)->m_auth_support;
}

int32_t sm_pmu_ctl_left_signal(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    (void)_this;
    (void)_value;
    LOG_ERR("TAG", "Current version PMU is NOT supported");

    if(_cb_fn){
        _cb_fn(-1, -1, -1,_arg);
    }

    return -1;
}

int32_t sm_pmu_ctl_right_signal(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    (void)_this;
    (void)_value;
    LOG_ERR("TAG", "Current version PMU is NOT supported");

    if(_cb_fn){
        _cb_fn(-1, -1, -1,_arg);
    }
    return -1;
}

int32_t sm_pmu_ctl_high_beam(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    (void)_this;
    (void)_value;
    LOG_ERR("TAG", "Current version PMU is NOT supported");

    if(_cb_fn){
        _cb_fn(-1, -1, -1,_arg);
    }

    return -1;
}

int32_t sm_pmu_ctl_low_beam(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    (void)_this;
    (void)_value;
    LOG_ERR("TAG", "Current version PMU is NOT supported");

    if(_cb_fn){
        _cb_fn(-1, -1, -1,_arg);
    }

    return -1;
}

int32_t sm_pmu_ctl_horn(sm_ev_module_t *_this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void *_arg) {
    if(!_this){
        return -1;
    }
    LOG_DBG("TAG", "Control horn command: %d", _value);
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_INDEX,
                                 SDO_PMU_HORN_CTRL_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb_fn,
                                 _arg);
}

int32_t sm_pmu_ctl_find_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    if(!_this){
        return -1;
    }

    LOG_DBG("TAG", "Find EV command: %d", _value);
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_INDEX,
                                 SDO_PMU_FIND_VEHICLE_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb_fn,
                                 _arg);
}

int32_t sm_pmu_ctl_lock_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    LOG_DBG("TAG", "Lock EV command");
    if(_value){
        _impl(_this)->m_cmd_temp[0] = 0x11;
    }else{
        _impl(_this)->m_cmd_temp[0]  = 0x10;
    }
    _impl(_this)->m_cmd_temp[1] = 0x00;
    _impl(_this)->m_cmd_temp[2] = 0x00;
    _impl(_this)->m_cmd_temp[3] = 0x00;
    _impl(_this)->m_cmd_temp[4] = 0x00;

    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_INDEX,
                                 SDO_PMU_EV_LOCK_SUB_INDEX,
                                 _this->m_id,
                                 _impl(_this)->m_cmd_temp,
                                 5,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_pmu_ctl_block_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    LOG_DBG("TAG", "Block EV command");
    if(_value){
        _impl(_this)->m_cmd_temp[0] = 0x11;
    }else{
        _impl(_this)->m_cmd_temp[0]  = 0x10;
    }
    _impl(_this)->m_cmd_temp[1] = 0x00;
    _impl(_this)->m_cmd_temp[2] = 0x00;
    _impl(_this)->m_cmd_temp[3] = 0x00;
    _impl(_this)->m_cmd_temp[4] = 0x00;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_INDEX,
                                 SDO_PMU_EV_BLOCK_SUB_INDEX,
                                 _this->m_id,
                                 _impl(_this)->m_cmd_temp,
                                 5,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_pmu_ctl_anti_theft_ev(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    LOG_DBG("TAG", "Anti-Theft EV command");
    if(_value){
        _impl(_this)->m_cmd_temp[0] = 0x11;
    }else{
        _impl(_this)->m_cmd_temp[0]  = 0x10;
    }
    _impl(_this)->m_cmd_temp[1] = 0x00;
    _impl(_this)->m_cmd_temp[2] = 0x00;
    _impl(_this)->m_cmd_temp[3] = 0x00;
    _impl(_this)->m_cmd_temp[4] = 0x00;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_INDEX,
                                 SDO_PMU_EV_ANTI_SUB_INDEX,
                                 _this->m_id,
                                 _impl(_this)->m_cmd_temp,
                                 5,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_pmu_config_verify_bp_offline(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    if(!_this){
        return -1;
    }
    LOG_DBG("TAG", "Configure verify BP Offline command");
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CONFIG_INDEX,
                                 SDO_PMU_VERIFY_BP_OFFLINE_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb_fn,
                                 _arg);
}

int32_t sm_pmu_set_lock_port(sm_ev_module_t* _this, uint8_t _port, uint8_t _value, sm_co_sdo_cb_fn_t _cb_fn, void* _arg){
    if(!_this){
        return -1;
    }

    LOG_DBG("TAG", "Lock Port command");
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_PORT_INDEX,
                                 SDO_PMU_LOCK_PORT0_SUB_INDEX + _port,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb_fn,
                                 _arg);
}

int32_t sm_pmu_set_enable_port(sm_ev_module_t* _this, uint8_t _port, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }

    LOG_DBG("TAG", "Lock Port command");
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_PMU_CTRL_PORT_INDEX,
                                 SDO_PMU_ENABLE_PORT0_SUB_INDEX + _port,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}
