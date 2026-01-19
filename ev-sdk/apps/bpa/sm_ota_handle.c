//
// Created by vnbk on 19/09/2024.
//
#include "sm_ota_handle.h"
#include "sm_logger.h"

#include "sm_bpa_app.h"
#include "sm_sv_iot.h"
#include "sm_sv_ota.h"
#include "sm_ev_cmd.h"

#define SM_EV_OTA_MINIMUM_HEX_FILE_SIZE (100*1024)

#define TAG "SM_OTA_HANDLE"

#define SM_BPA_MODULE       SM_EV_MODULE_HMI
#define SM_BPA_MODULE_NAME  "bpa"

#define SM_OTA_HANDLE_WAITING_BP_ONLINE        (8000)
#define SM_OTA_HANDLE_WAITING_BP_DISCHARGING   (25000)

#define SM_BP_OTA_INITIALIZED_FLAG      (-1)
#define SM_BP_OTA_DONE_FLAG             (2)
#define SM_BP_OTA_IN_PROCESS_FLAG       (1)

typedef struct {
    char m_new_version[16];
    int32_t m_ota_bp_flag[SM_BP_NUMBER_DEFAULT];
    elapsed_timer_t m_waiting_time;

    void* m_app;
}sm_ota_bp_handle_t;

static sm_ota_bp_handle_t g_ota_bp_handle = {
        .m_ota_bp_flag = {SM_BP_OTA_INITIALIZED_FLAG,
                          SM_BP_OTA_INITIALIZED_FLAG,
                          SM_BP_OTA_INITIALIZED_FLAG},
};

static void sm_ota_bp_handle_process(void* _arg);
static bool sm_ota_handle_check_bp_upgrade_condition(sm_sv_bp_t* _bp_service, uint8_t _slot, const char* _new_version);

void sm_ota_on_start(void *_arg){
    LOG_INF(TAG, "OTA progress START");
}

/**
 * @brief: Notify to observer about ota progress on this module
 * @param: const char* module name
 * @param: int32_t total frame downloading,
 * @param: void* _arg
 */
void sm_ota_on_start_module(const char *_module_name, int32_t _type, void *_arg){
    LOG_INF(TAG, "OTA progress of module %s START", _module_name);
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
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
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_iot_t* iot = NULL;
    if(!app){
        return;
    }
    iot = (sm_sv_iot_t*)app->m_iot_service;
    if(!iot){
        LOG_ERR(TAG, "Missing iot service");
        return;
    }

    bool is_last_frame = (_frame_index + 1) >= _total_seg;

    if(strstr(_module_name, "hmi") != NULL){
        sm_sv_iot_notify_ota_download_status(iot, SM_BPA_MODULE_NAME, (int32_t)_total_seg, _frame_index, is_last_frame, _err, _err_msg);
    }else{ /// OTA BP
        sm_sv_iot_notify_ota_download_status(iot, _module_name, (int32_t)_total_seg, _frame_index, is_last_frame, _err, _err_msg);
        if(is_last_frame && _err == SM_OTA_ERR_NONE){
            sm_sv_bp_enter_low_data_mode(app->m_bp_service);
        }
    }
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

    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_iot_t* iot = (sm_sv_iot_t*)app->m_iot_service;

    if(!iot){
        LOG_ERR(TAG, "Missing iot service");
        return;
    }

    LOG_INF(TAG, "Upgrade module %s err %s", _module_name, _err_msg);

#ifdef OTA_TESTING
    if(!strcmp(_module_name, "mc")){
        g_ota_report.m_success[0] += _err == SM_OTA_ERR_NONE;
    }else if(!strcmp(_module_name, "pmu")){
        g_ota_report.m_success[1] += _err == SM_OTA_ERR_NONE;
    }else{
        g_ota_report.m_success[2] += _err == SM_OTA_ERR_NONE;
    }
#endif

    if(strstr(_module_name, "hmi") != NULL){
        sm_sv_iot_notify_ota_upgrade_status(iot, SM_BPA_MODULE_NAME, _version, _err, _err_msg);

        app->m_system_reboot_request.m_reboot = true;
        elapsed_timer_resetz(&app->m_system_reboot_request.m_time, 500);
    }else{
        sm_sv_iot_notify_ota_upgrade_status(iot, _module_name, _version, _err, _err_msg);

        for(int32_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
            if(g_ota_bp_handle.m_ota_bp_flag[index] == SM_BP_OTA_IN_PROCESS_FLAG){
                g_ota_bp_handle.m_ota_bp_flag[index] = SM_BP_OTA_DONE_FLAG;
                sm_sv_bp_reset(app->m_bp_service, index);
                break;
            }
        }
        elapsed_timer_resetz(&g_ota_bp_handle.m_waiting_time, SM_OTA_HANDLE_WAITING_BP_DISCHARGING);
        sm_sch_start_task(app->m_sch_task, 500, SM_SCH_REPEAT_FOREVER, sm_ota_bp_handle_process, &g_ota_bp_handle);
    }
}

