//
// Created by vnbk on 19/09/2024.
//
#include "sm_ev_handle.h"
#include "sm_logger.h"
#include "sm_app_hmi.h"
#include "sm_sv_iot_define.h"
#include "sm_l76x_gps.h"

#define TAG "SM_EV_HANDLE"


static int32_t sm_convert_event_to_string(int32_t _event, int32_t _value, char* _buf){
    if(_event >= SM_EV_EVENT_NUMBER){
        LOG_ERR(TAG, "Event INVALID");
        return -1;
    }

    switch (_event) {
        case SM_EV_EVENT_KEY_CHANGED:
            sprintf(_buf, "\"%s\": %d", SM_IOT_KEY_FIELD, _value);
            break;
        case SM_EV_EVENT_PARKING_CHANGED:
            sprintf(_buf, "\"%s\": %d", SM_IOT_PARKING_FIELD, _value);
            break;
        case SM_EV_EVENT_DRIVE_MODE_CHANGED:{
            sprintf(_buf, "\"%s\": %d", SM_IOT_DRIVE_MODE_FIELD, _value);
            break;
        }
        case SM_EV_EVENT_LOCK_MODE_CHANGED:
            sprintf(_buf, "\"%s\": %d,\"%s\": %d",
                    SM_IOT_LOCK_MODE_FIELD,
                    _value == EV_LOCK_STATE ? 1 : 0,
                    SM_IOT_LOCK_STEP_FIELD,
                    _value == EV_PRE_LOCK_STATE ? 1 : 0);
            break;
        case SM_EV_EVENT_BLOCK_MODE_CHANGED:
            sprintf(_buf, "\"%s\": %d, \"%s\": %d",
                    SM_IOT_BLOCK_MODE_FIELD,
                    _value == EV_BLOCK_STATE ? 1 : 0,
                    SM_IOT_BLOCK_STEP_FIELD,
                    _value == EV_PRE_BLOCK_STATE ? 1 : 0);
            break;
        case SM_EV_EVENT_ERROR_CHANGED:
            sprintf(_buf, "\"%s\": %d", SM_IOT_ERROR_FIELD, _value);
            break;
        default:
            return 0;
    }
    return (int32_t)strlen(_buf);
};

static void sm_ev_on_drive_mode(int32_t _value, void* _arg);
static void sm_ev_on_phase_light(int32_t _value, void* _arg);
static void sm_ev_on_cos_light(int32_t _value, void* _arg);

