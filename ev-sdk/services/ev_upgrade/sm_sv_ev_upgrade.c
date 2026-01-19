//
// Created by vuonglk on 29/08/2024.
//
#include "sm_sv_ev_upgrade.h"

#include "sm_sv_upgrading.h"
#include "sm_logger.h"
#include "sm_core_co.h"
#include "sm_boot_impl.h"
#include "sm_time_utils.h"

#define TAG "sm_sv_ev_upgrade"

#define REBOOT_DEV_RETRY_TIME   5

#define BP_ID_DEFAULT           4
#define BP_ID_OFFSET            5
#define MC_CANOPEN_ID           2
#define PMU_CANOPEN_ID          1
#define HMI_CANOPEN_ID          8

#define PMU_OTA_STT_ENABLE      2
#define PMU_OTA_STT_DISABLE     0


#define SM_DEVICE_BP_BOOT_INDEX                             0x2001
#define SN_DEVICE_BP_REBOOT_SUB_INDEX                       0x07

#define SM_DEVICE_HMI_BOOT_INDEX                            0x2801
#define SM_DEVICE_HMI_REBOOT_SUB_INDEX                      0x00

#define SM_DEVICE_PMU_BOOT_INDEX                            0x2300
#define SM_DEVICE_PMU_REBOOT_SUB_INDEX                      0x01

#define SM_DEVICE_MC_BOOT_INDEX                             0x2200
#define SM_DEVICE_MC_REBOOT_SUB_INDEX                       0x00

#define SM_DEVICE_REBOOT_TIMEOUT                            500

#define SM_SV_EV_UPGRADE_HMI_HEX_ADDR                       0x0

typedef struct{
    sm_sv_boot_master_t* m_boot_sv;
    sm_sv_boot_master_events_callback_t m_boot_events;
    sm_sv_boot_dev_if_fn_t m_dev_canopen_output_control_if;
    sm_sv_boot_dev_if_fn_t m_dev_flash_output_control_if;
    sm_sv_ev_upgrade_callback_t* m_cb;
    volatile bool m_is_dev_rebooted;
    uint8_t m_current_dev_id;
    uint8_t m_current_dev_err;
}sm_sv_ev_upgrade_impl_t;

static sm_sv_ev_upgrade_impl_t g_sv_ev_upgrade;

sm_co_t* g_sv_upgrade_core_co;

#define _impl(x) (sm_sv_ev_upgrade_impl_t*)(x)


void sm_sv_ev_enable_pmu_ota_stt_canopen_callback(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_INF(TAG, "Enable pmu ota status %s, tx: 0x%x, rx: 0x%x", _sdoStt == SM_SDO_ST_SUCCESS?"SUCCEED":"FAILED", _txCode, _rxCode);
    if(this->m_current_dev_id == MC_CANOPEN_ID){
        this->m_is_dev_rebooted = true;
    }
}

void sm_sv_ev_disable_pmu_ota_stt_canopen_callback(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_INF(TAG, "Disable pmu ota status %s, tx: 0x%x, rx: 0x%x", _sdoStt == SM_SDO_ST_SUCCESS?"SUCCEED":"FAILED", _txCode, _rxCode);
    if(this->m_cb && this->m_cb->m_onUpgradeDev){
        this->m_cb->m_onUpgradeDev(this->m_current_dev_id, this->m_current_dev_err, this->m_cb->arg);
    }
    if(this->m_cb&& this->m_cb->m_onFinishUpgradeProcess){
        this->m_cb->m_onFinishUpgradeProcess(this->m_cb->arg);
    }
    sm_co_set_node_id(g_sv_upgrade_core_co, HMI_CANOPEN_ID);
    sm_co_enable_sync_mode(g_sv_upgrade_core_co);
}


