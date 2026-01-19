//
// Created by vnbk on 05/08/2024.
//

#ifndef EV_SDK_SM_CO_OD_COMMON_H
#define EV_SDK_SM_CO_OD_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define NODE_ID_DEFAULT                                     (0x08)
#define BOOT_MASTER_NODE_ID                                 (0x03)

#define PMU_NODE_ID_DEFAULT                                 (1)
#define MC_NODE_ID_DEFAULT                                  (2)
#define HMI_NODE_ID_DEFAULT                                 (8)
#define BP_NODE_ID_DEFAULT								    (4)
#define BP_NODE_ID_OFFSET                                   (5)

#define SDO_VERSION_INDEX                                   (0x100A)
#define SDO_VERSION_SUB_INDEX                               (0x00)

#define SEGMENT_MEMORY_SIZE                                 (1024)      //Not edit!

#define SDO_BOOTLOADER_INDEX                                (0x2001)    //Not edit!
#define SDO_BOOTLOADER_FW_VERSION_SUB_INDEX                 (0x00)      //Not edit!
#define SDO_BOOTLOADER_FW_SIZE_SUB_INDEX                    (0x01)      //Not edit!
#define SDO_BOOTLOADER_FW_CRC_SUB_INDEX                     (0x02)      //Not edit!
#define SDO_BOOTLOADER_SEG_ADDR_SUB_INDEX                   (0x03)      //Not edit!
#define SDO_BOOTLOADER_SEG_DATA_SUB_INDEX                   (0x04)      //Not edit!
#define SDO_BOOTLOADER_SEG_CRC_SUB_INDEX                    (0x05)      //Not edit!
#define SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX                 (0x06)      //Not edit!
#define SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX               (0x07)      //Not edit!

#define SDO_FILE_TRANSFER_INFO_INDEX                        (0x201A)    //Not edit!
#define SDO_FILE_TRANSFER_INFO_MEMORY_TYPE_SUB_INDEX        (0x00)      //Not edit!
#define SDO_FILE_TRANSFER_INFO_MEMORY_EXT_SUB_INDEX         (0x01)      //Not edit!
#define SDO_FILE_TRANSFER_INFO_START_ADDR_SUB_INDEX         (0x02)      //Not edit!
#define SDO_FILE_TRANSFER_INFO_FILE_SIZE_SUB_INDEX          (0x03)      //Not edit!
#define SDO_FILE_TRANSFER_INFO_FILE_CRC_16_SUB_INDEX        (0x04)      //Not edit!

#define SDO_FILE_TRANSFER_DATA_INDEX                        (0x201B)    //Not edit!
#define SDO_FILE_TRANSFER_DATA_FRAME_ID_SUB_INDEX           (0x00)      //Not edit!
#define SDO_FILE_TRANSFER_DATA_FRAME_SIZE_SUB_INDEX         (0x01)      //Not edit!
#define SDO_FILE_TRANSFER_DATA_FRAME_DATA_SUB_INDEX         (0x02)      //Not edit!
#define SDO_FILE_TRANSFER_DATA_FRAME_CRC_16_SUB_INDEX       (0x03)      //Not edit!

#define BP_VOL_CUR_TPDO_COBID                           CO_CAN_ID_TPDO_1
#define BP_CELLS_VOL_1_TO_4         		            CO_CAN_ID_TPDO_2
#define BP_CELLS_VOL_5_TO_8			                    CO_CAN_ID_TPDO_3
#define BP_CELLS_VOL_9_TO_12							CO_CAN_ID_RPDO_1
#define BP_CELLS_VOL_13_TO_16							CO_CAN_ID_RPDO_2
#define BP_TEMP_TPDO_COBID                              CO_CAN_ID_TPDO_4
#define BP_SOH_CYCLE_COBID                              CO_CAN_ID_RPDO_3

//SDO_CONTROL_VEHICLE SUB-INDEX
#define SDO_PMU_INFO_INDEX                  (0x2301)
#define SDO_PMU_INFO_SUB_INDEX              (0x01)
#define SDO_PMU_READ_INFO_SUB_INDEX         (0x01)
#define SDO_PMU_READ_SN_SUB_INDEX           (0x02)
#define SDO_PMU_CRC_OF_INFO_SUB_INDEX       (0x03)

