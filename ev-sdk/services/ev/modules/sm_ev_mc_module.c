//
// Created by vnbk on 12/06/2023.
//
#include <math.h>
#include "sm_ev_module.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"
#include "sm_core_co.h"
#include "sm_co_od_common.h"
#include "sm_ev_mc_module.h"
#include "sm_ev_internal_service.h"
#include "sm_types.h"
#define TAG "sm_ev_mc_module"

#define _impl(p)   ((sm_mc_module_t*)(p))

typedef struct sm_mc_module{
    sm_ev_module_t m_base;
    sm_co_t * m_co;

    void* m_owner;

    sm_mc_data_t m_mc_data;
    uint8_t m_cmd_temp[8];
}sm_mc_module_t;

static int32_t sm_mc_init(sm_ev_module_t*);
static int32_t sm_mc_free(sm_ev_module_t*);
static int32_t sm_mc_read_info(sm_ev_module_t*);
static void    sm_mc_reset_data(sm_ev_module_t*);
static const char* sm_mc_get_name(sm_ev_module_t*);
static int32_t sm_mc_process(sm_ev_module_t*);
static int32_t sm_mc_module_handle_data(sm_ev_module_t* _this, int32_t _msg_id, const uint8_t* _data, int32_t _data_len);

static sm_module_proc_t g_module_proc = {
        .init = sm_mc_init,
        .free = sm_mc_free,
        .read_info = sm_mc_read_info,
        .reset_data = sm_mc_reset_data,
        .get_data = sm_mc_get_data,
        .get_name = sm_mc_get_name,
        .handle_data = sm_mc_module_handle_data,
        .process = sm_mc_process,
};
static sm_mc_module_t g_mc_module = {
        .m_base.m_proc = &g_module_proc,
        .m_base.m_id = SM_MODULE_ID_DEFAULT,
        .m_base.m_callback_fn = NULL,
        .m_base.m_arg = NULL,
};

static int32_t sm_mc_init(sm_ev_module_t* _this){
    sm_mc_reset_data(_this);
    return 0;
}
static int32_t sm_mc_free(sm_ev_module_t* _this){
    _this->m_callback_fn = NULL;
    _this->m_arg = NULL;
    return 0;
}

static const char* sm_mc_get_name(sm_ev_module_t* _this){
	(void)_this;
    return "mc";
}

static void sm_mc_reset_data(sm_ev_module_t* _this){
    _impl(_this)->m_mc_data.m_speed_kmh = EV_DATA_SPEED_DEFAULT;
    _impl(_this)->m_mc_data.m_speed_rpm = EV_DATA_SPEED_DEFAULT;
    _impl(_this)->m_mc_data.m_trip_m = 0;
    _impl(_this)->m_mc_data.m_reverse_state = EV_DATA_REVERSER_STATE_DEFAULT;
    _impl(_this)->m_mc_data.m_thr_cmd = EV_DATA_THROTTLE_CMD_DEFAULT;
    _impl(_this)->m_mc_data.m_est_tor = EV_DATA_ESTIMATE_TOR_DEFAULT;
    _impl(_this)->m_mc_data.m_est_dc_cur = EV_DATA_ESTIMATE_DC_CUR_DEFAULT;
    _impl(_this)->m_mc_data.m_allow_dc_cur = EV_DATA_ALLOW_DC_CUR_DEFAULT;
    _impl(_this)->m_mc_data.m_motor_temp = EV_DATA_MOTOR_TEMP_DEFAULT;
    _impl(_this)->m_mc_data.m_board_temp = EV_DATA_BOARD_TEMP_DEFAULT;
    _impl(_this)->m_mc_data.m_aver_effic = EV_DATA_AVE_EFFIC_DEFAULT;
    _impl(_this)->m_mc_data.m_enr_con_effic = EV_DATA_ENR_CON_EFFIC_DEFAULT;
    _impl(_this)->m_mc_data.m_enr_dur_trip = EV_DATA_ENR_DUR_TRIP_DEFAULT;
    _impl(_this)->m_mc_data.m_revol_cnt = EV_DATA_MC_REV_CNT_DEFAULT;
    _impl(_this)->m_mc_data.m_status = EV_DATA_MC_STATUS_DEFAULT;
    _impl(_this)->m_mc_data.m_cur_mode = EV_MC_HAFT_ECO_MODE_1;
    _impl(_this)->m_mc_data.m_anti_theft_st = EV_DATA_ANTI_STATUS_DEFAULT;

    memset(_this->m_info.m_sw_ver, 0, EV_SW_VERSION_SIZE);
    memset(_this->m_version, '\0', sizeof(_this->m_version));

    _this->m_sync_info = false;

    _this->m_connection_state = MODULE_STATE_DISCONNECTED;
    if(_this->m_callback_fn){
        _this->m_callback_fn(_this->m_id, MODULE_STATE_DISCONNECTED, NULL, _this->m_arg);
    }
    elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
}

