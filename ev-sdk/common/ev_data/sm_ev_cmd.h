//
// Created by vnbk on 17/08/2024.
//

#ifndef EV_SDK_SM_EV_CMD_H
#define EV_SDK_SM_EV_CMD_H

#include "sm_types.h"

#define SM_EV_CMD_SUCCESS   (0)
#define SM_EV_CMD_FAILURE   (-1)

#define SM_EV_CMD_ODO_PASS_LENGTH       16
#define SM_EV_CMD_ASSIGNED_ID_LENGTH    32

enum {
    SM_EV_CMD_CONTROL_LEFT_SIGNAL = 0,
    SM_EV_CMD_CONTROL_RIGHT_SIGNAL,
    SM_EV_CMD_CONTROL_SIGNAL,
    SM_EV_CMD_CONTROL_HIGH_BEAM,
    SM_EV_CMD_CONTROL_LOW_BEAM,
    SM_EV_CMD_CONTROL_HORN,
    SM_EV_CMD_SET_DRIVE_MODE,
    SM_EV_CMD_SET_MAX_SPEED,
    SM_EV_CMD_SET_ODO,
    SM_EV_CMD_FIND_EV,
    SM_EV_CMD_LOCK_EV,
    SM_EV_CMD_BLOCK_EV,
    SM_EV_CMD_ANTI_THEFT_EV,
    SM_EV_CMD_INACTIVE_EV,
    SM_EV_CMD_SET_LOCK_PORT,
    SM_EV_CMD_SET_UNLOCK_PORT,
    SM_EV_CMD_CONFIG_VERIFY_BP_OFFLINE,
    SM_EV_CMD_REBOOT_MODULE,
    SM_EV_CMD_WRITE_DEV_TO_BP,
    SM_EV_CMD_SET_STATE_BP,
    SM_EV_CMD_SET_BLOCK_BP,
    SM_EV_CMD_SET_ACTIVE_BP,
    SM_EV_CMD_SET_CYCLE_BP,
    SM_EV_CMD_PORT_LOCK,
    SM_EV_CMD_PORT_ENABLE,
    SM_EV_CMD_PORT_FORCE,
    SM_EV_CMD_NUMBER
};

typedef struct {
    int32_t m_id;
    const char* m_data;
}sm_cmd_extended_data_t;

typedef struct{
    uint8_t m_slot;
    const char* m_new_version;
    int32_t m_size;
    uint16_t m_crc;
    const char* m_link;
}sm_cmd_upgrade_bp_data_t;

#endif //EV_SDK_SM_EV_CMD_H
