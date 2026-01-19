//
// Created by vnbk on 28/09/2024.
//
#include "sm_ev_storage.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_utils.h"
#include "sm_memory.h"

#define TAG "SM_EV_CONFIG_STORAGE"

#define _impl(x)                ((sm_config_storage_t*)(x))

#define MAGIC_NUMBER_LENGTH     4
#define CRC_LENGTH              2

#define HEADER_CONFIG_DATA_SIZE     (MAGIC_NUMBER_LENGTH + CRC_LENGTH)

static const uint32_t MAGIC_NUMBER  = 0x5A5A5A5A;

typedef struct {
    sm_storage_t m_base;
    sm_hal_flash_t* m_flash;
    uint32_t m_start_address;
    volatile uint32_t m_config_size;
}sm_config_storage_t;

static int32_t sm_ev_config_init(sm_storage_t* _this);
static int32_t sm_ev_config_free(sm_storage_t* _this);
static int32_t sm_ev_config_load(sm_storage_t* _this, void* _config);
static int32_t sm_ev_config_store(sm_storage_t* _this, const void* _config);
static int32_t sm_ev_config_clear(sm_storage_t* _this);

static sm_storage_proc_t g_ev_storage_proc_default = {
        .init = sm_ev_config_init,
        .free = sm_ev_config_free,
        .load = sm_ev_config_load,
        .store = sm_ev_config_store,
        .clear = sm_ev_config_clear
};

static sm_config_storage_t g_ev_config_storage = {
        .m_base.m_proc = &g_ev_storage_proc_default,
        .m_flash = NULL,
        .m_start_address = 0x00,
        .m_config_size = 0
};

sm_storage_t* sm_ev_config_create_default(sm_hal_flash_t* _data_flash,
                                          uint32_t _start_address,
                                          uint32_t _config_size){
    if(!_data_flash){
        return NULL;
    }
    g_ev_config_storage.m_flash = _data_flash;
    g_ev_config_storage.m_start_address = _start_address;
    g_ev_config_storage.m_config_size = _config_size;

    return &g_ev_config_storage.m_base;
}

sm_storage_t* sm_ev_config_create(sm_hal_flash_t* _data_flash,
                                  uint32_t _start_address,
                                  uint32_t _config_size){
    if(!_data_flash){
        return NULL;
    }

    sm_config_storage_t* config_storage = (sm_config_storage_t*) mem_alloc(sizeof(sm_config_storage_t));
    if(!config_storage){
        LOG_ERR(TAG, "Could NOT Allocated memory for create configure");
        return NULL;
    }

    config_storage->m_flash = _data_flash;
    config_storage->m_start_address = _start_address;
    config_storage->m_config_size = _config_size;
    config_storage->m_base.m_proc = &g_ev_storage_proc_default;

    return &config_storage->m_base;
}

static int32_t sm_ev_config_init(sm_storage_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

static int32_t sm_ev_config_free(sm_storage_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_flash = NULL;
    mem_free(_this);

    return 0;
}

static int32_t sm_ev_config_load(sm_storage_t* _this, void* _config){
    if(!_this || !_config){
        return -1;
    }

    volatile int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
    volatile int32_t buff_size = (int32_t)(HEADER_CONFIG_DATA_SIZE + _impl(_this)->m_config_size);

    if(buff_size % block_size){
        buff_size = (buff_size/block_size + 1)*block_size;
    }else{
        buff_size = (buff_size/block_size)*block_size;
    }

    uint8_t config_data[buff_size];

    if(sm_hal_flash_read(_impl(_this)->m_flash, _impl(_this)->m_start_address, config_data, buff_size) < 0){
        LOG_ERR(TAG, "Could NOT load EV Config");
        return -1;
    }
    if(memcmp(config_data, &MAGIC_NUMBER, MAGIC_NUMBER_LENGTH) != 0){
        LOG_WRN(TAG, "EV config is NOT initialized. Initializing configuration default RIGHT NOW");
        return -1;
    }

    if(getUint16(&config_data[MAGIC_NUMBER_LENGTH]) != sm_CRC_CalculateCRC16(&config_data[HEADER_CONFIG_DATA_SIZE], _impl(_this)->m_config_size)){
        LOG_WRN(TAG, "EV config is initialized. BUT Initialized value INVALID.");
        memcpy(_config, config_data + HEADER_CONFIG_DATA_SIZE, _impl(_this)->m_config_size);
        return -2;
    }

    memcpy(_config, config_data + HEADER_CONFIG_DATA_SIZE, _impl(_this)->m_config_size);

    return (buff_size/block_size); // block wrote
}

static int32_t sm_ev_config_store(sm_storage_t* _this, const void* _config){
    if(!_this || !_config){
        return -1;
    }
    volatile int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
    volatile int32_t buff_size =(int32_t)(HEADER_CONFIG_DATA_SIZE + _impl(_this)->m_config_size);

    if(buff_size % block_size){
        buff_size = (buff_size/block_size + 1)*block_size;
    }else{
        buff_size = (buff_size/block_size)*block_size;
    }

    uint8_t config_data[buff_size];

    uint16_t crc = sm_CRC_CalculateCRC16((uint8_t*)_config, _impl(_this)->m_config_size);

    memcpy(&config_data[0], &MAGIC_NUMBER, MAGIC_NUMBER_LENGTH);
    memcpy(&config_data[MAGIC_NUMBER_LENGTH], &crc, CRC_LENGTH);
    memcpy(&config_data[HEADER_CONFIG_DATA_SIZE], _config, _impl(_this)->m_config_size);

    if(sm_hal_flash_write_block(_impl(_this)->m_flash, _impl(_this)->m_start_address, config_data, buff_size) < 0){
        LOG_ERR(TAG, "Could NOT write EV Configuration to flash");
        return -1;
    }

    return (buff_size/block_size); // block wrote
}

static int32_t sm_ev_config_clear(sm_storage_t* _this){
    if(!_this){
        return -1;
    }

    volatile int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
    volatile int32_t buff_size = (int32_t)_impl(_this)->m_config_size;

    if(buff_size % block_size){
        buff_size = (buff_size/block_size + 1)*block_size;
    }else{
        buff_size = (buff_size/block_size)*block_size;
    }

    return sm_hal_flash_erase_page(_impl(_this)->m_flash, _impl(_this)->m_start_address, buff_size);
}