void*  sm_mc_get_data(sm_ev_module_t* _this){
    return &_impl(_this)->m_mc_data;
}

static void sm_mc_co_sdo_cb(SM_SDO_STATUS_t _status,  int32_t _tx_err, int32_t _rx_err, void* _arg){
	(void) _tx_err;
	(void) _rx_err;
    sm_ev_module_t* _this = (sm_ev_module_t*)_arg;
    LOG_WRN(TAG, "MC read info FAILURE. tx_err: 0x%2x, rx_err: 0x%2x", _tx_err, _rx_err);
    if(_status == SM_SDO_ST_ABORT){
        _this->m_sync_info = false;
        return;
    }
    if(!_this->m_info.m_sw_ver[0] && !_this->m_info.m_sw_ver[1] && !_this->m_info.m_sw_ver[2]){
        LOG_WRN(TAG, "MC version is INVALID, try again");
        _this->m_sync_info = false;
        return;
    }
    sm_ev_version_to_string(_this->m_info.m_sw_ver, _this->m_version);
    LOG_ERR(TAG, "%s version: %s", _this->m_proc->get_name(_this), _this->m_version);
}
static int32_t sm_mc_read_info(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    return sm_co_sdo_client_receive(_impl(_this)->m_co,
                                    SDO_VERSION_INDEX,
                                    SDO_VERSION_SUB_INDEX,
                                    _this->m_id,
                                    (void *) _this->m_info.m_sw_ver,
                                    EV_SW_VERSION_SIZE,
                                    SDO_TIMEOUT_DEFAULT,
                                    sm_mc_co_sdo_cb,
                                    _this);
}

static int32_t sm_mc_process(sm_ev_module_t* _this){
//    return sm_module_process(_this);
	(void)_this;
	return 0;
}

