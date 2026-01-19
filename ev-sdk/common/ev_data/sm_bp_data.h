#ifndef SM_BP_DATA_H
#define SM_BP_DATA_H

#include <stdint.h>
#include <stdio.h>
#include "sm_types.h"

#define SM_BP_NUMBER_DEFAULT            3

#define BP_CELL_TEMP_SIZE               6
#define BP_CELL_VOL_SIZE                16
#define BP_VERSION_SIZE                 32
#define BP_DEVICE_SN_SIZE               32

#define BP_VOL_MAX                      67200 //mV
#define BP_VOL_MIN                      48000 //mV

typedef enum {
    BP_STATE_INIT = 0,
    BP_STATE_IDLE,
    BP_STATE_SOFT_START,
    BP_STATE_DISCHARGING,
    BP_STATE_CHARGING,
    BP_STATE_FAULT,
    BP_STATE_SHIP_MODE,
    BP_STATE_SYSTEM_BOOST_UP,
    BP_STATE_ID_ASSIGN_START,
    BP_STATE_ID_ASSIGN_WAIT_CONFIRM,
    BP_STATE_ID_ASSIGN_CONFIRMED,
    BP_STATE_ID_ASSIGN_WAIT_SLAVE_SELECT,
    BP_STATE_START_AUTHENTICATE,
    BP_STATE_AUTHENTICATING,
    BP_STATE_STANDBY,
    BP_STATE_SHUTDOWN,
    BP_STATE_ONLY_DISCHARGING,
    BP_STATE_SOFT_START_CHARGE,
    BP_STATE_SOFT_START_DISCHARGE,
    BP_STATE_FORCE_DISCHARGE,
	BP_ST_RETAIN_DISCHARGE = 21
} BP_STATE;

typedef enum {
    BP_STT_OK       = 0,
    BP_STT_RSVD_0   = 1,
    BP_STT_CUV      = 4,
    BP_STT_COV      = 8,
    BP_STT_OCC      = 16,
    BP_STT_OCD1     = 32,
    BP_STT_OCD2     = 64,
    BP_STT_SCD      = 128
}BP_STATUS;

typedef struct sm_bp_data{
    char m_sn[BP_DEVICE_SN_SIZE];
    int32_t m_vol;
    int32_t m_cur;
    BP_STATE    m_state;
    BP_STATUS   m_status;
    int32_t m_soc;
    int32_t m_soh;
    int32_t m_cycle;
    uint8_t m_temps[BP_CELL_TEMP_SIZE];
    uint16_t m_cellVols[BP_CELL_VOL_SIZE];

    uint8_t m_nfc_state;
    uint8_t m_active_mode;
    uint8_t m_block_mode;
    uint16_t m_cycle_decimal;

    char m_version[BP_VERSION_SIZE];
    char m_assignedSn[BP_DEVICE_SN_SIZE];
}sm_bp_data_t;

static inline void sm_bp_reset_data(sm_bp_data_t* _this){
    memset(_this->m_sn, '\0', BP_DEVICE_SN_SIZE);
    _this->m_vol = 0;
    _this->m_cur = 0;
    _this->m_state = 0;
    _this->m_status = 0;
    _this->m_soc = -1;
    _this->m_soh = -1;
    _this->m_cycle = -1;
    _this->m_nfc_state = 0;
    _this->m_block_mode = 0;
    _this->m_active_mode = 0;
    _this->m_cycle_decimal = 0;

    memset(_this->m_temps, 0, BP_CELL_TEMP_SIZE);
    memset(_this->m_cellVols, 0, sizeof(_this->m_cellVols));

    memset(_this->m_version, '\0', BP_VERSION_SIZE);
    memset(_this->m_assignedSn, '\0', BP_DEVICE_SN_SIZE);
}

static inline void sm_bp_clone_data(sm_bp_data_t* _this, const sm_bp_data_t* _other){
    if(!_this || !_other){
        return;
    }
    memcpy(_this->m_sn, _other->m_sn, BP_DEVICE_SN_SIZE);

    _this->m_vol = _other->m_vol;
    _this->m_cur = _other->m_cur;
    _this->m_state = _other->m_state;
    _this->m_status = _other->m_status;
    _this->m_soc = _other->m_soc;
    _this->m_soh = _other->m_soh;
    _this->m_cycle = _other->m_cycle;
    _this->m_nfc_state = _other->m_nfc_state;
    _this->m_block_mode = _other->m_block_mode;
    _this->m_active_mode = _other->m_active_mode;
    _this->m_cycle_decimal = _other->m_cycle_decimal;

    memcpy(_this->m_temps, _other->m_temps, BP_CELL_TEMP_SIZE);
    memcpy(_this->m_cellVols, _other->m_cellVols, BP_CELL_VOL_SIZE);

    memcpy(_this->m_version, _other->m_version, BP_VERSION_SIZE);
    memcpy(_this->m_assignedSn, _other->m_assignedSn, BP_DEVICE_SN_SIZE);   
}

#endif
