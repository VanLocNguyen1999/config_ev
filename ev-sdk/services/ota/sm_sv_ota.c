//
// Created by vnbk on 30/08/2024.
//
#include "sm_sv_ota.h"
#include "sm_logger.h"
#include "sm_string_t.h"

#include "sm_ota_download.h"
#include "sm_sv_ota_config.h"

#define TAG "sm_sv_ota"

#define _impl(x)            ((sm_ota_impl_t*)(x))

#define DOWNLOAD_INTERFACE_NUMBER   1

typedef struct{
    uint8_t m_module;
    uint8_t m_module_id;
    string_t* m_new_version;
    int32_t m_size;
    int32_t m_crc;
    string_t* m_link;
}sm_ota_fw_info_t;

typedef struct {
    sm_http_client_t* m_http_client;
    sm_sv_upgrade_t* m_upgrading_service;
    sm_ota_download_t* m_download[DOWNLOAD_INTERFACE_NUMBER];
    sm_ota_download_t* m_download_if_using;

    sm_ota_fw_storage_t* m_fw_hex_storage;

    sm_ota_fw_storage_t* m_fw_bin_file_output;

    uint8_t m_state;

    sm_ota_fw_info_t m_fws_info[SM_EV_MODULE_NUMBER];
    sm_ota_fw_info_t* m_module_in_process;

    fw_signature_storage_fn_t m_fw_signature_storage_fn;
    void* m_fw_signature_storage_fn_arg;

    sm_ota_event_t* m_event_handle;
    void* m_event_arg;
}sm_ota_impl_t;

static sm_ota_impl_t g_ota_service = {
        .m_http_client = NULL,
        .m_upgrading_service = NULL,
        .m_download = NULL,
        .m_state = SM_OTA_STT_IDLE,
        .m_event_arg = NULL,
        .m_event_handle = NULL
};

static bool sm_ota_check_module_is_ready(sm_ota_impl_t* _this, uint8_t _module_type){
    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        if(_impl(_this)->m_fws_info[index].m_module == _module_type){
            return true;
        }
    }
    return false;
}

static void sm_ota_reset_fw_info(sm_ota_impl_t* _this, sm_ota_fw_info_t* _fw_info){
    (void)_this;
    _fw_info->m_module = SM_EV_MODULE_NUMBER;
    _fw_info->m_crc = 0;
    _fw_info->m_size = 0;

    if(_fw_info->m_new_version){
        string_dispose(_fw_info->m_new_version);
    }
    _fw_info->m_new_version = NULL;

    if(_fw_info->m_link){
        string_dispose(_fw_info->m_link);
    }
    _fw_info->m_link = NULL;
}

static void sm_http_download_on_err(int32_t _seg, bool _last_seg, int32_t _err, const char* _err_msg, void* _arg){
    sm_ota_impl_t* _this = (sm_ota_impl_t*)_arg;
    if(!_this){
        LOG_ERR(TAG, "Could NOT create download owner");
        return;
    }
    LOG_ERR(TAG, "Download segment %d %s", _seg, _err == 0 ? "SUCCESS" : "FAILURE");
    if(_this->m_event_handle && _this->m_event_handle->on_downloading_status){

        uint32_t total_seg = g_ota_service.m_module_in_process->m_size / SM_OTA_DOWNLOAD_SEG_FRAME_SIZE;
        if(g_ota_service.m_module_in_process->m_size % SM_OTA_DOWNLOAD_SEG_FRAME_SIZE != 0){
            total_seg++;
        }

        _this->m_event_handle->on_downloading_status(sm_ev_convert_module_to_string(_this->m_module_in_process->m_module),
                                                     _seg,
                                                     total_seg,
                                                     _err,
                                                     _err_msg,
                                                     _this->m_event_arg);
    }
}

