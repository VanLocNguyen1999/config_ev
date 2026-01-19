#include <stdlib.h>
#include "hal_data.h"
#include "sm_hal_io.h"

#define MAX_IO_SUPPORT      32

typedef struct {
    uint16_t m_pin;
    uint16_t m_port;
    sm_hal_io_mode_t m_mode;
} ra_io_t;
#define impl(x)         ((ra_io_t*)(x))

static uint8_t  GPIO_OPEN_FLAG = 0;
static ra_io_t g_ra_io[MAX_IO_SUPPORT];
static uint8_t io_numbs_use = 0;

sm_hal_io_t* sm_hal_io_init(uint16_t _pin){
    if (io_numbs_use >= MAX_IO_SUPPORT)
        return NULL;
    ra_io_t* gpio = &g_ra_io[io_numbs_use];
    io_numbs_use++;
    gpio->m_pin = _pin;
    return (sm_hal_io_t*)gpio;
}

void sm_hal_io_deinit(sm_hal_io_t *_this){
    if (!_this) return;
    free(_this);
}

int32_t sm_hal_io_open(sm_hal_io_t *_this, sm_hal_io_mode_t _mode){
    if (!_this) {
        return -1;
    }
    int32_t err;
    if (!GPIO_OPEN_FLAG){
        err = R_IOPORT_Open(g_ioport.p_ctrl, g_ioport.p_cfg);
        if (err) {
            return -1;
        }
        GPIO_OPEN_FLAG = 1;
    }
    impl(_this)->m_mode = _mode;
    uint32_t cfg;
    if (_mode == SM_HAL_IO_INPUT){
        cfg = (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT;
    } else {
        cfg = (uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT;
    }

    err = R_IOPORT_PinCfg(g_ioport.p_ctrl, impl(_this)->m_pin, cfg);
    if (err) {
         return -1;
     }

    return 0;
}

int32_t sm_hal_io_close(sm_hal_io_t *_this){
    if (!_this) {
        return -1;
    }
    int32_t err = R_IOPORT_Close(g_ioport.p_ctrl);

    if (err) {
         return -1;
     }

    return 0;
}

int32_t sm_hal_io_set_value(sm_hal_io_t *_this, uint8_t _value){
    if (!_this) {
        return -1;
    }
    int32_t err = R_IOPORT_PinWrite(g_ioport.p_ctrl, impl(_this)->m_pin, _value);
    if (err) {
            return -1;
        }

   return 0;
}

uint8_t sm_hal_io_get_value(sm_hal_io_t *_this){
    if (!_this) {
        return 0;
    }
    uint8_t ret = 0;
    int32_t err = R_IOPORT_PinRead(g_ioport.p_ctrl, impl(_this)->m_pin, &ret);

    if (err) {
       return 0;
    }
    return ret;
}


