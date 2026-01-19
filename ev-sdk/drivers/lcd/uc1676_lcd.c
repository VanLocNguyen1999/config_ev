/*
 * uc1676_lcd.c

 *
 *  Created on: Sep 28, 2024
 *      Author: admin
 */

#include "sm_types.h"
#include "uc1676_lcd.h"
#include "sm_hal.h"

#define MAX_I2C_TIMEOUT_mS 100
volatile static bool lcd_state = false;


#define  impl(x)  ((uc1676_impl_t*)(x))

typedef struct {
    sm_hal_i2c_t *m_i2c;
    sm_hal_io_t *m_power;
} uc1676_impl_t;

static void sci_i2c_master_callback(sm_hal_i2c_t* _i2c, uint8_t _event, void* _arg){
    (void) _arg;
    (void) _i2c;
    (void) _event;
    lcd_state = false;
}

static uc1676_impl_t g_uc1676_impl_t_default = {
                                                .m_i2c = NULL,
                                                .m_power = NULL,
};

uc1676_t* uc1676_create(sm_hal_i2c_t* _i2c, sm_hal_io_t *_power){
    if (!_i2c || !_power)
        return NULL;

     uc1676_impl_t* uc1676 = &g_uc1676_impl_t_default;

    uc1676->m_i2c = _i2c;
    uc1676->m_power = _power;
    sm_hal_i2c_set_callback(_i2c, sci_i2c_master_callback, NULL);

    return (uc1676_t*)uc1676;

}

int32_t uc1676_reset(uc1676_t* _this)
{
    sm_hal_io_set_value((impl(_this)->m_power), SM_HAL_IO_OFF);
    sm_hal_delay_ms(100);
    return sm_hal_io_set_value((impl(_this)->m_power), SM_HAL_IO_ON);
}
int32_t uc1676_power_on(uc1676_t* _this){
    return sm_hal_io_set_value(impl(_this)->m_power, SM_HAL_IO_ON);
}

int32_t uc1676_power_off(uc1676_t* _this){
    return sm_hal_io_set_value(impl(_this)->m_power, SM_HAL_IO_OFF);
}

int32_t uc1676_write(uc1676_t* _this, uint8_t *_data, uint32_t _len)
{
    uint32_t timeout = 0;
    do {
        lcd_state = true;
        sm_hal_i2c_write((impl(_this)->m_i2c), _data, _len);

        timeout = 0;
        while (lcd_state) {
            sm_hal_delay_ms(1);
            timeout++;
            if (timeout >= MAX_I2C_TIMEOUT_mS) {
//                uc1676_reset(_this);
                return -1;
            }
        }
    } while (timeout >= MAX_I2C_TIMEOUT_mS);
    return _len;
}