static void sm_ev_on_turn_on_key(void *_arg)
{
    sm_hmi_app_t *app = (sm_hmi_app_t*) (_arg);
    sm_sv_hmi_t *hmi = app->m_hmi_service;

    sys_datetime_t datetime;
    sm_sync_time_get_time(app->m_sync_time, &datetime);
    sm_sv_hmi_set_time(app->m_hmi_service, datetime.hour, datetime.min);

    sm_sv_hmi_set_ble_icon(hmi, app->m_ble_service->m_proc->is_connected(app->m_ble_service) ? SM_DRV_LCD_SHOW : SM_DRV_LCD_HIDE);

    const sm_ev_data_t *ev_data = sm_sv_ev_get_data (app->m_ev_service);

    sm_sv_hmi_set_warning_icon (hmi, ev_data->m_err, ev_data->m_err == EV_ERR_NONE ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
    sm_sv_hmi_set_odo (hmi, ev_data->m_odo, SM_DRV_LCD_SHOW);
    sm_sv_hmi_set_trip (hmi, ev_data->m_trip < 0 ? 0 : ev_data->m_trip, SM_DRV_LCD_SHOW);
    sm_sv_hmi_set_parking (hmi, SM_DRV_LCD_SHOW);
    sm_sv_hmi_set_range (hmi, ev_data->m_pmu_data->m_range, SM_DRV_LCD_SHOW);

    sm_sv_hmi_set_speed(hmi, 0, SM_DRV_LCD_SHOW, POSITIVE_SPEED);

    sm_ev_on_drive_mode(ev_data->m_driver_mode, app);
    sm_ev_on_phase_light(ev_data->m_pmu_data->m_low_beam_state, app);
    sm_ev_on_cos_light(ev_data->m_pmu_data->m_high_beam_state, app);

    sm_sv_bp_t *bp_service = app->m_bp_manager;
    const sm_bp_data_t *bp_data = NULL;
    for (uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++){
        if (!sm_sv_bp_is_connected (bp_service, index)){
            continue;
        }
        bp_data = sm_sv_bp_get_data (bp_service, index);
        if (bp_data->m_state == BP_STATE_FAULT ||
            ev_data->m_pmu_data->m_bp_checking_state[index] == EV_BP_INVALID ||
            ev_data->m_pmu_data->m_port_lock_status[index] == EV_LOCK_PORT){

            sm_sv_hmi_blink_bp (hmi,
                                index,
                                bp_data->m_soc,
                                bp_data->m_temps[0],
                                SM_SV_HMI_BLINK_DURATION_DEFAULT);
        }else{
            sm_sv_hmi_set_bp (hmi, index, bp_data->m_soc, bp_data->m_temps[0], SM_DRV_LCD_SHOW);
        }
    }
}

static void sm_ev_on_key(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    if(_value == EV_KEY_OFF){
        sm_sv_hmi_power_off(hmi);
    }else{
        sm_sv_hmi_power_on(hmi);
        sm_sv_hmi_run_animation(hmi);
        sm_sv_hmi_init_default(hmi);

        sm_ev_on_turn_on_key(app);
    }
}
static void sm_ev_on_left_signal(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_left_signal(hmi, (_value == 0) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);

    if(app->m_ev_manu.m_registration == SM_MODULE_INFO_REGISTRATION){
        return;
    }
    sm_hmi_io_set_left_signal(&app->m_modules.m_hmi_io, _value);
}
static void sm_ev_on_right_signal(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_right_signal(hmi, (_value == 0) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);

    if(app->m_ev_manu.m_registration == SM_MODULE_INFO_REGISTRATION){
        return;
    }
    sm_hmi_io_set_right_signal(&app->m_modules.m_hmi_io, _value);
}
static void sm_ev_on_parking(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    sm_sv_hmi_set_parking(hmi, (_value == EV_EXIT_PARKING) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);

    const sm_ev_data_t *ev_data = sm_sv_ev_get_data (app->m_ev_service);
    if(_value == EV_EXIT_PARKING){
        sm_sv_hmi_set_mc_temp(hmi, ev_data->m_mc_data->m_motor_temp, SM_DRV_LCD_SHOW);

        sm_hmi_io_set_signal(&app->m_modules.m_hmi_io, SM_HAL_IO_ON);
        sm_sch_start_task(app->m_sch_task, 500, 1, (sm_sch_task_fn_t)sm_hmi_io_turn_off_signal, &app->m_modules.m_hmi_io);
    }else{
        sm_sv_hmi_set_mc_temp(hmi, ev_data->m_mc_data->m_motor_temp, SM_DRV_LCD_HIDE);
    }
}
/* static void sm_ev_on_drive_mode_signal(int32_t _value, void* _arg){
   LOG_WRN(TAG, "No handle drive mode signal");
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
   sm_sv_hmi_t* hmi = app->m_hmi_service;
    if(_value == EV_ECO_MODE_SIGNAL){
        sm_sv_hmi_set_sport_mode(hmi, SM_DRV_LCD_HIDE);
        sm_sv_hmi_set_eco_mode(hmi, SM_DRV_LCD_SHOW);
    }else{
        sm_sv_hmi_set_eco_mode(hmi, SM_DRV_LCD_HIDE);
        sm_sv_hmi_set_sport_mode(hmi, SM_DRV_LCD_SHOW);
    }
}*/
static void sm_ev_on_drive_mode(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    const sm_ev_data_t *ev_data = sm_sv_ev_get_data (app->m_ev_service);

    if(_value == EV_ECO_MODE_1){
        sm_sv_hmi_set_sport_mode(hmi, SM_DRV_LCD_HIDE);
        sm_sv_hmi_set_eco_mode(hmi, SM_DRV_LCD_SHOW);
    }else if(_value == EV_SPORT_MODE_1){
        sm_sv_hmi_set_eco_mode(hmi, SM_DRV_LCD_HIDE);
        sm_sv_hmi_set_sport_mode(hmi, SM_DRV_LCD_SHOW);
    }else if(_value == EV_MC_HAFT_UP_HILL_MODE && ev_data->m_mc_data->m_reverse_state == EV_REVERSE_STATE_INACTIVE){
        sm_sv_hmi_set_eco_mode(hmi, SM_DRV_LCD_HIDE);
        sm_sv_hmi_blink_sport_mode(hmi, SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }
}
static void sm_ev_on_phase_light(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_phase_light(hmi, (_value == 0) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}
static void sm_ev_on_cos_light(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_cos_light(hmi, (_value == 0) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}
static void sm_ev_on_range(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_range(hmi, _value, SM_DRV_LCD_SHOW);
}

static void sm_ev_on_backward_mode(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    if(_value == EV_REVERSE_STATE_ACTIVE_REVERSE){
        sm_sv_hmi_blink_speed(hmi, 0, SM_SV_HMI_BLINK_SPEED_DEFAULT, NEGATIVE_SPEED);
    }else if (_value == EV_REVERSE_STATE_ACTIVE_FORWARD){
        sm_sv_hmi_blink_speed(hmi, 0, SM_SV_HMI_BLINK_SPEED_DEFAULT, POSITIVE_SPEED);
    }else{
        sm_sv_hmi_set_speed(hmi, 0, SM_DRV_LCD_SHOW, POSITIVE_SPEED);
    }
}

//static void sm_ev_on_backward_mode_signal(int32_t _value, void* _arg){
//    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
//    sm_sv_hmi_t* hmi = app->m_hmi_service;
//}


static void sm_ev_on_lock_mode(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    if(_value == EV_PRE_LOCK_STATE){
        sm_sv_hmi_set_warning_icon(hmi, EV_ERR_BLOCK_STATE, SM_DRV_LCD_SHOW);
        sm_sv_hmi_blink_warning_icon(hmi, SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else if(_value == EV_LOCK_STATE){
        sm_sv_hmi_set_speed(hmi, 0, SM_DRV_LCD_SHOW, POSITIVE_SPEED);
    }
}
static void sm_ev_on_block_mode(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    if(_value == EV_PRE_BLOCK_STATE){
        sm_sv_hmi_set_warning_icon(hmi, EV_ERR_BLOCK_STATE, SM_DRV_LCD_SHOW);
        sm_sv_hmi_blink_warning_icon(hmi, SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else if(_value == EV_BLOCK_STATE){
        sm_sv_hmi_set_speed(hmi, 0, SM_DRV_LCD_SHOW, POSITIVE_SPEED);
    }
}

/*static void sm_ev_on_anti_theft_mode(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
//    sm_sv_hmi_set_warning_icon(hmi, _value, (_value != EV_LOCK_STATE) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}
static void sm_ev_on_inactive_mode(int32_t _value, void* _arg){
//    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
//    sm_sv_hmi_t* hmi = app->m_hmi_service;
//    sm_sv_hmi_set_warning_icon(hmi, _value, (_value == 0) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}*/

static void sm_ev_on_odo(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_odo(hmi, _value, SM_DRV_LCD_SHOW);
}
static void sm_ev_on_speed(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    const sm_ev_data_t *ev_data = sm_sv_ev_get_data (app->m_ev_service);
    if(ev_data->m_mc_data->m_reverse_state == EV_REVERSE_STATE_ACTIVE_FORWARD){
        sm_sv_hmi_blink_speed(hmi, _value, SM_SV_HMI_BLINK_SPEED_DEFAULT, POSITIVE_SPEED);
    }else if (ev_data->m_mc_data->m_reverse_state == EV_REVERSE_STATE_ACTIVE_REVERSE){
        sm_sv_hmi_blink_speed(hmi, _value, SM_SV_HMI_BLINK_SPEED_DEFAULT, NEGATIVE_SPEED);
    }else{
        sm_sv_hmi_set_speed(hmi, _value, SM_DRV_LCD_SHOW, POSITIVE_SPEED);
    }
}
static void sm_ev_on_trip(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;
    sm_sv_hmi_set_trip(hmi, _value, SM_DRV_LCD_SHOW);
}
static void sm_ev_on_error(int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    if(_value == EV_ERR_MC_OVERHEAT_LV1){
        _value = EV_ERR_NONE;
    }

    sm_sv_hmi_set_warning_icon(hmi, _value, (_value == EV_ERR_NONE) ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}

typedef void (*sm_ev_event_handle_fn_t)(int32_t, void*);
typedef void (*sm_bp_event_handle_fn_t)(int32_t, int32_t, void*);
static sm_ev_event_handle_fn_t g_ev_event_handle[SM_EV_EVENT_NUMBER] = {
        sm_ev_on_key,
        sm_ev_on_left_signal,
        sm_ev_on_right_signal,
        sm_ev_on_parking,
        NULL, //sm_ev_on_drive_mode_signal,
        sm_ev_on_drive_mode,
        sm_ev_on_cos_light,
        sm_ev_on_phase_light,
        sm_ev_on_range,
        NULL, //sm_ev_on_backward_mode_signal,
        sm_ev_on_backward_mode, //sm_ev_on_backward_mode,
        NULL,
        sm_ev_on_lock_mode,
        sm_ev_on_block_mode,
        NULL, //sm_ev_on_anti_theft_mode,
        NULL, //sm_ev_on_inactive_mode,
        sm_ev_on_odo,
        sm_ev_on_speed,
        NULL,
        sm_ev_on_trip,
        sm_ev_on_error
};

static void sm_ev_on_lock_port(int32_t _port, int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(app->m_ev_service, _port);
    if(!bp_data){
        return;
    }

    if(_value){
        sm_sv_hmi_blink_bp(hmi,
                           _port,
                           bp_data->m_soc,
                           bp_data->m_temps[0],
                           SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else{
    	sm_sv_hmi_set_bp(hmi, _port, bp_data->m_soc, bp_data->m_temps[0], SM_DRV_LCD_SHOW);
    }
}

static void sm_ev_on_bp_validated(int32_t _port, int32_t _value, void* _arg) {
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(app->m_ev_service, _port);
    if(!bp_data){
        return;
    }

    if(_value == EV_BP_INVALID){
        sm_sv_hmi_blink_bp(hmi,
                           _port,
                           bp_data->m_soc,
                           bp_data->m_temps[0],
                           SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else if(_value == EV_BP_VALID){
        sm_sv_hmi_set_bp(hmi, _port, bp_data->m_soc, bp_data->m_temps[0], SM_DRV_LCD_SHOW);
    }
}

static void sm_ev_on_bp_connected(int32_t _port, int32_t _value, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(app->m_ev_service, _port);
    if(!bp_data){
        return;
    }

    sm_sv_hmi_set_bp(hmi, _port, _value, bp_data->m_temps[0], SM_DRV_LCD_SHOW);
    sm_auth_bp_connection_update(app->m_auth_bp, _port, MODULE_STATE_CONNECTED);
}

static void sm_ev_on_bp_disconnected(int32_t _port, int32_t _value, void* _arg) {
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    sm_sv_hmi_set_bp(hmi, _port, _value, 0, SM_DRV_LCD_HIDE);

    sm_auth_bp_connection_update(app->m_auth_bp, _port, MODULE_STATE_DISCONNECTED);
}

static void sm_ev_on_bp_state_changed(int32_t _port, int32_t _value, void* _arg) {
    sm_hmi_app_t* app = (sm_hmi_app_t*)(_arg);
    sm_sv_hmi_t* hmi = app->m_hmi_service;

    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(app->m_ev_service, _port);
    if(!bp_data){
        return;
    }

    if(_value == EV_BP_INVALID){
        sm_sv_hmi_blink_bp(hmi,
                           _port,
                           bp_data->m_soc,
                           bp_data->m_temps[0],
                           SM_SV_HMI_BLINK_DURATION_DEFAULT);
    }else{
        sm_sv_hmi_set_bp(hmi, _port, bp_data->m_soc, bp_data->m_temps[0], SM_DRV_LCD_HIDE);
    }
}

static sm_bp_event_handle_fn_t g_bp_event_handle[SM_EV_EVENT_NUMBER] = {
        sm_ev_on_lock_port,
        sm_ev_on_bp_validated,
        sm_ev_on_bp_connected,
        sm_ev_on_bp_disconnected,
        NULL, //sm_ev_on_bp_state_changed,
};

void sm_ev_on_event(int32_t _event, int32_t _value, void* _arg){
    sm_hmi_app_t* hmi_app = (sm_hmi_app_t*)_arg;
    if(!hmi_app){
        LOG_ERR(TAG, "Missing parameter");
        return;
    }

    sm_host_api_t* iot_service = (sm_host_api_t*)hmi_app->m_iot_service;
    if(iot_service && iot_service->m_proc->is_connected(iot_service)){
        char buf[32];
        int32_t len = sm_convert_event_to_string(_event, _value, buf);
        if(len > 0){
            iot_service->m_proc->update_datas_to_host(iot_service, buf);
        }
    }
    /// TODO: For BLE interface

    //Handle event
    if(g_ev_event_handle[_event]){
        g_ev_event_handle[_event](_value, _arg);
    }
}

void sm_ev_on_bp_event(int32_t _port, int32_t _event, int32_t _value, void* _arg){
    sm_hmi_app_t* hmi_app = (sm_hmi_app_t*)_arg;
    if(!hmi_app){
        LOG_ERR(TAG, "Missing parameter");
        return;
    }
    sm_host_api_t* iot_service = (sm_host_api_t*)hmi_app->m_iot_service;
    if(iot_service && iot_service->m_proc->is_connected(iot_service) && _event == SM_BP_EVENT_CONNECTED){
        const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(hmi_app->m_ev_service, _port);
        if(bp_data && bp_data->m_sn[0] != '\0'){
            iot_service->m_proc->push_event_to_host(iot_service, (void*)bp_data->m_sn);
        }
    }
    /// TODO: For BLE interface

    if(g_bp_event_handle[_event]){
        g_bp_event_handle[_event](_port, _value, _arg);
    }
}

/************************************** GET ev ev_data **********************************/
void* sm_ev_get_manu(void*_arg) {
    sm_hmi_app_t *app = (sm_hmi_app_t *) _arg;
    return &app->m_ev_manu;
}

void* sm_ev_get_module_info(int32_t _type, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ev_t* ev_service = (sm_sv_ev_t*)app->m_ev_service;
    if(!ev_service){
        return NULL;
    }
    return (void*)sm_sv_ev_get_module_version(ev_service, _type);
}

void* sm_ev_get_data(void*_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ev_t* ev_service = (sm_sv_ev_t*)app->m_ev_service;
    if(!ev_service){
        return NULL;
    }
    return (void*)sm_sv_ev_get_data(ev_service);
}

void* sm_ev_get_ev_config(void*_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    return &app->m_config.m_ev_config;
}

void* sm_ev_get_bp_data(int32_t _id, void*_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ev_t* ev_service = (sm_sv_ev_t*)app->m_ev_service;
    if(!ev_service){
        return NULL;
    }
    return (void*)sm_sv_ev_get_bp_data(ev_service, _id);
}

int32_t sm_ev_get_gps_data(void* _gps_coordinate, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    if(!app->m_modules.m_gps){
        return -1;
    }
    if(!app->m_modules.m_gps->proc->data_is_valid(app->m_modules.m_gps)){
        return -1;
    }
    app->m_modules.m_gps->proc->get_coordinate(app->m_modules.m_gps, _gps_coordinate);
    return 0;
}

int32_t sm_ev_get_network_data(int32_t* _rssi, char* _simNb, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;

    if(app->m_driver.m_ec200_driver->rssi >= 90){
        return -1;
    }
    *_rssi = app->m_driver.m_ec200_driver->rssi;
    memcpy(_simNb, &app->m_driver.m_ec200_driver->IMSI, sizeof(app->m_driver.m_ec200_driver->IMSI));

    return 0;
}

int32_t sm_ev_get_optional_data(char* _data, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;

    int len = sprintf(_data, "\"%s\": %d", SM_IOT_INPUT_VOLTAGE_FIELD,  sm_hmi_io_get_input_vol(&app->m_modules.m_hmi_io));

    uint8_t ble_connected = app->m_ble_service->m_proc->is_connected(app->m_ble_service);
    len += sprintf(_data + len, ",\"ble\":{\"connected\": %d}", ble_connected);

    return len;
}
