//
// Created by vuonglk on 06/05/2024.
//

#include "sm_sv_upgrading.h"
#include "sm_logger.h"
#include "sm_boot_impl.h"
#include "mb_master/sm_mb_master_impl.h"
#include "sm_host.h"

#define TAG "sm_sv_boot_master"

#define SM_SV_BOOT_MAX_DEV_SUPPORT               64
#define SM_SV_BOOT_WAITING_ONLINE_TIMEOUT       45000
#define SM_SV_BOOT_REBOOT_DEV_TIMEOUT           10000


#define SM_SV_BOOT_FILE_INPUT_USE               0
#define SM_SV_BOOT_FLASH_INPUT_USE              1

#define SM_SV_BOOT_CANOPEN_OUTPUT_USE           1
#define SM_SV_BOOT_FLASH_OUTPUT_USE             1

typedef enum
{
    UPGRADING_DEV_WAITING_ONLINE = 0,
    UPGRADING_DEV_WAITING_REBOOT,
    UPGRADING_DEV_UPGRADING,
} UPGRADING_DEVICE_STATE;

typedef struct sm_sv_boot_dev_info{
    sm_sv_boot_request_dev_info_t m_info;

    elapsed_timer_t m_timeout;
    uint8_t m_state;
} sm_sv_boot_upgrade_dev_info_t;

typedef struct sm_sv_boot_master_impl {
    sm_vector_t* m_upgradeDev;
    sm_sv_boot_upgrade_dev_info_t* m_currentUpgradingDev;
    sm_sv_boot_master_events_callback_t* m_eventCallback;
    elapsed_timer_t m_timeout;
    sm_boot_master_t* m_bootMaster;
} sm_sv_boot_master_impl_t;

#define impl(x) ((sm_sv_boot_master_impl_t*)(x))


static void sm_sv_boot_reset_upgrading_dev(sm_sv_boot_master_t* _this, SM_SV_BOOT_MASTER_UPGRADING_ERROR _error);

static int sm_sv_boot_prepare_upgrading_dev(sm_sv_boot_master_t* _this);

static int sm_sv_boot_start_upgrading_dev(sm_sv_boot_master_t* _this);

static void boot_master_event_callback(int32_t _error, int32_t _id, void* _arg) {
    sm_sv_boot_master_impl_t* this = impl(_arg);
    if (this == NULL) return;

    if(!_error){
        LOG_INF(TAG, "Upgrading device %d is SUCCESS", _id);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_NONE);
    }else{
        LOG_ERR(TAG, "Upgrading device %d is FAILURE, error code of boot-core %d", this->m_currentUpgradingDev->m_info.m_network_id, _error);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_DOWNLOAD);
    }
}

sm_sv_boot_master_t* sm_sv_boot_master_create(sm_sv_boot_master_events_callback_t* _eventCallback) {
    sm_sv_boot_master_impl_t* this = malloc(sizeof(sm_sv_boot_master_impl_t));
    if (this == NULL) return NULL;

    this->m_upgradeDev = sm_vector_create(SM_SV_BOOT_MAX_DEV_SUPPORT, sizeof(sm_sv_boot_upgrade_dev_info_t));
    if (this->m_upgradeDev == NULL) return NULL;

    this->m_eventCallback = _eventCallback;
    this->m_currentUpgradingDev = NULL;
    this->m_bootMaster = sm_boot_master_create(boot_master_event_callback, this);

    if(this->m_bootMaster == NULL){
        LOG_ERR(TAG, "Create Boot Master FAILURE");
        return NULL;
    }

    return this;
}

int sm_sv_boot_master_request_upgrade(sm_sv_boot_master_t *_this, sm_sv_boot_request_dev_info_t _devInfo){

    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    sm_sv_boot_upgrade_dev_info_t devInfo = {.m_info = _devInfo};

    if (sm_vector_is_full(this->m_upgradeDev)) {
        return -1;
    }

    devInfo.m_state = UPGRADING_DEV_WAITING_ONLINE;
    elapsed_timer_resetz(&devInfo.m_timeout, SM_SV_BOOT_WAITING_ONLINE_TIMEOUT);
    if (sm_vector_push_back(this->m_upgradeDev, &devInfo) < 0) {
        return -1;
    }
    LOG_INF(TAG, "New device with id %d is push to queue to upgrading", _devInfo.m_network_id);
    return 0;
}