static void sm_ota_on_upgrade_dev(uint8_t _devId, int32_t _err, void* _arg){
    sm_ota_impl_t* _this = (sm_ota_impl_t*)_arg;
    if(!_this){
        LOG_ERR(TAG, "Could NOT create download owner");
        return;
    }
    if(!_this->m_event_handle || !_this->m_event_handle->on_upgrading_status){
        LOG_WRN(TAG, "No handle process upgrading status");
        return;
    }

    if(_err == SM_SV_EV_UPGRADE_ERR_NONE){
        LOG_INF(TAG, "Upgrade status SUCCESS");
        _this->m_event_handle->on_upgrading_status(sm_ev_convert_module_to_string(_this->m_module_in_process->m_module),
                                                   _this->m_module_in_process->m_new_version->buffer,
                                                   SM_OTA_ERR_NONE,
                                                   sm_sv_ota_convert_err_to_string(SM_OTA_ERR_NONE),
                                                   _this->m_event_arg);
    }else if(_err == SM_SV_EV_UPGRADE_ERR_REBOOT){
        LOG_INF(TAG, "Upgrade status FAILURE, Could NOT reboot device");
        _this->m_event_handle->on_upgrading_status(sm_ev_convert_module_to_string(_this->m_module_in_process->m_module),
                                                   _this->m_module_in_process->m_new_version->buffer,
                                                   SM_OTA_ERR_DEVICE_NOT_RESPONSE,
                                                   sm_sv_ota_convert_err_to_string(SM_OTA_ERR_DEVICE_NOT_RESPONSE),
                                                   _this->m_event_arg);
    }else if(_err == SM_SV_EV_UPGRADE_ERR_TIMEOUT){
        LOG_INF(TAG, "Upgrade status FAILURE, Timeout upgrading");
        _this->m_event_handle->on_upgrading_status(sm_ev_convert_module_to_string(_this->m_module_in_process->m_module),
                                                   _this->m_module_in_process->m_new_version->buffer,
                                                   SM_OTA_ERR_UPGRADING_TIMEOUT,
                                                   sm_sv_ota_convert_err_to_string(SM_OTA_ERR_UPGRADING_TIMEOUT),
                                                   _this->m_event_arg);
    }else{
        LOG_INF(TAG, "Upgrade status FAILURE, System error");
        _this->m_event_handle->on_upgrading_status(sm_ev_convert_module_to_string(_this->m_module_in_process->m_module),
                                                   _this->m_module_in_process->m_new_version->buffer,
                                                   SM_OTA_ERR_INTERNAL_SYSTEM_ERROR,
                                                   sm_sv_ota_convert_err_to_string(SM_OTA_ERR_INTERNAL_SYSTEM_ERROR),
                                                   _this->m_event_arg);
    }
}

static void sm_ota_on_finish_upgrade_dev(void* _arg){
    sm_ota_impl_t* _this = (sm_ota_impl_t*)_arg;
    if(!_this) {
        LOG_ERR(TAG, "Could NOT create download owner");
        return;
    }
    LOG_INF(TAG, "Finish upgrade module %s", sm_ev_convert_module_to_string(_this->m_module_in_process->m_module));

    _this->m_state = SM_OTA_STT_UPGRADED;
}

static sm_sv_ev_upgrade_callback_t g_upgrade_dev_cb = {
        .m_onUpgradeDev = sm_ota_on_upgrade_dev,
        .m_onFinishUpgradeProcess = sm_ota_on_finish_upgrade_dev
};