#define SDO_PMU_CTRL_INDEX                  (0x2002)
#define SDO_PMU_FIND_VEHICLE_SUB_INDEX      (0x00)
#define SDO_PMU_LOCK_VEHICLE_SUB_INDEX      (0x01)
#define SDO_PMU_HORN_CTRL_SUB_INDEX         (0x02)
#define SDO_PMU_EV_BLOCK_SUB_INDEX			(0x03)
#define SDO_PMU_EV_LOCK_SUB_INDEX			(0x04)
#define	SDO_PMU_EV_ANTI_SUB_INDEX			(0x05)

#define SDO_PMU_CTRL_PORT_INDEX               (0x2003)
#define SDO_PMU_LOCK_PORT0_SUB_INDEX				(0x00)
#define SDO_PMU_LOCK_PORT1_SUB_INDEX				(0x01)
#define SDO_PMU_LOCK_PORT2_SUB_INDEX				(0x02)
#define SDO_PMU_ENABLE_PORT0_SUB_INDEX				(0x03)
#define SDO_PMU_ENABLE_PORT1_SUB_INDEX				(0x04)
#define SDO_PMU_ENABLE_PORT2_SUB_INDEX				(0x05)

#define SDO_PMU_CONFIG_INDEX				(0x2004)
#define SDO_PMU_VERIFY_BP_OFFLINE_INDEX		(0x00)
#define SDO_PMU_MAX_CURRENT_SUB_INDEX		(0x01)
#define SDO_PMU_EV_VERSION_SUB_INDEX		(0x02)
#define SDO_PMU_ABP_SUB_INDEX		        (0x03)

#define SDO_PMU_REBOOT_INDEX				(0x2300)
#define SDO_PMU_REBOOT_SUB_INDEX			(0x00)
#define SDO_PMU_FW_REQ_UPDATE_SUB_INDEX		(0x01)
#define SDO_PMU_REQ_EV_UPGRADE_INDEX    	(0x2200)
#define SDO_PMU_REQ_EV_UPGRADE_SUB_INDEX    (0x00)

#define SDO_MC_ANTI_THEFT_ST_INDEX          (0x2100)
#define SDO_MC_ANTI_THEFT_ST_SUB_INDEX      (0x00)
#define SDO_MC_ANTI_THEFT_TIMEOUT_SUB_INDEX (0x01)
#define SDO_MC_DRIVER_MODE_INDEX            (0x2130)
#define SDO_MC_DRIVER_MODE_SUB_INDEX        (0x00)
#define SDO_MC_LIMIT_SPEED_INDEX            (0x2140)
#define SDO_MC_LIMIT_SPEED_SUB_INDEX        (0x00)
#define SDO_MC_STOP_MODE_INDEX              (0x2150)
#define SDO_MC_STOP_MODE_SUB_INDEX          (0x00)

#define SDO_EV_CONFIG_MANU_INDEX                                (0x2800)
#define SDO_EV_CONFIG_MANU_READ_EV_SN_SUB_INDEX                 (0x00)
#define SDO_EV_CONFIG_MANU_WRITE_EV_SN_SUB_INDEX                (0x01)
#define SDO_EV_CONFIG_MANU_VALIDATE_CRC_SUB_INDEX               (0x02)
#define SDO_EV_CONFIG_MANU_READ_LTE_SIMNB_SUB_INDEX             (0x03)
#define SDO_EV_CONFIG_MANU_READ_LTE_IP_SUB_INDEX                (0x04)
#define SDO_EV_CONFIG_MANU_READ_LTE_RSSI_SUB_INDEX              (0x05)
#define SDO_EV_CONFIG_MANU_WRITE_INFO_SUB_INDEX                 (0x06)
#define SDO_EV_CONFIG_MANU_READ_INFO_SUB_INDEX                  (0x07)
#define SDO_EV_CONFIG_MANU_RESET_SETTING_SUB_INDEX              (0x08)

#define SDO_EV_REBOOT_INDEX            (0x2801)
#define SDO_EV_REBOOT_SUB_INDEX        (0)

#define SDO_EV_CONFIG_ODO_INDEX             (0x2802)
#define SDO_EV_CONFIG_ODO_SUB_INDEX         (0x00)

#define SDO_EV_CONFIG_INDEX                     (0x2803)
#define SDO_EV_CONFIG_WRITE_SUB_INDEX           (0x00)
#define SDO_EV_CONFIG_READ_SUB_INDEX            (0x01)
#define SDO_EV_NET_CONFIG_WRITE_SUB_INDEX       (0x02)
#define SDO_EV_NET_CONFIG_READ_SUB_INDEX        (0x03)


#define SDO_TIMEOUT_DEFAULT                 1000

#define SDO_DEVICE_FIRMWARE_NAME            (0x5000)
#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_CO_OD_COMMON_H
