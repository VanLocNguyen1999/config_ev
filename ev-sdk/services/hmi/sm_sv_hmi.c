/*

/*
 * sm_sv_hmi.c
 *
 *  Created on: Sep 26, 2024
 *      Author: admin
 */

#include "sm_sv_hmi.h"
#include "sm_core_sch.h"
#include <stdlib.h>
#include "sm_memory.h"

#define impl(x) ((sm_sv_hmi *)(x))

typedef struct
{
    sm_drv_lcd_seg_t *lcd;
    void* value;
    int32_t (*set_func)(sm_drv_lcd_seg_t *, void*, uint8_t);
    bool state;
} sm_sv_blink_task_data_t;

typedef enum {
    TIME_BLINK_ACTIVE = 0,
    MC_TEMP_BLINK_ACTIVE,
    LEFT_SIGNAL_BLINK_ACTIVE,
    RIGHT_SIGNAL_BLINK_ACTIVE,
    ECO_MODE_BLINK_ACTIVE,
    SPORT_MODE_BLINK_ACTIVE,
    SPEED_BLINK_ACTIVE,
    RANGE_BLINK_ACTIVE,
    TRIP_BLINK_ACTIVE,
    ODO_BLINK_ACTIVE,
    BLE_BLINK_ACTIVE,
    DIALING_BLINK_ACTIVE,
    MESSAGE_BLINK_ACTIVE,
    PARKING_BLINK_ACTIVE,
    PHASE_LIGHT_BLINK_ACTIVE,
    COS_LIGHT_BLINK_ACTIVE,
    WARNING_BLINK_ACTIVE,
    BP0_BLINK_ACTIVE,
    BP1_BLINK_ACTIVE,
    BP2_BLINK_ACTIVE,
    BLINK_MAX
} sm_sv_blink_active_flags_t;

typedef struct {
    sm_drv_lcd_seg_t * driver_lcd;
    int32_t  sm_temp;
    int32_t  sm_speed;
    int32_t  m_speed_mode;
    int32_t  sm_trip;
    int32_t  sm_odo;
    int32_t  sm_range;
    int32_t  bp_temp[3];
    int32_t  bp_soc[3];
    int32_t  sm_err_code;
}sm_sv_hmi;

static sm_sv_blink_task_data_t *blink_data_list[BLINK_MAX] = {NULL};
static int32_t                  blink_task_index[BLINK_MAX] = {-1,};
static sm_sch_t *g_shared_scheduler = NULL;

static sm_sv_hmi g_sv_hmi_default = {
                                     .driver_lcd = NULL,
                                     .sm_err_code = 0,
                                     .sm_odo = 0,
                                     .sm_range = 0,
                                     .sm_speed = 0,
                                     .sm_temp = 0,
                                     .sm_trip = 0,
                                     .bp_soc = {0,0,0},
                                     .bp_temp = {0,0,0},
                                     .m_speed_mode = 0
};

sm_sv_hmi_t *sm_sv_hmi_create(void *_driver, void* _sch){
    sm_sv_hmi *sm_hmi = &g_sv_hmi_default;

    sm_hmi->driver_lcd = (sm_drv_lcd_seg_t *)(_driver);

    if(_sch){
        g_shared_scheduler = _sch;
    }else{
        g_shared_scheduler = sm_sch_create();
    }

    memset(blink_task_index, -1, sizeof(blink_task_index));
    return (sm_sv_hmi_t *)sm_hmi;
}

int32_t sm_sv_hmi_destroy(sm_sv_hmi_t *_this){
    if (!_this) return -1;
    impl(_this)->driver_lcd = NULL;
    sm_sch_free(g_shared_scheduler);
    return 0;
}

static void shared_blink_task(void *arg){
    sm_sv_blink_task_data_t *data = (sm_sv_blink_task_data_t *)arg;
    data->state = !data->state;
    data->set_func(data->lcd, data->value, data->state ? SM_DRV_LCD_HIDE : SM_DRV_LCD_SHOW);
}

