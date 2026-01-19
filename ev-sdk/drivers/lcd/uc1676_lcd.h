//
// Created by vnbk on 09/06/2023.
//

#ifndef NEW_S2_HMI_UC1766_LCD_H
#define NEW_S2_HMI_UC1766_LCD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sm_hal_i2c.h"
#include "sm_hal_io.h"

#define LCD_MAX_RAM_ADDRESS      80
#define LCD_MAX_DATA_POINTER     (LCD_MAX_RAM_ADDRESS / 2)
#define LCD_MAX_CMD_BUFFER       5
#define LCD_MAX_TX_BUFFER        (LCD_MAX_DATA_POINTER + LCD_MAX_CMD_BUFFER)

typedef void uc1676_t;

uc1676_t* uc1676_create(sm_hal_i2c_t* _i2c, sm_hal_io_t *_power);

int32_t uc1676_reset(uc1676_t * _this);
int32_t uc1676_power_on(uc1676_t* _this);
int32_t uc1676_power_off(uc1676_t* _this);

int32_t uc1676_write(uc1676_t* _this, uint8_t *_data, uint32_t _len);

#ifdef __cplusplus
};
#endif

#endif //NEW_S2_HMI_UC1766_LCD_H