static void sm_sv_ev_upgrade_set_pmu_ota_status(sm_sv_upgrade_t* _this, uint8_t _stt){
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);

    static int32_t reboot_id = 0;
    static uint32_t reboot_index = 0;
    static uint8_t reboot_sub_index = 0;
    static uint8_t reboot_value = 0;

    reboot_id = PMU_CANOPEN_ID;
    reboot_index = SM_DEVICE_MC_BOOT_INDEX;
    reboot_sub_index = SM_DEVICE_MC_REBOOT_SUB_INDEX;
    reboot_value = _stt;

    LOG_INF(TAG, "Now %s pmu ota status", (_stt == PMU_OTA_STT_ENABLE)?"Enable":"Disable");

    sm_co_sdo_client_send(g_sv_upgrade_core_co,
                       reboot_index,
                       reboot_sub_index,
                       reboot_id,
                       &reboot_value,
                       1,
                       1000,
                       (_stt == PMU_OTA_STT_ENABLE)?sm_sv_ev_enable_pmu_ota_stt_canopen_callback:sm_sv_ev_disable_pmu_ota_stt_canopen_callback,
                       this);
}

static void sm_sv_ev_upgrade_on_dev_reboot_callback(uint8_t _devId, uint8_t _issSuccess, void *_arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_INF(TAG, "Reboot Device %d %s", _devId, _issSuccess ? "SUCCESS" : "FAILED");
}

static void sm_sv_ev_upgrade_on_dev_upgraded_callback(uint8_t _devId, uint8_t _err, void *_arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_ERR(TAG, "Upgrade Device %d - %s", _devId, _err == SM_SV_BOOT_MASTER_ERR_NONE?"SUCCESS":"FAILED");

    switch (_devId) {
        case BP_ID_OFFSET:
        case BP_ID_OFFSET + 1:
        case BP_ID_OFFSET + 2:
        case MC_CANOPEN_ID:
            this->m_current_dev_id = _devId;
            this->m_current_dev_err = _err;
            delayMs(1000);
            sm_sv_ev_upgrade_set_pmu_ota_status(this, PMU_OTA_STT_DISABLE); /// turn on pmu sync
             return;
        default:
            if(this->m_cb && this->m_cb->m_onUpgradeDev){
                this->m_cb->m_onUpgradeDev(_devId, _err, this->m_cb->arg);
            }
            if(this->m_cb&& this->m_cb->m_onFinishUpgradeProcess){
                this->m_cb->m_onFinishUpgradeProcess(this->m_cb->arg);
            }
            sm_co_set_node_id(g_sv_upgrade_core_co, HMI_CANOPEN_ID);
            sm_co_enable_sync_mode(g_sv_upgrade_core_co);
    }
}

static void sm_sv_ev_upgrade_on_finish_upgrading_callback(void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_INF(TAG, "All device have been upgraded");
}


/******************************************************* CANOPEN output control interface define **********************************************************/

void sm_sv_ev_upgrade_canopen_reboot_if_dev_callback(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    LOG_INF(TAG, "Write reboot dev %s, tx: 0x%x, rx: 0x%x", _sdoStt == SM_SDO_ST_SUCCESS?"SUCCEED":"FAILED", _txCode, _rxCode);
    this->m_is_dev_rebooted = true;
}

int sm_sv_ev_upgrade_canopen_reboot_dev_if(uint8_t _id, uint32_t _timeout, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    this->m_is_dev_rebooted = false;

    static int32_t reboot_id = 0;
    static uint32_t reboot_index = 0;
    static uint8_t reboot_sub_index = 0;
    static uint8_t reboot_value = 0;
    this->m_current_dev_id = _id;

    switch (_id) {
        case BP_ID_OFFSET:
        case BP_ID_OFFSET + 1:
        case BP_ID_OFFSET + 2:
            sm_sv_ev_upgrade_set_pmu_ota_status(this, PMU_OTA_STT_ENABLE); /// first change pmu to upgrading status
            reboot_id = _id;
            reboot_index = SM_DEVICE_BP_BOOT_INDEX;
            reboot_sub_index = SN_DEVICE_BP_REBOOT_SUB_INDEX;
            reboot_value = 1;
            break;
        case MC_CANOPEN_ID:
            sm_sv_ev_upgrade_set_pmu_ota_status(this, PMU_OTA_STT_ENABLE); /// special reboot method for MC module by control pmu status
            return true;
        case PMU_CANOPEN_ID:
            reboot_id = _id;
            reboot_index = SM_DEVICE_PMU_BOOT_INDEX;
            reboot_sub_index = SM_DEVICE_PMU_REBOOT_SUB_INDEX;
            reboot_value = 1;
            break;
        case HMI_CANOPEN_ID:
            reboot_id = _id;
            reboot_index = SM_DEVICE_HMI_BOOT_INDEX;
            reboot_sub_index = SM_DEVICE_HMI_REBOOT_SUB_INDEX;
            reboot_value = 1;
            break;
        default:
            return -1;
    }

    sm_co_sdo_client_send(g_sv_upgrade_core_co,
                              reboot_index,
                              reboot_sub_index,
                              reboot_id,
                              &reboot_value,
                              1,
                              1000,
                              sm_sv_ev_upgrade_canopen_reboot_if_dev_callback, this);

    return true;
}

