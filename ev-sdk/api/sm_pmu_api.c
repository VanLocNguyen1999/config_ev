//
// Created by vnbk on 31/05/2024.
//
#include "porting/sm_bsp_pmu.h"
#include "sm_pmu_api.h"
#include "sm_co_od_common.h"
#include "sm_logger.h"
#include "sm_sv_io.h"
#include "sm_sv_pms.h"

#define SM_PMU_BP_NUM_MAX                    3
#define SM_DUTY_CYCLE_IN_BLYNK_SIGNAL        50   // %
#define SM_PMU_CANOPEN_PROCESS_TIME_INTERVAL 1    //ms
static const char*   TAG = "PMU_API";

typedef struct {
     sm_pmu_event_t      *m_event_handle;
     void                *m_event_arg;

     sm_sv_io_t          *m_io_service;
     uint8_t             m_io_status;

     sm_sv_bp_t          *m_bp_service;
     uint8_t             m_bpm_status;

     sm_sv_charger_t     *m_charger_service;
     uint8_t             m_charger_status;

     sm_sv_pms_t         *m_pms_service;
     uint8_t             m_pms_status;

     sm_co_if_t          *m_co_if;
     sm_co_t             *m_canopen;
     uint8_t             m_co_status;

     elapsed_timer_t     m_co_interval;
} sm_pmu_t;

sm_pmu_t g_pmu;
/* IO SV */
sm_sv_io_if_t g_io_if = {
     .in_key_fn_t            = sm_bsp_pmu_io_get_key,
     .in_left_light_fn_t     = sm_bsp_pmu_io_get_left_light,
     .in_right_light_fn_t    = sm_bsp_pmu_io_get_right_light,
     .in_code_light_fn_t     = sm_bsp_pmu_io_get_low_beam,
     .in_phare_light_fn_t    = sm_bsp_pmu_io_get_high_beam,
     .in_driving_mode_fn_t   = sm_bsp_pmu_io_get_driving_mode,
     .in_parking_fn_t        = sm_bsp_pmu_io_get_parking,
     .in_left_brake_fn_t     = sm_bsp_pmu_io_get_left_brake,
     .in_right_brake_fn_t    = sm_bsp_pmu_io_get_right_brake,
     .in_reverse_fn_t        = sm_bsp_pmu_io_get_reverse_mode,
     .in_horn_fn_t           = sm_bsp_pmu_io_get_horn,
     .out_left_light_fn_t    = (void*)sm_bsp_pmu_io_set_left_light,
     .out_right_light_fn_t   = (void*)sm_bsp_pmu_io_set_right_light,
     .out_code_light_fn_t    = (void*)sm_bsp_pmu_io_set_low_beam,
     .out_phare_light_fn_t   = (void*)sm_bsp_pmu_io_get_high_beam,
     .out_horn_fn_t          = (void*)sm_bsp_pmu_io_set_horn,
     .out_tail_light_fn_t    = (void*)sm_bsp_pmu_io_set_tail_light
};

/* BP SV */
typedef void (*node_id_ctrl)(uint8_t);
static node_id_ctrl sm_bsp_slaves_node_id_ctrl[SM_PMU_BP_NUM_MAX] = {
     sm_bsp_pmu_io_set_node_id1,
     sm_bsp_pmu_io_set_node_id2,
     sm_bsp_pmu_io_set_node_id3
};

static int32_t sm_slave_node_id_select(int32_t _id){
     if (_id < SM_PMU_BP_NUM_MAX)
          sm_bsp_slaves_node_id_ctrl[_id](1);
     return 0;
}

static int32_t sm_slave_node_id_deselect(int32_t _id){
     if (_id < SM_PMU_BP_NUM_MAX)
          sm_bsp_slaves_node_id_ctrl[_id](0);
     return 0;
}

