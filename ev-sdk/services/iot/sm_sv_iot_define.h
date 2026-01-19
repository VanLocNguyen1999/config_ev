//
// Created by vnbk on 04/09/2024.
//

#ifndef EV_SDK_SM_SV_IOT_DEFINE_H
#define EV_SDK_SM_SV_IOT_DEFINE_H

/// IOT Command
#define SM_IOT_CMD_MAX          64

#define SM_IOT_CMD_EV_LOCK      "lock"
#define SM_IOT_CMD_EV_BLOCK     "block"

#define SM_IOT_CMD_FIND_EV      "find_ev"
#define SM_IOT_CMD_DRIVE_MODE   "drive_mode"
#define SM_IOT_CMD_MAX_SPEED    "max_speed"

#define SM_IOT_CMD_EV_ACTIVE                "active_ev"
#define SM_IOT_CMD_LOCK_PORT                "lock_port"
#define SM_IOT_CMD_UNLOCK_PORT              "unlock_port"
#define SM_IOT_CONFIG_VERIFY_BP_OFFLINE     "verify_bp_offline"

#define SM_IOT_CMD_BP                       "bp"
#define SM_IOT_CMD_ASSIGN_BP                "assign"
#define SM_IOT_CMD_UPGRADE_BP               "upgrade"
#define SM_IOT_CMD_ASSIGNED_DEV             "dev_sn"

#define SM_IOT_CMD_REBOOT_MODULE            "reboot_module"
#define SM_IOT_CMD_SET_ODO                  "odo"

#define SM_IOT_CMD_MODULE                   "module"
#define SM_IOT_CMD_MODULE_REBOOT            "reboot"
#define SM_IOT_CMD_MODULE_VERSION           "version"

#define SM_IOT_CMD_PORT                     "port"
#define SM_IOT_CMD_PORT_LOCK                "lock"
#define SM_IOT_CMD_PORT_ENABLE              "enable"
#define SM_IOT_CMD_PORT_FORCE               "force"

#define SM_IOT_MSG_ID_FIELD             "msg_id"
#define SM_IOT_MSG_ID_DEFAULT           "abc-xyz-123-987"

#define SM_IOT_TYPE_FIELD               "type"
#define SM_IOT_DATA_FIELD               "data"
#define SM_IOT_BUSY_FIELD               "busy"
#define SM_IOT_VERSION_FIELD            SM_IOT_CMD_MODULE_VERSION
#define SM_IOT_DEV_TYPE_FILED           "device_type"
#define SM_IOT_MODEL_FILED              "model"
#define SM_IOT_FW_NEW_VERSION_FIELD     "new_version"
#define SM_IOT_FW_OLD_VERSION_FIELD     "old_version"
#define SM_IOT_FW_LINK_FIELD            "link"
#define SM_IOT_FW_SIZE_FIELD            "size"
#define SM_IOT_FW_CRC_FIELD             "crc16"
#define SM_IOT_FW_MODULE_FIELD          "module"
#define SM_IOT_FW_MODULES_FILED         "modules"

#define SM_IOT_ERROR_FIELD              "error"
#define SM_IOT_ERROR_MSG_FIELD          "error_msg"

#define SM_IOT_FW_TOTAL_FRAME_FIELD         "total_frame"
#define SM_IOT_FW_DOWNLOADED_FRAME_FIELD    "downloaded_frame"
#define SM_IOT_FW_DOWNLOADED_IS_DONE_FIELD  "is_done"

#define SM_IOT_OTA_TYPE_PING_REQUEST        "PING_REQUEST"
#define SM_IOT_OTA_TYPE_PING_RESPONSE       "PING_RESPONSE"
#define SM_IOT_OTA_TYPE_FW_INFO             "FW_INFO"
#define SM_IOT_OTA_TYPE_FW_INFO_RESPONSE    "FW_INFO_RESPONSE"
#define SM_IOT_OTA_TYPE_DOWNLOADING         "DOWNLOADING"
#define SM_IOT_OTA_TYPE_DOWNLOAD_STATUS     "DOWNLOAD_STATUS"
#define SM_IOT_OTA_TYPE_UPGRADE_STATUS      "UPGRADING_STATUS"