bool sm_sv_ev_upgrade_canopen_check_is_dev_rebooted(uint8_t _id, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    return this->m_is_dev_rebooted;
}

bool sm_sv_ev_upgrade_canopen_check_dev_upgrade_condition_if(uint8_t _id, void* _arg) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_arg);
    return true;
}



/*********************************************** FLASH output control interface define ************************************************/

int sm_sv_ev_upgrade_flash_output_reboot_dev_if(uint8_t _id, uint32_t _timeout, void* _arg) {
    LOG_INF(TAG, "Flash reboot dev id %d", _id);
    return true;
}

bool sm_sv_ev_upgrade_flash_output_check_is_dev_rebooted(uint8_t _id, void* _arg) {
    LOG_INF(TAG, "Flash reboot dev id %d SUCCESS", _id);
    return true;
}

bool sm_sv_ev_upgrade_flash_output_check_dev_upgrade_condition_if(uint8_t _id, void* _arg) {
    return true;
}

sm_sv_upgrade_t* sm_sv_ev_upgrade_create_default(sm_co_t* _co){

    if(!_co){
        LOG_ERR(TAG, "Cannot create ev upgrade service because invalid co core instance");
        return NULL;
    }

    g_sv_ev_upgrade.m_dev_canopen_output_control_if.check_dev_upgrading_condition = sm_sv_ev_upgrade_canopen_check_dev_upgrade_condition_if;
    g_sv_ev_upgrade.m_dev_canopen_output_control_if.reboot_dev = sm_sv_ev_upgrade_canopen_reboot_dev_if;
    g_sv_ev_upgrade.m_dev_canopen_output_control_if.check_is_dev_rebooted = sm_sv_ev_upgrade_canopen_check_is_dev_rebooted;
    g_sv_ev_upgrade.m_dev_canopen_output_control_if.arg = &g_sv_ev_upgrade;

    g_sv_ev_upgrade.m_dev_flash_output_control_if.check_dev_upgrading_condition = sm_sv_ev_upgrade_flash_output_check_dev_upgrade_condition_if;
    g_sv_ev_upgrade.m_dev_flash_output_control_if.reboot_dev = sm_sv_ev_upgrade_flash_output_reboot_dev_if;
    g_sv_ev_upgrade.m_dev_flash_output_control_if.check_is_dev_rebooted = sm_sv_ev_upgrade_flash_output_check_is_dev_rebooted;
    g_sv_ev_upgrade.m_dev_flash_output_control_if.arg = &g_sv_ev_upgrade;

    g_sv_ev_upgrade.m_boot_events.on_rebooted_dev = sm_sv_ev_upgrade_on_dev_reboot_callback;
    g_sv_ev_upgrade.m_boot_events.on_upgraded_dev = sm_sv_ev_upgrade_on_dev_upgraded_callback;
    g_sv_ev_upgrade.m_boot_events.on_finish_upgrading = sm_sv_ev_upgrade_on_finish_upgrading_callback;
    g_sv_ev_upgrade.m_boot_events.arg = &g_sv_ev_upgrade;

    g_sv_upgrade_core_co = _co;

    g_sv_ev_upgrade.m_boot_sv = sm_sv_boot_master_create(&g_sv_ev_upgrade.m_boot_events);
    return &g_sv_ev_upgrade;
}

