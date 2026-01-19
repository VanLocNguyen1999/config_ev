//
// Created by vnbk on 28/09/2024.
//
#include "sm_ev_sec_storage.h"
#include "sm_logger.h"
#include "sm_memory.h"

#define TAG "SM_EV_SEC_STORAGE"

#define _impl(x)                ((sm_ev_sec_storage_t*)(x))

#define DEVICE_PASS_LEN                     (9)
static const char* DEVICE_PASS_DEFAULT   =   "selex123@";

typedef struct{
    sm_storage_t m_base;
    sm_hal_flash_t* m_flash;
    uint32_t m_start_address;
    uint32_t m_config_size;
}sm_ev_sec_storage_t;

static int32_t sm_ev_config_init(sm_storage_t* _this);
static int32_t sm_ev_config_free(sm_storage_t* _this);
static int32_t sm_ev_config_clear(sm_storage_t* _this);

static sm_storage_proc_t g_ev_storage_proc_default = {
        .init = sm_ev_config_init,
        .free = sm_ev_config_free,
        .load = NULL,
        .store = NULL,
        .clear = sm_ev_config_clear
};

static sm_ev_sec_storage_t g_ev_config_storage = {
        .m_base.m_proc = &g_ev_storage_proc_default,
        .m_flash = NULL,
        .m_start_address = 0x00,
        .m_config_size = 0
};


sm_sec_storage_t* sm_ev_security_storage_create_default(sm_hal_flash_t* _flash, uint32_t _address, uint32_t _size){
    if(!_flash){
        return NULL;
    }

    sm_ev_sec_storage_t* this = &g_ev_config_storage;
    this->m_flash = _flash;
    this->m_start_address = _address;
    this->m_config_size = _size;
    this->m_base.m_proc = &g_ev_storage_proc_default;

    return &this->m_base;
}

sm_sec_storage_t* sm_ev_security_storage_create(sm_hal_flash_t* _flash, uint32_t _address, uint32_t _size){
    if(!_flash){
        return NULL;
    }

    sm_ev_sec_storage_t* this = mem_alloc(sizeof(sm_ev_sec_storage_t));
    this->m_flash = _flash;
    this->m_start_address = _address;
    this->m_config_size = _size;
    this->m_base.m_proc = &g_ev_storage_proc_default;

    return &this->m_base;
}

int32_t sm_ev_security_storage_load(sm_sec_storage_t* _this, const char* _pass, void* _data){
    if(!_this){
        return -1;
    }

    sm_ev_sec_storage_t* this = _impl(_this);
    if(memcmp(_pass, DEVICE_PASS_DEFAULT, DEVICE_PASS_LEN) != 0){
        LOG_ERR(TAG, "Password WRONG");
        return -1;
    }

    volatile int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
    volatile int32_t buff_size = (int32_t)this->m_config_size;

    if(buff_size % block_size){
        buff_size = (buff_size/block_size + 1)*block_size;
    }else{
        buff_size = (buff_size/block_size)*block_size;
    }

    uint8_t config_data[buff_size];

    if(sm_hal_flash_read(this->m_flash, this->m_start_address, config_data, buff_size) < 0){
        LOG_ERR(TAG, "Could NOT read Security Flash");
        return -1;
    }

    memcpy(_data, config_data, this->m_config_size);
    return (buff_size/block_size); // block wrote
}

int32_t sm_ev_security_storage_store(sm_sec_storage_t* _this, const char* _pass, const void* _data){
    if(!_this){
        return -1;
    }

    if(memcmp(_pass, DEVICE_PASS_DEFAULT, DEVICE_PASS_LEN) != 0){
        LOG_ERR(TAG, "Password WRONG");
        return -1;
    }

    volatile int32_t block_size = sm_hal_flash_get_block_size(_impl(_this)->m_flash);
    volatile int32_t buff_size = (int32_t)_impl(_this)->m_config_size;

    if(buff_size % block_size){
        buff_size = (buff_size/block_size + 1)*block_size;
    }else{
        buff_size = (buff_size/block_size)*block_size;
    }

    uint8_t config_data[buff_size];
    memcpy(config_data, _data, _impl(_this)->m_config_size);

    if(sm_hal_flash_write_page(_impl(_this)->m_flash, _impl(_this)->m_start_address, config_data, buff_size) < 0){
        LOG_ERR(TAG, "Could NOT Write Security Flash");
        return -1;
    }

    return (buff_size/block_size); // block wrote
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
    mem_free(_this);
    return 0;
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