static int32_t sm_sv_hmi_blink(sm_drv_lcd_seg_t *lcd,
                               sm_sv_blink_active_flags_t flag,
                               int32_t (*set_func)(sm_drv_lcd_seg_t *, void*, uint8_t),
                               void* value,
                               int32_t _duration)
{
    if(!blink_data_list[flag]){
        blink_data_list[flag] = (sm_sv_blink_task_data_t*)mem_alloc(sizeof(sm_sv_blink_task_data_t));
    }

    if (!blink_data_list[flag]){
        return -1;
    }

    blink_data_list[flag]->lcd = lcd;
    blink_data_list[flag]->value = value;
    blink_data_list[flag]->set_func = set_func;
    blink_data_list[flag]->state = 0;

    blink_task_index[flag] = sm_sch_start_task(g_shared_scheduler, _duration, SM_SCH_REPEAT_FOREVER, shared_blink_task, blink_data_list[flag]);
    return 0;
}

static int32_t set_mc_temp(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;
    sm_drv_lcd_seg_set_mc_temp(lcd, hmi->sm_temp, show);
    sm_drv_lcd_seg_set_mc_degrees_icon(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_left_signal(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_left_signal(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_right_signal(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_right_signal(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_dot_time(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_dot_time(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_ble_icon(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_ble_icon(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_calling_icon(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_calling_icon(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_message_icon(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_message_icon(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_parking(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_parking(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_phase_light(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_phase_light(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_cos_light(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_cos_light(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_warning(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;

    sm_drv_lcd_seg_set_warning_icon(lcd, show);
    sm_drv_lcd_seg_set_warning_number(lcd, hmi->sm_err_code, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_eco_mode(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_eco_mode(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_sport_mode(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    (void)value;
    sm_drv_lcd_seg_set_sport_mode(lcd, show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_bp0(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;

    uint8_t level = (hmi->bp_soc[0] == 0 ? SM_DRV_LCD_BP_LEVEL_NUMBER : ((uint8_t)(hmi->bp_soc[0] - 1) / 25));

    sm_drv_lcd_seg_set_bp0(lcd, level, show);
    sm_drv_lcd_seg_set_bp0_soc(lcd, hmi->bp_soc[0], show);
    sm_drv_lcd_seg_set_bp0_temp(lcd, hmi->bp_temp[0], show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_bp1(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;

    uint8_t level = (hmi->bp_soc[1] == 0 ? SM_DRV_LCD_BP_LEVEL_NUMBER : ((uint8_t)(hmi->bp_soc[1] - 1) / 25));

    sm_drv_lcd_seg_set_bp1(lcd, level, show);
    sm_drv_lcd_seg_set_bp1_soc(lcd, hmi->bp_soc[1], show);
    sm_drv_lcd_seg_set_bp1_temp(lcd, hmi->bp_temp[1], show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_bp2(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;

    uint8_t level = (hmi->bp_soc[2] == 0 ? SM_DRV_LCD_BP_LEVEL_NUMBER : ((uint8_t)(hmi->bp_soc[2] - 1) / 25));

    sm_drv_lcd_seg_set_bp2(lcd, level, show);
    sm_drv_lcd_seg_set_bp2_soc(lcd, hmi->bp_soc[2], show);
    sm_drv_lcd_seg_set_bp2_temp(lcd, hmi->bp_temp[2], show);
    return sm_drv_lcd_update_data(lcd);
}

static int32_t set_speed(sm_drv_lcd_seg_t *lcd, void* value, uint8_t show){
    sm_sv_hmi* hmi = (sm_sv_hmi*)value;

    if(hmi->m_speed_mode){
        sm_drv_lcd_seg_set_speed_neg(lcd, hmi->sm_speed, show);
    }else{
        sm_drv_lcd_seg_set_speed(lcd, hmi->sm_speed, show);
    }

    return sm_drv_lcd_update_data(lcd);
}

////
int32_t sm_sv_hmi_power_on(sm_sv_hmi_t *_this){
    if (!_this) return -1;
    return sm_drv_lcd_seg_power_on(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_power_off(sm_sv_hmi_t *_this){
    if (!_this) return -1;
    return sm_drv_lcd_seg_power_off(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_set_on_all_icons(sm_sv_hmi_t *_this){
    if (!_this) return -1;
    sm_drv_lcd_seg_set_ble_icon(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_calling_icon(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_message_icon(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_parking(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_phase_light(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_cos_light(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_warning_icon(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_eco_mode(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_sport_mode(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_range_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_range_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_trip_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_trip_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_update_data(impl(_this)->driver_lcd);
    return 0;
}

int32_t sm_sv_hmi_set_off_all_icons(sm_sv_hmi_t *_this){
    if (!_this) return -1;

    sm_drv_lcd_seg_set_ble_icon(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_calling_icon(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_message_icon(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_parking(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_phase_light(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_cos_light(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_warning_icon(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_eco_mode(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_sport_mode(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);

    sm_drv_lcd_seg_set_range_name(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_range_unit(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_trip_name(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_trip_unit(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_update_data(impl(_this)->driver_lcd);
    return 0;
}

int32_t sm_sv_hmi_init_default(sm_sv_hmi_t *_this)
{
    if (!_this)
        return -1;

    sm_sv_hmi_set_time(_this, 0, 0);

    sm_drv_lcd_seg_set_speed_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_speed(impl(_this)->driver_lcd, 0, SM_DRV_LCD_SHOW);

    sm_drv_lcd_seg_set_range_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_range_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_range_value(impl(_this)->driver_lcd, 0, SM_DRV_LCD_SHOW);

    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_value(impl(_this)->driver_lcd, 0, SM_DRV_LCD_SHOW);

    sm_drv_lcd_seg_set_trip_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_trip_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_trip_value(impl(_this)->driver_lcd, 0, SM_DRV_LCD_SHOW);

    sm_drv_lcd_seg_set_mc_degrees_icon(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_mc_temp(impl(_this)->driver_lcd, 0, SM_DRV_LCD_HIDE);

    sm_drv_lcd_seg_set_sport_mode(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);

    sm_sv_hmi_set_bp(impl(_this), 0, 0, 0, SM_DRV_LCD_HIDE);
    sm_sv_hmi_set_bp(impl(_this), 1, 0, 0, SM_DRV_LCD_HIDE);
    sm_sv_hmi_set_bp(impl(_this), 2, 0, 0, SM_DRV_LCD_HIDE);

    sm_sv_hmi_set_warning_icon(impl(_this), 0, SM_DRV_LCD_HIDE);
    sm_drv_lcd_update_data(impl(_this)->driver_lcd);
    return 0;
}

int32_t sm_sv_hmi_run_animation(sm_sv_hmi_t *_this){
    if (!_this) {
        return -1;
    }
    for (uint8_t j = 0; j< 5; j++){
        for (uint8_t i = 0; i < 10; i++){
           sm_drv_lcd_seg_set_speed (impl(_this)->driver_lcd, i * 10 + i, SM_DRV_LCD_SHOW);
           sm_drv_lcd_seg_set_trip_value (impl(_this)->driver_lcd, i * 100 + i * 10 + i, SM_DRV_LCD_SHOW);
           sm_drv_lcd_seg_set_range_value (impl(_this)->driver_lcd, i * 100 + i * 10 + i, SM_DRV_LCD_SHOW);
           sm_drv_lcd_seg_set_warning_number (impl(_this)->driver_lcd, i * 100 + i * 10 + i, SM_DRV_LCD_SHOW);

           sm_sv_hmi_set_bp(_this, 0, i * 10 + i, i * 10 + i, SM_DRV_LCD_SHOW);
           sm_sv_hmi_set_bp(_this, 1, i * 10 + i, i * 10 + i, SM_DRV_LCD_SHOW);
           sm_sv_hmi_set_bp(_this, 2, i * 10 + i, i * 10 + i, SM_DRV_LCD_SHOW);

           sm_drv_lcd_seg_set_odo_value (impl(_this)->driver_lcd,
                                         i * 1000000 + i * 100000 + i * 10000 + i * 1000 + i * 100 + i * 10 + i,
                                         SM_DRV_LCD_SHOW);
           sm_drv_lcd_seg_set_mc_temp (impl(_this)->driver_lcd, i * 10 + i, SM_DRV_LCD_SHOW);
           sm_drv_lcd_seg_set_time (impl(_this)->driver_lcd, i * 10 + i, i * 10 + i, SM_DRV_LCD_SHOW);

           if (i%5 >= 2)
           {
               sm_drv_lcd_seg_set_left_signal (impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
               sm_drv_lcd_seg_set_right_signal (impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
           }
           else
           {
               sm_drv_lcd_seg_set_left_signal (impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
               sm_drv_lcd_seg_set_right_signal (impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
           }
           if (i%5 >= 2){
               sm_sv_hmi_set_off_all_icons(_this);
           }else{
               sm_sv_hmi_set_on_all_icons(_this);
           }
       }
    }


    sm_drv_lcd_seg_set_speed (impl(_this)->driver_lcd, 0, SM_DRV_LCD_SHOW);
    sm_sv_hmi_set_off_all_icons(_this);
    return 0;
}

int32_t sm_sv_hmi_process(sm_sv_hmi_t *_this){
    if (!_this || !g_shared_scheduler) {
        return -1;
    }
    return sm_sch_process(g_shared_scheduler);
}

int32_t sm_sv_hmi_set_time(sm_sv_hmi_t *_this, int32_t _hour, int32_t _min){
    if (!_this) {
        return -1;
    }

    sm_drv_lcd_seg_set_hour(impl(_this)->driver_lcd, _hour, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_min(impl(_this)->driver_lcd, _min, SM_DRV_LCD_SHOW);
    sm_drv_lcd_update_data(impl(_this)->driver_lcd);

    if(blink_task_index[TIME_BLINK_ACTIVE] != -1){
        return 0;
    }

    return sm_sv_hmi_blink(impl(_this)->driver_lcd, TIME_BLINK_ACTIVE, set_dot_time, 0, SM_SV_HMI_BLINK_DURATION_DEFAULT);
}

int32_t sm_sv_hmi_set_mc_temp(sm_sv_hmi_t* _this, int32_t _temp, uint8_t _hide) {
    if (!_this) return -1;
    impl(_this)->sm_temp = _temp;
    if (blink_task_index[MC_TEMP_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[MC_TEMP_BLINK_ACTIVE]);
        blink_task_index[MC_TEMP_BLINK_ACTIVE] = -1;
    }

    sm_drv_lcd_seg_set_mc_temp(impl(_this)->driver_lcd, _temp, _hide);
    sm_drv_lcd_seg_set_mc_degrees_icon(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_mc_temp(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, MC_TEMP_BLINK_ACTIVE, set_mc_temp, impl(_this), _duration);
}

int32_t sm_sv_hmi_set_left_signal(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[LEFT_SIGNAL_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[LEFT_SIGNAL_BLINK_ACTIVE]);
        blink_task_index[LEFT_SIGNAL_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_left_signal(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_left_signal(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, LEFT_SIGNAL_BLINK_ACTIVE, set_left_signal, 0, _duration);
}

int32_t sm_sv_hmi_set_right_signal(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[RIGHT_SIGNAL_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[RIGHT_SIGNAL_BLINK_ACTIVE]);
        blink_task_index[RIGHT_SIGNAL_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_right_signal(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_right_signal(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, RIGHT_SIGNAL_BLINK_ACTIVE, set_right_signal, 0, _duration);
}

int32_t sm_sv_hmi_blink_signal(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    sm_sv_hmi_blink_left_signal(_this, _duration);
    sm_sv_hmi_blink_right_signal(_this, _duration);
    return 0;
}

int32_t sm_sv_hmi_set_eco_mode(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[ECO_MODE_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[ECO_MODE_BLINK_ACTIVE]);
        blink_task_index[ECO_MODE_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_eco_mode(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_eco_mode(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, ECO_MODE_BLINK_ACTIVE, set_eco_mode, NULL, _duration);
}

int32_t sm_sv_hmi_set_sport_mode(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[SPORT_MODE_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[SPORT_MODE_BLINK_ACTIVE]);
        blink_task_index[SPORT_MODE_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_sport_mode(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_sport_mode(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, SPORT_MODE_BLINK_ACTIVE, set_sport_mode, NULL, _duration);
}

int32_t sm_sv_hmi_set_speed(sm_sv_hmi_t* _this, int32_t _speed, uint8_t _hide, uint8_t _mode) {
    if (!_this) return -1;

    impl(_this)->sm_speed = _speed;
    impl(_this)->m_speed_mode = _mode;

    if (blink_task_index[SPEED_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[SPEED_BLINK_ACTIVE]);
        blink_task_index[SPEED_BLINK_ACTIVE] = -1;
    }

    if (impl(_this)->m_speed_mode){
        sm_drv_lcd_seg_set_speed_neg(impl(_this)->driver_lcd, _speed, _hide);
    }else{
        sm_drv_lcd_seg_set_speed(impl(_this)->driver_lcd, _speed, _hide);
    }
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_speed(sm_sv_hmi_t* _this, int32_t _speed, int32_t _duration, uint8_t _mode){
    if (!_this) return -1;

    impl(_this)->m_speed_mode = _mode;
    impl(_this)->sm_speed = _speed;

    if (blink_task_index[SPEED_BLINK_ACTIVE] == -1){
        return sm_sv_hmi_blink(impl(_this)->driver_lcd, SPEED_BLINK_ACTIVE, set_speed, impl(_this), _duration);
    }

    return 0;
}

int32_t sm_sv_hmi_set_range(sm_sv_hmi_t* _this, int32_t _range, uint8_t _hide) {
    if (!_this) return -1;
    impl(_this)->sm_range = _range;
    if (blink_task_index[RANGE_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[RANGE_BLINK_ACTIVE]);
        blink_task_index[RANGE_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_range_value(impl(_this)->driver_lcd, _range, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_range(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return -1;
//    return sm_sv_hmi_blink(impl(_this)->driver_lcd, RANGE_BLINK_ACTIVE, sm_drv_lcd_seg_set_range_value, impl(_this), _duration);
}

int32_t sm_sv_hmi_set_odo(sm_sv_hmi_t* _this, int32_t _odo, uint8_t _hide) {
    if (!_this) return -1;
    impl(_this)->sm_odo = _odo;
    if (blink_task_index[ODO_BLINK_ACTIVE] != -1)
    {
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[ODO_BLINK_ACTIVE]);
        blink_task_index[ODO_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_odo_value(impl(_this)->driver_lcd, _odo/100, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_odo(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return -1;
//    return sm_sv_hmi_blink(impl(_this)->driver_lcd, ODO_BLINK_ACTIVE, sm_drv_lcd_seg_set_odo_value, impl(_this), _duration);
}

int32_t sm_sv_hmi_set_ota_progress(sm_sv_hmi_t* _this, int32_t _percent){
    if (!_this) return -1;

    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_HIDE);

    sm_drv_lcd_seg_set_odo_value(impl(_this)->driver_lcd, _percent, SM_DRV_LCD_SHOW);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_clear_ota_progress(sm_sv_hmi_t* _this){
    if (!_this) return -1;
    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_set_trip(sm_sv_hmi_t* _this, int32_t _trip, uint8_t _hide) {
    if (!_this) return -1;
    impl(_this)->sm_trip = _trip;
    if (blink_task_index[TRIP_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[TRIP_BLINK_ACTIVE]);
        blink_task_index[TRIP_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_trip_value(impl(_this)->driver_lcd, _trip/1000, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_trip(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return -1;
//    return sm_sv_hmi_blink(impl(_this)->driver_lcd, TRIP_BLINK_ACTIVE, sm_drv_lcd_seg_set_trip_value, impl(_this), _duration);
}

int32_t sm_sv_hmi_set_ble_icon(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[BLE_BLINK_ACTIVE] != -1)
    {
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[BLE_BLINK_ACTIVE]);
        blink_task_index[BLE_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_ble_icon(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_set_ble_key(sm_sv_hmi_t* _this, int32_t _key, uint8_t _hide)
{
    if (!_this) return -1;
    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, !_hide);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, !_hide);
    sm_drv_lcd_seg_set_ble_key(impl(_this)->driver_lcd, _key, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_clear_ble_key(sm_sv_hmi_t* _this){
    if (!_this) return -1;
    sm_drv_lcd_seg_set_odo_name(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    sm_drv_lcd_seg_set_odo_unit(impl(_this)->driver_lcd, SM_DRV_LCD_SHOW);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_ble_icon(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, BLE_BLINK_ACTIVE, set_ble_icon, NULL, _duration);
}

int32_t sm_sv_hmi_set_dialing_icon(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[DIALING_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[DIALING_BLINK_ACTIVE]);
        blink_task_index[DIALING_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_calling_icon(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_dialing_icon(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, DIALING_BLINK_ACTIVE, set_calling_icon, NULL, _duration);
}

int32_t sm_sv_hmi_set_message_icon(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[MESSAGE_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[MESSAGE_BLINK_ACTIVE]);
        blink_task_index[MESSAGE_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_message_icon(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_message_icon(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, MESSAGE_BLINK_ACTIVE, set_message_icon, NULL, _duration);
}

int32_t sm_sv_hmi_set_parking(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[PARKING_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[PARKING_BLINK_ACTIVE]);
        blink_task_index[PARKING_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_parking(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_parking(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, PARKING_BLINK_ACTIVE, set_parking, NULL, _duration);
}

int32_t sm_sv_hmi_set_phase_light(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[PHASE_LIGHT_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[PHASE_LIGHT_BLINK_ACTIVE]);
        blink_task_index[PHASE_LIGHT_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_phase_light(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_phase_light(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, PHASE_LIGHT_BLINK_ACTIVE, set_phase_light, NULL, _duration);
}

int32_t sm_sv_hmi_set_cos_light(sm_sv_hmi_t* _this, uint8_t _hide) {
    if (!_this) return -1;
    if (blink_task_index[COS_LIGHT_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[COS_LIGHT_BLINK_ACTIVE]);
        blink_task_index[COS_LIGHT_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_cos_light(impl(_this)->driver_lcd, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_cos_light(sm_sv_hmi_t* _this, int32_t _duration) {
    if (!_this) return -1;
    return sm_sv_hmi_blink(impl(_this)->driver_lcd, COS_LIGHT_BLINK_ACTIVE, set_cos_light, 0, _duration);
}

int32_t sm_sv_hmi_set_warning_icon(sm_sv_hmi_t* _this, int32_t _code, uint8_t _hide) {
    if (!_this) return -1;
    impl(_this)->sm_err_code = _code;
    if (blink_task_index[WARNING_BLINK_ACTIVE] != -1){
        sm_sch_cancel_task(g_shared_scheduler, blink_task_index[WARNING_BLINK_ACTIVE]);
        blink_task_index[WARNING_BLINK_ACTIVE] = -1;
    }
    sm_drv_lcd_seg_set_warning_icon(impl(_this)->driver_lcd, _hide);
    sm_drv_lcd_seg_set_warning_number(impl(_this)->driver_lcd, _code, _hide);
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);
}

int32_t sm_sv_hmi_blink_warning_icon(sm_sv_hmi_t* _this, int32_t _duration) {
    sm_sv_hmi_blink(impl(_this)->driver_lcd, WARNING_BLINK_ACTIVE, set_warning, impl(_this), _duration);
    return 0;
}

int32_t sm_sv_hmi_set_bp(sm_sv_hmi_t* _this, uint8_t _pos, int32_t _soc, int32_t _temp, uint8_t _hide) {
    if(!_this){
        return -1;
    }

    uint8_t soc = (_hide == SM_DRV_LCD_HIDE) ? 0 : (uint8_t)_soc;
    uint8_t temp = (_hide == SM_DRV_LCD_HIDE) ? 0 : (uint8_t) _temp;

    uint8_t level = (_soc == 0 ? SM_DRV_LCD_BP_LEVEL_NUMBER : ((uint8_t)(_soc - 1) / 25));
    switch (_pos) {
        case 0:
            impl(_this)->bp_soc[0] = soc;
            impl(_this)->bp_temp[0] = temp;
            if (blink_task_index[BP0_BLINK_ACTIVE] != -1){
                sm_sch_cancel_task(g_shared_scheduler, blink_task_index[BP0_BLINK_ACTIVE]);
                blink_task_index[BP0_BLINK_ACTIVE] = -1;
            }
            sm_drv_lcd_seg_set_bp0(impl(_this)->driver_lcd, level, _hide);
            sm_drv_lcd_seg_set_bp0_soc(impl(_this)->driver_lcd, soc, _hide);
            sm_drv_lcd_seg_set_bp0_temp(impl(_this)->driver_lcd, temp, _hide);
            break;
        case 1:
            impl(_this)->bp_soc[1] = soc;
            impl(_this)->bp_temp[1] = temp;
            if (blink_task_index[BP1_BLINK_ACTIVE] != -1){
                sm_sch_cancel_task(g_shared_scheduler, blink_task_index[BP1_BLINK_ACTIVE]);
                blink_task_index[BP1_BLINK_ACTIVE] = -1;
            }
            sm_drv_lcd_seg_set_bp1(impl(_this)->driver_lcd, level, _hide);
            sm_drv_lcd_seg_set_bp1_soc(impl(_this)->driver_lcd, soc, _hide);
            sm_drv_lcd_seg_set_bp1_temp(impl(_this)->driver_lcd, temp, _hide);
            break;
        case 2:
            impl(_this)->bp_soc[2] = soc;
            impl(_this)->bp_temp[2] = temp;
            if (blink_task_index[BP2_BLINK_ACTIVE] != -1){
                sm_sch_cancel_task(g_shared_scheduler, blink_task_index[BP2_BLINK_ACTIVE]);
                blink_task_index[BP2_BLINK_ACTIVE] = -1;
            }
            sm_drv_lcd_seg_set_bp2(impl(_this)->driver_lcd, level, _hide);
            sm_drv_lcd_seg_set_bp2_soc(impl(_this)->driver_lcd, soc, _hide);
            sm_drv_lcd_seg_set_bp2_temp(impl(_this)->driver_lcd, temp, _hide);
            break;
        default:
            return -1;
    }
    return sm_drv_lcd_update_data(impl(_this)->driver_lcd);;
}

int32_t sm_sv_hmi_blink_bp(sm_sv_hmi_t* _this, uint8_t _pos, int32_t _soc, int32_t _temp, int32_t _duration) {
    if(!_this){
        return -1;
    }

    impl(_this)->bp_soc[_pos] = _soc;
    impl(_this)->bp_temp[_pos] = _temp;

    if(blink_task_index[BP0_BLINK_ACTIVE + _pos] != -1){
        return 0;
    }

    switch (_pos) {
        case 0:
            sm_sv_hmi_blink(impl(_this)->driver_lcd, BP0_BLINK_ACTIVE, set_bp0, _this, _duration);
            break;
        case 1:
            sm_sv_hmi_blink(impl(_this)->driver_lcd, BP1_BLINK_ACTIVE, set_bp1, _this, _duration);
            break;
        case 2:
            sm_sv_hmi_blink(impl(_this)->driver_lcd, BP2_BLINK_ACTIVE, set_bp2, _this, _duration);
            break;
        default:
            return -1;
    }
    return 0;
}
