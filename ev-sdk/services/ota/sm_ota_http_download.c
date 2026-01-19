//
// Created by vnbk on 16/09/2024.
//
#include "sm_ota_download.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"

#include "sm_crc.h"

#define TAG "SM_OTA_DOWNLOAD"

#define HTTP_DOWNLOAD_SEGMENT_TIMEOUT       (20000) //ms
#define HTTP_DOWNLOAD_SEGMENT_RETRY         20

#define _impl(x)    ((sm_http_download_impl_t*)(x))

typedef struct {
    sm_ota_download_t m_base;

    const char* m_url;
    sm_http_client_t* m_http_client;

    sm_ota_fw_storage_t* m_fw_storage;
    uint32_t m_fw_size;

    struct {
        uint8_t m_buffer[SM_OTA_DOWNLOAD_SEG_FRAME_SIZE];
        int32_t m_seg_index;
        int32_t m_total_seg;
        uint16_t m_crc;
        bool m_is_last;
    }m_seg;

    uint8_t m_retry;
    elapsed_timer_t m_timeout;

    void (*m_cb)(int32_t, bool, int32_t, const char*, void*);
    void* m_arg;
}sm_http_download_impl_t;

int32_t sm_http_download_init(sm_ota_download_t* _this, sm_ota_fw_storage_t* _storage, int32_t _fw_size);
int32_t sm_http_download_free(sm_ota_download_t* _this);
int32_t sm_http_download_reg_cb(sm_ota_download_t* _this, void (*on_err)(int32_t, bool, int32_t, const char*, void*), void* _arg);
int32_t sm_http_download_set_url(sm_ota_download_t* _this, void* _url);
int32_t sm_http_download(sm_ota_download_t* _this, void* _param);
int32_t sm_http_download_validate(sm_ota_download_t* _this, uint16_t _crc16);

static sm_ota_download_proc_t g_http_download_proc = {
        .init = sm_http_download_init,
        .free = sm_http_download_free,
        .reg_cb = sm_http_download_reg_cb,
        .set_param = sm_http_download_set_url,
        .download = sm_http_download,
        .validate = sm_http_download_validate,
};

static sm_http_download_impl_t g_ota_download = {
        .m_base = &g_http_download_proc,
        .m_url = NULL,
        .m_http_client = NULL,
        .m_fw_storage = NULL,
        .m_cb = NULL,
        .m_arg = NULL,
        .m_seg = {
                .m_seg_index = 0,
                .m_total_seg = 0,
                .m_is_last = false,
                .m_crc = CRC_16_INIT_VALUE,
        }
};

static void sm_http_client_response(const sm_http_header_t* _header, const uint8_t* _payload, uint32_t _payload_len, void* _arg);

sm_ota_download_t* sm_ota_http_download_create(sm_http_client_t* _http_client){
    if(!_http_client){
        LOG_ERR(TAG, "Create OTA download FAILURE, Reason param INVALID");
        return NULL;
    }

    g_ota_download.m_http_client = _http_client;
    g_ota_download.m_url = NULL;
    g_ota_download.m_fw_storage = NULL;
    g_ota_download.m_fw_size = 0;

    return &g_ota_download.m_base;
}


int32_t sm_http_download_init(sm_ota_download_t* _this, sm_ota_fw_storage_t* _storage, int32_t _fw_size){
    if(!_this || !_storage){
        return -1;
    }

    _impl(_this)->m_fw_storage = _storage;
    _impl(_this)->m_fw_size = _fw_size;

    _impl(_this)->m_seg.m_total_seg = _fw_size/SM_OTA_DOWNLOAD_SEG_FRAME_SIZE;
    if(_fw_size%SM_OTA_DOWNLOAD_SEG_FRAME_SIZE != 0) {
        _impl(_this)->m_seg.m_total_seg += 1;
    }
    _impl(_this)->m_seg.m_is_last = false;
    _impl(_this)->m_seg.m_crc = CRC_16_INIT_VALUE;
    _impl(_this)->m_seg.m_seg_index = 0;

    _impl(_this)->m_retry = 0;
    elapsed_timer_resetz(&_impl(_this)->m_timeout, HTTP_DOWNLOAD_SEGMENT_TIMEOUT);

    sm_ex_flash_storage_erase_full_partition(_storage);

    return 0;
}

int32_t sm_http_download_free(sm_ota_download_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_url = NULL;
    _impl(_this)->m_http_client = NULL;

    _impl(_this)->m_seg.m_is_last = false;
    _impl(_this)->m_seg.m_crc = CRC_16_INIT_VALUE;
    _impl(_this)->m_seg.m_seg_index = 0;
    _impl(_this)->m_seg.m_total_seg = 0;

    _impl(_this)->m_fw_storage = NULL;
    _impl(_this)->m_fw_size = 0;
    return 0;
}

int32_t sm_http_download_reg_cb(sm_ota_download_t* _this, void (*on_err)(int32_t, bool, int32_t, const char*, void*), void* _arg){
    if(!_this || !on_err){
        return -1;
    }
    _impl(_this)->m_cb = on_err;
    _impl(_this)->m_arg = _arg;

    return 0;
}

