//
// Created by vnbk on 18/09/2024.
//
#include <curl/curl.h>

#include "sm_http_client.h"
#include "sm_logger.h"
#include "sm_string_t.h"

#define TAG "HTTP_CLIENT_CURL"

#define _impl(x)    ((sm_http_curl_t*)(x))

typedef struct{
    CURL* m_handle;
    struct curl_slist* m_headers;

    sm_http_client_response_fn_t m_response;
    void* m_arg;
}sm_http_curl_t;

static sm_http_curl_t g_http_client = {
        .m_handle = NULL,
        .m_headers = NULL,
        .m_response = NULL,
        .m_arg = NULL
};

static size_t http_write_callback(void *data, size_t size, size_t nmemb, void *_arg){
    sm_http_curl_t* http_client = (sm_http_curl_t*)_arg;

//    LOG_DBG(TAG, "Data len is recveied: %d", size*nmemb);
    int32_t data_len = (int32_t)(size*nmemb);

    if(http_client && http_client->m_response){
        http_client->m_response(NULL, data, data_len, http_client->m_arg);
    }

    return data_len;
}

sm_http_client_t* sm_http_client_create(void* _net){
    (void)_net;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    g_http_client.m_handle = curl_easy_init();
    if(!g_http_client.m_handle){
        LOG_ERR(TAG, "CURL handle NULL");
        return NULL;
    }

    return &g_http_client;
}

int32_t sm_http_client_free(sm_http_client_t* _this){
    if(_impl(_this)->m_handle){
        LOG_DBG(TAG, "Clear up CURL handle: 0x%2X", _impl(_this)->m_handle);
        curl_easy_cleanup(_impl(_this)->m_handle);
    }
    if(_impl(_this)->m_headers){
        curl_slist_free_all(_impl(_this)->m_headers);
    }
    _impl(_this)->m_handle = NULL;
    _impl(_this)->m_headers = NULL;

    curl_global_cleanup();
    return 0;
}

int32_t sm_http_client_add_header_default(sm_http_client_t* _this){
    if(!_this){
        return -1;
    }
    curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_HEADER, 0);

    if(curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_WRITEFUNCTION, http_write_callback) != CURLE_OK){
        LOG_ERR(TAG, "Error: HTTPDownload: WriteFunction INVALID\n");
        return -1;
    }

    if(curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_WRITEDATA, _this) != CURLE_OK){
        LOG_ERR(TAG, "Error: HTTPDownload: WRITE data INVALID\n");
        return -1;
    }

    return 0;
}

int32_t sm_http_client_add_header(sm_http_client_t* _this, const char* _type, const char* _content){
    if(!_this || !_type || !_content){
        return -1;
    }

    char value[512] = {0,};
    int len = sprintf(value, "%s: %s", _type, _content);
    value[len] = '\0';

    _impl(_this)->m_headers = curl_slist_append(_impl(_this)->m_handle, value);
    /* set our custom set of headers */
    int  ret = curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_HTTPHEADER, _impl(_this)->m_headers);
    if(ret != CURLE_OK){
        LOG_ERR(TAG, "Set HTTP header FAILURE");
    }
    return ret;
}

int32_t sm_http_client_add_headers(sm_http_client_t* _this, const sm_http_header_t* _headers, uint32_t _len){
    return 0;
}

int32_t sm_http_client_set_range_header(sm_http_client_t* _this, uint32_t _start, uint32_t _length){
    if(!_this){
        return -1;
    }

    char value[128] = {0,};
    int len = sprintf(value, "%d-%d", _start, _start + _length - 1);
    value[len] = '\0';

    if(curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_RANGE, value) != CURLE_OK){
        LOG_ERR(TAG, "Could NOT set range to download resource");
        return -1;
    }

    return 0;
}

int32_t sm_http_client_set_url(sm_http_client_t* _this, const char* _url){
    if(!_this || !_url){
        return -1;
    }
    if(curl_easy_setopt(_impl(_this)->m_handle, CURLOPT_URL, _url) != CURLE_OK){
        LOG_ERR(TAG, "Error: HTTPDownload: download: url invalid\n");
        return -1;
    }
    return 0;
}

/**
 *
 * @param _this
 * @param _url
 * @param _response_header
 * @param _payload
 * @return HTTP_STATUS_CODE
 */
int32_t sm_http_client_get_method(sm_http_client_t* _this,
                                  const char* _url,
                                  sm_http_client_response_fn_t _cb,
                                  void* _arg){
    if(!_this){
        return -1;
    }
    int result = -1;
    if(_url){
        result = sm_http_client_set_url(_this, _url);
        if(result != CURLE_OK){
            return -1;
        }
    }

    _impl(_this)->m_response = _cb;
    _impl(_this)->m_arg = _arg;

    result = curl_easy_perform(_impl(_this)->m_handle);

    if(result != CURLE_OK){
        LOG_ERR(TAG, "Error: HTTPDownload: download: Don't download file\n");
    }else{
//        LOG_DBG(TAG, "Download SUCCESS");
    }

    if(_impl(_this)->m_headers){
        curl_slist_free_all(_impl(_this)->m_headers);
    }
    _impl(_this)->m_headers = NULL;

    return result;
}

/**
 *
 * @param _this
 * @param _url
 * @param _header
 * @return HTTP_STATUS_CODE
 */
int32_t sm_http_client_post_method(sm_http_client_t* _this,
                                   const char* _url,
                                   const char* _request_payload,
                                   sm_http_client_response_fn_t _cb,
                                   void* _arg){
    return -1;
}