void sm_ota_on_finished_module(const char *_module_name, void *_arg){
    (void)_module_name;
    (void)_arg;
}

/**
 * @brief: Notify OTA progress finished
 */
void sm_ota_on_finished(void *_arg){
    LOG_INF(TAG, "OTA progress FINISH");
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    app->m_iot_service->m_proc->sync_resume(app->m_iot_service);
    sm_sv_bp_exit_low_data_mode(app->m_bp_service);

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
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    if(!ota){
        LOG_ERR(TAG, "Param INVALID");
        return -1;
    }

    return sm_sv_ota_is_busy(ota);
}

const char *sm_ota_handle_ev_new_fw(const char *_module_name,
                                    const char *_new_version,
                                    int32_t _size,
                                    uint16_t _crc16,
                                    const char *_link,
                                    int32_t *_bypass,
                                    void *_arg) {
    LOG_ERR(TAG, "Event from IOT system: NEW FIRMWARE INFO is available. Module: %s, New version: %s", _module_name,
            _new_version);
    sm_bpa_app_t *app = (sm_bpa_app_t *) _arg;
    sm_sv_ota_t *ota = app->m_ota_service;

    if (!ota) {
        LOG_ERR(TAG, "Param INVALID");
        *_bypass = true;
        return NULL;
    }

    if (sm_sv_ota_is_busy(ota) || _size < SM_EV_OTA_MINIMUM_HEX_FILE_SIZE) {
        *_bypass = true;
        return NULL;
    }

    char current_version[16];
    memset(current_version, '\0', 16);
    sm_ev_version_to_string((char *) app->m_sw_version, g_ota_bp_handle.m_new_version);

    if (!strcmp(g_ota_bp_handle.m_new_version, _new_version)) {
        LOG_ERR(TAG, "Module %s versions are identical: %s", _module_name, g_ota_bp_handle.m_new_version);
        *_bypass = true;
        return _new_version;
    }

    if (sm_sv_ota_request_upgrade_ev_module(ota,
                                            SM_OTA_LTE_IF,
                                            SM_BPA_MODULE,
                                            _new_version,
                                            _size,
                                            _crc16,
                                            _link) < 0) {
        LOG_ERR(TAG, "Could NOT request upgrade Module: %s", _module_name);
        *_bypass = true;
        return _new_version;
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
    return g_ota_bp_handle.m_new_version;
}

void sm_ota_handle_finish_new_fw_extract(int32_t _module_count, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;
    if(!ota){
        return;
    }
    if(_module_count > 0){
        sm_sv_ota_start(ota);
    }
}

/***************************************** Upgrade BP *******************************************************/

static void sm_ota_bp_handle_process(void* _arg){
    sm_ota_bp_handle_t* ota = (sm_ota_bp_handle_t*)_arg;
    if(!ota){
        return;
    }
    sm_bpa_app_t* app = (sm_bpa_app_t*)ota->m_app;

    if(!elapsed_timer_get_remain(&ota->m_waiting_time)){
        ota->m_ota_bp_flag[0] = SM_BP_OTA_INITIALIZED_FLAG;
        ota->m_ota_bp_flag[1] = SM_BP_OTA_INITIALIZED_FLAG;
        ota->m_ota_bp_flag[2] = SM_BP_OTA_INITIALIZED_FLAG;
        elapsed_timer_reset(&ota->m_waiting_time);
        ota->m_app = NULL;

        sm_sv_bp_exit_low_data_mode(app->m_bp_service);

        sm_sch_cancel_taskz(app->m_sch_task, sm_ota_bp_handle_process);
        return;
    }
    bool condition = false;
    int32_t index = 0;
    for(index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        condition = sm_ota_handle_check_bp_upgrade_condition(app->m_bp_service, index, ota->m_new_version);

        if(condition && g_ota_bp_handle.m_ota_bp_flag[index] == SM_BP_OTA_DONE_FLAG){
            continue;
        }
        if(condition){
            g_ota_bp_handle.m_ota_bp_flag[index] = SM_BP_OTA_IN_PROCESS_FLAG;
            break;
        }
    }

    if(condition && index < SM_BP_NUMBER_DEFAULT){
        sm_sv_bp_enter_low_data_mode(app->m_bp_service);
        sm_sv_ota_request_upgrade_bp_continue(app->m_ota_service,
                                              index,
                                              ota->m_new_version);
        sm_sch_cancel_taskz(app->m_sch_task, sm_ota_bp_handle_process);
    }
}

int32_t sm_ota_handle_request_bp_upgrade(void* _arg){
    return sm_ota_handle_request_upgrade(_arg);
}

static bool sm_ota_handle_check_bp_upgrade_common_condition(sm_sv_bp_t* _bp_service, uint8_t _slot){
    for(uint8_t id = 0; id < SM_BP_NUMBER_DEFAULT; id++){
        if(!sm_sv_bp_is_connected(_bp_service, id)){
            continue;
        }

        const sm_bp_data_t* bp_data = sm_sv_bp_get_data(_bp_service, id);

        // check all bp
        if(bp_data->m_cur >= EV_BP_LOWEST_CHARGING_CUR){
            LOG_INF(TAG, "Start ota because ev is charging");
            return true;
        }

        if(id == _slot){
            continue;
        }

        if(bp_data->m_state == BP_STATE_FORCE_DISCHARGE || bp_data->m_state == BP_STATE_DISCHARGING){
            LOG_INF(TAG, "Start ota bp %d because have other bp discharge with cur %d", _slot, bp_data->m_cur);
            return true;
        }
    }
    return false;
}

static bool sm_ota_handle_check_bp_upgrade_condition(sm_sv_bp_t* _bp_service, uint8_t _slot, const char* _new_version){
    if(!sm_sv_bp_is_connected(_bp_service, _slot)){
        return false;
    }

    const sm_bp_data_t* bp_data = sm_sv_bp_get_data(_bp_service, _slot);

    char current_version[16];
    memset(current_version, '\0', 16);
    sm_ev_version_to_string((char*)bp_data->m_version, current_version);

    if(!strcmp(current_version, _new_version)){
        LOG_ERR(TAG, "BP %d versions are identical: %s", _slot, current_version);
        return false;
    }

    return sm_ota_handle_check_bp_upgrade_common_condition(_bp_service, _slot);
}

int32_t sm_ota_handle_bp_new_fw(void* _data, void* _arg){
    LOG_INF(TAG, "Event from IOT system: NEW FIRMWARE BP is available");
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    if(!_data || !app){
        LOG_ERR(TAG, "Param INVALID");
        return -1;
    }
    sm_sv_ota_t* ota = app->m_ota_service;
    sm_sv_bp_t* bpm = app->m_bp_service;

    if(!ota || !bpm){
        LOG_ERR(TAG, "Param INVALID");
        return -1;
    }

    if(sm_sv_ota_is_busy(ota)){
        return -1;
    }

    sm_cmd_upgrade_bp_data_t* upgrade_bp_data = (sm_cmd_upgrade_bp_data_t*)_data;
    if(upgrade_bp_data->m_size < SM_EV_OTA_MINIMUM_HEX_FILE_SIZE){
        LOG_WRN(TAG, "Hex file is too small, mean bypass module");
        return -1;
    }

    bool condition = false;
    uint8_t slot  = upgrade_bp_data->m_slot;
    if(slot < SM_SV_BP_NUMBER_DEFAULT){
        condition = sm_ota_handle_check_bp_upgrade_condition(bpm, slot, upgrade_bp_data->m_new_version);
    }else{
        memset(g_ota_bp_handle.m_new_version, '\0', sizeof(g_ota_bp_handle.m_new_version));
        strcpy(g_ota_bp_handle.m_new_version, upgrade_bp_data->m_new_version);
        for(uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++){
            condition = sm_ota_handle_check_bp_upgrade_condition(bpm, index, upgrade_bp_data->m_new_version);
            if(condition){
                g_ota_bp_handle.m_ota_bp_flag[index] = SM_BP_OTA_IN_PROCESS_FLAG;
                g_ota_bp_handle.m_app = app;
                slot = index;
                break;
            }
        }
    }

    if (!condition || sm_sv_ota_request_upgrade_bp(ota,
                                                   SM_OTA_LTE_IF,
                                                   slot,
                                                   upgrade_bp_data->m_new_version,
                                                   upgrade_bp_data->m_size,
                                                   upgrade_bp_data->m_crc,
                                                   upgrade_bp_data->m_link) < 0) {
        return -1;
    }
    return sm_sv_ota_start(ota);

   /* if(!condition){
        return -1;
    }
    sm_sv_bp_enter_low_data_mode(app->m_bp_service);
    sm_sv_ota_request_upgrade_bp_continue(app->m_ota_service,
                                          slot);
    return 0;*/
}

int32_t sm_ota_handle_cmd_progress(const char* _cmd, void* _arg){
    sm_bpa_app_t* app = (sm_bpa_app_t*)_arg;
    sm_sv_ota_t* ota = app->m_ota_service;

    if(!strcmp(_cmd, "STOP")){
        sm_sv_ota_stop(ota);
    }
    return 0;
}