int32_t sm_http_download_set_url(sm_ota_download_t* _this, void* _url){
    if(!_this || !_url){
        return -1;
    }
    _impl(_this)->m_url = _url;
    return sm_http_client_set_url(_impl(_this)->m_http_client, _url);
}

int32_t sm_http_download_validate(sm_ota_download_t* _this, uint16_t _crc16){
    if(!_this){
        return -1;
    }

    return _impl(_this)->m_seg.m_crc == _crc16;
}

int32_t sm_http_download(sm_ota_download_t* _this, void* _param){
    if(!_this){
        return -1;
    }
    int32_t ret = -1;

    if(_param){
        _impl(_this)->m_url = _param;
        sm_http_client_set_url(_impl(_this)->m_http_client, _param);
    }

    if(_impl(_this)->m_seg.m_seg_index >= (_impl(_this)->m_seg.m_total_seg - 1)){
        _impl(_this)->m_seg.m_is_last = true;
    }

    if(_impl(_this)->m_seg.m_is_last){
        sm_http_client_add_header_default(_impl(_this)->m_http_client);

        int32_t frame_size = _impl(_this)->m_fw_size - _impl(_this)->m_seg.m_seg_index*SM_OTA_DOWNLOAD_SEG_FRAME_SIZE;

        sm_http_client_set_range_header(_impl(_this)->m_http_client,
                                        _impl(_this)->m_seg.m_seg_index*SM_OTA_DOWNLOAD_SEG_FRAME_SIZE,
                                        frame_size);

        ret = sm_http_client_get_method(_impl(_this)->m_http_client,
                                        _impl(_this)->m_url,
                                        sm_http_client_response,
                                        _this);
    }else{
        sm_http_client_add_header_default(_impl(_this)->m_http_client);
        sm_http_client_set_range_header(_impl(_this)->m_http_client,
                                        _impl(_this)->m_seg.m_seg_index*SM_OTA_DOWNLOAD_SEG_FRAME_SIZE,
                                        SM_OTA_DOWNLOAD_SEG_FRAME_SIZE);

        ret = sm_http_client_get_method(_impl(_this)->m_http_client,
                                        _impl(_this)->m_url,
                                        sm_http_client_response,
                                        _this);
    }

    if(ret < 0){
        LOG_ERR(TAG, "Download segment %d FAILURE", _impl(_this)->m_seg.m_seg_index);

        if(_impl(_this)->m_cb){
            _impl(_this)->m_cb(_impl(_this)->m_seg.m_seg_index,
                               _impl(_this)->m_seg.m_is_last,
                               -3,  "DOWNLOAD_FAIL",
                               _impl(_this)->m_arg);
        }

        _impl(_this)->m_retry++;
        if(_impl(_this)->m_retry >= HTTP_DOWNLOAD_SEGMENT_RETRY){
            return SM_OTA_DOWNLOAD_FAILED;
        }
        return 0;
    }

    _impl(_this)->m_seg.m_seg_index++;
    if(_impl(_this)->m_seg.m_seg_index >= _impl(_this)->m_seg.m_total_seg){
        return SM_OTA_DOWNLOAD_DONE;
    }

    return SM_OTA_DOWNLOAD_IN_PROCESS;
}

static void sm_http_client_response(const sm_http_header_t* _header, const uint8_t* _payload, uint32_t _payload_len, void* _arg){
    sm_http_download_impl_t* _this = (sm_http_download_impl_t*)_arg;
    if(!_this || _payload_len <= 0){
        LOG_ERR(TAG, "Ota download FAILURE, Reason payload length INVALID");

        if(_impl(_this)->m_cb){
            _impl(_this)->m_cb(_impl(_this)->m_seg.m_seg_index,
                               _impl(_this)->m_seg.m_is_last,
                               -1,
                               "WRONG_LENGTH",
                               _impl(_this)->m_arg);
        }

        return;
    }

    if(sm_ex_flash_storage_partition_write(_impl(_this)->m_fw_storage,
                                        _impl(_this)->m_seg.m_seg_index*SM_OTA_DOWNLOAD_SEG_FRAME_SIZE,
                                        _payload,
                                        _payload_len) < 0){
        LOG_ERR(TAG, "Could NOT store segment %d into storage", _impl(_this)->m_seg.m_seg_index);
        if(_impl(_this)->m_cb){
            _impl(_this)->m_cb(_impl(_this)->m_seg.m_seg_index,
                               _impl(_this)->m_seg.m_is_last,
                               -2,
                               "STORE_SEGMENT_FAILURE",
                               _impl(_this)->m_arg);
        }
        return;
    }

    _impl(_this)->m_seg.m_crc = sm_CRC_CalculateCRC16_init_value(_impl(_this)->m_seg.m_crc,
                                                                 _payload,
                                                                 _payload_len);

    if(_impl(_this)->m_cb){
        _impl(_this)->m_cb(_impl(_this)->m_seg.m_seg_index,
                           _impl(_this)->m_seg.m_is_last,
                           0,
                           "SUCCESS",
                           _impl(_this)->m_arg);
    }
}