static sm_bp_node_id_controller_t g_nodeid_controller = {
     .sm_bp_node_id_select  		= sm_slave_node_id_select,
     .sm_bp_node_id_deselect 	     = sm_slave_node_id_deselect,
 };

/* CHARGER SV*/
sm_sv_charger_prof_t g_charger_prof = {
     .m_max_cur = 0,
     .m_max_volt = 0,
     .m_max_temp = 0,
     .m_min_volt = 0,
     .m_power_stable_time = 0,
};

sm_sv_charger_if_t g_charger_if = {
     .get_charger_vol_fn_t = sm_bsp_pmu_adc_get_charger_vol,
};
/* IO EVENT CALLBACK */
void pmu_io_event_cb(uint8_t _event, void* _arg) {
     if (!_arg) return;
     sm_pmu_t *p_pmu = (sm_pmu_t*) _arg;
     if (!p_pmu->m_event_handle) return;

     switch (_event) {
          case SM_SV_IO_EVENT_IN_KEY_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_key_changed != NULL)
                    p_pmu->m_event_handle->on_io_key_changed(SM_PMU_IO_ON, p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_KEY_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_key_changed != NULL)
                    p_pmu->m_event_handle->on_io_key_changed(SM_PMU_IO_OFF, p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_REVERSE_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_emergency_status_changed)
                    p_pmu->m_event_handle->on_io_emergency_status_changed(SM_PMU_IO_ON, p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_REVERSE_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_emergency_status_changed)
                    p_pmu->m_event_handle->on_io_emergency_status_changed(SM_PMU_IO_OFF, p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_LEFT_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_left_signal_status_changed)
                    p_pmu->m_event_handle->on_io_left_signal_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_LEFT_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_left_signal_status_changed)
                    p_pmu->m_event_handle->on_io_left_signal_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_LEFT_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_left_signal_status_changed)
                    p_pmu->m_event_handle->on_io_left_signal_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_left_signal_status_changed)
                    p_pmu->m_event_handle->on_io_left_signal_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_LEFT_CHANGED_BLINK:
               if (p_pmu->m_event_handle->on_io_left_signal_status_changed)
                    p_pmu->m_event_handle->on_io_left_signal_status_changed(SM_PMU_IO_BLINK,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_RIGHT_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_right_signal_status_changed)
                    p_pmu->m_event_handle->on_io_right_signal_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_RIGHT_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_right_signal_status_changed)
                    p_pmu->m_event_handle->on_io_right_signal_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_right_signal_status_changed)
                    p_pmu->m_event_handle->on_io_right_signal_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_right_signal_status_changed)
                    p_pmu->m_event_handle->on_io_right_signal_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_BLINK:
               if (p_pmu->m_event_handle->on_io_right_signal_status_changed)
                    p_pmu->m_event_handle->on_io_right_signal_status_changed(SM_PMU_IO_BLINK,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_PHARE_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_high_beam_status_changed)
                    p_pmu->m_event_handle->on_io_high_beam_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_PHARE_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_high_beam_status_changed)
                    p_pmu->m_event_handle->on_io_high_beam_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_PHARE_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_high_beam_status_changed)
                    p_pmu->m_event_handle->on_io_high_beam_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_PHARE_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_high_beam_status_changed)
                    p_pmu->m_event_handle->on_io_high_beam_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_PHARE_CHANGED_BLINK:
               if (p_pmu->m_event_handle->on_io_high_beam_status_changed)
                    p_pmu->m_event_handle->on_io_high_beam_status_changed(SM_PMU_IO_BLINK,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_HORN_CHANGED_ON:
          case SM_SV_IO_EVENT_IN_HORN_CHANGED_OFF:
          case SM_SV_IO_EVENT_OUT_HORN_CHANGED_ON:
          case SM_SV_IO_EVENT_OUT_HORN_CHANGED_OFF:
          case SM_SV_IO_EVENT_OUT_HORN_CHANGED_BLINK:
               break;
          case SM_SV_IO_EVENT_IN_CODE_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_low_beam_status_changed)
                    p_pmu->m_event_handle->on_io_low_beam_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_CODE_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_low_beam_status_changed)
                    p_pmu->m_event_handle->on_io_low_beam_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_CODE_CHANGED_ON:
               if (p_pmu->m_event_handle->on_io_low_beam_status_changed)
                    p_pmu->m_event_handle->on_io_low_beam_status_changed(SM_PMU_IO_ON,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_CODE_CHANGED_OFF:
               if (p_pmu->m_event_handle->on_io_low_beam_status_changed)
                    p_pmu->m_event_handle->on_io_low_beam_status_changed(SM_PMU_IO_OFF,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_OUT_CODE_CHANGED_BLINK:
               if (p_pmu->m_event_handle->on_io_low_beam_status_changed)
                    p_pmu->m_event_handle->on_io_low_beam_status_changed(SM_PMU_IO_BLINK,p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_BRAKE_CHANGED_ON:
          case SM_SV_IO_EVENT_IN_BRAKE_CHANGED_OFF:
          case SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGED_ON:
          case SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGED_OFF:
               break;
          case SM_SV_IO_EVENT_IN_PARKING_PRESSED:
               if (p_pmu->m_event_handle->on_io_parking_bt_press)
                    p_pmu->m_event_handle->on_io_parking_bt_press(p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_PARKING_HELD:
               if (p_pmu->m_event_handle->on_io_parking_bt_hold)
                    p_pmu->m_event_handle->on_io_parking_bt_hold(p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_DRIV_MODE_PRESSED:
               if (p_pmu->m_event_handle->on_io_mode_bt_press)
                    p_pmu->m_event_handle->on_io_mode_bt_press(p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_DRIV_MODE_HELD:
               if (p_pmu->m_event_handle->on_io_mode_bt_hold)
                    p_pmu->m_event_handle->on_io_mode_bt_hold(p_pmu->m_event_arg);
               break;
          case SM_SV_IO_EVENT_IN_TOGGLED_REVERSE_MODE:
               break;
          default:
               break;
     }
}

/* BP EVENT CALLBACK */
sm_sv_bp_event_cb_t pmu_bpm_event_cb;
/* PMS EVENT CALLBACK */
sm_sv_pms_event_cb_fn_t pmu_pms_event_cb;
/* CHARGER EVENT CALLBACK */
sm_sv_charger_event_cb_fn_t pmu_charger_event_cb;

int32_t sm_pmu_api_init(sm_pmu_event_t* _event, void* _arg) {
     g_pmu.m_event_handle     = _event;
     g_pmu.m_event_arg        = _arg;
     pmu_bpm_event_cb.on_bp_connected = g_pmu.m_event_handle->on_bp_connected;
     pmu_bpm_event_cb.on_bp_disconnected = g_pmu.m_event_handle->on_bp_disconnected;
     pmu_bpm_event_cb.on_bp_update_data = g_pmu.m_event_handle->on_bp_update_data;

     pmu_pms_event_cb.switch_merge_cb_fn = g_pmu.m_event_handle->on_pms_switch_merge;
     pmu_pms_event_cb.update_est_data_cb_fn = g_pmu.m_event_handle->on_pms_update_est_data;
     pmu_pms_event_cb.on_paused = g_pmu.m_event_handle->on_paused_pms;
     pmu_pms_event_cb.on_resumed = g_pmu.m_event_handle->on_resumed_pms;

     pmu_charger_event_cb.on_plugged_in = g_pmu.m_event_handle->on_charger_plugged;
     pmu_charger_event_cb.on_charged = g_pmu.m_event_handle->on_charged;
     pmu_charger_event_cb.on_stop_charging = g_pmu.m_event_handle->on_stoped_charging;
     pmu_charger_event_cb.on_paused = g_pmu.m_event_handle->on_paused_charging;
     pmu_charger_event_cb.on_resumed = g_pmu.m_event_handle->on_resumed_charging;

     g_pmu.m_io_service       = sm_sv_io_create(&g_io_if);
     g_pmu.m_co_if            = sm_co_if_create(0,NULL,0,NULL);
     g_pmu.m_canopen          = sm_co_create(NODE_ID_DEFAULT, 1, g_pmu.m_co_if);
     g_pmu.m_bp_service       = sm_sv_bp_create(SM_PMU_BP_NUM_MAX, g_pmu.m_canopen, true, &g_nodeid_controller);
     g_pmu.m_pms_service      = sm_sv_pms_create(g_pmu.m_bp_service);
     g_pmu.m_charger_service  = sm_sv_charger_create(&g_charger_prof, &g_charger_if, g_pmu.m_bp_service);

     sm_sv_io_reg_event(g_pmu.m_io_service, pmu_io_event_cb, &g_pmu);
     sm_sv_bp_reg_event(g_pmu.m_bp_service, &pmu_bpm_event_cb, &g_pmu.m_event_arg);
     sm_sv_pms_reg_event(g_pmu.m_pms_service, &pmu_pms_event_cb, &g_pmu.m_event_arg);
     sm_sv_charger_reg_event(g_pmu.m_charger_service, &pmu_charger_event_cb,&g_pmu.m_event_arg);

     elapsed_timer_resetz(&g_pmu.m_co_interval, SM_PMU_CANOPEN_PROCESS_TIME_INTERVAL);
     LOG_DBG(TAG, "PMU API INITIATED");
     return 0;
}

int32_t sm_pmu_api_free() {
     sm_sv_io_destroy(g_pmu.m_io_service);
     sm_sv_bp_destroy(g_pmu.m_bp_service);
     sm_sv_pms_destroy(g_pmu.m_pms_service);
     sm_sv_charger_destroy(g_pmu.m_charger_service);

     g_pmu.m_event_handle     = NULL;
     g_pmu.m_io_service       = NULL;
     g_pmu.m_co_if            = NULL;
     g_pmu.m_canopen          = NULL;
     g_pmu.m_bp_service       = NULL;
     g_pmu.m_pms_service      = NULL;
     g_pmu.m_charger_service  = NULL;
     return 0;
}

int32_t sm_pmu_api_process() {
     sm_sv_io_process(g_pmu.m_io_service);
     sm_sv_bp_process(g_pmu.m_bp_service);
     sm_sv_pms_process(g_pmu.m_pms_service);
     sm_sv_charger_process(g_pmu.m_charger_service);
     if (!elapsed_timer_get_remain(&g_pmu.m_co_interval)) {
          sm_co_process(g_pmu.m_canopen, g_pmu.m_co_interval.m_duration);
          elapsed_timer_reset(&g_pmu.m_co_interval);
     }
     return 0;
}

int32_t sm_pmu_api_pause_pms_sv() {
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     if (sm_sv_pms_pause(p_sv_pms)) {
          g_pmu.m_pms_status = SM_PMU_SV_STATUS_PAUSED;
          return 0;
     }
     return -1;
}
int32_t sm_pmu_api_resume_pms_sv() {
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     if (sm_sv_pms_resume(p_sv_pms)) {
          g_pmu.m_pms_status = SM_PMU_SV_STATUS_PROCESSING;
          return 0;
     }
     return -1;
}

int32_t sm_pmu_api_pause_charge_sv() {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_charger_pause(p_sv_charger) != -1) {
          g_pmu.m_charger_status = SM_PMU_SV_STATUS_PAUSED;
          return 0;
     }
     return -1;
}
int32_t sm_pmu_api_resume_charge_sv() {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_charger_resume(p_sv_charger) != -1) {
          g_pmu.m_charger_status = SM_PMU_SV_STATUS_PROCESSING;
          return 0;
     }
     return -1;
}

/* CHARGER */
int32_t sm_pmu_api_charger_get_bp_number() {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     return sm_sv_charger_get_bp_num(p_sv_charger);
}
int32_t sm_pmu_api_charger_get_vol() {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     return sm_sv_charger_get_volt(p_sv_charger);
}
int32_t sm_pmu_api_charger_get_cur() {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     return sm_sv_charger_get_cur(p_sv_charger);
}
int32_t sm_pmu_api_charger_charge(int32_t _bp_id) {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     return sm_sv_charger_force_bp(p_sv_charger, _bp_id);
}
int32_t sm_pmu_api_charger_stop_charge(int32_t _bp_id) {
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     return sm_sv_charger_disable_bp(p_sv_charger, _bp_id);
}

/* IO */
int32_t sm_pmu_api_io_get_key_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     return (int32_t)sm_sv_io_get_input_key(p_sv_io);
}
int32_t sm_pmu_api_io_get_left_signal_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
    return sm_sv_io_get_input_left_light(p_sv_io);
}
int32_t sm_pmu_api_io_get_right_signal_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
    return sm_sv_io_get_input_left_light(p_sv_io);
}
int32_t sm_pmu_api_io_get_high_beam_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
    return sm_sv_io_get_input_phare_light(p_sv_io);
}
int32_t sm_pmu_api_io_get_low_beam_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
    return sm_sv_io_get_input_code_light(p_sv_io);
}