uint8_t tpdo3_data[8];
static int32_t sm_mc_module_handle_data(sm_ev_module_t* _this, int32_t _msg_id, const uint8_t* _data, int32_t _data_len){
    (void)_data_len;

    switch ((uint32_t)_msg_id & 0xFFFFFF80){
        case CO_CAN_ID_TPDO_1:
            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_speed_rpm, CO_getUint16(_data + 6))){
                _impl(_this)->m_mc_data.m_speed_rpm = CO_getUint16 (_data + 6);
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_SPEED_CHANGED,
//                                           _impl(_this)->m_mc_data.m_speed_kmh_mul_10);
            }

            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_reverse_state, (_data[0] & 0x0C) >> 2)){
                _impl(_this)->m_mc_data.m_reverse_state = (_data[0] & 0x0C) >> 2;
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_BACKWARD_MODE_CHANGED,
//                                           _impl(_this)->m_mc_data.m_reverse_state);
            }

            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_anti_theft_st, (_data[0] & 0x60) >> 5)){
                _impl(_this)->m_mc_data.m_anti_theft_st = (_data[0] & 0x60) >> 5;
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_ANTI_THEFT_STATUS_CHANGED,
//                                           _impl(_this)->m_mc_data.m_anti_theft_st);
            }

            _impl(_this)->m_mc_data.m_thr_cmd = _data[5];
            _impl(_this)->m_mc_data.m_est_tor = _data[4];
            _impl(_this)->m_mc_data.m_est_dc_cur = _data[3];
            _impl(_this)->m_mc_data.m_allow_dc_cur = _data[2];

            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_err_code, (uint16_t)_data[1])){
                _impl(_this)->m_mc_data.m_err_code = (uint16_t)_data[1];
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_ERROR_CHANGED,
//                                           _impl(_this)->m_mc_data.m_err_code);
            }
            sm_module_set_state_connect(_this, MODULE_STATE_CONNECTED);
            elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
            break;
        case CO_CAN_ID_TPDO_2:
            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_trip_m, CO_getUint32(_data + 4))){
                _impl(_this)->m_mc_data.m_trip_m =  CO_getUint32(_data + 4);
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_TRIP_CHANGED,
//                                           (int32_t)_impl(_this)->m_mc_data.m_trip_m);
            }

            _impl(_this)->m_mc_data.m_revol_cnt = CO_getUint32(_data + 4);
            _impl(_this)->m_mc_data.m_board_temp = _data[3];
            _impl(_this)->m_mc_data.m_motor_temp = _data[2];

            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_status, _data[1])){
                _impl(_this)->m_mc_data.m_status = _data[1];
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_MC_STATUS_CHANGED,
//                                           _impl(_this)->m_mc_data.m_status);
            }
            sm_module_set_state_connect(_this, MODULE_STATE_CONNECTED);
            elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
            break;
        case CO_CAN_ID_TPDO_3:
        	memcpy(tpdo3_data,_data,8);
            _impl(_this)->m_mc_data.m_aver_effic = _data[6];
            _impl(_this)->m_mc_data.m_enr_con_effic = _data[5];
            _impl(_this)->m_mc_data.m_enr_dur_trip = CO_getUint16(_data + 3);
            uint8_t cur_mode = (_data[7] >> 4);
            uint8_t purpose = _data[7] & 0x0F;
            _impl(_this)->m_mc_data.m_cur_mode = cur_mode;
            if (purpose == EV_MC_INSPECTION_MODE)
                purpose = PURPOSE_FOR_VEHICLE_INSPECTION;
            else
                purpose = PURPOSE_FOR_VEHICLE_MARKET;
                  if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_mc_purpose, purpose)){

                      _impl(_this)->m_mc_data.m_mc_purpose = purpose;
                  }
            if(SM_EV_DATA_IS_CHANGED(_impl(_this)->m_mc_data.m_cur_mode, cur_mode)){
                _impl(_this)->m_mc_data.m_cur_mode = cur_mode;
//                sm_ev_service_notify_ev_event(_impl(_this)->m_owner,
//                                           SM_EV_EVENT_DRIVE_MODE_CHANGED,
//                                           _impl(_this)->m_mc_data.m_cur_mode);
            }

            sm_module_set_state_connect(_this, MODULE_STATE_CONNECTED);
            elapsed_timer_resetz(&_this->m_connected_timeout, SM_MODULE_CONNECTED_TIMEOUT);
            break;
        case CO_CAN_ID_TPDO_4:

        default:
            break;
    }
    return 0;
}

sm_ev_module_t* sm_mc_create(void* _owner, sm_co_t* _co){
    if(!_owner || !_co){
        return NULL;
    }
    g_mc_module.m_base.m_id = MC_NODE_ID_DEFAULT;
    g_mc_module.m_base.m_connection_state = MODULE_STATE_DISCONNECTED;

    g_mc_module.m_co = _co;
    g_mc_module.m_owner = _owner;

    return &g_mc_module.m_base;
}

int32_t sm_mc_destroy(sm_ev_module_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_mc_set_drive_mode(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_MC_DRIVER_MODE_INDEX,
                                 SDO_MC_DRIVER_MODE_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_mc_set_max_speed(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_MC_LIMIT_SPEED_INDEX,
                                 SDO_MC_LIMIT_SPEED_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_mc_set_anti_theft_state(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_MC_ANTI_THEFT_ST_INDEX,
                                 SDO_MC_ANTI_THEFT_ST_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

int32_t sm_mc_set_stop_mode(sm_ev_module_t* _this, uint8_t _value, sm_co_sdo_cb_fn_t _cb, void* _arg){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd_temp[0] = _value;
    return sm_co_sdo_client_send(_impl(_this)->m_co,
                                 SDO_MC_STOP_MODE_INDEX,
                                 SDO_MC_STOP_MODE_SUB_INDEX,
                                 _this->m_id,
                                 &_impl(_this)->m_cmd_temp[0],
                                 1,
                                 SDO_TIMEOUT_DEFAULT,
                                 _cb,
                                 _arg);
}

void sm_mc_co_received_data(uint32_t _can_id, uint8_t* _data, void* _arg){
    (void)_arg;
    (void)_data;
    (void)_can_id;
    uint8_t node_id = (uint8_t) (_can_id & 0x7F);
    if(node_id != g_mc_module.m_base.m_id) return;
    sm_ev_module_t* _this = (sm_ev_module_t*) _arg;
    sm_mc_module_handle_data(_this, (int32_t)_can_id, _data, 8);
}
