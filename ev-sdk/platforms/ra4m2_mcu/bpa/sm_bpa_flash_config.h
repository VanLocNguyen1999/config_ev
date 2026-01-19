//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_HMI_FLASH_CONFIG_H
#define EV_SDK_SM_HMI_FLASH_CONFIG_H

#include "sm_hal_flash.h"

/* Data Flash */
#define FLASH_HP_DF_BLOCK_0               0x08000000U /*   64 B:  0x08000000 - 0x0800003F */
#define FLASH_HP_DF_BLOCK_1               0x08000040U /*   64 B:  0x08000040 - 0x0800007F */
#define FLASH_HP_DF_BLOCK_2               0x08000080U /*   64 B:  0x08000080 - 0x080000BF */
#define FLASH_HP_DF_BLOCK_3               0x080000C0U /*   64 B:  0x080000C0 - 0x080000FF */
#define FLASH_HP_DF_BLOCK_4               0x08000100U /*   64 B:  0x08000100 - 0x0800013F */
#define FLASH_HP_DF_BLOCK_5               0x08000140U /*   64 B:  0x08000140 - 0x0800017F */
#define FLASH_HP_DF_BLOCK_6               0x08000180U /*   64 B:  0x08000180 - 0x080001BF */
#define FLASH_HP_DF_BLOCK_7               0x080001C0U /*   64 B:  0x080001C0 - 0x080001FF */
#define FLASH_HP_DF_BLOCK_8               0x08000200U /*   64 B:  0x08000200 - 0x0800023F */
#define FLASH_HP_DF_BLOCK_9               0x08000240U /*   64 B:  0x08000240 - 0x0800027F */

#define FLASH_HP_DF_BLOCK_18              0x08000480U /*   64 B:  0x08000480 - 0x080004BF */
#define FLASH_HP_DF_BLOCK_19              0x080004C0U /*   64 B:  0x080004C0 - 0x080004FF */


// Internal Flash config
#define SM_STORAGE_EV_CONFIG_FLASH_ADDR         FLASH_HP_DF_BLOCK_1     //Data block 1 - 64 bytes
#define SM_STORAGE_EV_CONFIG_FLASH_SIZE         64

#define SM_STORAGE_EV_OPT_FLASH_ADDR            FLASH_HP_DF_BLOCK_2     //Data block 2 - 64 bytes
#define SM_STORAGE_EV_OPT_FLASH_SIZE            64

#define SM_STORAGE_EV_INFO_FLASH_ADDR           FLASH_HP_DF_BLOCK_3     //Data block 3 + 4 - 128 bytes
#define SM_STORAGE_EV_INFO_FLASH_SIZE           EV_INFO_SIZE

#define SM_STORAGE_EV_ODO_FLASH_ADDR            FLASH_HP_DF_BLOCK_5     //Data block 5 - 64 bytes
#define SM_STORAGE_EV_ODO_FLASH_SIZE            64

#define SM_STORAGE_NET_CONFIG_FLASH_ADDR        FLASH_HP_DF_BLOCK_6     //Data block 6 - 64 bytes
#define SM_STORAGE_NET_CONFIG_FLASH_SIZE        64

#define SM_STORAGE_FW_SIGNATURE_FLASH_ADDR	    FLASH_HP_DF_BLOCK_19    //Data block 19 - 64 B
#define SM_STORAGE_FW_SIGNATURE_FLASH_SIZE      64

//Exflash config
#define SM_SSL_CA_CERT_STORAGE_ADDR             0x000000U
#define SM_SSL_CA_CERT_STORAGE_SIZE             (4*1024)

#define SM_SSL_CLIENT_CERT_STORAGE_ADDR         0x001000U
#define SM_SSL_CLIENT_CERT_STORAGE_SIZE         (4*1024)

#define SM_SSL_CLIENT_KEY_STORAGE_ADDR          0x002000U
#define SM_SSL_CLIENT_KEY_STORAGE_SIZE          (4*1024)

#define SM_NEW_MAINAPP_STORAGE_ADDR			    0x010000U
#define SM_NEW_MAINAPP_STORAGE_SIZE             (388*1024)

#define SM_DOWNLOADED_HEX_FILE_STORAGE_ADDR     0x071000U
#define SM_DOWNLOADED_FW_STORAGE_SIZE           (572*1024)

#define SM_MAINAPP_DEFAULT_ADDR                 0x10000

#endif //EV_SDK_SM_HMI_FLASH_CONFIG_H
