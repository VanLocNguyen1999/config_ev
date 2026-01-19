/*
 * sm_ev_odo_storage.c
 *
 *  Created on: Oct 25, 2024
 *      Author: vuonglk
 */
#include "sm_ev_odo_storage.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_utils.h"
#include "sm_math.h"
#include <string.h>

#define TAG "sm_ev_odo_storage"

#define _impl(x) (sm_odo_storage_impl_t*)(x)

#define ODO_BUFFER_SIZE                 64
static const uint32_t MAGIC_KEY_VALUE   = 0x5A5A5A5A;
#define MAGIC_NUMBER_POS                0       // 4byte
#define REAL_ODO_VAL_POS                4       // 4 byte
#define TWO_COMPL_ODO_VAL_POS           8       // 4 byte
#define CRC_BUFFER_POS                  12      // 2 byte - of real val + two's compl val

typedef struct{
    sm_hal_flash_t* m_flash;
    uint32_t m_start_address;
}sm_odo_storage_impl_t;

sm_odo_storage_impl_t g_odo_storage = {
       .m_flash = NULL,
       .m_start_address = 0
};

sm_odo_storage_t* sm_ev_odo_storage_create_default(sm_hal_flash_t* _flash, uint32_t _address){
    g_odo_storage.m_flash = _flash;
    g_odo_storage.m_start_address = _address;

    return &g_odo_storage;
}

int32_t sm_ev_odo_storage_load(sm_odo_storage_t* _this, uint32_t* _odo_storage){
    sm_odo_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    uint8_t load_buff[ODO_BUFFER_SIZE] = {0,};
    sm_hal_flash_read(this->m_flash, this->m_start_address, load_buff, ODO_BUFFER_SIZE);

    uint32_t magic_val = getUint32(load_buff + MAGIC_NUMBER_POS);
    uint16_t crc = getUint16(load_buff + CRC_BUFFER_POS);

    volatile uint32_t real_odo = getUint32(load_buff + REAL_ODO_VAL_POS);
    volatile uint32_t two_compl_odo = getUint32(load_buff + TWO_COMPL_ODO_VAL_POS);


    if(magic_val != MAGIC_KEY_VALUE
            || two_compl_odo != sm_math_get_two_complement_value(real_odo)
            || crc != sm_CRC_CalculateCRC16(load_buff + REAL_ODO_VAL_POS, 8)){

        LOG_WRN(TAG, "EV new odo is NOT initialized");
        return -1;
    }

    LOG_INF(TAG, "New odo loaded is %d", real_odo);
    *_odo_storage = real_odo;
    return 0;
}

int32_t sm_ev_odo_storage_store(sm_odo_storage_t* _this, uint32_t _odo){
    sm_odo_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    volatile uint8_t load_buff[ODO_BUFFER_SIZE] = {0,};
    volatile uint32_t two_compl_odo = sm_math_get_two_complement_value(_odo);

    memcpy((uint8_t*)load_buff + MAGIC_NUMBER_POS, &MAGIC_KEY_VALUE, 4);
    memcpy((uint8_t*)load_buff + REAL_ODO_VAL_POS, &_odo, 4);
    memcpy(((uint8_t*)load_buff) + TWO_COMPL_ODO_VAL_POS, (uint8_t*)&two_compl_odo, 4);

    uint16_t crc = sm_CRC_CalculateCRC16((uint8_t*)load_buff + REAL_ODO_VAL_POS, 8);

    memcpy(((uint8_t*)load_buff) + CRC_BUFFER_POS, &crc, 2);


    if(sm_hal_flash_write_block(this->m_flash, this->m_start_address, (uint8_t *)load_buff, ODO_BUFFER_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT write new odo to flash");
        return -1;
    }
    return 0;
}

int32_t sm_ev_odo_clear(sm_odo_storage_t* _this){
    sm_odo_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return sm_hal_flash_erase_block(this->m_flash, this->m_start_address, ODO_BUFFER_SIZE);
}
