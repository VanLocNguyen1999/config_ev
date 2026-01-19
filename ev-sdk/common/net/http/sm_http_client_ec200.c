//
// Created by admin on 9/16/2024.
//
#include <sm_logger.h>
#include <stdio.h>

#include "sm_http_client.h"
#include "sm_ec200_http.h"
#include "sm_hal_delay.h"
#include "string.h"

#define RESPONSE_HEADER (0)

#define SM_HTTP_CLIENT_HEADER_NUM_MAX           32
#define SM_HTTP_CLIENT_HEADER_SIZE              1024
#define SM_HTTP_CLIENT_BODY_SIZE                2048

static const char* TAG = "HTTP_CLIENT_EC200";

#define _impl(x)    ((sm_mqtt_client_impl_t*)(x))

enum {
    FAILURE = -1,
    SUCCESS = 0,
};

typedef enum {
    GET,
    POST
}METHOD;


typedef struct {
    sm_ec200_t*         m_if;
    uint8_t             m_header_num;
    sm_http_header_t*   m_header;
    uint32_t            m_get_start;
    uint32_t            m_get_len;
#if RESPONSE_HEADER
    char                m_header_str[SM_HTTP_CLIENT_HEADER_SIZE];
#endif
    char                m_body[SM_HTTP_CLIENT_BODY_SIZE];
}sm_mqtt_client_impl_t;

sm_mqtt_client_impl_t g_http_client;

#if RESPONSE_HEADER
sm_http_header_t res_header_buffer[SM_HTTP_CLIENT_HEADER_NUM_MAX];
sm_http_header_t req_header_buffer[SM_HTTP_CLIENT_HEADER_NUM_MAX];
#endif


static void http_unpack(sm_http_client_t *client, char *data);
static void http_parse_header(sm_http_header_t* _header, const char* _buff);

sm_http_client_t* sm_http_client_create(void* _net) {
    if (!_net) {
        return NULL;
    }
    g_http_client.m_if = _net;
    g_http_client.m_header_num = 0;

#if RESPONSE_HEADER
    g_http_client.m_header = req_header_buffer;
#endif
    return (sm_http_client_t*)&g_http_client;
}

int32_t sm_http_client_init(sm_http_client_t* _this){
    if (!_this) {
        return FAILURE;
    }
    int32_t err = 0;
    err += sm_ec200_http_config(_impl(_this)->m_if, "contextid",1);
    err += sm_ec200_http_config(_impl(_this)->m_if, "contenttype",2);
#if RESPONSE_HEADER
    err += sm_ec200_http_config(_impl(_this)->m_if, "responseheader",1);
#endif


    if (err){
       return FAILURE;
    }
    return SUCCESS;
}

int32_t sm_http_client_free(sm_http_client_t* _this) {
    if (!_this) {
        return FAILURE;
    }
    g_http_client.m_if = NULL;
    return SUCCESS;
}
int32_t sm_http_client_add_header_default(sm_http_client_t* _this) {
    if (!_this) return FAILURE;
#if RESPONSE_HEADER
    const char* default_type = "Content-Type";
    const char* default_content = "binary/octet-stream";

    return sm_http_client_add_header(_this, default_type, default_content);
#endif
    return SUCCESS;
}

int32_t sm_http_client_add_header(sm_http_client_t* _this, const char* _type, const char* _content) {
    if (!_this || !_content || !_type) {
        return FAILURE;
    }
#if RESPONSE_HEADER
        if (_impl(_this)->m_header_num < SM_HTTP_CLIENT_HEADER_NUM_MAX){
        char str_tmp[128] = {0};
        sprintf(str_tmp, "%s: %s",_type, _content);
        if (sm_ec200_http_add_header(_impl(_this)->m_if,str_tmp) != SUCCESS) {
            return FAILURE;
        }
        _impl(_this)->m_header[_impl(_this)->m_header_num].m_type = _type;
        _impl(_this)->m_header[_impl(_this)->m_header_num].m_content = _content;
        _impl(_this)->m_header_num++;

        return SUCCESS;
    } else {
        return -2;  //over
    }
#endif
    return SUCCESS;
}
int32_t sm_http_client_add_headers(sm_http_client_t* _this, const sm_http_header_t* _headers, uint32_t _len) {
    if (!_this) {
        return FAILURE;
    }
#if RESPONSE_HEADER
    if (_impl(_this)->m_header_num + _len <= SM_HTTP_CLIENT_HEADER_NUM_MAX){
        for (int i = 0; i < _len; i++)
        {
            if (_headers[i].m_type == NULL || _headers[i].m_content == NULL){
                return FAILURE;
            }
            sm_http_client_add_header(_this, _headers[i].m_type, _headers[i].m_content);
        }
    }else{
        return -2;  // over
    }
#endif

    return SUCCESS;
}

int32_t sm_http_client_set_range_header(sm_http_client_t* _this, uint32_t _start, uint32_t _length) {
    if (!_this) {
        return FAILURE;
    }
    _impl(_this)->m_get_start = _start;
    _impl(_this)->m_get_len = _length;
    return SUCCESS;
}

int32_t sm_http_client_set_url(sm_http_client_t* _this, const char* _url){
    return 0;
}

