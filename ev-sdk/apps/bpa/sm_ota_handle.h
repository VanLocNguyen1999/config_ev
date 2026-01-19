//
// Created by vnbk on 19/09/2024.
//

#ifndef EV_SDK_SM_EV_OTA_H
#define EV_SDK_SM_EV_OTA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_types.h"

/**
 * @brief: Notify to observer about ota progress starting
 * @param: void* arg
 */
void sm_ota_on_start(void *_arg);

/**
 * @brief: Notify to observer about ota progress on this module
 * @param: const char* module name
 * @param: int32_t total frame downloading,
 * @param: void* _arg
 */
void sm_ota_on_start_module(const char *_module_name, int32_t _type, void *_arg);

/**
 *@brief: Notify to observer about downloading status
 * @param: const char* module name
 * @param: int32_t frame index
 * @param: bool _is done
 * @param: int32_t error code
 * @param: const char* error message
 * @param: void* _arg
 */
void sm_ota_on_downloading_status(const char *_module_name, int32_t _frame_index, uint32_t _total_seg, int32_t _err,
                                  const char *_err_msg, void *_arg);

/**
 * @brief: Notify Upgrading status
* const char*: module name (pmu/mc/hmi/bp)
* const char*: new version
* int32_t: error code
* const char* : error message
* void* arg
*/
void sm_ota_on_upgrading_status(const char *_module_name, const char *_version, int32_t _err, const char *_err_msg,
                                void *_arg);


/**
 * @brief: Notify OTA progress module is finished
 *  const char*: module name (pmu/mc/hmi/bp)
 */
void sm_ota_on_finished_module(const char *_module_name, void *_arg);

/**
 * @brief: Notify OTA progress finished
 */
void sm_ota_on_finished(void *_arg);


int32_t sm_ota_handle_request_upgrade(void* _arg);

const char* sm_ota_handle_ev_new_fw(const char* _module_name,
                                const char* _new_version,
                                int32_t _size,
                                uint16_t _crc16,
                                const char* _link,
                                int32_t* _bypass,
                                void* _arg);

void sm_ota_handle_finish_new_fw_extract(int32_t _module_count, void* _arg);

int32_t sm_ota_handle_request_bp_upgrade(void* _arg);

int32_t sm_ota_handle_bp_new_fw(void* _data, void* _arg);

int32_t sm_ota_handle_cmd_progress(const char* _cmd, void* _arg);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_OTA_H
