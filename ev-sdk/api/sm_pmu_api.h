//
// Created by vnbk on 31/05/2024.
//

#ifndef EV_SDK_SM_PMU_API_H
#define EV_SDK_SM_PMU_API_H

#include <stdint.h>
#include "sm_bp_data.h"
#include "sm_sv_pms.h"

typedef enum {
    SM_PMU_IO_KEY_CHANGED_EVENT,
    SM_PMU_BP_CONNECTED_EVENT,
    SM_PMU_BP_DISCONNECTED_EVENT,
    SM_PMU_EVENT_NUMBER
}SM_PMU_EVENT_T;

typedef enum {
    SM_PMU_IO_OFF = 0,
    SM_PMU_IO_ON,
    SM_PMU_IO_BLINK,
} SM_PMU_IO_ST;

typedef enum {
    SM_PMU_SV_STATUS_PROCESSING,
    SM_PMU_SV_STATUS_PAUSED,
}SM_PMU_SV_STATUS;

typedef struct {
    // IO
    void (*on_io_key_changed)(int32_t _value, void*);
    void (*on_io_parking_bt_press)(void*);
    void (*on_io_parking_bt_release)(void*);
    void (*on_io_parking_bt_hold)(void*);
    void (*on_io_mode_bt_press)(void*);
    void (*on_io_mode_bt_release)(void*);
    void (*on_io_mode_bt_hold)(void*);

    void (*on_io_left_signal_status_changed)(int32_t _value, void*);
    void (*on_io_right_signal_status_changed)(int32_t _value, void*);
    void (*on_io_high_beam_status_changed)(int32_t _value, void*);
    void (*on_io_low_beam_status_changed)(int32_t _value, void*);
    void (*on_io_emergency_status_changed)(int32_t _value, void*);

    // CHARGER
    void (*on_charger_plugged)(int32_t _value, void*);
    void (*on_charged)(int32_t _bp_id, void*);
    void (*on_stoped_charging)(int32_t _bp_id, void*);
    void (*on_paused_charging)(void*);
    void (*on_resumed_charging)(void*);
    // BP
    void (*on_bp_auth)(int32_t, SM_BP_AUTH_EVENT, const char*, int32_t, void*);
    void (*on_bp_connected)(int32_t _bp_id, const char* _sn, int32_t _soc, void*);
    void (*on_bp_disconnected)(int32_t _bp_id, const char* _sn, void*);
    void (*on_bp_update_data)(int32_t _bp_id, const sm_bp_data_t*, void*);
    // PMS
    void (*on_pms_switch_merge)(uint8_t _event, uint8_t _from, uint8_t _into, void*);
    void (*on_pms_update_est_data)(const est_data_t* , void*);
    void (*on_paused_pms)(void*);
    void (*on_resumed_pms)(void*);
}sm_pmu_event_t;

int32_t sm_pmu_api_init(sm_pmu_event_t*, void* _arg);
int32_t sm_pmu_api_free();
int32_t sm_pmu_api_process();

int32_t sm_pmu_api_pause_pms_sv();
int32_t sm_pmu_api_resume_pms_sv();

int32_t sm_pmu_api_pause_charge_sv();
int32_t sm_pmu_api_resume_charge_sv();

int32_t sm_pmu_api_pause_io_sv();
int32_t sm_pmmu_api_resume_io_sv();

int32_t sm_pmu_api_pause_bpm_sv();
int32_t sm_pmmu_api_resume_bpm_sv();

int32_t sm_pmu_api_pause_co_sv();
int32_t sm_pmmu_api_resume_co_sv();

/* CHARGER */
int32_t sm_pmu_api_charger_get_bp_number();
int32_t sm_pmu_api_charger_get_vol();
int32_t sm_pmu_api_charger_get_cur();
int32_t sm_pmu_api_charger_charge(int32_t _bp_id);
int32_t sm_pmu_api_charger_stop_charge(int32_t _bp_id);

/* IO */
int32_t sm_pmu_api_io_get_key_status();
int32_t sm_pmu_api_io_get_left_signal_status();
int32_t sm_pmu_api_io_get_right_signal_status();
int32_t sm_pmu_api_io_get_high_beam_status();
int32_t sm_pmu_api_io_get_low_beam_status();

int32_t sm_pmu_api_io_get_emergency_status();

int32_t sm_pmu_api_io_get_parking_button_status();
int32_t sm_pmu_api_io_get_mode_button_status();
int32_t sm_pmu_api_io_get_horn_status();

int32_t sm_pmu_api_io_set_left_signal(int32_t _value);
int32_t sm_pmu_api_io_reset_left_signal();
int32_t sm_pmu_api_io_blink_left_signal(int32_t _repeat, int32_t _period);

int32_t sm_pmu_api_io_set_right_signal(int32_t _value);
int32_t sm_pmu_api_io_reset_right_signal();
int32_t sm_pmu_api_io_blink_right_signal(int32_t _repeat, int32_t _period);

int32_t sm_pmu_api_io_set_high_beam(int32_t _value);
int32_t sm_pmu_api_io_reset_high_beam();
int32_t sm_pmu_api_io_blink_high_beam(int32_t _repeat, int32_t _period);

int32_t sm_pmu_api_io_set_low_beam(int32_t _value);
int32_t sm_pmu_api_io_reset_low_beam();
int32_t sm_pmu_api_io_blink_low_beam(int32_t _repeat, int32_t _period);

int32_t sm_pmu_api_io_set_tail_light(int32_t _value);
int32_t sm_pmu_api_io_reset_tail_light();
int32_t sm_pmu_api_io_blink_tail_light(int32_t _repeat, int32_t _period);

int32_t sm_pmu_api_io_set_horn(int32_t _value);
int32_t sm_pmu_api_io_reset_horn();
int32_t sm_pmu_api_io_blink_horn(int32_t _repeat, int32_t _period);

/* BATTERY PACKS */
int32_t sm_pmu_api_bp_auth(int32_t _bp_id);
int32_t sm_pmu_api_first_bp_auth(int32_t _bp_id);
int32_t sm_pmu_api_bp_get_number();
const sm_bp_data_t* sm_pmu_api_bp_get_data(int32_t _index);
int32_t sm_pmu_api_bp_is_connected(int32_t _index);
int32_t sm_pmu_api_disable_bp(int32_t _bp_id);
int32_t sm_pmu_api_enable_bp(int32_t _bp_id);

int32_t sm_pmu_api_force_discharging_bp(int32_t _bp_id);
int32_t sm_pmu_api_release_discharging();

int32_t sm_pmu_api_force_charging_bp(int32_t _bp_id);
int32_t sm_pmu_api_release_charging();

/* MOTOR CONTROLLER */
int32_t sm_pmu_api_mc_set_power(int32_t _is_enable);

/* HUMAN MACHINE INTERFACES */
int32_t sm_pmu_api_hmi_set_power(int32_t _is_enable);

/* POWER */
int32_t sm_pmu_api_pw_get_12v_pw_supply_state();

/* SYSTEM */
int32_t sm_pmu_api_sys_reboot();

#endif //EV_SDK_SM_PMU_API_H
