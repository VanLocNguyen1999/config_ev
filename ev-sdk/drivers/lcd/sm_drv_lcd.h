// Created by vnbk on 09/06/2023.
//

#ifndef LCD_SEGMENT_H
#define LCD_SEGMENT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"
#include "segment_driver.h"
#include "uc1676_lcd.h"

#define SM_DRV_LCD_HIDE (0)
#define SM_DRV_LCD_SHOW (!SM_DRV_LCD_HIDE)

enum {
    SM_DRV_LCD_BP_LEVEL_0,
    SM_DRV_LCD_BP_LEVEL_1,
    SM_DRV_LCD_BP_LEVEL_2,
    SM_DRV_LCD_BP_LEVEL_3,
    SM_DRV_LCD_BP_LEVEL_NUMBER /// WHEN SOC = 0
};

typedef void sm_drv_lcd_seg_t;

sm_drv_lcd_seg_t *sm_drv_lcd_seg_create(void *_driver);

int32_t sm_drv_lcd_seg_destroy(sm_drv_lcd_seg_t *_this);

int32_t sm_drv_lcd_seg_power_on(sm_drv_lcd_seg_t *_this);

int32_t sm_drv_lcd_seg_power_off(sm_drv_lcd_seg_t *_this);

int32_t sm_drv_lcd_seg_set_odo_name(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_odo_value(sm_drv_lcd_seg_t *_this, int32_t _odo, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_odo_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_range_name(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_range_value(sm_drv_lcd_seg_t *_this, int32_t _range, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_range_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_time(sm_drv_lcd_seg_t *_this, uint8_t _hour, uint8_t _min, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_hour(sm_drv_lcd_seg_t *_this, int32_t _hour, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_min(sm_drv_lcd_seg_t *_this, int32_t _min, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_dot_time(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_trip_name(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_trip_value(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_trip_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_warning_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_warning_number(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_phase_light(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_cos_light(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_left_signal(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_right_signal(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_parking(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_ble_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide);
int32_t sm_drv_lcd_seg_set_ble_key(sm_drv_lcd_seg_t *_this, int32_t _key, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_calling_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_message_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_mc_degrees_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_mc_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_eco_mode(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_sport_mode(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_speed(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_speed_neg(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_speed_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp0(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp0_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp0_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp1(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp1_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp1_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp2(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp2_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_seg_set_bp2_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide);

int32_t sm_drv_lcd_update_data(sm_drv_lcd_seg_t* _this);
#ifdef __cplusplus
};
#endif

#endif // LCD_SEGMENT_H
