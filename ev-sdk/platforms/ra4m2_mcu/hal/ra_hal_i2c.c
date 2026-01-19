#include "sm_hal_i2c.h"
#include "hal_data.h"

typedef struct {
    i2c_master_instance_t   *m_channel;
    sm_hal_i2c_irq_fn_t     m_cb;
    void                    *m_arg;
}ra_i2c_t;
static ra_i2c_t g_i2c;
#define impl(x)         ((ra_i2c_t*)(x))

sm_hal_i2c_t* sm_hal_i2c_init(void* _channel, uint8_t _addr){
    ra_i2c_t* i2c = &g_i2c;
    i2c->m_channel = _channel;
    R_SCI_I2C_SlaveAddressSet(i2c->m_channel->p_ctrl,
                                                  (uint32_t)_addr,
                                                  I2C_MASTER_ADDR_MODE_7BIT);
    return (sm_hal_i2c_t*)i2c;
}

void sm_hal_i2c_deinit(sm_hal_i2c_t *_this){
    if (!_this) return;
    impl(_this)->m_channel = NULL;
}

void sm_hal_i2c_set_callback(sm_hal_i2c_t *_this, sm_hal_i2c_irq_fn_t _cb, void*_arg){
    if (!_this) return;
    impl(_this)->m_cb = _cb;
    impl(_this)->m_arg = _arg;
}

int32_t sm_hal_i2c_set_addr(sm_hal_i2c_t *_this, uint8_t _addr){
    if (!_this) return -1;
    return (int32_t)R_SCI_I2C_SlaveAddressSet(impl(_this)->m_channel->p_ctrl,
                                              (uint32_t)_addr,
                                              I2C_MASTER_ADDR_MODE_7BIT);
}

int32_t sm_hal_i2c_open(sm_hal_i2c_t *_this){
    if (!_this) return -1;
    return (int32_t)R_SCI_I2C_Open(impl(_this)->m_channel->p_ctrl,impl(_this)->m_channel->p_cfg);
}

int32_t sm_hal_i2c_close(sm_hal_i2c_t *_this){
    if (!_this) return -1;
    return (int32_t)R_SCI_I2C_Close(impl(_this)->m_channel->p_ctrl);
}

int32_t sm_hal_i2c_write(sm_hal_i2c_t *_this, const uint8_t *_data, uint32_t _len){
    if (!_this) return -1;
    return (int32_t)R_SCI_I2C_Write(impl(_this)->m_channel->p_ctrl,
                                    _data,
                                    _len,
                                    true);
}

int32_t sm_hal_i2c_read(sm_hal_i2c_t *_this, uint8_t *_data, uint32_t _len){
    if (!_this) return -1;
    return (int32_t)R_SCI_I2C_Read(impl(_this)->m_channel->p_ctrl,
                                   _data,
                                   _len,
                                   0);
}

int32_t sm_hal_i2c_callback(sm_hal_i2c_t *_this, uint8_t event){
    if (!_this || !impl(_this)->m_cb) return -1;
    impl(_this)->m_cb(_this, event, impl(_this)->m_arg);
    return 0;
}