#define SM_IOT_DEVICE_TYPE_EV               "ev"
#define SM_IOT_MODEL_TYPE_S2                "s2"
#define SM_IOT_MODULE_MC                    "mc"
#define SM_IOT_MODULE_PMU                   "pmu"
#define SM_IOT_MODULE_HMI                   "hmi"

#define SM_IOT_DEVICE_TYPE_ADAPTER          "adt"
#define SM_IOT_MODEL_TYPE_BPA               "bpa"

#define SM_IOT_DEVICE_TYPE_BP               "bp"

#define SM_IOT_KEY_FIELD                    "key"
#define SM_IOT_PARKING_FIELD                "parking"
#define SM_IOT_DRIVE_MODE_FIELD             SM_IOT_CMD_DRIVE_MODE
#define SM_IOT_RANGE_FIELD                  "range"
#define SM_IOT_ANTI_THEFT_STATUS_FIELD      "anti-theft"
#define SM_IOT_LOCK_MODE_FIELD              SM_IOT_CMD_EV_LOCK
#define SM_IOT_LOCK_STEP_FIELD              "lock_step"
#define SM_IOT_BLOCK_MODE_FIELD             SM_IOT_CMD_EV_BLOCK
#define SM_IOT_BLOCK_STEP_FIELD             "block_step"
#define SM_IOT_ACTIVE_MODE_FIELD            "active"
#define SM_IOT_MOTOR_STATE_FIELD            "motor_state"
#define SM_IOT_STATUS_FIELD                 "status"
#define SM_IOT_INPUT_VOLTAGE_FIELD          "input_vol"

#define SM_IOT_TRIP_FIELD                   "trip"
#define SM_IOT_ODO_FIELD                    SM_IOT_CMD_SET_ODO
#define SM_IOT_SPEED_FIELD                  "speed"
#define SM_IOT_MAX_SPEED_FIELD              SM_IOT_CMD_MAX_SPEED

/// EV Configure
#define SM_IOT_LOAD_CONFIG_REQUEST          "LOAD_CONFIG"
#define SM_IOT_LOAD_CONFIG_RESPONSE         "LOAD_CONFIG_RES"
#define SM_IOT_SET_CONFIG_REQUEST           "SET_CONFIG"
#define SM_IOT_SET_CONFIG_RESPONSE          "SET_CONFIG_RES"

#define SM_IOT_CONF_AUTH_BP                 "auth_bp"
#define SM_IOT_CONF_AUTH_BP_ONLINE          "online"
#define SM_IOT_CONF_AUTH_BP_OFFLINE         "offline"

#define SM_IOT_CONF_UPHILL_MODE             "uphill_mode"
#define SM_IOT_CONF_LOCK_PORT               "lock_port"
#define SM_IOT_CONF_INACTIVE_MODE           "inactive_mode"
#define SM_IOT_CONF_INACTIVE_MODE_KM_WARNING    "km_warning"
#define SM_IOT_CONF_INACTIVE_MODE_KM_STOP       "km_stop"

#define SM_IOT_CONF_WHEEL_RADIUS            "wheel_radius"
#define SM_IOT_CONF_KM_ODO_STORED           "km_odo_stored"
#define SM_IOT_CONF_ODO_PASS                "odo_pass"

#define SM_IOT_CONF_SYNC_TIME               "sync_time"

#define SM_IOT_CONF_AUTH_MODULE                 "auth_module"
#define SM_IOT_CONF_AUTH_MODULE_LEVEL           "level"
#define SM_IOT_CONF_AUTH_MODULE_DETECTED_TIME   "detected_time"

#define SM_IOT_COMMON_STATE_FIELD           "state"
#define SM_IOT_COMMON_REPORTED_FIELD        "reported"
#define SM_IOT_COMMON_SN_FIELD              "sn"

#define SM_IOT_FW_VERSION                   "fw_ver"

#endif //EV_SDK_SM_SV_IOT_DEFINE_H