int32_t sm_sv_ev_upgrade_process(sm_sv_upgrade_t* _this){
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this || !this->m_boot_sv){
        return -1;
    }
    sm_sv_boot_master_process(this->m_boot_sv);
    return 0;
}

int32_t sm_sv_ev_upgrade_set_callback(sm_sv_upgrade_t* _this, sm_sv_ev_upgrade_callback_t* _cb){
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    this->m_cb = _cb;
    return 0;
}

int32_t sm_sv_ev_upgrade_bp(sm_sv_upgrade_t *_this, uint8_t _bp_id, sm_ex_flash_storage_partition_t* _flash_in) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this || !this->m_boot_sv || _bp_id >= 3){
        return -1;
    }

    sm_sv_boot_request_dev_info_t info = {
            .m_boot_id = BP_ID_DEFAULT,
            .m_network_id = BP_ID_OFFSET + _bp_id,
            .m_if = &this->m_dev_canopen_output_control_if,
            .m_input_type = SM_SV_BOOT_MASTER_INPUT_FLASH,
            .m_flash_part_input = _flash_in,
            .m_output_type = SM_SV_BOOT_MASTER_OUTPUT_CANOPEN
    };
    sm_co_disable_sync_mode(g_sv_upgrade_core_co);
    return sm_sv_boot_master_request_upgrade(this->m_boot_sv, info);
}

int32_t sm_sv_ev_upgrade_pmu(sm_sv_upgrade_t *_this, sm_ex_flash_storage_partition_t* _flash_in) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this || !this->m_boot_sv){
        return -1;
    }

    sm_sv_boot_request_dev_info_t info = {
            .m_boot_id = PMU_CANOPEN_ID,
            .m_network_id = PMU_CANOPEN_ID,
            .m_if = &this->m_dev_canopen_output_control_if,
            .m_input_type = SM_SV_BOOT_MASTER_INPUT_FLASH,
            .m_flash_part_input = _flash_in,
            .m_output_type = SM_SV_BOOT_MASTER_OUTPUT_CANOPEN
    };
    sm_co_disable_sync_mode(g_sv_upgrade_core_co);
    return sm_sv_boot_master_request_upgrade(this->m_boot_sv, info);

}

int32_t sm_sv_ev_upgrade_mc(sm_sv_upgrade_t *_this, sm_ex_flash_storage_partition_t* _flash_in) {
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this || !this->m_boot_sv){
        return -1;
    }

    sm_sv_boot_request_dev_info_t info = {
            .m_boot_id = MC_CANOPEN_ID,
            .m_network_id = MC_CANOPEN_ID,
            .m_if = &this->m_dev_canopen_output_control_if,
            .m_input_type = SM_SV_BOOT_MASTER_INPUT_FLASH,
            .m_flash_part_input = _flash_in,
            .m_output_type = SM_SV_BOOT_MASTER_OUTPUT_CANOPEN
    };
    sm_co_disable_sync_mode(g_sv_upgrade_core_co);
    return sm_sv_boot_master_request_upgrade(this->m_boot_sv, info);
}

int32_t sm_sv_ev_upgrade_hmi(sm_sv_upgrade_t *_this,
                             sm_ex_flash_storage_partition_t* _flash_in,
                             sm_ex_flash_storage_partition_t* _flash_out,
                             fw_signature_storage_fn_t _signature_storage_fn,
                             void* _arg){
    sm_sv_ev_upgrade_impl_t* this = _impl(_this);
    if(!this || !this->m_boot_sv){
        return -1;
    }

    sm_sv_boot_request_dev_info_t info = {
            .m_boot_id = HMI_CANOPEN_ID,
            .m_network_id = HMI_CANOPEN_ID,
            .m_if = &this->m_dev_flash_output_control_if,
            .m_input_type = SM_SV_BOOT_MASTER_INPUT_FLASH,
            .m_flash_part_input = _flash_in,
            .m_output_type = SM_SV_BOOT_MASTER_OUTPUT_FLASH,
            .m_flash_part_output = _flash_out,
            .m_signature_storage_fn = _signature_storage_fn,
            .m_signature_storage_arg = _arg
    };

    return sm_sv_boot_master_request_upgrade(this->m_boot_sv, info);
}

