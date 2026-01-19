#include "sm_drv_lcd.h"
#include "uc1676_lcd.h"
#include <stdlib.h>
#define _impl(x) ((sm_drv_lcd_seg_impl_t*)(x))

typedef struct segment_lcd segment_lcd_t;

struct segment_lcd{
    Segment_bytes   data_bytes[LCD_MAX_TX_BUFFER];
    Segment_bytes   cmd_bytes[LCD_MAX_CMD_BUFFER];
};

typedef struct{
    uc1676_t* m_driver;
    segment_lcd_t lcd;
} sm_drv_lcd_seg_impl_t;

const Segment_bytes   g_lcd_init_write_cmd[LCD_MAX_CMD_BUFFER] = { {.byte = 0x80},
                                                                  {.byte = 0xe0},
                                                                  {.byte = 0x80},
                                                                  {.byte = 0x00},
                                                                  {.byte = 0x40} };

const Segment_bytes   g_lcd_config_cmd[LCD_MAX_CMD_BUFFER] = { {.byte = 0x80},
                                                              {.byte = 0xc8},
                                                              {.byte = 0x80},
                                                              {.byte = 0xf0},
                                                              {.byte = 0xe0} };

static void sm_drv_lcd_config(sm_drv_lcd_seg_impl_t* _this);

static sm_drv_lcd_seg_impl_t g_drv_lcd_default = {
        .m_driver = NULL,
};

sm_drv_lcd_seg_t *sm_drv_lcd_seg_create(void* _driver){
    if(!_driver){
        return NULL;
    }
    sm_drv_lcd_seg_impl_t *driver = &g_drv_lcd_default;

    driver->m_driver = (uc1676_t*)_driver;

    memset(driver->lcd.data_bytes, 0x00, LCD_MAX_TX_BUFFER);
    memset(driver->lcd.cmd_bytes, 0x00, LCD_MAX_CMD_BUFFER );

    sm_drv_lcd_config(driver);
//    sm_hal_delay_ms(1000);

    return (sm_drv_lcd_seg_t *)driver;
}

int32_t sm_drv_lcd_seg_destroy(sm_drv_lcd_seg_t *_this){
    if (!_this)
        return -1;

//    free(_this);
    return 0;
}