sm_sv_ota_t* sm_sv_ota_create(sm_http_client_t* _http_client, sm_sv_upgrade_t* _upgrade_service, sm_ota_fw_storage_t* _fw_storage){
    if(!_http_client || !_upgrade_service || !_fw_storage){
        LOG_ERR(TAG, "Create OTA Service FAILURE, Reason param INVALID");
        return NULL;
    }
    sm_ota_impl_t* ota_service = &g_ota_service;

    ota_service->m_http_client = _http_client;
    ota_service->m_upgrading_service = _upgrade_service;

    sm_ota_download_t* ota_download = sm_ota_http_download_create(_http_client);
    if(!ota_download){
        LOG_ERR(TAG, "Create OTA service FAILURE, Reason: Could NOT create downloading");
        return NULL;
    }
    ota_download->m_proc->reg_cb(ota_download, sm_http_download_on_err, ota_service);
    ota_service->m_download[SM_OTA_LTE_IF] = ota_download;

    ota_service->m_upgrading_service = _upgrade_service;
    g_upgrade_dev_cb.arg = ota_service;
    sm_sv_ev_upgrade_set_callback(_upgrade_service, &g_upgrade_dev_cb);

    ota_service->m_fw_hex_storage = _fw_storage;

    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        sm_ota_reset_fw_info(ota_service, &ota_service->m_fws_info[index]);
    }

    return ota_service;
}


int32_t sm_sv_ota_set_fw_output_storage_if(sm_sv_ota_t* _this,
                                           sm_ota_fw_storage_t* _fw_storage_output,
                                           fw_signature_storage_fn_t _fn,
                                           void* _arg){
    if(!_this){
        return -1;
    }

    _impl(_this)->m_fw_bin_file_output = _fw_storage_output;
    _impl(_this)->m_fw_signature_storage_fn = _fn;
    _impl(_this)->m_fw_signature_storage_fn_arg = _arg;
    return 0;
}

int32_t sm_sv_ota_destroy(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }
    for(int index = 0; index < SM_OTA_IF_NUMBER; index++){
        if(_impl(_this)->m_download[index]){
            _impl(_this)->m_download[index]->m_proc->free(_impl(_this)->m_download[index]);
            _impl(_this)->m_download[index] = NULL;
        }
    }

    _impl(_this)->m_download_if_using = NULL;
    _impl(_this)->m_upgrading_service = NULL;
    _impl(_this)->m_state = SM_OTA_STT_IDLE;
    _impl(_this)->m_event_handle = NULL;
    _impl(_this)->m_event_arg = NULL;

    return 0;
}

int32_t sm_sv_ota_reg_event(sm_sv_ota_t* _this, sm_ota_event_t* _event_handle, void* _arg){
    if(!_this || !_event_handle){
        return -1;
    }

    _impl(_this)->m_event_handle = _event_handle;
    _impl(_this)->m_event_arg = _arg;

    return 0;
}

int32_t sm_sv_ota_is_busy(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }

    return (_impl(_this)->m_state == SM_OTA_STT_IDLE) ? 0 : 1;
}

int32_t sm_sv_ota_get_status(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }

    return _impl(_this)->m_state;
}

int32_t sm_sv_ota_request_upgrade_bp(sm_sv_ota_t* _this,
                                  uint8_t _if_type,
                                  uint8_t _bp_id,
                                  const char* _new_version,
                                  int32_t _size,
                                  int32_t _crc,
                                  const char* _link){
    if(!_this){
        return -1;
    }

    if(_if_type >= SM_OTA_IF_NUMBER || !_impl(_this)->m_download[_if_type]){
        LOG_ERR(TAG, "OTA Interface NOT support");
        return -1;
    }

    if(sm_ota_check_module_is_ready(_impl(_this), SM_EV_MODULE_BP)){
        LOG_WRN(TAG, "Module %d is in OTA process, Please try again after the process finish", SM_EV_MODULE_BP);
        return -1;
    }

    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        if(_impl(_this)->m_fws_info[index].m_module == SM_EV_MODULE_NUMBER){
            _impl(_this)->m_fws_info[index].m_module = SM_EV_MODULE_BP;
            _impl(_this)->m_fws_info[index].m_module_id = _bp_id;
            _impl(_this)->m_fws_info[index].m_size = _size;
            _impl(_this)->m_fws_info[index].m_crc = _crc;
            _impl(_this)->m_fws_info[index].m_new_version = string_createz(_new_version);
            _impl(_this)->m_fws_info[index].m_link = string_createz(_link);
            _impl(_this)->m_download_if_using = _impl(_this)->m_download[_if_type];

            return 0;
        }
    }

    return -1;
}

