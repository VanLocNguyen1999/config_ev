//
// Created by vnbk on 19/09/2024.
//
#include "sm_ev_ota.h"
#include "sm_logger.h"

#define SM_EV_OTA_MINIMUM_HEX_FILE_SIZE (100*1024)

#define TAG "SM_OTA_HANDLE"

void sm_ota_on_start(void *_arg){
   // LOG_INF(TAG, "OTA progress START");
}

/**
 * @brief: Notify to observer about ota progress on this module
 * @param: const char* module name
 * @param: int32_t total frame downloading,
 * @param: void* _arg
 */
void sm_ota_on_start_module(const char *_module_name, int32_t _type, void *_arg){
   // LOG_INF(TAG, "OTA progress of module %s START", _module_name);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    app->m_iot_service->m_proc->sync_pause(app->m_iot_service);
}

/**
 *@brief: Notify to observer about downloading status
 * @param: const char* module name
 * @param: int32_t frame index
 * @param: bool _is done
 * @param: int32_t error code
 * @param: const char* error message
 * @param: void* _arg
 */
void sm_ota_on_downloading_status(const char *_module_name,
                                  int32_t _frame_index,
                                  uint32_t _total_seg,
                                  int32_t _err,
                                  const char *_err_msg,
                                  void *_arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_iot_t* iot = (sm_sv_iot_t*)app->m_iot_service;
    if(!iot){
        LOG_ERR(TAG, "Missing iot service");
        return;
    }

    bool is_last_frame = (_frame_index + 1) >= _total_seg;
    sm_sv_iot_notify_ota_download_status(iot, _module_name, (int32_t)_total_seg, _frame_index, is_last_frame, _err, _err_msg);

    sm_sv_hmi_set_ota_progress(app->m_hmi_service, (_frame_index + 1) * 10 * 95 / _total_seg);
}

/**
 * @brief: Notify Upgrading status
* const char*: module name (pmu/mc/hmi/bp)
* const char*: new version
* int32_t: error code
* const char* : error message
* void* arg
*/
void sm_ota_on_upgrading_status(const char *_module_name,
                                const char *_version,
                                int32_t _err,
                                const char *_err_msg,
                                void *_arg){

    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_iot_t* iot = (sm_sv_iot_t*)app->m_iot_service;

    if(!iot){
        LOG_ERR(TAG, "Missing iot service");
        return;
    }

    LOG_ERR(TAG, "Upgrade module %s err %s", _module_name, _err_msg);

#ifdef OTA_TESTING
    if(!strcmp(_module_name, "mc")){
        g_ota_report.m_success[0] += _err == SM_OTA_ERR_NONE;
    }else if(!strcmp(_module_name, "pmu")){
        g_ota_report.m_success[1] += _err == SM_OTA_ERR_NONE;
    }else{
        g_ota_report.m_success[2] += _err == SM_OTA_ERR_NONE;
    }
#endif

    sm_sv_iot_notify_ota_upgrade_status(iot, _module_name, _version, _err, _err_msg);

    if(_err == SM_OTA_ERR_NONE){
        sm_sv_hmi_set_ota_progress(app->m_hmi_service, 100);
    }else{
        sm_sv_hmi_set_ota_progress(app->m_hmi_service, 255);
    }

    if(!strcmp(_module_name, "hmi") && _err == SM_OTA_ERR_NONE){
        app->m_flag_sys_reset = true;
    }
    if(!strcmp(_module_name, "mc")) {
        sm_sv_ev_reset_module_info(app->m_ev_service, SM_EV_MODULE_MC);
    }
    if(!strcmp(_module_name, "pmu")){
        sm_sv_ev_reset_module_info(app->m_ev_service, SM_EV_MODULE_PMU);
    }
}


/**
 * @brief: Notify OTA progress finished
 */
void sm_ota_on_finished(void *_arg){
   // LOG_INF(TAG, "OTA progress FINISH");
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    app->m_iot_service->m_proc->sync_resume(app->m_iot_service);

    sm_sv_hmi_clear_ota_progress(app->m_hmi_service);
    sm_sv_hmi_set_odo(app->m_hmi_service, (int32_t)sm_sv_ev_get_data(app->m_ev_service)->m_odo, SM_DRV_LCD_SHOW);

    if(app->m_flag_sys_reset){
        sm_sv_ev_reboot_module(app->m_ev_service, SM_EV_MODULE_HMI, 3000);
        app->m_flag_sys_reset = false;
    }

#ifdef OTA_TESTING
    LOG_ERR(TAG, "-------------------OTA Report--------------------");
    LOG_ERR(TAG, "Mc: success %d / %d", g_ota_report.m_success[0], g_ota_report.m_total[0]);
    LOG_ERR(TAG, "Pmu: success %d / %d", g_ota_report.m_success[1], g_ota_report.m_total[1]);
    LOG_ERR(TAG, "Hmi: success %d / %d", g_ota_report.m_success[2], g_ota_report.m_total[2]);
    LOG_ERR(TAG, "-------------------------------------------------");

    g_test_storage->m_proc->store(g_test_storage, &g_ota_report);
    /*sm_hal_delay_ms(2000);
    NVIC_SystemReset();*/
#endif
}


int32_t sm_ota_handle_request_upgrade(void* _arg){
   // LOG_INF(TAG, "Event from IOT system: REQUEST UPGRADE FIRMWARE");
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    if(!ota){
        LOG_ERR(TAG, "Param INVALID");
        return -1;
    }

    return sm_sv_ota_is_busy(ota);
}