static void sm_sv_boot_reset_upgrading_dev(sm_sv_boot_master_t* _this, SM_SV_BOOT_MASTER_UPGRADING_ERROR _error) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this->m_currentUpgradingDev == NULL) return;
    sm_sv_boot_upgrade_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    if (this->m_eventCallback->on_upgraded_dev) {
        LOG_INF(TAG, "Notify upgrading error %d of dev %d to Observer", _error, upgradingDev->m_info.m_network_id);
        this->m_eventCallback->on_upgraded_dev(upgradingDev->m_info.m_network_id,
                                               _error,
                                               this->m_eventCallback->arg);
    }

    sm_vector_erase_item(this->m_upgradeDev, upgradingDev);
    this->m_currentUpgradingDev = NULL;

    if (sm_vector_get_size(this->m_upgradeDev) == 0) {
        if (this->m_eventCallback->on_finish_upgrading) {
            LOG_INF(TAG, "Finished upgrading firmware, Notify to observer");
            this->m_eventCallback->on_finish_upgrading(this->m_eventCallback->arg);
        }
    }else {
        for (int index = 0; index < sm_vector_get_size(this->m_upgradeDev); index++) {
            sm_sv_boot_upgrade_dev_info_t* dev = sm_vector_get_item(this->m_upgradeDev, index);
            elapsed_timer_resetz(&dev->m_timeout, SM_SV_BOOT_WAITING_ONLINE_TIMEOUT);
            dev->m_state = UPGRADING_DEV_WAITING_ONLINE;
        }
    }
}

static int sm_sv_boot_prepare_upgrading_dev(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this->m_currentUpgradingDev == NULL) return -1;
    sm_sv_boot_upgrade_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    LOG_INF(TAG, "Now reboot device id %d", upgradingDev->m_info.m_network_id);
    if(upgradingDev->m_info.m_if->reboot_dev == NULL){
        upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
        sm_sv_boot_start_upgrading_dev(this);
        return 0;
    }

    if (upgradingDev->m_info.m_if->reboot_dev(upgradingDev->m_info.m_network_id, SM_SV_BOOT_REBOOT_DEV_TIMEOUT,
                                                      upgradingDev->m_info.m_if->arg) < 0) {
        LOG_ERR(TAG, "Reboot device id %d FAILED", upgradingDev->m_info.m_network_id);
        this->m_eventCallback->on_rebooted_dev(upgradingDev->m_info.m_network_id, false, this->m_eventCallback->arg);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_REBOOT);
        return -1;
    }

    if(upgradingDev->m_info.m_if->check_is_dev_rebooted == NULL){
        upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
        this->m_eventCallback->on_rebooted_dev(upgradingDev->m_info.m_network_id, true, this->m_eventCallback->arg);
        sm_sv_boot_start_upgrading_dev(this);
    }else{
        elapsed_timer_resetz(&upgradingDev->m_timeout, SM_SV_BOOT_REBOOT_DEV_TIMEOUT);
        upgradingDev->m_state = UPGRADING_DEV_WAITING_REBOOT;
    }
    return 0;
}

