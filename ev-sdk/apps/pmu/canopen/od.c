/*
 * od.c
 *
 *  Created on: Oct 12, 2022
 *      Author: Dakaka
 */


#include "od.h"
#include "sm_co_od_common.h"

/*--------------------------------------------------------------------------------------------*/
CO_Sub_Object_Ext_t g_cfg_verify_bp_ext;
CO_Sub_Object_Ext_t g_cfg_max_cur_mc_ext;
CO_Sub_Object_Ext_t g_cfg_ev_version_ext;
CO_Sub_Object_Ext_t g_cfg_ev_abp_ext;

CO_Sub_Object g_cfg_pmu[] = {
        {(void*)NULL,  ODA_SDO_RW,       1, &g_cfg_verify_bp_ext, NULL},
        {(void*)NULL,  ODA_SDO_RW,       1, &g_cfg_max_cur_mc_ext, NULL},
        {(void*)NULL,  ODA_SDO_RW,       1, &g_cfg_ev_version_ext, NULL},
        {(void*)NULL,  ODA_SDO_RW,       1, &g_cfg_ev_abp_ext, NULL}
};

CO_Sub_Object_Ext_t g_lock_bp[3];
CO_Sub_Object_Ext_t g_disable_bp[3];

CO_Sub_Object g_ctrl_bp[] = {
        {(void*)NULL,  ODA_SDO_W,       1, &g_lock_bp[0], NULL},
        {(void*)NULL,  ODA_SDO_W,       1, &g_lock_bp[1], NULL},
        {(void*)NULL,  ODA_SDO_W,       1, &g_lock_bp[2], NULL},
        {(void*)NULL,  ODA_SDO_W,       1, &g_disable_bp[0], NULL},
        {(void*)NULL,  ODA_SDO_W,       1, &g_disable_bp[1], NULL},
        {(void*)NULL,  ODA_SDO_W,       1, &g_disable_bp[2], NULL},
};

CO_Sub_Object_Ext_t g_block_ev_ext;
CO_Sub_Object_Ext_t g_lock_ev_ext;
CO_Sub_Object_Ext_t g_anti_theft_ev_ext;
CO_Sub_Object_Ext_t g_signal_lights_ev_ext;
CO_Sub_Object_Ext_t g_horn_ev_ext;

CO_Sub_Object g_ctrl_pmu[] = {
        {(void*)NULL,  ODA_SDO_RW,      1, &g_signal_lights_ev_ext, NULL},
        {(void*)NULL,  ODA_SDO_W,       1, NULL, NULL},
        {(void*)NULL,  ODA_SDO_RW,      1, &g_horn_ev_ext, NULL},
        {(void*)NULL,  ODA_SDO_W,       5, &g_block_ev_ext,NULL},
        {(void*)NULL,  ODA_SDO_W,       5, &g_lock_ev_ext,NULL},
        {(void*)NULL,  ODA_SDO_W,       5, &g_anti_theft_ev_ext,NULL}
};

CO_Sub_Object_Ext_t g_req_upgrade_ext;
CO_Sub_Object g_req_ev_upgrade[] = {
        {(void*)NULL, ODA_SDO_RW, 5, &g_req_upgrade_ext, NULL},
};


CO_Sub_Object g_fw_name[] = {
        {(void*)NULL, ODA_SDO_RW, 1, NULL, NULL}
};

CO_Sub_Object_Ext_t g_info_ext;
CO_Sub_Object_Ext_t crc_save_ext;
CO_Sub_Object g_info_pmu[] = {
        {(void*)NULL,ODA_SDO_W, 128+9, &g_info_ext , NULL},
        {(void*)NULL,ODA_SDO_R, 128, NULL, NULL},
        {(void*)NULL,ODA_SDO_R, 32, NULL, NULL},
        {(void*)NULL,ODA_SDO_W, 16, &crc_save_ext, NULL},
};

CO_Sub_Object_Ext_t g_reboot ;
CO_Sub_Object g_reboot_request[] = {
        {NULL, ODA_SDO_RW, 1, NULL, NULL},
        {NULL, ODA_SDO_RW, 1, &g_reboot, NULL},
};

/*******************************************************************************************************
 *          Declare and define OD
 *
 *          WARNING: Must sort the object so that the index value ascending from smallest to largest!
 *******************************************************************************************************/
