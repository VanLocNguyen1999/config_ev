#include "sm_hal_spi.h"
#include "hal_data.h"

typedef struct {
    spi_instance_t  *m_channel;
    void            *m_tmp;
}ra_spi_t;
static ra_spi_t g_spi;
#define impl(x)         ((ra_spi_t*)(x))

sm_hal_spi_t* sm_hal_spi_init(void* _channel){
    if (!_channel){
        return NULL;
    }
    ra_spi_t* spi = &g_spi;
    spi->m_channel = _channel;

    return (sm_hal_spi_t*) spi;
}

void sm_hal_spi_open(sm_hal_spi_t *_this){
    if (!_this){
        return;
    }
    R_SPI_Open(impl(_this)->m_channel->p_ctrl,impl(_this)->m_channel->p_cfg);
}




void sm_hal_spi_deinit(sm_hal_spi_t *_this){
    if (!_this){
        return;
    }
    impl(_this)->m_channel = NULL;
}

int32_t sm_hal_spi_read(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len){
    if (!_this){
        return -1;
    }
    int32_t err = R_SPI_Read(impl(_this)->m_channel->p_ctrl,
                             _buff,
                             _len,
                             SPI_BIT_WIDTH_8_BITS );
    return err ? -1 : 0;
}

int32_t sm_hal_spi_write(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len){
    if (!_this){
        return -1;
    }
    int32_t err = R_SPI_Write(impl(_this)->m_channel->p_ctrl,
                              _buff,
                              _len,
                              SPI_BIT_WIDTH_8_BITS);
    return err ? -1 : 0;
}

int32_t sm_hal_spi_write_read(sm_hal_spi_t *_this, uint8_t *_src, uint8_t *_dest, uint16_t _len){
    if (!_this){
        return -1;
    }
    int32_t err = R_SPI_WriteRead(impl(_this)->m_channel->p_ctrl,
                                _src,
                                _dest,
                                _len,
                                SPI_BIT_WIDTH_8_BITS);
    return err ? -1 : 0;
}
