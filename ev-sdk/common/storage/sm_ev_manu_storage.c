//
// Created by vnbk on 28/09/2024.
//
#include "sm_ev_manu_storage.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_utils.h"

#define TAG "SM_EV_MANU_STORAGE"

#define _impl(x)            ((sm_ev_manu_storage_t*)(x))

static int32_t sm_ev_manu_storage_init(sm_storage_t* _this);
static int32_t sm_ev_manu_storage_free(sm_storage_t* _this);
static int32_t sm_ev_manu_storage_load(sm_storage_t* _this, void* _info);
static int32_t sm_ev_manu_storage_store(sm_storage_t* _this, const void* _info);
static int32_t sm_ev_manu_storage_clear(sm_storage_t* _this);

typedef struct{
    sm_storage_t m_base;
    sm_hal_flash_t* m_flash;
    uint32_t m_start_address;
}sm_ev_manu_storage_t;

static sm_storage_proc_t g_ev_manu_storage_proc = {
        .init = sm_ev_manu_storage_init,
        .free = sm_ev_manu_storage_free,
        .load = sm_ev_manu_storage_load,
        .store = sm_ev_manu_storage_store,
        .clear = sm_ev_manu_storage_clear
};

static sm_ev_manu_storage_t g_ev_manu_storage = {
        .m_base.m_proc = &g_ev_manu_storage_proc,
        .m_flash = NULL,
        .m_start_address = 0
};

sm_storage_t* sm_ev_manu_storage_create(sm_hal_flash_t* _data_flash,
                                        uint32_t _start_address){
    if(!_data_flash){
        return NULL;
    }
    g_ev_manu_storage.m_flash = _data_flash;
    g_ev_manu_storage.m_start_address = _start_address;

    return &g_ev_manu_storage.m_base;
}

int32_t sm_ev_manu_storage_validate(sm_storage_t* _this, const uint8_t* _data, int32_t _len){

    (void) _len;
    if(!_this){
        return -1;
    }
    if(memcmp(DEVICE_KEY_DEFAULT, _data, DEVICE_KEY_LENGTH) != 0){
        LOG_ERR(TAG, "EV Manufacture validate wrong KEY");
        return -1;
    }
    uint8_t manu_data[EV_INFO_SIZE] = {0, };
    if(sm_hal_flash_read(_impl(_this)->m_flash, _impl(_this)->m_start_address, manu_data, EV_INFO_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT read EV Manufacture INFO");
        return -1;
    }

    uint16_t crc = sm_CRC_CalculateCRC16(&manu_data[2], EV_INFO_SIZE - 2);
    setUint16(manu_data, crc);

    if(sm_hal_flash_write_page(_impl(_this)->m_flash, _impl(_this)->m_start_address, (void*)manu_data, EV_INFO_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT write EV Manufacture CRC");
        return -1;
    }

    return 1;
}

static int32_t sm_ev_manu_storage_init(sm_storage_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}
static int32_t sm_ev_manu_storage_free(sm_storage_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}
static int32_t sm_ev_manu_storage_load(sm_storage_t* _this, void* _info){
    if(!_this){
        return -1;
    }

    uint8_t load_buff[EV_INFO_SIZE];
    if(sm_hal_flash_read(_impl(_this)->m_flash, _impl(_this)->m_start_address, load_buff, EV_INFO_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT read EV Manufacture INFO");
        return -1;
    }

    volatile uint16_t crc = CO_getUint16(load_buff);

    if(crc == sm_CRC_CalculateCRC16(load_buff + 2, EV_INFO_SIZE - 2)){
        memcpy(_info, load_buff, sizeof(sm_ev_manu_t));
    }else{
        LOG_ERR(TAG, "EV Manufacture INFO crc is not valid");
        return -1;
    }

    return 0;
}

static int32_t sm_ev_manu_storage_store(sm_storage_t* _this, const void* _info){
    if(!_this){
        return -1;
    }

    uint8_t load_buff[EV_INFO_SIZE] = {0,};
    memcpy(load_buff + 2, _info, sizeof(sm_ev_manu_t) - 2);

    if(sm_hal_flash_write_page(_impl(_this)->m_flash, _impl(_this)->m_start_address, (void*)load_buff, EV_INFO_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT write EV Manufacture INFO");
        return -1;
    }
    return 0;
}

static int32_t sm_ev_manu_storage_clear(sm_storage_t* _this){
    if(!_this){
        return -1;
    }

    return sm_hal_flash_erase_block(_impl(_this)->m_flash, _impl(_this)->m_start_address, EV_INFO_SIZE);
}