static CO_Object co_od_list[]=
        {
                /*** [0x1000- 0x1FFF] Communication profile objects***/

                /** General communication object**/
                {.index = 0x1000, .sub_number = 1, .subs = obj_1000_device_type},
                {.index = 0x1001, .sub_number = 1, .subs = obj_1001_error_reg},
                {.index = 0x1002, .sub_number = 1, .subs = obj_1002_status},
                {.index = 0x1003, .sub_number = 9, .subs = obj_1003_error_field},

                {.index = 0x1005, .sub_number = 1, .subs = obj_1005_sync_cob_id},
                {.index = 0x1006, .sub_number = 1, .subs = obj_1006_comm_cycle},
                {.index = 0x1007, .sub_number = 1, .subs = obj_1007_sync_window_len},
                {.index = 0x1008, .sub_number = 1, .subs = obj_1008_device_name},
                {.index = 0x1009, .sub_number = 1, .subs = obj_1009_hw_version},
                {.index = 0x100A, .sub_number = 1, .subs = obj_100A_sw_version},
                {.index = 0x100C, .sub_number = 1, .subs = obj_100C_guard_time},
                {.index = 0x100D, .sub_number = 1, .subs = obj_100D_life_time_factor},
                {.index = 0x1010, .sub_number = 3, .subs = obj_1010_store_para},
                {.index = 0x1011, .sub_number = 3, .subs = obj_1011_restore_default_para},
                {.index = 0x1012, .sub_number = 1, .subs = obj_1012_time_cob_id},
                {.index = 0x1013, .sub_number = 1, .subs = obj_1013_high_res_timestamp},
                {.index = 0x1014, .sub_number = 1, .subs = obj_1014_emcy_cob_id},
                {.index = 0x1015, .sub_number = 1, .subs = obj_1015_inhibit_time_emcy},
                {.index = 0x1016, .sub_number = 6, .subs = obj_1016_consumer_heartbeat},
                {.index = 0x1017, .sub_number = 1, .subs = obj_1017_producer_heartbeat},
                {.index = 0x1018, .sub_number = 4, .subs = obj_1018_identity},
                {.index = 0x1019, .sub_number = 1, .subs = obj_1019_sync_cter_over_flow_value},
                {.index = 0x1030, .sub_number = 1, .subs = obj_1030_sync_bit_pos_over_flow_value},
                {.index = 0x1031, .sub_number = 3, .subs = obj_1031_sdo_server_comm_para},
                {.index = 0x1032, .sub_number = 2, .subs = obj_1032_sdo_client_comm_para},
                /** SDO parameter object [0x1200- 0x12FF] **/
                /** PDO parameter object [0x1400- 0x1BFF] **/

                /*** [0x2000- 0x5FFF] Manufacturer-specific profile object ***/

                /* USER CODE 2 begin [0x2000- 0x4FFF] -----------------------------------------------------------*/

                // {.index = SDO_BOOTLOADER_INDEX,           .sub_number = 9, .subs = boot_param},   /* 0x2001 ARRAY Bootloader parameter */
                // {.index = SDO_FILE_TRANSFER_INFO_INDEX,   .sub_number = 5, .subs = file_info_param}, /* 0x201A array file info parameter */
                // {.index = SDO_FILE_TRANSFER_DATA_INDEX,   .sub_number = 4, .subs = file_data_param}, /* 0x201B array file data parameter */
                {.index = SDO_PMU_CTRL_INDEX,             .sub_number = 6, .subs = g_ctrl_pmu},
                {.index = SDO_PMU_CTRL_PORT_INDEX,        .sub_number = 3, .subs = g_ctrl_bp},
                {.index = SDO_PMU_CONFIG_INDEX,           .sub_number = 4, .subs = g_cfg_pmu},
                {.index = SDO_PMU_REQ_EV_UPGRADE_INDEX,   .sub_number = 1, .subs = g_req_ev_upgrade},
                {.index = SDO_PMU_REBOOT_INDEX,           .sub_number = 2, .subs = g_reboot_request},
                {.index = SDO_PMU_INFO_INDEX,             .sub_number = 4, .subs = g_info_pmu},
                {.index = SDO_DEVICE_FIRMWARE_NAME,       .sub_number = 1, .subs = g_fw_name},


                /* USER CODE 2 the end ---------------------------------------------------------*/
        };

static CO_OD co_od =
        {
                (sizeof(co_od_list) / sizeof(co_od_list[0])), co_od_list
        };

CO_OD *p_co_od = &co_od;