int32_t sm_drv_lcd_seg_power_on(sm_drv_lcd_seg_t *_this){
    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    if (!driver)
        return -1;

    uc1676_power_on(driver->m_driver);

    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    seg_bytes[0].bits.b5 = 1;
    seg_bytes[22].bits.b4 = 1;

    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_power_off(sm_drv_lcd_seg_t *_this){
    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    if (!driver)
        return -1;

    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    memset(seg_bytes, 0x00, LCD_MAX_TX_BUFFER );
    sm_drv_lcd_update_data(driver);

    uc1676_power_off(driver->m_driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_odo_name(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[32].bits.b4 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_odo_value(sm_drv_lcd_seg_t *_this, int32_t _odo, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = (sm_drv_lcd_seg_impl_t *)_this;
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 7;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _odo);

    for (uint8_t i = 0; i < digit_num - 2; i++){
        seg_bytes[26 + i].bits.b0 = digits[i + 1].a;
        seg_bytes[26 + i].bits.b1 = digits[i + 1].d;
        seg_bytes[26 + i].bits.b2 = digits[i + 1].b;
        seg_bytes[26 + i].bits.b3 = digits[i + 1].c;
        seg_bytes[27 + i].bits.b4 = digits[i + 1].f;
        seg_bytes[27 + i].bits.b6 = digits[i + 1].g;
        seg_bytes[27 + i].bits.b7 = digits[i + 1].e;
    }

    seg_bytes[31].bits.b0 = digits[6].a;
    seg_bytes[31].bits.b1 = digits[6].d;
    seg_bytes[31].bits.b2 = digits[6].b;
    seg_bytes[31].bits.b3 = digits[6].c;
    seg_bytes[32].bits.b5 = digits[6].e;
    seg_bytes[32].bits.b6 = digits[6].f;
    seg_bytes[32].bits.b7 = digits[6].g;

    seg_bytes[25].bits.b0 = digits[0].a;
    seg_bytes[25].bits.b1 = digits[0].d;
    seg_bytes[25].bits.b2 = digits[0].b;
    seg_bytes[25].bits.b3 = digits[0].c;
    seg_bytes[26].bits.b4 = digits[0].f;
    seg_bytes[26].bits.b6 = digits[0].g;
    seg_bytes[26].bits.b7 = digits[0].e;

    seg_bytes[27].bits.b5 = 1;

    if (!_hide){
        for (uint8_t i = 0; i < digit_num - 2; i++){
            seg_bytes[26 + i].bits.b0 = 0;
            seg_bytes[26 + i].bits.b1 = 0;
            seg_bytes[26 + i].bits.b2 = 0;
            seg_bytes[26 + i].bits.b3 = 0;
            seg_bytes[27 + i].bits.b4 = 0;
            seg_bytes[27 + i].bits.b6 = 0;
            seg_bytes[27 + i].bits.b7 = 0;
        }

        seg_bytes[31].bits.b0 = 0;
        seg_bytes[31].bits.b1 = 0;
        seg_bytes[31].bits.b2 = 0;
        seg_bytes[31].bits.b3 = 0;
        seg_bytes[32].bits.b5 = 0;
        seg_bytes[32].bits.b6 = 0;
        seg_bytes[32].bits.b7 = 0;

        seg_bytes[25].bits.b0 = 0;
        seg_bytes[25].bits.b1 = 0;
        seg_bytes[25].bits.b2 = 0;
        seg_bytes[25].bits.b3 = 0;
        seg_bytes[26].bits.b4 = 0;
        seg_bytes[26].bits.b6 = 0;
        seg_bytes[26].bits.b7 = 0;

        seg_bytes[27].bits.b5 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_odo_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;
    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    seg_bytes[26].bits.b5 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_range_name(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[38].bits.b4 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_range_value(sm_drv_lcd_seg_t *_this, int32_t _range, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    if (!_hide){
        seg_bytes[38].bits.b0 = 0;
        seg_bytes[38].bits.b1 = 0;
        seg_bytes[38].bits.b2 = 0;
        seg_bytes[38].bits.b3 = 0;
        seg_bytes[38].bits.b5 = 0;
        seg_bytes[38].bits.b6 = 0;
        seg_bytes[38].bits.b7 = 0;

        seg_bytes[33].bits.b0 = 0;
        seg_bytes[33].bits.b1 = 0;
        seg_bytes[33].bits.b2 = 0;
        seg_bytes[33].bits.b3 = 0;
        seg_bytes[39].bits.b4 = 0;
        seg_bytes[39].bits.b6 = 0;
        seg_bytes[39].bits.b7 = 0;

        seg_bytes[32].bits.b0 = 0;
        seg_bytes[32].bits.b1 = 0;
        seg_bytes[32].bits.b2 = 0;
        seg_bytes[32].bits.b3 = 0;
        seg_bytes[33].bits.b4 = 0;
        seg_bytes[33].bits.b6 = 0;
        seg_bytes[33].bits.b7 = 0;
    }else{
        segment_update_digit_value(digits, digit_num, min_digit_show, _range);

        seg_bytes[38].bits.b0 = digits[2].a;
        seg_bytes[38].bits.b1 = digits[2].d;
        seg_bytes[38].bits.b2 = digits[2].b;
        seg_bytes[38].bits.b3 = digits[2].c;
        seg_bytes[38].bits.b5 = digits[2].e;
        seg_bytes[38].bits.b6 = digits[2].f;
        seg_bytes[38].bits.b7 = digits[2].g;

        seg_bytes[33].bits.b0 = digits[1].a;
        seg_bytes[33].bits.b1 = digits[1].d;
        seg_bytes[33].bits.b2 = digits[1].b;
        seg_bytes[33].bits.b3 = digits[1].c;
        seg_bytes[39].bits.b4 = digits[1].f;
        seg_bytes[39].bits.b6 = digits[1].g;
        seg_bytes[39].bits.b7 = digits[1].e;

        seg_bytes[32].bits.b0 = digits[0].a;
        seg_bytes[32].bits.b1 = digits[0].d;
        seg_bytes[32].bits.b2 = digits[0].b;
        seg_bytes[32].bits.b3 = digits[0].c;
        seg_bytes[33].bits.b4 = digits[0].f;
        seg_bytes[33].bits.b6 = digits[0].g;
        seg_bytes[33].bits.b7 = digits[0].e;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_range_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[33].bits.b5 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_time(sm_drv_lcd_seg_t *_this, uint8_t _hour, uint8_t _min, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_set_hour(_this, _hour, _hide);
    sm_drv_lcd_seg_set_min(_this, _min, _hide);
    sm_drv_lcd_seg_set_dot_time(_this, _hide);
//    sm_drv_lcd_update_data(_impl(_this));
    return 0;
}

int32_t sm_drv_lcd_seg_set_hour(sm_drv_lcd_seg_t *_this, int32_t _hour, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _hour);

    for (uint8_t i = 0; i < digit_num; i++){
        seg_bytes[18 + i].bits.b0 = digits[i].b;
        seg_bytes[18 + i].bits.b2 = digits[i].g;
        seg_bytes[18 + i].bits.b3 = digits[i].c;
        seg_bytes[19 + i].bits.b4 = digits[i].a;
        seg_bytes[19 + i].bits.b5 = digits[i].d;
        seg_bytes[19 + i].bits.b6 = digits[i].f;
        seg_bytes[19 + i].bits.b7 = digits[i].e;
    }

    if (!_hide){
        for (uint8_t i = 0; i < digit_num; i++){
            seg_bytes[18 + i].bits.b0 = 0;
            seg_bytes[18 + i].bits.b2 = 0;
            seg_bytes[18 + i].bits.b3 = 0;
            seg_bytes[19 + i].bits.b4 = 0;
            seg_bytes[19 + i].bits.b5 = 0;
            seg_bytes[19 + i].bits.b6 = 0;
            seg_bytes[19 + i].bits.b7 = 0;
        }
    }
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_min(sm_drv_lcd_seg_t *_this, int32_t _min, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _min);

    for (uint8_t i = 0; i < digit_num; i++){
        seg_bytes[16 + i].bits.b0 = digits[i].b;
        seg_bytes[16 + i].bits.b2 = digits[i].g;
        seg_bytes[16 + i].bits.b3 = digits[i].c;
        seg_bytes[17 + i].bits.b4 = digits[i].a;
        seg_bytes[17 + i].bits.b5 = digits[i].d;
        seg_bytes[17 + i].bits.b6 = digits[i].f;
        seg_bytes[17 + i].bits.b7 = digits[i].e;
    }

    if (!_hide){
        for (uint8_t i = 0; i < digit_num; i++){
            seg_bytes[16 + i].bits.b0 = 0;
            seg_bytes[16 + i].bits.b2 = 0;
            seg_bytes[16 + i].bits.b3 = 0;
            seg_bytes[17 + i].bits.b4 = 0;
            seg_bytes[17 + i].bits.b5 = 0;
            seg_bytes[17 + i].bits.b6 = 0;
            seg_bytes[17 + i].bits.b7 = 0;
        }
    }
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_dot_time(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = (sm_drv_lcd_seg_impl_t *)_this;
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    if (_hide){
        seg_bytes[18].bits.b1 = 1;
    }else{
        seg_bytes[18].bits.b1 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_trip_name(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[25].bits.b4 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_trip_value(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;
    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _value);

    for (uint8_t i = 0; i < digit_num - 1; i++){
        seg_bytes[22 + i].bits.b0 = digits[i].a;
        seg_bytes[22 + i].bits.b1 = digits[i].d;
        seg_bytes[22 + i].bits.b2 = digits[i].b;
        seg_bytes[22 + i].bits.b3 = digits[i].c;
        seg_bytes[23 + i].bits.b4 = digits[i].f;
        seg_bytes[23 + i].bits.b6 = digits[i].g;
        seg_bytes[23 + i].bits.b7 = digits[i].e;
    }
    seg_bytes[24].bits.b0 = digits[2].a;
    seg_bytes[24].bits.b1 = digits[2].d;
    seg_bytes[24].bits.b2 = digits[2].b;
    seg_bytes[24].bits.b3 = digits[2].c;
    seg_bytes[25].bits.b5 = digits[2].e;
    seg_bytes[25].bits.b6 = digits[2].f;
    seg_bytes[25].bits.b7 = digits[2].g;

    if (!_hide) {
        for (uint8_t i = 0; i < digit_num - 1; i++) {
            seg_bytes[22 + i].bits.b0 = 0;
            seg_bytes[22 + i].bits.b1 = 0;
            seg_bytes[22 + i].bits.b2 = 0;
            seg_bytes[22 + i].bits.b3 = 0;
            seg_bytes[23 + i].bits.b4 = 0;
            seg_bytes[23 + i].bits.b6 = 0;
            seg_bytes[23 + i].bits.b7 = 0;
        }
        seg_bytes[24].bits.b0 = 0;
        seg_bytes[24].bits.b1 = 0;
        seg_bytes[24].bits.b2 = 0;
        seg_bytes[24].bits.b3 = 0;
        seg_bytes[25].bits.b5 = 0;
        seg_bytes[25].bits.b6 = 0;
        seg_bytes[25].bits.b7 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_trip_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[23].bits.b5 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_warning_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[34].bits.b1 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_warning_number(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    if (_value <= 0){
        segment_clear_digit_value(digits, digit_num);
    }else{
        segment_update_digit_value(digits, digit_num, min_digit_show, _value);
    }

    for (uint8_t i = 0; i < digit_num - 1; i++){
        seg_bytes[36 + i].bits.b0 = digits[i + 1].d;
        seg_bytes[36 + i].bits.b1 = digits[i + 1].a;
        seg_bytes[36 + i].bits.b2 = digits[i + 1].e;
        seg_bytes[36 + i].bits.b3 = digits[i + 1].f;
        seg_bytes[36 + i].bits.b4 = digits[i + 1].c;
        seg_bytes[36 + i].bits.b6 = digits[i + 1].g;
        seg_bytes[36 + i].bits.b7 = digits[i + 1].b;
    }
    seg_bytes[34].bits.b4 = digits[0].c;
    seg_bytes[34].bits.b6 = digits[0].g;
    seg_bytes[34].bits.b7 = digits[0].b;
    seg_bytes[35].bits.b0 = digits[0].d;
    seg_bytes[35].bits.b1 = digits[0].a;
    seg_bytes[35].bits.b2 = digits[0].e;
    seg_bytes[35].bits.b3 = digits[0].f;

    if (!_hide){
        for (uint8_t i = 0; i < digit_num - 1; i++){
            seg_bytes[36 + i].bits.b0 = 0;
            seg_bytes[36 + i].bits.b1 = 0;
            seg_bytes[36 + i].bits.b2 = 0;
            seg_bytes[36 + i].bits.b3 = 0;
            seg_bytes[36 + i].bits.b4 = 0;
            seg_bytes[36 + i].bits.b6 = 0;
            seg_bytes[36 + i].bits.b7 = 0;
        }
        seg_bytes[34].bits.b4 = 0;
        seg_bytes[34].bits.b6 = 0;
        seg_bytes[34].bits.b7 = 0;
        seg_bytes[35].bits.b0 = 0;
        seg_bytes[35].bits.b1 = 0;
        seg_bytes[35].bits.b2 = 0;
        seg_bytes[35].bits.b3 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_phase_light(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[34].bits.b2 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_cos_light(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[34].bits.b3 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_left_signal(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[35].bits.b4 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_right_signal(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[14].bits.b4 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_parking(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[34].bits.b0 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_ble_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[35].bits.b6 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_ble_key(sm_drv_lcd_seg_t *_this, int32_t _key, uint8_t _hide)
{
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = (sm_drv_lcd_seg_impl_t *)_this;
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 6;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _key);

    for (uint8_t i = 0; i < digit_num - 1; i++){
        seg_bytes[26 + i].bits.b0 = digits[i].a;
        seg_bytes[26 + i].bits.b1 = digits[i].d;
        seg_bytes[26 + i].bits.b2 = digits[i].b;
        seg_bytes[26 + i].bits.b3 = digits[i].c;
        seg_bytes[27 + i].bits.b4 = digits[i].f;
        seg_bytes[27 + i].bits.b6 = digits[i].g;
        seg_bytes[27 + i].bits.b7 = digits[i].e;
    }

    seg_bytes[31].bits.b0 = digits[5].a;
    seg_bytes[31].bits.b1 = digits[5].d;
    seg_bytes[31].bits.b2 = digits[5].b;
    seg_bytes[31].bits.b3 = digits[5].c;
    seg_bytes[32].bits.b5 = digits[5].e;
    seg_bytes[32].bits.b6 = digits[5].f;
    seg_bytes[32].bits.b7 = digits[5].g;

    seg_bytes[25].bits.b0 = 0;
    seg_bytes[25].bits.b1 = 0;
    seg_bytes[25].bits.b2 = 0;
    seg_bytes[25].bits.b3 = 0;
    seg_bytes[26].bits.b4 = 0;
    seg_bytes[26].bits.b6 = 0;
    seg_bytes[26].bits.b7 = 0;

    seg_bytes[27].bits.b5 = 0;

    if (!_hide){
        for (uint8_t i = 0; i < digit_num - 1; i++){
            seg_bytes[26 + i].bits.b0 = 0;
            seg_bytes[26 + i].bits.b1 = 0;
            seg_bytes[26 + i].bits.b2 = 0;
            seg_bytes[26 + i].bits.b3 = 0;
            seg_bytes[27 + i].bits.b4 = 0;
            seg_bytes[27 + i].bits.b6 = 0;
            seg_bytes[27 + i].bits.b7 = 0;
        }

        seg_bytes[31].bits.b0 = 0;
        seg_bytes[31].bits.b1 = 0;
        seg_bytes[31].bits.b2 = 0;
        seg_bytes[31].bits.b3 = 0;
        seg_bytes[32].bits.b5 = 0;
        seg_bytes[32].bits.b6 = 0;
        seg_bytes[32].bits.b7 = 0;

        seg_bytes[25].bits.b0 = 0;
        seg_bytes[25].bits.b1 = 0;
        seg_bytes[25].bits.b2 = 0;
        seg_bytes[25].bits.b3 = 0;
        seg_bytes[26].bits.b4 = 0;
        seg_bytes[26].bits.b6 = 0;
        seg_bytes[26].bits.b7 = 0;

        seg_bytes[27].bits.b5 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_calling_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[35].bits.b7 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_message_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[35].bits.b5 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_mc_degrees_icon(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[20].bits.b0 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_mc_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    segment_update_digit_value(digits, digit_num, min_digit_show, _value);

    seg_bytes[20].bits.b1 = digits[0].c;
    seg_bytes[20].bits.b2 = digits[0].b;
    seg_bytes[20].bits.b3 = digits[0].g;
    seg_bytes[21].bits.b4 = digits[0].a;
    seg_bytes[21].bits.b5 = digits[0].d;
    seg_bytes[21].bits.b6 = digits[0].f;
    seg_bytes[21].bits.b7 = digits[0].e;

    seg_bytes[21].bits.b0 = digits[1].a;
    seg_bytes[21].bits.b1 = digits[1].d;
    seg_bytes[21].bits.b2 = digits[1].b;
    seg_bytes[21].bits.b3 = digits[1].c;
    seg_bytes[22].bits.b5 = digits[1].e;
    seg_bytes[22].bits.b6 = digits[1].f;
    seg_bytes[22].bits.b7 = digits[1].g;

    if (!_hide){
        seg_bytes[20].bits.b1 = 0;
        seg_bytes[20].bits.b2 = 0;
        seg_bytes[20].bits.b3 = 0;
        seg_bytes[21].bits.b4 = 0;
        seg_bytes[21].bits.b5 = 0;
        seg_bytes[21].bits.b6 = 0;
        seg_bytes[21].bits.b7 = 0;

        seg_bytes[21].bits.b0 = 0;
        seg_bytes[21].bits.b1 = 0;
        seg_bytes[21].bits.b2 = 0;
        seg_bytes[21].bits.b3 = 0;
        seg_bytes[22].bits.b5 = 0;
        seg_bytes[22].bits.b6 = 0;
        seg_bytes[22].bits.b7 = 0;
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_speed(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];

    if (_value < 0){
        segment_set_negative_value(digits, digit_num, _value);
    }else{
        segment_update_digit_value(digits, digit_num, min_digit_show, _value);
    }

    for (uint8_t i = 0; i < digit_num; i++) {
        seg_bytes[14 + i].bits.b3 = digits[i].c;
        seg_bytes[14 + i].bits.b2 = digits[i].g;
        seg_bytes[14 + i].bits.b0 = digits[i].b;
        seg_bytes[15 + i].bits.b7 = digits[i].e;
        seg_bytes[15 + i].bits.b6 = digits[i].f;
        seg_bytes[15 + i].bits.b5 = digits[i].d;
        seg_bytes[15 + i].bits.b4 = digits[i].a;
    }
    if (!_hide){
        for (uint8_t i = 0; i < digit_num; i++){
            seg_bytes[14 + i].bits.b3 = 0;
            seg_bytes[14 + i].bits.b2 = 0;
            seg_bytes[14 + i].bits.b0 = 0;
            seg_bytes[15 + i].bits.b7 = 0;
            seg_bytes[15 + i].bits.b6 = 0;
            seg_bytes[15 + i].bits.b5 = 0;
            seg_bytes[15 + i].bits.b4 = 0;
        }
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}


int32_t sm_drv_lcd_seg_set_speed_neg(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    Digit_bitmap digits[digit_num];
    int32_t value = _value;
    if (value > 0){
        value = 0;
    }else{
        value %= 10;
    }

    segment_set_negative_value(digits, digit_num, value);

    for (uint8_t i = 0; i < digit_num; i++) {
        seg_bytes[14 + i].bits.b3 = digits[i].c;
        seg_bytes[14 + i].bits.b2 = digits[i].g;
        seg_bytes[14 + i].bits.b0 = digits[i].b;
        seg_bytes[15 + i].bits.b7 = digits[i].e;
        seg_bytes[15 + i].bits.b6 = digits[i].f;
        seg_bytes[15 + i].bits.b5 = digits[i].d;
        seg_bytes[15 + i].bits.b4 = digits[i].a;
    }
    if (!_hide){
        for (uint8_t i = 0; i < digit_num; i++){
            seg_bytes[14 + i].bits.b3 = 0;
            seg_bytes[14 + i].bits.b2 = 0;
            seg_bytes[14 + i].bits.b0 = 0;
            seg_bytes[15 + i].bits.b7 = 0;
            seg_bytes[15 + i].bits.b6 = 0;
            seg_bytes[15 + i].bits.b5 = 0;
            seg_bytes[15 + i].bits.b4 = 0;
        }
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_speed_unit(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[0].bits.b7 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_eco_mode(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[15].bits.b1 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_sport_mode(sm_drv_lcd_seg_t *_this, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    seg_bytes[14].bits.b1 = _hide ? 1 : 0;
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp0(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    if (!_hide){
        seg_bytes[12].bits.b5 = 0;
        seg_bytes[12].bits.b0 = 0;
        seg_bytes[12].bits.b2 = 0;
        seg_bytes[12].bits.b3 = 0;
        seg_bytes[12].bits.b1 = 0;
    }else{
        seg_bytes[12].bits.b5 = 1;
        if (_level == SM_DRV_LCD_BP_LEVEL_0){
            seg_bytes[12].bits.b0 = 1;
            seg_bytes[12].bits.b2 = 0;
            seg_bytes[12].bits.b3 = 0;
            seg_bytes[12].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_1){
            seg_bytes[12].bits.b0 = 1;
            seg_bytes[12].bits.b2 = 1;
            seg_bytes[12].bits.b3 = 0;
            seg_bytes[12].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_2){
            seg_bytes[12].bits.b0 = 1;
            seg_bytes[12].bits.b2 = 1;
            seg_bytes[12].bits.b3 = 1;
            seg_bytes[12].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_3){
            seg_bytes[12].bits.b0 = 1;
            seg_bytes[12].bits.b2 = 1;
            seg_bytes[12].bits.b3 = 1;
            seg_bytes[12].bits.b1 = 1;
        }else{
            seg_bytes[12].bits.b0 = 0;
            seg_bytes[12].bits.b2 = 0;
            seg_bytes[12].bits.b3 = 0;
            seg_bytes[12].bits.b1 = 0;
        }
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp0_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];
    int32_t tmp_value = _value;

    if (!_hide){
        seg_bytes[9].bits.b4  = 0;
        seg_bytes[11].bits.b4 = 0;

        seg_bytes[10].bits.b0 = 0;
        seg_bytes[10].bits.b1 = 0;
        seg_bytes[10].bits.b2 = 0;
        seg_bytes[10].bits.b3 = 0;
        seg_bytes[11].bits.b5 = 0;
        seg_bytes[11].bits.b6 = 0;
        seg_bytes[11].bits.b7 = 0;

        seg_bytes[11].bits.b0 = 0;
        seg_bytes[11].bits.b1 = 0;
        seg_bytes[11].bits.b2 = 0;
        seg_bytes[11].bits.b3 = 0;
        seg_bytes[12].bits.b4 = 0;
        seg_bytes[12].bits.b6 = 0;
        seg_bytes[12].bits.b7 = 0;
    }else{
        if (tmp_value > 100){
            tmp_value = 100;
        }
        segment_update_digit_value(digits, digit_num, min_digit_show, tmp_value);
        seg_bytes[9].bits.b4 = 1;

        if (_value >= 100){
            seg_bytes[11].bits.b4 = 1;
        }else{
            seg_bytes[11].bits.b4 = 0;
        }

        seg_bytes[10].bits.b0 = digits[0].d;
        seg_bytes[10].bits.b1 = digits[0].a;
        seg_bytes[10].bits.b2 = digits[0].c;
        seg_bytes[10].bits.b3 = digits[0].b;
        seg_bytes[11].bits.b5 = digits[0].f;
        seg_bytes[11].bits.b6 = digits[0].e;
        seg_bytes[11].bits.b7 = digits[0].g;

        seg_bytes[11].bits.b0 = digits[1].d;
        seg_bytes[11].bits.b1 = digits[1].a;
        seg_bytes[11].bits.b2 = digits[1].c;
        seg_bytes[11].bits.b3 = digits[1].b;
        seg_bytes[12].bits.b4 = digits[1].e;
        seg_bytes[12].bits.b6 = digits[1].g;
        seg_bytes[12].bits.b7 = digits[1].f;
    }
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp0_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    if (!_hide || _value <= 0){
        segment_clear_digit_value(digits, digit_num);
        seg_bytes[10].bits.b5 = 0;

        seg_bytes[8].bits.b0 = 0;
        seg_bytes[8].bits.b1 = 0;
        seg_bytes[8].bits.b2 = 0;
        seg_bytes[8].bits.b3 = 0;
        seg_bytes[9].bits.b5 = 0;
        seg_bytes[9].bits.b6 = 0;
        seg_bytes[9].bits.b7 = 0;

        seg_bytes[9].bits.b0 = 0;
        seg_bytes[9].bits.b1 = 0;
        seg_bytes[9].bits.b2 = 0;
        seg_bytes[9].bits.b3 = 0;
        seg_bytes[10].bits.b4 = 0;
        seg_bytes[10].bits.b6 = 0;
        seg_bytes[10].bits.b7 = 0;
    }else{
        int32_t temp_val = _value;
        segment_update_digit_value(digits, digit_num, min_digit_show, temp_val);
        seg_bytes[10].bits.b5 = 1;

        seg_bytes[8].bits.b0 = digits[1].a;
        seg_bytes[8].bits.b1 = digits[1].d;
        seg_bytes[8].bits.b2 = digits[1].f;
        seg_bytes[8].bits.b3 = digits[1].e;
        seg_bytes[9].bits.b5 = digits[1].c;
        seg_bytes[9].bits.b6 = digits[1].b;
        seg_bytes[9].bits.b7 = digits[1].g;

        seg_bytes[9].bits.b0 = digits[0].a;
        seg_bytes[9].bits.b1 = digits[0].d;
        seg_bytes[9].bits.b2 = digits[0].f;
        seg_bytes[9].bits.b3 = digits[0].e;
        seg_bytes[10].bits.b4 = digits[0].b;
        seg_bytes[10].bits.b6 = digits[0].g;
        seg_bytes[10].bits.b7 = digits[0].c;
    }
//    sm_drv_lcd_update_data(driver);

    return 0;
}

int32_t sm_drv_lcd_seg_set_bp1(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    if (!_hide){
        seg_bytes[8].bits.b5 = 0;
        seg_bytes[13].bits.b4 = 0;
        seg_bytes[13].bits.b6 = 0;
        seg_bytes[13].bits.b7 = 0;
        seg_bytes[13].bits.b5 = 0;
    }else{
        seg_bytes[8].bits.b5 = 1;
        if (_level == SM_DRV_LCD_BP_LEVEL_0){
            seg_bytes[13].bits.b4 = 1;
            seg_bytes[13].bits.b6 = 0;
            seg_bytes[13].bits.b7 = 0;
            seg_bytes[13].bits.b5 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_1){
            seg_bytes[13].bits.b4 = 1;
            seg_bytes[13].bits.b6 = 1;
            seg_bytes[13].bits.b7 = 0;
            seg_bytes[13].bits.b5 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_2){
            seg_bytes[13].bits.b4 = 1;
            seg_bytes[13].bits.b6 = 1;
            seg_bytes[13].bits.b7 = 1;
            seg_bytes[13].bits.b5 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_3){
            seg_bytes[13].bits.b4 = 1;
            seg_bytes[13].bits.b6 = 1;
            seg_bytes[13].bits.b7 = 1;
            seg_bytes[13].bits.b5 = 1;
        }
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp1_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];
    int32_t tmp_value = _value;

    if (!_hide){
        seg_bytes[3].bits.b0 = 0;
        seg_bytes[7].bits.b4 = 0;

        seg_bytes[6].bits.b0 = 0;
        seg_bytes[6].bits.b1 = 0;
        seg_bytes[6].bits.b2 = 0;
        seg_bytes[6].bits.b3 = 0;
        seg_bytes[7].bits.b5 = 0;
        seg_bytes[7].bits.b6 = 0;
        seg_bytes[7].bits.b7 = 0;

        seg_bytes[7].bits.b0 = 0;
        seg_bytes[7].bits.b1 = 0;
        seg_bytes[7].bits.b2 = 0;
        seg_bytes[7].bits.b3 = 0;
        seg_bytes[8].bits.b4 = 0;
        seg_bytes[8].bits.b6 = 0;
        seg_bytes[8].bits.b7 = 0;
    }else{
        if (tmp_value > 100){
            tmp_value = 100;
        }
        segment_update_digit_value(digits, digit_num, min_digit_show, tmp_value);
        seg_bytes[3].bits.b0 = 1;

        if (_value >= 100){
            seg_bytes[7].bits.b4  = 1;
        }else{
            seg_bytes[7].bits.b4  = 0;
        }
        seg_bytes[6].bits.b0 = digits[0].d;
        seg_bytes[6].bits.b1 = digits[0].a;
        seg_bytes[6].bits.b2 = digits[0].c;
        seg_bytes[6].bits.b3 = digits[0].b;
        seg_bytes[7].bits.b5 = digits[0].f;
        seg_bytes[7].bits.b6 = digits[0].e;
        seg_bytes[7].bits.b7 = digits[0].g;

        seg_bytes[7].bits.b0 = digits[1].d;
        seg_bytes[7].bits.b1 = digits[1].a;
        seg_bytes[7].bits.b2 = digits[1].c;
        seg_bytes[7].bits.b3 = digits[1].b;
        seg_bytes[8].bits.b4 = digits[1].e;
        seg_bytes[8].bits.b6 = digits[1].g;
        seg_bytes[8].bits.b7 = digits[1].f;
    }
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp1_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    if (!_hide || _value <= 0){
        segment_clear_digit_value(digits, digit_num);
        seg_bytes[6].bits.b5 = 0;

        seg_bytes[3].bits.b4 = 0;
        seg_bytes[3].bits.b5 = 0;
        seg_bytes[3].bits.b6 = 0;
        seg_bytes[3].bits.b7 = 0;
        seg_bytes[6].bits.b4 = 0;
        seg_bytes[6].bits.b6 = 0;
        seg_bytes[6].bits.b7 = 0;

        seg_bytes[3].bits.b1 = 0;
        seg_bytes[3].bits.b2 = 0;
        seg_bytes[3].bits.b3 = 0;
        seg_bytes[4].bits.b4 = 0;
        seg_bytes[4].bits.b5 = 0;
        seg_bytes[4].bits.b6 = 0;
        seg_bytes[4].bits.b7 = 0;
    }else{
        int32_t temp_val = _value;
        segment_update_digit_value(digits, digit_num, min_digit_show, temp_val);
        seg_bytes[6].bits.b5 = 1;

        seg_bytes[3].bits.b4 = digits[0].a;
        seg_bytes[3].bits.b5 = digits[0].d;
        seg_bytes[3].bits.b6 = digits[0].f;
        seg_bytes[3].bits.b7 = digits[0].e;
        seg_bytes[6].bits.b4 = digits[0].b;
        seg_bytes[6].bits.b6 = digits[0].g;
        seg_bytes[6].bits.b7 = digits[0].c;

        seg_bytes[3].bits.b1 = digits[1].c;
        seg_bytes[3].bits.b2 = digits[1].b;
        seg_bytes[3].bits.b3 = digits[1].g;
        seg_bytes[4].bits.b4 = digits[1].a;
        seg_bytes[4].bits.b5 = digits[1].d;
        seg_bytes[4].bits.b6 = digits[1].f;
        seg_bytes[4].bits.b7 = digits[1].e;
    }
//    sm_drv_lcd_update_data(driver);

    return 0;
}

int32_t sm_drv_lcd_seg_set_bp2(sm_drv_lcd_seg_t *_this, int32_t _level, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    if (!_hide){
        seg_bytes[4].bits.b1 = 0;
        seg_bytes[13].bits.b0 = 0;
        seg_bytes[13].bits.b2 = 0;
        seg_bytes[13].bits.b3 = 0;
        seg_bytes[13].bits.b1 = 0;
    }else{
        seg_bytes[4].bits.b1= 1;
        if (_level == SM_DRV_LCD_BP_LEVEL_0){
            seg_bytes[13].bits.b0 = 1;
            seg_bytes[13].bits.b2 = 0;
            seg_bytes[13].bits.b3 = 0;
            seg_bytes[13].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_1){
            seg_bytes[13].bits.b0 = 1;
            seg_bytes[13].bits.b2 = 1;
            seg_bytes[13].bits.b3 = 0;
            seg_bytes[13].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_2){
            seg_bytes[13].bits.b0 = 1;
            seg_bytes[13].bits.b2 = 1;
            seg_bytes[13].bits.b3 = 1;
            seg_bytes[13].bits.b1 = 0;
        }else if (_level == SM_DRV_LCD_BP_LEVEL_3){
            seg_bytes[13].bits.b0 = 1;
            seg_bytes[13].bits.b2 = 1;
            seg_bytes[13].bits.b3 = 1;
            seg_bytes[13].bits.b1 = 1;
        }else{
            seg_bytes[13].bits.b0 = 0;
            seg_bytes[13].bits.b2 = 0;
            seg_bytes[13].bits.b3 = 0;
            seg_bytes[13].bits.b1 = 0;
        }
    }

//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp2_soc(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];

    const uint8_t digit_num = 3;
    const uint8_t min_digit_show = 1;
    Digit_bitmap digits[digit_num];
    int32_t tmp_value = _value;

    if (!_hide){
        seg_bytes[1].bits.b4 = 0;
        seg_bytes[5].bits.b0 = 0;

        seg_bytes[2].bits.b0 = 0;
        seg_bytes[2].bits.b1 = 0;
        seg_bytes[2].bits.b2 = 0;
        seg_bytes[2].bits.b3 = 0;
        seg_bytes[5].bits.b1 = 0;
        seg_bytes[5].bits.b2 = 0;
        seg_bytes[5].bits.b3 = 0;

        seg_bytes[4].bits.b0 = 0;
        seg_bytes[4].bits.b2 = 0;
        seg_bytes[4].bits.b3 = 0;
        seg_bytes[5].bits.b4 = 0;
        seg_bytes[5].bits.b5 = 0;
        seg_bytes[5].bits.b6 = 0;
        seg_bytes[5].bits.b7 = 0;
    }
    else{
        if (tmp_value > 100){
            tmp_value = 100;
        }
        segment_update_digit_value(digits, digit_num, min_digit_show, tmp_value);
        seg_bytes[1].bits.b4 = 1;

        if (_value >= 100){
            seg_bytes[5].bits.b0 = 1;
        }else{
            seg_bytes[5].bits.b0 = 0;
        }

        seg_bytes[2].bits.b0 = digits[0].d;
        seg_bytes[2].bits.b1 = digits[0].a;
        seg_bytes[2].bits.b2 = digits[0].c;
        seg_bytes[2].bits.b3 = digits[0].b;
        seg_bytes[5].bits.b1 = digits[0].f;
        seg_bytes[5].bits.b2 = digits[0].e;
        seg_bytes[5].bits.b3 = digits[0].g;

        seg_bytes[4].bits.b0 = digits[1].e;
        seg_bytes[4].bits.b2 = digits[1].g;
        seg_bytes[4].bits.b3 = digits[1].f;
        seg_bytes[5].bits.b4 = digits[1].d;
        seg_bytes[5].bits.b5 = digits[1].a;
        seg_bytes[5].bits.b6 = digits[1].c;
        seg_bytes[5].bits.b7 = digits[1].b;
    }
//    sm_drv_lcd_update_data(driver);
    return 0;
}

int32_t sm_drv_lcd_seg_set_bp2_temp(sm_drv_lcd_seg_t *_this, int32_t _value, uint8_t _hide){
    if (!_this)
        return -1;

    sm_drv_lcd_seg_impl_t *driver = _impl(_this);
    Segment_bytes *seg_bytes = &driver->lcd.data_bytes[5];
    const uint8_t digit_num = 2;
    const uint8_t min_digit_show = 2;
    Digit_bitmap digits[digit_num];

    if (!_hide || _value <= 0){
        segment_clear_digit_value(digits, digit_num);
        seg_bytes[2].bits.b5 = 0;

        seg_bytes[1].bits.b0 = 0;
        seg_bytes[1].bits.b1 = 0;
        seg_bytes[1].bits.b2 = 0;
        seg_bytes[1].bits.b3 = 0;
        seg_bytes[2].bits.b4 = 0;
        seg_bytes[2].bits.b6 = 0;
        seg_bytes[2].bits.b7 = 0;

        seg_bytes[0].bits.b0 = 0;
        seg_bytes[0].bits.b1 = 0;
        seg_bytes[0].bits.b2 = 0;
        seg_bytes[0].bits.b3 = 0;
        seg_bytes[1].bits.b5 = 0;
        seg_bytes[1].bits.b6 = 0;
        seg_bytes[1].bits.b7 = 0;
    }else{
        int32_t temp_val = _value;
        segment_update_digit_value(digits, digit_num, min_digit_show, temp_val);
        seg_bytes[2].bits.b5 = 1;

        seg_bytes[1].bits.b0 = digits[0].a;
        seg_bytes[1].bits.b1 = digits[0].d;
        seg_bytes[1].bits.b2 = digits[0].f;
        seg_bytes[1].bits.b3 = digits[0].e;
        seg_bytes[2].bits.b4 = digits[0].b;
        seg_bytes[2].bits.b6 = digits[0].g;
        seg_bytes[2].bits.b7 = digits[0].c;

        seg_bytes[0].bits.b0 = digits[1].a;
        seg_bytes[0].bits.b1 = digits[1].d;
        seg_bytes[0].bits.b2 = digits[1].f;
        seg_bytes[0].bits.b3 = digits[1].e;
        seg_bytes[1].bits.b5 = digits[1].c;
        seg_bytes[1].bits.b6 = digits[1].b;
        seg_bytes[1].bits.b7 = digits[1].g;
    }
//    sm_drv_lcd_update_data(driver);

    return 0;
}

static void sm_drv_lcd_config(sm_drv_lcd_seg_impl_t* _this){
    memcpy(_this->lcd.cmd_bytes, g_lcd_config_cmd, LCD_MAX_CMD_BUFFER);
    uc1676_write(_this->m_driver,(uint8_t*) _this->lcd.cmd_bytes, LCD_MAX_TX_BUFFER);
    memcpy(_this->lcd.data_bytes, g_lcd_init_write_cmd, LCD_MAX_CMD_BUFFER);
}

int32_t sm_drv_lcd_update_data(sm_drv_lcd_seg_t* _this){
    const Segment_bytes* tx_buffer = _impl(_this)->lcd.data_bytes;
    return uc1676_write(_impl(_this)->m_driver, (uint8_t*)tx_buffer, LCD_MAX_TX_BUFFER);
}