int32_t sm_sv_ota_request_upgrade_bp_continue(sm_sv_ota_t* _this,
                                              uint8_t _bp_id,
                                              const char* _new_version){
    if(!_this){
        return -1;
    }

    if(sm_ota_check_module_is_ready(_impl(_this), SM_EV_MODULE_BP)){
        LOG_WRN(TAG, "Module %d is in OTA process, Please try again after the process finish", SM_EV_MODULE_BP);
        return -1;
    }

    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        if(_impl(_this)->m_fws_info[index].m_module == SM_EV_MODULE_NUMBER){
            _impl(_this)->m_fws_info[index].m_module = SM_EV_MODULE_BP;
            _impl(_this)->m_fws_info[index].m_module_id = _bp_id;
            _impl(_this)->m_fws_info[index].m_size = 0;
            _impl(_this)->m_fws_info[index].m_crc = 0;
            _impl(_this)->m_fws_info[index].m_new_version = string_createz(_new_version);
            _impl(_this)->m_fws_info[index].m_link = NULL;

            _impl(_this)->m_module_in_process = &_impl(_this)->m_fws_info[index];
            _impl(_this)->m_state = SM_OTA_STT_PREPARE_UPGRADING;

            return 0;
        }
    }

    return -1;
}


int32_t sm_sv_ota_request_upgrade_ev_module(sm_sv_ota_t* _this,
                                  uint8_t _if_type,
                                  uint8_t _module_type,
                                  const char* _new_version,
                                  int32_t _size,
                                  int32_t _crc,
                                  const char* _link){
    if(!_this){
        return -1;
    }

    if(_if_type >= SM_OTA_IF_NUMBER || !_impl(_this)->m_download[_if_type]){
        LOG_ERR(TAG, "OTA Interface NOT support");
        return -1;
    }

    if(sm_ota_check_module_is_ready(_impl(_this), _module_type)){
        LOG_WRN(TAG, "Module %d is in OTA process, Please try again after the process finish", _module_type);
        return -1;
    }

    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        if(_impl(_this)->m_fws_info[index].m_module == SM_EV_MODULE_NUMBER){
            _impl(_this)->m_fws_info[index].m_module = _module_type;
            _impl(_this)->m_fws_info[index].m_size = _size;
            _impl(_this)->m_fws_info[index].m_crc = _crc;
            _impl(_this)->m_fws_info[index].m_new_version = string_createz(_new_version);
            _impl(_this)->m_fws_info[index].m_link = string_createz(_link);
            _impl(_this)->m_download_if_using = _impl(_this)->m_download[_if_type];

            return 0;
        }
    }

    return -1;
}

static sm_ota_fw_info_t* sm_sv_ota_choose_module(sm_sv_ota_t* _this, uint8_t _module_type){
    sm_ota_fw_info_t* fw_info = NULL;
    for(int index = 0; index < SM_EV_MODULE_NUMBER; index++){
        fw_info = &_impl(_this)->m_fws_info[index];
        if(!fw_info){
            continue;
        }
        if(fw_info->m_module == _module_type){
            _impl(_this)->m_module_in_process = fw_info;
            return fw_info;
        }
    }
    return NULL;
}

int32_t sm_sv_ota_start(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_state = SM_OTA_STT_PREPARE;
    return 0;
}

