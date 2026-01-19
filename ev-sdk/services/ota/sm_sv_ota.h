//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_OTA_H
#define EV_SDK_SM_SV_OTA_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_ev_data.h"
#include "sm_http_client.h"
#include "sm_sv_ev_upgrade.h"
#include "sm_ex_flash_storage.h"

#include "sm_types.h"

typedef void sm_sv_ota_t;
typedef sm_ex_flash_storage_partition_t sm_ota_fw_storage_t;

enum {
    SM_OTA_LTE_IF,
    SM_OTA_BLE_IF,
    SM_OTA_IF_NUMBER,
};

enum {
    SM_OTA_ERR_NONE = 0,
    SM_OTA_ERR_INCOMPATIBLE_VERSION = -1,
    SM_OTA_ERR_DOWNLOAD_FAILURE = -2,
    SM_OTA_ERR_DOWNLOAD_TIMEOUT = -3,
    SM_OTA_ERR_CRC_FILE_INVALID = -4,
    SM_OTA_ERR_CONVERT_FILE_FAILURE = -5,
    SM_OTA_ERR_DEVICE_NOT_RESPONSE = -6,
    SM_OTA_ERR_UPGRADING_FAILURE = -7,
    SM_OTA_ERR_UPGRADING_TIMEOUT = -8,
    SM_OTA_ERR_INTERNAL_SYSTEM_ERROR = -9
};

enum {
    SM_OTA_STT_IDLE = 0,
    SM_OTA_STT_PREPARE,
    SM_OTA_STT_STARTING,
    SM_OTA_STT_DOWNLOADING,
    SM_OTA_STT_DOWNLOADED,
    SM_OTA_STT_FW_VERIFYING,
    SM_OTA_STT_PREPARE_UPGRADING,
    SM_OTA_STT_UPGRADING,
    SM_OTA_STT_UPGRADED,
    SM_OTA_STT_NUMBER,
};

typedef struct {
    /**
     * @brief: Notify to observer about ota progress starting
     * @param: void* arg
     */
    void (*on_start)(void*);

    /**
     * @brief: Notify to observer about ota progress on this module
     * @param: const char* module name
     * @param: int32_t total frame downloading,
     * @param: void* _arg
     */
    void (*on_start_module)(const char*, int32_t, void*);

    /**
     *@brief: Notify to observer about downloading status
     * @param: const char* module name
     * @param: int32_t frame index
     * @param: bool _is done
     * @param: int32_t error code
     * @param: const char* error message
     * @param: void* _arg
     */
    void (*on_downloading_status)(const char*, int32_t, uint32_t, int32_t, const char*, void*);

    /**
     * @brief: Notify Upgrading status
    * const char*: module name (pmu/mc/hmi/bp)
    * const char*: new version
    * int32_t: error code
    * const char* : error message
    * void* arg
    */
    void (*on_upgrading_status)(const char *, const char *, int32_t, const char*, void *);

    /**
     * @brief: Notify OTA progress module is finished
     *  const char*: module name (pmu/mc/hmi/bp)
     */
    void (*on_finished_module)(const char *, void *);

    /**
     * @brief: Notify OTA progress finished
     */
    void (*on_finished)(void *);
} sm_ota_event_t;

sm_sv_ota_t* sm_sv_ota_create(sm_http_client_t* _http_client, sm_sv_upgrade_t* _upgrade_service, sm_ota_fw_storage_t* _fw_storage);

int32_t sm_sv_ota_set_fw_output_storage_if(sm_sv_ota_t* _this,
                                           sm_ota_fw_storage_t* _fw_storage_output,
                                           fw_signature_storage_fn_t _fn,
                                           void* _arg);

int32_t sm_sv_ota_destroy(sm_sv_ota_t* _this);

int32_t sm_sv_ota_reg_event(sm_sv_ota_t* _this, sm_ota_event_t* _event_handle, void* _arg);

int32_t sm_sv_ota_is_busy(sm_sv_ota_t* _this);

/**
 *
 * @param _this
 * @return
 */
int32_t sm_sv_ota_get_status(sm_sv_ota_t* _this);

int32_t sm_sv_ota_request_upgrade_ev_module(sm_sv_ota_t* _this,
                                  uint8_t _if_type,
                                  uint8_t _module_type,
                                  const char* _new_version,
                                  int32_t _size,
                                  int32_t _crc,
                                  const char* _link);

int32_t sm_sv_ota_request_upgrade_bp(sm_sv_ota_t* _this,
                                  uint8_t _if_type,
                                  uint8_t _bp_id,
                                  const char* _new_version,
                                  int32_t _size,
                                  int32_t _crc,
                                  const char* _link);

int32_t sm_sv_ota_request_upgrade_bp_continue(sm_sv_ota_t* _this,
                                              uint8_t _bp_id,
                                              const char* _new_version);

int32_t sm_sv_ota_start(sm_sv_ota_t* _this);

int32_t sm_sv_ota_stop(sm_sv_ota_t* _this);

int32_t sm_sv_ota_pause(sm_sv_ota_t* _this);

int32_t sm_sv_ota_resume(sm_sv_ota_t* _this);

int32_t sm_sv_ota_process(sm_sv_ota_t* _this);

static inline const char* sm_sv_ota_convert_err_to_string(int32_t _err){
    switch (_err) {
        case SM_OTA_ERR_NONE:
            return "SM_OTA_ERR_NONE";
        case SM_OTA_ERR_INCOMPATIBLE_VERSION:
            return "SM_OTA_ERR_INCOMPATIBLE_VERSION";
        case SM_OTA_ERR_DOWNLOAD_FAILURE:
            return "SM_OTA_ERR_DOWNLOAD_FAILURE";
        case SM_OTA_ERR_DOWNLOAD_TIMEOUT:
            return "SM_OTA_ERR_DOWNLOAD_TIMEOUT";
        case SM_OTA_ERR_CRC_FILE_INVALID:
            return "SM_OTA_ERR_CRC_FILE_INVALID";
        case SM_OTA_ERR_CONVERT_FILE_FAILURE:
            return "SM_OTA_ERR_CONVERT_FILE_FAILURE";
        case SM_OTA_ERR_DEVICE_NOT_RESPONSE:
            return "SM_OTA_ERR_DEVICE_NOT_RESPONSE";
        case SM_OTA_ERR_UPGRADING_FAILURE:
            return "SM_OTA_ERR_UPGRADING_FAILURE";
        case SM_OTA_ERR_UPGRADING_TIMEOUT:
            return "SM_OTA_ERR_UPGRADING_TIMEOUT";
        case SM_OTA_ERR_INTERNAL_SYSTEM_ERROR:
            return "SM_OTA_ERR_INTERNAL_SYSTEM_ERROR";
        default:
            return "SM_OTA_ERR_UNKNOWN";
    }
}


#ifdef __cplusplus
;
#endif

#endif //EV_SDK_SM_SV_OTA_H