static int sm_sv_boot_start_upgrading_dev(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    sm_sv_boot_upgrade_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    LOG_INF(TAG, "Start upgrading device id %d", upgradingDev->m_info.m_network_id);

    static volatile sm_boot_output_if_t* bootOutput = NULL;
    static volatile sm_boot_input_if_t* bootInput = NULL;
    bootOutput = NULL;
    bootInput = NULL;

    switch (upgradingDev->m_info.m_input_type) {
#if SM_SV_BOOT_FILE_INPUT_USE
        case SM_SV_BOOT_MASTER_INPUT_FILE:
            bootInput = sm_get_file_boot_input(upgradingDev->m_info.m_file_path);
            break;
#endif

#if SM_SV_BOOT_FLASH_INPUT_USE
        case SM_SV_BOOT_MASTER_INPUT_FLASH:
            bootInput = sm_get_flash_boot_input(upgradingDev->m_info.m_flash_part_input);
            break;
#endif
        case SM_SV_BOOT_MASTER_INPUT_TYPE_NUMBER:
        case SM_SV_BOOT_MASTER_INPUT_NONE:
        default:
            LOG_ERR(TAG, "Not valid dev input if type");
            sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_NOT_SUPPORT);
            return -1;

    }

    switch (upgradingDev->m_info.m_output_type) {
#if SM_SV_BOOT_CANOPEN_OUTPUT_USE
        case SM_SV_BOOT_MASTER_OUTPUT_CANOPEN:
            bootOutput = sm_get_co_boot_output();
            break;
#endif

#if SM_SV_BOOT_FLASH_OUTPUT_USE
        case SM_SV_BOOT_MASTER_OUTPUT_FLASH:
            bootOutput = sm_get_flash_boot_output(upgradingDev->m_info.m_flash_part_output,
                                                  upgradingDev->m_info.m_signature_storage_fn,
                                                  upgradingDev->m_info.m_signature_storage_arg);
            break;

#endif

        case SM_SV_BOOT_MASTER_OUTPUT_NONE:
        case SM_SV_BOOT_MASTER_OUTPUT_TYPE_NUMBER:
        default:
            LOG_ERR(TAG, "Not valid dev output if type");
            sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_NOT_SUPPORT);
            return -1;
    }

    if(!bootOutput || !bootInput){
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_INTERNAL);
        return -1;
    }

    if (sm_boot_master_add_slave(this->m_bootMaster, upgradingDev->m_info.m_boot_id, bootInput, bootOutput) < 0) {
        LOG_ERR(TAG, "Add Boot slave FAILURE");
        bootOutput->m_proc->free(bootOutput);
        bootInput->free();
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_INTERNAL);
        return -1;
    }

    upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
    elapsed_timer_reset(&upgradingDev->m_timeout);
    return 0;
}

void sm_sv_boot_master_process(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return;

    if (sm_vector_get_size(this->m_upgradeDev) > 0 && this->m_currentUpgradingDev == NULL) {
        for (int index = 0; index < sm_vector_get_size(this->m_upgradeDev); index++) {
            sm_sv_boot_upgrade_dev_info_t* dev = sm_vector_get_item(this->m_upgradeDev, index);
            if (dev->m_state == UPGRADING_DEV_WAITING_ONLINE && !elapsed_timer_get_remain(&dev->m_timeout)) {
                LOG_ERR(TAG, "The device %d is TIMEOUT while waiting online", dev->m_info.m_network_id);
                if (this->m_eventCallback->on_upgraded_dev) {
                    this->m_eventCallback->on_upgraded_dev(dev->m_info.m_network_id, SM_SV_BOOT_MASTER_ERR_TIMEOUT,
                                                           this->m_eventCallback->arg);
                }
                sm_vector_erase_item(this->m_upgradeDev, dev);

                if (this->m_eventCallback->on_finish_upgrading != NULL && sm_vector_get_size(this->m_upgradeDev) == 0) {
                    LOG_INF(TAG, "Finished upgrading firmware, Notify to observer");
                    this->m_eventCallback->on_finish_upgrading(this->m_eventCallback->arg);
                }
                return;
            }

            if (dev->m_info.m_if->check_dev_upgrading_condition(dev->m_info.m_network_id, dev->m_info.m_if->arg)) {
                this->m_currentUpgradingDev = dev;
                LOG_INF(TAG, "Device %d is enough condition to upgrading", dev->m_info.m_network_id);
                sm_sv_boot_prepare_upgrading_dev(this);
                return;
            }
        }
    }

    if (this->m_currentUpgradingDev) {
        sm_sv_boot_upgrade_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

        if (upgradingDev->m_state == UPGRADING_DEV_WAITING_REBOOT){
            if(!elapsed_timer_get_remain(&upgradingDev->m_timeout)){
                this->m_eventCallback->on_rebooted_dev(upgradingDev->m_info.m_network_id, false, this->m_eventCallback->arg);
                sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_REBOOT);
                return;
            }
            if(upgradingDev->m_info.m_if->check_is_dev_rebooted(upgradingDev->m_info.m_network_id, upgradingDev->m_info.m_if->arg)){
                this->m_eventCallback->on_rebooted_dev(upgradingDev->m_info.m_network_id, true, this->m_eventCallback->arg);
                sm_sv_boot_start_upgrading_dev(this);
            }
        }

        if(upgradingDev->m_state == UPGRADING_DEV_UPGRADING){
            sm_boot_master_process(this->m_bootMaster);
        }
    }
}

int sm_sv_boot_master_destroy(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    sm_vector_destroy(this->m_upgradeDev);
    free(this);
    return 0;
}

#define TAG "sm_sv_boot_master"
