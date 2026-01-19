//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_PMU_FLASH_CONFIG_H
#define EV_SDK_SM_PMU_FLASH_CONFIG_H

#include "sm_hal_flash.h"

/* 8k flash layout
._____________________________________________.
|_____Block_____|_____Config_____|____Size____|
|   block0-15   |    sys_param   |     1K     |
|---------------|----------------|------------|
|   block16-17  | boot1_setting  |    128B    |    => DO NOT EDIT !
|---------------|----------------|------------|
|   block18-19  | boot2_setting  |    128B    |    => DO NOT EDIT !
|---------------|----------------|------------|
|   block20-35  |    boot1_OD    |     1K     |    => DO NOT EDIT !
|---------------|----------------|------------|
|   block36-51  |    boot2_OD    |     1K     |    => DO NOT EDIT !
|---------------|----------------|------------|
|   block52-83  |    MAIN_APP    |            |
----------------------------------------------
*/

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
#define FLASH_HP_DF_BLOCK_10              0x08000280U /*   64 B:  0x08000280 - 0x080002BF */


// Internal Flash config
#define SM_STORAGE_PMU_MANU_INFO_FLASH_ADDR             FLASH_HP_DF_BLOCK_2
#define SM_STORAGE_PMU_MANU_INFO_FLASH_SIZE             192 // BLOCK_2 + 3 + 4

#define SM_STORAGE_PMU_CONFIG_FLASH_ADDR                FLASH_HP_DF_BLOCK_9
#define SM_STORAGE_PMU_CONFIG_FLASH_SIZE                64

#define SM_STORAGE_PMU_EV_DATA_FLASH_ADDR               FLASH_HP_DF_BLOCK_10
#define SM_STORAGE_PMU_EV_DATA_FLASH_SIZE               64
#endif //EV_SDK_SM_PMU_FLASH_CONFIG_H