int32_t sm_http_client_get_method(sm_http_client_t* _this,
                                  const char* _url,
                                  sm_http_client_response_fn_t _cb,
                                  void* _arg) {
    int32_t err = 0;

#if RESPONSE_HEADER
    uint8_t buff[2048];
#else
    uint8_t buff[SM_HTTP_CLIENT_BODY_SIZE];
#endif
    int32_t status_code;
    memset(buff, 0, sizeof(buff));

#if RESPONSE_HEADER
    for(int i=0; i < SM_HTTP_CLIENT_HEADER_NUM_MAX; i++) {
            res_header_buffer[i].m_type = NULL;
            res_header_buffer[i].m_content = NULL;
        }
#endif

    if (!_this || !_url){
        return -1;
    }
    // SEND GET request
    err = sm_ec200_set_http_url(_impl(_this)->m_if, _url);
    if (err){
        return -1;
    }
    if (_impl(_this)->m_get_len == 0) {
        return -1;
    }
    err = sm_ec200_http_get_range(_impl(_this)->m_if,
                                _impl(_this)->m_get_start,
                                _impl(_this)->m_get_len,
                                &status_code);
    if (err){
        return -1;
    }
    LOG_ERR(TAG, "HTTP Status code %d", status_code);
    if (status_code < 0 || status_code > HTTP_STATUS_CODE_PARTITION_CONTENT) {
        return status_code;
    }
    // READ GET response
    err = sm_ec200_http_read(_impl(_this)->m_if, buff);
    if (err){
        return -1;
    }

    http_unpack(_this, buff);

#if RESPONSE_HEADER
    http_parse_header(res_header_buffer, _impl(_this)->m_header_str);
#endif


    if (_cb) {
#if RESPONSE_HEADER
        _cb(res_header_buffer,
            (uint8_t*)_impl(_this)->m_body,
            strlen(_impl(_this)->m_body),
            _arg);
#else
        _cb(NULL,
            (uint8_t*)_impl(_this)->m_body,
            strlen(_impl(_this)->m_body),
            _arg);
#endif
    }
    return status_code;
}

int32_t sm_http_client_post_method(sm_http_client_t* _this,
                                   const char* _url,
                                   const char* _request_payload,
                                   sm_http_client_response_fn_t _cb,
                                   void* _arg) {
    int32_t err = 0;
    uint8_t buff[2048];
    int32_t status_code;
    memset(buff, 0, sizeof(buff));
    if (!_this || !_url){
        return -1;
    }
    err = sm_ec200_set_http_url(_impl(_this)->m_if, _url);
    if (err){
        return -1;
    }
    err = sm_ec200_http_write(_impl(_this)->m_if,
                                    _request_payload,
                                    strlen(_request_payload),
                                    &status_code);
    if (err){
        return -1;
    }

    if (status_code < 0 || status_code > HTTP_STATUS_CODE_PARTITION_CONTENT) {
        return status_code;
    }

    err = sm_ec200_http_read(_impl(_this)->m_if, buff);
    if (err){
        return -1;
    }
    memcpy(_impl(_this)->m_body, buff, sizeof(_impl(_this)->m_body));
    if (_cb) {
        _cb(_impl(_this)->m_header,
            (uint8_t*)_impl(_this)->m_body,
            strlen(_impl(_this)->m_body),
            _arg);
    }

    return status_code;
}

void http_unpack(sm_http_client_t *_this, char *data) {
#if RESPONSE_HEADER
    memset(_impl(_this)->m_body, 0, sizeof(_impl(_this)->m_body));
    memset (_impl(_this)->m_header, 0, sizeof(_impl(_this)->m_header));
    char* buff = data;
    char *p_header_l = NULL;
    char *p_header_r = NULL;
    char *p_body_l = NULL;
    char *p_body_r  = NULL;
    p_header_l = strstr(buff, "HTTP/1.1");
    if (p_header_l) {
        p_header_r = strstr(p_header_l, "\r\n\r\n");
        if (p_header_r) {
            memcpy(_impl(_this)->m_header_str, p_header_l, p_header_r - p_header_l);
        }
    }

    p_body_l = strstr(p_header_r, "\r\n\r\n");
    p_body_l += 4;
    if (p_body_l) {
        p_body_r = strstr(p_body_l, "\r\nOK\r\n");
        if (p_body_r) {
            memcpy(_impl(_this)->m_body, p_body_l, p_body_r - p_body_l);
        }
    }
#else
    memset(_impl(_this)->m_body, 0, sizeof(_impl(_this)->m_body));
    char* buff = data;
    char *p_body_l = NULL;
    char *p_body_r  = NULL;
    p_body_l = strstr(buff, "CONNECT\r\n");
    p_body_l += 9;
    if (p_body_l) {
        p_body_r = strstr(p_body_l, "\r\nOK\r\n");
        if (p_body_r) {
            memcpy(_impl(_this)->m_body, p_body_l, p_body_r - p_body_l);
        }
    }
#endif

}

void http_parse_header(sm_http_header_t* _header, const char* _buff) {
    uint8_t num = 0;
    char type_name[64];
    char content_value[256];
    char* line = strtok(_buff, "\n");
    while (line != NULL) {
        char* separator = strchr(line, ':');
        if (separator != NULL) {
            int name_len = separator - line;
            strncpy(type_name, line, name_len);
            type_name[name_len] = '\0';

            const char* value_start = separator + 1;
            while (*value_start == ' ') value_start++;  // Skip spaces
            strncpy(content_value, value_start, 256 - 1);
            content_value[256 - 1] = '\0';
            if (strstr(type_name, "Content-Range") ||
                strstr(type_name, "Date"))
                LOG_INF(TAG, "Field: %s, Value: %s\n", type_name, content_value);
            _header[num].m_type = type_name;
            _header[num].m_content = content_value;
            num++;
            if (num > SM_HTTP_CLIENT_HEADER_NUM_MAX) break;
        }
        line = strtok(NULL, "\n");
    }
}
