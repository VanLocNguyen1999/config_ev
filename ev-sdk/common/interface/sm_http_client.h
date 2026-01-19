//
// Created by vnbk on 13/09/2024.
//

#ifndef EV_SDK_SM_HTTP_CLIENT_H
#define EV_SDK_SM_HTTP_CLIENT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"

typedef void sm_http_client_t;

#define HTTP_HEADER_ACCEPT                  "Accept"
#define HTTP_HEADER_ACCEPT_ENCODING         "Accept-Encoding"
#define HTTP_HEADER_ACCEPT_LANGUAGE         "Accept-Language"
#define HTTP_HEADER_ORIGIN                  "Origin"
#define HTTP_HEADER_CONTENT_LENGTH          "Content-Length"
#define HTTP_HEADER_CONTENT_TYPE            "Content-Type"
#define HTTP_HEADER_CONTENT_ENCODING        "Content-Encoding"
#define HTTP_HEADER_CONTENT_LANGUAGE        "Content-Language"
#define HTTP_HEADER_ACCEPT_RANGE            "Accept-Ranges"
#define HTTP_HEADER_RANGE                   "Range"
#define HTTP_HEADER_HOST                    "Host"
#define HTTP_HEADER_ALLOW                   "Allow"
#define HTTP_HEADER_SERVER                  "Server"
#define HTTP_HEADER_TRANSFER_ENCODING       "Transfer-Encoding"

typedef enum{
    HTTP_STATUS_CODE_OK                     = 200,
    HTTP_STATUS_CODE_PARTITION_CONTENT      = 206,
    HTTP_STATUS_CODE_BAD_REQUEST            = 400,
    HTTP_STATUS_CODE_UNAUTHORIZED           = 401,
    HTTP_STATUS_CODE_FORBIDDEN              = 403,
    HTTP_STATUS_CODE_NOT_FOUND              = 404,
    HTTP_STATUS_CODE_URL_TOO_LONG           = 414,
    HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR  = 500,
    HTTP_STATUS_CODE_BAD_GATEWAY            = 502,
    HTTP_STATUS_CODE_SERVICE_UNAVAILABLE    = 503,
    HTTP_STATUS_CODE_GATEWAY_TIMEOUT        = 504
}HTTP_STATUS_CODE;

typedef struct{
    const char* m_type;
    const char* m_content;
}sm_http_header_t;

typedef void (*sm_http_client_response_fn_t)(const sm_http_header_t*, const uint8_t*, uint32_t, void*);

sm_http_client_t* sm_http_client_create(void* _net);

int32_t sm_http_client_init(sm_http_client_t* _this);

int32_t sm_http_client_free(sm_http_client_t* _this);

int32_t sm_http_client_add_header_default(sm_http_client_t* _this);

int32_t sm_http_client_add_header(sm_http_client_t* _this, const char* _type, const char* _content);

int32_t sm_http_client_add_headers(sm_http_client_t* _this, const sm_http_header_t* _headers, uint32_t _len);

int32_t sm_http_client_set_range_header(sm_http_client_t* _this, uint32_t _start, uint32_t _length);

 int32_t sm_http_client_set_url(sm_http_client_t* _this, const char* _url);

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
                                  void* _arg);

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
                                   void* _arg);



#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_HTTP_CLIENT_H
