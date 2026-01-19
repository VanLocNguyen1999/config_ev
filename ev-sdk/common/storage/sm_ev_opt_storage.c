//
// Created by vnbk on 28/09/2024.
//
#include "sm_ev_opt_storage.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_utils.h"


#define TAG "SM_EV_OPT_STORAGE"

#define _impl(x)                        ((sm_opt_storage_impl_t*)(x))

#define STORAGE_SIZE                    64
static const uint32_t MAGIC_KEY_VALUE = 0x5A5A5A5A;
#define MAGIC_NUMBER_LENGTH             4
#define CRC_LENGTH                      2
#define HEADER_DATA_LENGHT              (MAGIC_NUMBER_LENGTH + CRC_LENGTH)

typedef struct{
    int32_t m_max_speed;
    int32_t m_driver_mode;
    struct{
        int32_t m_inactive_mode;
        uint32_t m_inactive_storage_odo;
    }m_inactive_data;
    uint8_t m_ble_uuid[32];
}sm_ev_opt_data_t;


typedef struct{
    sm_hal_flash_t* m_flash;
    uint32_t m_start_address;
}sm_opt_storage_impl_t;


sm_opt_storage_impl_t g_opt_storage = {
       .m_flash = NULL,
       .m_start_address = 0
};

static void sm_ev_opt_reset_data(sm_ev_opt_data_t* _data_buff){
    memset(_data_buff, 0, sizeof(sm_ev_opt_data_t));
    _data_buff->m_max_speed = -1;
    _data_buff->m_driver_mode = -1;
    _data_buff->m_inactive_data.m_inactive_mode = -1;
    _data_buff->m_inactive_data.m_inactive_storage_odo = 0;
    _data_buff->m_ble_uuid[0] = '\0';
}


static int32_t sm_ev_opt_load_data(sm_opt_storage_impl_t* this, sm_ev_opt_data_t* _data_buff){
    uint8_t load_buff[STORAGE_SIZE] = {0,};

    sm_hal_flash_read(this->m_flash, this->m_start_address, load_buff, STORAGE_SIZE);

    uint16_t crc = getUint16(load_buff + MAGIC_NUMBER_LENGTH);
    uint32_t magic_val = getUint32(load_buff);

    if(magic_val != MAGIC_KEY_VALUE
            || crc != sm_CRC_CalculateCRC16(load_buff + HEADER_DATA_LENGHT, sizeof(sm_ev_opt_data_t))){

        LOG_WRN(TAG, "EV opt is NOT initialized. Initializing configuration default (= -1) RIGHT NOW");
        sm_ev_opt_reset_data(_data_buff);
        return -1;
    }
    memcpy(_data_buff, load_buff + HEADER_DATA_LENGHT, sizeof(sm_ev_opt_data_t));
    return 0;
}

static int32_t sm_ev_opt_storage_data(sm_opt_storage_impl_t* this, sm_ev_opt_data_t* _data_buff){
    uint8_t load_buff[STORAGE_SIZE] = {0,};
    uint16_t crc = sm_CRC_CalculateCRC16((uint8_t *)_data_buff, sizeof(sm_ev_opt_data_t));

    memcpy(load_buff, &MAGIC_KEY_VALUE, MAGIC_NUMBER_LENGTH);
    memcpy(load_buff + MAGIC_NUMBER_LENGTH, &crc, CRC_LENGTH);
    memcpy(load_buff + HEADER_DATA_LENGHT, _data_buff, sizeof(sm_ev_opt_data_t));

    if(sm_hal_flash_write_block(this->m_flash, this->m_start_address, load_buff, STORAGE_SIZE) < 0){
        LOG_ERR(TAG, "Could NOT write EV opt to flash");
        return -1;
    }
    return 0;
}

sm_opt_storage_t* sm_ev_opt_storage_create_default(sm_hal_flash_t* _flash, uint32_t _start_address){
    g_opt_storage.m_flash = _flash;
    g_opt_storage.m_start_address = _start_address;

    return &g_opt_storage;
}

int32_t sm_ev_opt_clear_storage(sm_storage_t* _this){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return sm_hal_flash_erase_block(this->m_flash, this->m_start_address, STORAGE_SIZE);
}

int32_t sm_ev_opt_load_max_speed(sm_storage_t* _this){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    return load_data.m_max_speed;
}

int32_t sm_ev_opt_store_max_speed(sm_storage_t* _this, int32_t _max_speed){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    load_data.m_max_speed = _max_speed;
    return sm_ev_opt_storage_data(this, &load_data);
}

int32_t sm_ev_opt_load_drive_mode(sm_storage_t* _this){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }


    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    return load_data.m_driver_mode;
}

int32_t sm_ev_opt_store_drive_mode(sm_storage_t* _this, int32_t _driver_mode){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    load_data.m_driver_mode = _driver_mode;
    return sm_ev_opt_storage_data(this, &load_data);
}

int32_t sm_ev_opt_load_inactive_mode(sm_storage_t* _this, uint32_t* _odo_storage){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);
  
    *_odo_storage = load_data.m_inactive_data.m_inactive_storage_odo;
    return load_data.m_inactive_data.m_inactive_mode;
}

int32_t sm_ev_opt_store_inactive_mode(sm_storage_t* _this, int32_t _inactive_mode, uint32_t _odo){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    load_data.m_inactive_data.m_inactive_mode = _inactive_mode;
    load_data.m_inactive_data.m_inactive_storage_odo = _odo;
    return sm_ev_opt_storage_data(this, &load_data);
}

int32_t sm_ev_opt_load_ble_uuid(sm_storage_t* _this, uint8_t* _uuid){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    memcpy(_uuid, load_data.m_ble_uuid, 32);
    return 0;
}

int32_t sm_ev_opt_store_ble_uuid(sm_storage_t* _this, const uint8_t* _uuid, int32_t _len){
    sm_opt_storage_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    sm_ev_opt_data_t load_data;
    sm_ev_opt_load_data(this, &load_data);

    memset(load_data.m_ble_uuid, '\0', 32);
    memcpy(load_data.m_ble_uuid, _uuid, (size_t)_len);
    return sm_ev_opt_storage_data(this, &load_data);
}