int32_t sm_pmu_api_io_get_emergency_status() {
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
    return sm_sv_io_get_input_reverse(p_sv_io);
}

int32_t sm_pmu_api_io_get_parking_button_status(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     return sm_sv_io_get_input_parking(p_sv_io);
}
int32_t sm_pmu_api_io_get_mode_button_status(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     return sm_sv_io_get_input_driving_mode(p_sv_io);
}
int32_t sm_pmu_api_io_get_horn_status(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     return sm_sv_io_get_input_horn(p_sv_io);
}

int32_t sm_pmu_api_io_set_left_signal(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_left_light(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_left_signal(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_left_light(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_left_signal(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_left_light_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

int32_t sm_pmu_api_io_set_right_signal(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_right_light(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_right_signal(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_right_light(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_right_signal(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_right_light_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

int32_t sm_pmu_api_io_set_high_beam(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_phare_light(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_high_beam(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_phare_light(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_high_beam(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_phare_light_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

int32_t sm_pmu_api_io_set_low_beam(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_code_light(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_low_beam(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_code_light(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_low_beam(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_code_light_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

int32_t sm_pmu_api_io_set_tail_light(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_tail_light(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_tail_light(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_tail_light(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_tail_light(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_tail_light_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

int32_t sm_pmu_api_io_set_horn(int32_t _value){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_output_horn(p_sv_io, _value);
     return 0;
}
int32_t sm_pmu_api_io_reset_horn(){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_release_output_horn(p_sv_io);
     return 0;
}
int32_t sm_pmu_api_io_blink_horn(int32_t _repeat, int32_t _period){
     sm_sv_io_t *p_sv_io = g_pmu.m_io_service;
     sm_sv_io_set_horn_blink(p_sv_io, SM_DUTY_CYCLE_IN_BLYNK_SIGNAL * _period/100, _period, _repeat);
     return 0;
}

/* BATTERY PACKS */
int32_t sm_pmu_api_bp_auth(int32_t _bp_id) {
     sm_sv_bp_t* p_sv_bpm = g_pmu.m_bp_service;
     return sm_sv_bp_auth(p_sv_bpm, _bp_id, g_pmu.m_event_handle->on_bp_auth, &g_pmu);
}
int32_t sm_pmu_api_first_bp_auth(int32_t _bp_id) {
     sm_sv_bp_t* p_sv_bpm = g_pmu.m_bp_service;
     return sm_sv_first_bp_auth(p_sv_bpm, _bp_id, g_pmu.m_event_handle->on_bp_auth, &g_pmu);
}
int32_t sm_pmu_api_bp_get_number(){
     sm_sv_bp_t* p_sv_bpm = g_pmu.m_bp_service;
     return sm_sv_bp_get_number(p_sv_bpm);
}
const sm_bp_data_t* sm_pmu_api_bp_get_data(int32_t _index){
     sm_sv_bp_t* p_sv_bpm = g_pmu.m_bp_service;
     return sm_sv_bp_get_data(p_sv_bpm, _index);
}
int32_t sm_pmu_api_bp_is_connected(int32_t _index){
     sm_sv_bp_t* p_sv_bpm = g_pmu.m_bp_service;
     return sm_sv_bp_is_connected(p_sv_bpm, _index);
}
int32_t sm_pmu_api_disable_bp(int32_t _bp_id){
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_pms_disable_bp(p_sv_pms, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_pms_disable_bp %d", _bp_id);
          return -1;
     }
     if (sm_sv_charger_disable_bp(p_sv_charger, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_charger_disable_bp %d", _bp_id);
          return -1;
     }
     return 0;
}
int32_t sm_pmu_api_enable_bp(int32_t _bp_id){
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_pms_enable_bp(p_sv_pms, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_pms_enable_bp %d", _bp_id);
          return -1;
     }
     if (sm_sv_charger_enable_bp(p_sv_charger, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_charger_enable_bp %d", _bp_id);
          return -1;
     }
     return 0;
}

int32_t sm_pmu_api_force_discharging_bp(int32_t _bp_id) {
     if (g_pmu.m_pms_status == SM_PMU_SV_STATUS_PAUSED) return -1;
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     if (sm_sv_pms_force_bp(p_sv_pms, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_pms_force_bp %d", _bp_id);
          return -1;
     }
     return 0;
}
int32_t sm_pmu_api_release_discharging() {
     if (g_pmu.m_pms_status == SM_PMU_SV_STATUS_PAUSED) return -1;
     sm_sv_pms_t *p_sv_pms = g_pmu.m_pms_service;
     if (sm_sv_pms_release_bp(p_sv_pms)<0) {
          LOG_ERR(TAG, "sm_sv_pms_release_bp");
          return -1;
     }
     return 0;
}

int32_t sm_pmu_api_force_charging_bp(int32_t _bp_id) {
     if (g_pmu.m_charger_status == SM_PMU_SV_STATUS_PAUSED) return -1;
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_charger_force_bp(p_sv_charger, _bp_id)<0) {
          LOG_ERR(TAG, "sm_sv_charger_force_bp %d", _bp_id);
          return -1;
     }
     return 0;
}
int32_t sm_pmu_api_release_charging() {
     if (g_pmu.m_charger_status == SM_PMU_SV_STATUS_PAUSED) return -1;
     sm_sv_charger_t *p_sv_charger = g_pmu.m_charger_service;
     if (sm_sv_charger_release_bp(p_sv_charger)<0) {
          LOG_ERR(TAG, "sm_sv_charger_release_bp");
          return -1;
     }
     return 0;
}

/* MOTOR CONTROLLER */
int32_t sm_pmu_api_mc_set_power(int32_t _is_enable) {
     sm_bsp_pmu_io_set_mc_power_supply(_is_enable);
     return 0;
}

/* HUMAN MACHINE INTERFACES */
int32_t sm_pmu_api_hmi_set_power(int32_t _is_enable){
     sm_bsp_pmu_io_set_hmi_power_supply(_is_enable);
     return 0;
}

/* POWER */
int32_t sm_pmu_api_pw_get_12v_pw_supply_state(){
     return sm_bsp_pmu_io_get_12v_pw_status();
}

/* SYSTEM */
int32_t sm_pmu_api_sys_reboot(){
     return sm_pmu_bsp_reboot();
}
/*-------------------------------------------------------*/

