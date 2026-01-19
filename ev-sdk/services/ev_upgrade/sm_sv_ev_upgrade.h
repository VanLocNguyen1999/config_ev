//
// Created by vuonglk on 29/08/2024.
//

#ifndef EV_SDK_SM_SV_EV_UPGRADE_H
#define EV_SDK_SM_SV_EV_UPGRADE_H
#include "stdint.h"
#include "sm_core_co.h"
#include "sm_ex_flash_storage.h"

#include "sm_boot_impl.h"

#ifdef __cplusplus
extern "C"{
#endif


/// USER CONFIG BEGIN

typedef uint64_t flash_addr_size_t;

/// USER CONFIG END

typedef void sm_sv_upgrade_t;

typedef enum{
    SM_SV_EV_UPGRADE_ERR_NONE = 0,
    SM_SV_EV_UPGRADE_ERR_INTERNAL = -1,
    SM_SV_EV_UPGRADE_ERR_TIMEOUT = -2,
    SM_SV_EV_UPGRADE_ERR_REBOOT = -3,
    SM_SV_EV_UPGRADE_ERR_DOWNLOAD = -4,
    SM_SV_EV_UPGRADE_ERR_NOT_SUPPORT = -5,
    SM_SV_EV_UPGRADE_ERR_UNKNOWN = -6
} SM_SV_EV_UPGRADE_UPGRADING_ERROR;

typedef struct sm_sv_ev_upgrade_callback{
    void (*m_onUpgradeDev)(uint8_t devId, int32_t err, void* arg);
    void (*m_onFinishUpgradeProcess)(void* arg);
    void* arg;
}sm_sv_ev_upgrade_callback_t;

sm_sv_upgrade_t* sm_sv_ev_upgrade_create_default(sm_co_t* _co);

int32_t sm_sv_ev_upgrade_process(sm_sv_upgrade_t* _this);

int32_t sm_sv_ev_upgrade_set_callback(sm_sv_upgrade_t* _this, sm_sv_ev_upgrade_callback_t* _cb);

int32_t sm_sv_ev_upgrade_bp(sm_sv_upgrade_t *_this, uint8_t _bp_id, sm_ex_flash_storage_partition_t* _flash_in);

int32_t sm_sv_ev_upgrade_pmu(sm_sv_upgrade_t *_this, sm_ex_flash_storage_partition_t* _flash_in);

int32_t sm_sv_ev_upgrade_mc(sm_sv_upgrade_t *_this, sm_ex_flash_storage_partition_t* _flash_in);

int32_t sm_sv_ev_upgrade_hmi(sm_sv_upgrade_t *_this,
                             sm_ex_flash_storage_partition_t* _flash_in,
                             sm_ex_flash_storage_partition_t* _flash_out,
                             fw_signature_storage_fn_t _signature_storage_fn,
                             void* _arg);

#ifdef __cplusplus
}
#endif

#endif //EV_SDK_SM_SV_EV_UPGRADE_H