static int32_t sm_sv_ota_get_module_requested(sm_sv_ota_t* _this){
    sm_ota_fw_info_t* module_fw = sm_sv_ota_choose_module(_this, SM_EV_MODULE_MC);
    if(module_fw){
        _impl(_this)->m_module_in_process = module_fw;
        _impl(_this)->m_state = SM_OTA_STT_STARTING;
        return 0;
    }

    module_fw = sm_sv_ota_choose_module(_this, SM_EV_MODULE_PMU);
    if(module_fw){
        _impl(_this)->m_module_in_process = module_fw;
        _impl(_this)->m_state = SM_OTA_STT_STARTING;
        return 0;
    }

    module_fw = sm_sv_ota_choose_module(_this, SM_EV_MODULE_HMI);
    if(module_fw){
        _impl(_this)->m_module_in_process = module_fw;
        _impl(_this)->m_state = SM_OTA_STT_STARTING;
        return 0;
    }

    module_fw = sm_sv_ota_choose_module(_this, SM_EV_MODULE_BP);
    if(module_fw){
        _impl(_this)->m_module_in_process = module_fw;
        _impl(_this)->m_state = SM_OTA_STT_STARTING;
        return 0;
    }

    return -1;
}

int32_t sm_sv_ota_stop(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_sv_ota_pause(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_sv_ota_resume(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

static int32_t sm_sv_ota_prepare_downloading(sm_ota_impl_t* _this){
    if(!_impl(_this)->m_download_if_using){
        return -1;
    }

    _impl(_this)->m_download_if_using->m_proc->init( _impl(_this)->m_download_if_using,
                                                     _impl(_this)->m_fw_hex_storage,
                                                     _impl(_this)->m_module_in_process->m_size);

    _impl(_this)->m_download_if_using->m_proc->set_param(_impl(_this)->m_download_if_using,
                                                         _impl(_this)->m_module_in_process->m_link->buffer);
    return 0;
}

static int32_t sm_sv_ota_prepare_upgrading(sm_ota_impl_t* _this){
    if(_impl(_this)->m_module_in_process->m_module == SM_EV_MODULE_MC){
        sm_sv_ev_upgrade_mc(_impl(_this)->m_upgrading_service,
                            _impl(_this)->m_fw_hex_storage);
    }else if(_impl(_this)->m_module_in_process->m_module == SM_EV_MODULE_PMU){
        sm_sv_ev_upgrade_pmu(_impl(_this)->m_upgrading_service,
                             _impl(_this)->m_fw_hex_storage);
    }else if(_impl(_this)->m_module_in_process->m_module == SM_EV_MODULE_HMI){
        sm_sv_ev_upgrade_hmi(_impl(_this)->m_upgrading_service,
                             _impl(_this)->m_fw_hex_storage,
                             _impl(_this)->m_fw_bin_file_output,
                             _impl(_this)->m_fw_signature_storage_fn,
                             _impl(_this)->m_fw_signature_storage_fn_arg);

    }else if(_impl(_this)->m_module_in_process->m_module == SM_EV_MODULE_BP){
        sm_sv_ev_upgrade_bp(_impl(_this)->m_upgrading_service,
                            _impl(_this)->m_module_in_process->m_module_id,
                            _impl(_this)->m_fw_hex_storage);
    }else{
        LOG_INF(TAG, "Device NOT support");
        return -1;
    }
    return 0;
}

int32_t sm_sv_ota_process(sm_sv_ota_t* _this){
    if(!_this){
        return -1;
    }

    int32_t ret = -1;

    switch (_impl(_this)->m_state) {
        case SM_OTA_STT_IDLE:
            break;
        case SM_OTA_STT_PREPARE:
            if(_impl(_this)->m_event_handle->on_start){
                _impl(_this)->m_event_handle->on_start(_impl(_this)->m_event_arg);
            }
            _impl(_this)->m_state = SM_OTA_STT_STARTING;
            break;
        case SM_OTA_STT_STARTING:
            ret =  sm_sv_ota_get_module_requested(_this);
            if(ret < 0){
                LOG_ERR(TAG, "No module is ota progress");

                if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_finished){
                    _impl(_this)->m_event_handle->on_finished(_impl(_this)->m_event_arg);
                }

                _impl(_this)->m_state = SM_OTA_STT_IDLE;
                break;
            }

            if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_start_module){
                _impl(_this)->m_event_handle->on_start_module(sm_ev_convert_module_to_string(_impl(_this)->m_module_in_process->m_module),
                                                              _impl(_this)->m_module_in_process->m_module,
                                                              _impl(_this)->m_event_arg);
            }

            if(_impl(_this)->m_module_in_process){
                sm_sv_ota_prepare_downloading(_this);
                _impl(_this)->m_state = SM_OTA_STT_DOWNLOADING;
            }
            LOG_INF(TAG, "Module %s prepare is ota new firmware", sm_ev_convert_module_to_string(_impl(_this)->m_module_in_process->m_module));
            break;
        case SM_OTA_STT_DOWNLOADING:
            ret = _impl(_this)->m_download_if_using->m_proc->download(_impl(_this)->m_download_if_using, NULL);
            if(ret == SM_OTA_DOWNLOAD_DONE){
                _impl(_this)->m_state = SM_OTA_STT_DOWNLOADED;
            }
            if(ret == SM_OTA_DOWNLOAD_FAILED){
                _impl(_this)->m_state = SM_OTA_STT_UPGRADED;
            }
            break;
        case SM_OTA_STT_DOWNLOADED:
            LOG_INF(TAG, "Downloading is finish, Next step: Validate firmware signature");
            _impl(_this)->m_state = SM_OTA_STT_FW_VERIFYING;
            break;
        case SM_OTA_STT_FW_VERIFYING:
            ret = _impl(_this)->m_download_if_using->m_proc->validate(_impl(_this)->m_download_if_using,
                                                                      _impl(_this)->m_module_in_process->m_crc);
            if(!ret){
                LOG_ERR(TAG, "Firmware is WRONG CRC. Please check again");
                if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_downloading_status){
                    int32_t total_seg = g_ota_service.m_module_in_process->m_size / SM_OTA_DOWNLOAD_SEG_FRAME_SIZE;
                    if(g_ota_service.m_module_in_process->m_size % SM_OTA_DOWNLOAD_SEG_FRAME_SIZE != 0){
                        total_seg++;
                    }
                    _impl(_this)->m_event_handle->on_downloading_status(sm_ev_convert_module_to_string(_impl(_this)->m_module_in_process->m_module),
                                                                        total_seg,
                                                                        total_seg,
                                                                        SM_OTA_ERR_CRC_FILE_INVALID,
                                                                        sm_sv_ota_convert_err_to_string(SM_OTA_ERR_CRC_FILE_INVALID),
                                                                        _impl(_this)->m_event_arg);

                    _impl(_this)->m_state = SM_OTA_STT_NUMBER;
                    break;
                }
            }
            LOG_INF(TAG, "Firmware download SUCCESSFULLY");
            _impl(_this)->m_state = SM_OTA_STT_PREPARE_UPGRADING;
            break;
        case SM_OTA_STT_PREPARE_UPGRADING:
            ret = sm_sv_ota_prepare_upgrading(_impl(_this));
            if(ret < 0){
                _impl(_this)->m_state = SM_OTA_STT_NUMBER;
            }else{
                _impl(_this)->m_state = SM_OTA_STT_UPGRADING;
            }
            break;
        case SM_OTA_STT_UPGRADING:
            sm_sv_ev_upgrade_process(_impl(_this)->m_upgrading_service);
            break;
        case SM_OTA_STT_UPGRADED:
            _impl(_this)->m_state = SM_OTA_STT_NUMBER;
            if(_impl(_this)->m_event_handle && _impl(_this)->m_event_handle->on_finished_module){
                _impl(_this)->m_event_handle->on_finished_module(sm_ev_convert_module_to_string(_impl(_this)->m_module_in_process->m_module),
                                                              _impl(_this)->m_event_arg);
            }
            break;
        case SM_OTA_STT_NUMBER:
            sm_ota_reset_fw_info(_this, _impl(_this)->m_module_in_process);
            _impl(_this)->m_module_in_process = NULL;

            _impl(_this)->m_state = SM_OTA_STT_STARTING;
            break;
        default:
            break;
    }

    return 0;
}