const char* sm_ota_handle_ev_new_fw(const char* _module_name,
                                const char* _new_version,
                                int32_t _size,
                                uint16_t _crc16,
                                const char* _link,
                                int32_t* _bypass,
                                void* _arg){
    LOG_ERR(TAG, "Event from IOT system: NEW FIRMWARE INFO is available. Module: %s, New version: %s", _module_name, _new_version);
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    sm_sv_ev_t* ev = app->m_ev_service;
    if(!ota || !ev){
        LOG_ERR(TAG, "Param INVALID");
        *_bypass = true;
        return NULL;
    }

    if(sm_sv_ota_is_busy(ota)){
        *_bypass = true;
        return NULL;
    }

    const char* _current_version = sm_sv_ev_get_module_version_by_name(ev, _module_name);
    if(!*_current_version){
        LOG_WRN(TAG, "Module %s could NOT read", _module_name);
    }

    if(_size < SM_EV_OTA_MINIMUM_HEX_FILE_SIZE){
        LOG_WRN(TAG, "Hex file is too small, mean bypass module");
        return _current_version;
    }

    if(!strcmp(_current_version, _new_version)){
        LOG_ERR(TAG, "Module %s versions are identical: %s", _module_name, _current_version);
        *_bypass = true;
        return _current_version;
    }

    if(sm_ev_check_module_pre_download_condition_by_name(ev, _module_name) != 0){
        LOG_ERR(TAG, "Module %s is cannot upgrade now, keep it at versions", _module_name, _current_version);
        return _current_version;
    }

    if(sm_sv_ota_request_upgrade_ev_module(ota,
                                           SM_OTA_LTE_IF,
                                           sm_sv_ev_get_module_type_by_name(ev, _module_name),
                                           _new_version,
                                           _size,
                                           _crc16,
                                           _link) < 0){
        LOG_ERR(TAG, "Could NOT request upgrade Module: %s", _module_name);
        *_bypass = true;
        return _current_version;
    }

#ifdef OTA_TESTING
    if(!strcmp(_module_name, "mc")){
        g_ota_report.m_total[0] ++;
    }else if(!strcmp(_module_name, "pmu")){
        g_ota_report.m_total[1] ++;
    }else{
        g_ota_report.m_total[2] ++;
    }
#endif

    *_bypass = false;
    return _current_version;
}

void sm_ota_handle_finish_new_fw_extract(int32_t _module_count, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    sm_sv_ev_t* ev = app->m_ev_service;
    if(!ota || !ev){
        return;
    }
    if(_module_count > 0){
        sm_sv_ota_start(ota);
    }
}

int32_t sm_ota_handle_request_bp_upgrade(void* _arg){
    LOG_WRN(TAG, "Event from IOT system: REQUEST UPGRADE FIRMWARE for BP. Currently NOT Support");
    return 0;
}

int32_t sm_ota_handle_bp_new_fw(void* _data, void* _arg){
    LOG_INF(TAG, "Event from IOT system: NEW FIRMWARE BP is available");
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    sm_sv_ev_t* ev = app->m_ev_service;

    if(!_data ||!ota || !ev){
        LOG_ERR(TAG, "Param INVALID");
        return -1;
    }

    if(sm_sv_ota_is_busy(ota)){
        return -1;
    }

    sm_cmd_upgrade_bp_data_t* upgrade_bp = (sm_cmd_upgrade_bp_data_t*)_data;
    if(upgrade_bp->m_size < SM_EV_OTA_MINIMUM_HEX_FILE_SIZE){
        LOG_WRN(TAG, "Hex file is too small, mean bypass module");
        return -1;
    }

    if(upgrade_bp->m_slot == SM_OTA_BP_ID_FOR_ALL){                     // at least 1 bp valid
        for(int id = 0; id < SM_BP_NUMBER_DEFAULT; id++){
            if(sm_ev_check_bp_general_ota_condition(ev, id, upgrade_bp->m_new_version) == 0){
                LOG_INF(TAG, "Recv request upgrade all bp, ACCEPT because have a bp can upgrade now");
                goto ACCEPT;
            }
        }
        LOG_ERR(TAG, "Recv request upgrade all bp, REJECT because none bp can upgrade now");
        return -1;
    }

    const sm_bp_data_t* bp_data = sm_sv_ev_get_bp_data(ev, upgrade_bp->m_slot);
    if(!bp_data){
        LOG_WRN(TAG, "BP is not connected, reject cmd");
        return -1;
    }
    if(sm_ev_check_bp_general_ota_condition(ev, upgrade_bp->m_slot, upgrade_bp->m_new_version) != 0){
        LOG_ERR(TAG, "Recv request upgrade bp %d, REJECT because bp cannot upgrade now", upgrade_bp->m_slot);
        return -1;
    }

    ACCEPT:
    if(sm_sv_ota_request_upgrade_bp(ota,
                                 SM_OTA_LTE_IF,
                                 upgrade_bp->m_slot,
                                 upgrade_bp->m_new_version,
                                 upgrade_bp->m_size,
                                 upgrade_bp->m_crc,
                                 upgrade_bp->m_link) < 0){
        return -1;
    }
    return sm_sv_ota_start(ota);
}

int32_t sm_ota_handle_cmd_progress(const char* _cmd, void* _arg){
    sm_hmi_app_t* app = (sm_hmi_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;

    if(!strcmp(_cmd, "STOP")){
        sm_sv_ota_stop(ota);
    }
    return 0;
}