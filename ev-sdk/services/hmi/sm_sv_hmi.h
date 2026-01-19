/*
 * sm_sv_hmi.h
 *
 *  Created on: Sep 20, 2024
 *      Author: admin
 */

#ifndef EV_SDK_SM_SV_HMI_H
#define EV_SDK_SM_SV_HMI_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_drv_lcd.h"
#include "sm_elapsed_timer.h"

#define SM_SV_HMI_BLINK_DURATION_DEFAULT    500
#define SM_SV_HMI_BLINK_SPEED_DEFAULT       200

#define NEGATIVE_SPEED (1)
#define POSITIVE_SPEED (0)

typedef void sm_sv_hmi_t;

sm_sv_hmi_t* sm_sv_hmi_create             (void* _driver, void* _sch);
int32_t sm_sv_hmi_destroy                 (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_init_default            (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_run_animation           (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_power_on                (sm_sv_hmi_t* _this);
int32_t sm_sv_hmi_power_off               (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_set_on_all_icons        (sm_sv_hmi_t *_this);
int32_t sm_sv_hmi_set_off_all_icons       (sm_sv_hmi_t *_this);

int32_t sm_sv_hmi_set_time                (sm_sv_hmi_t* _this, int32_t _hour, int32_t _min);

int32_t sm_sv_hmi_set_mc_temp             (sm_sv_hmi_t* _this, int32_t _temp, uint8_t _hide);
int32_t sm_sv_hmi_blink_mc_temp           (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_left_signal         (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_left_signal       (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_set_right_signal        (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_right_signal      (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_blink_signal            (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_eco_mode            (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_eco_mode          (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_set_sport_mode          (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_sport_mode        (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_speed               (sm_sv_hmi_t* _this, int32_t _speed, uint8_t _hide, uint8_t _mode);
int32_t sm_sv_hmi_blink_speed             (sm_sv_hmi_t* _this, int32_t _speed, int32_t _duration, uint8_t _mode);

int32_t sm_sv_hmi_set_range               (sm_sv_hmi_t* _this, int32_t _range, uint8_t _hide);
int32_t sm_sv_hmi_blink_range             (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_odo                 (sm_sv_hmi_t* _this, int32_t _odo, uint8_t _hide);
int32_t sm_sv_hmi_blink_odo               (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_ota_progress        (sm_sv_hmi_t* _this, int32_t _percent);
int32_t sm_sv_hmi_clear_ota_progress      (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_set_trip                (sm_sv_hmi_t* _this, int32_t _trip, uint8_t _hide);
int32_t sm_sv_hmi_blink_trip              (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_ble_icon            (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_set_ble_key             (sm_sv_hmi_t* _this, int32_t _key, uint8_t _hide);
int32_t sm_sv_hmi_clear_ble_key           (sm_sv_hmi_t* _this);

int32_t sm_sv_hmi_blink_ble_icon          (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_set_dialing_icon        (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_dialing_icon      (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_set_message_icon        (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_message_icon      (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_parking             (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_parking           (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_phase_light         (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_phase_light       (sm_sv_hmi_t* _this, int32_t _duration);
int32_t sm_sv_hmi_set_cos_light           (sm_sv_hmi_t* _this, uint8_t _hide);
int32_t sm_sv_hmi_blink_cos_light         (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_warning_icon        (sm_sv_hmi_t* _this, int32_t _code, uint8_t _hide);
int32_t sm_sv_hmi_blink_warning_icon      (sm_sv_hmi_t* _this, int32_t _duration);

int32_t sm_sv_hmi_set_bp                  (sm_sv_hmi_t* _this, uint8_t _pos, int32_t _soc, int32_t _temp, uint8_t _hide);
int32_t sm_sv_hmi_blink_bp                (sm_sv_hmi_t* _this, uint8_t _pos, int32_t _soc, int32_t _temp, int32_t _duration);

int32_t sm_sv_hmi_process                 (sm_sv_hmi_t* _this);
#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_SV_HMI_H
