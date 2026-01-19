//
// Created by admin on 9/16/2024.
//
#include <stdio.h>
#include "sm_ec200_http.h"
#include "sm_hal.h"
#include <string.h>
#define SEC(x)	                        (uint32_t)(1000*x)

#define HTTP_CONFIG_TIMEOUT_MS          SEC(5)
#define HTTP_OPEN_TIMEOUT_MS            SEC(5)
#define HTTP_CLOSE_TIMEOUT_MS           SEC(5)
#define HTTP_READ_TIMEOUT_MS            SEC(60)
#define HTTP_WRITE_TIMEOUT_MS           SEC(5)
#define HTTP_COMMON_TIMEOUT_MS          SEC(5)

int32_t sm_ec200_http_config(sm_ec200_t* modem, const char* cfg, int val) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char cmd[128];
    sprintf(cmd, "AT+QHTTPCFG=\"%s\",%d\r\n",cfg, val);
    return sm_modem_send_cmd(p_modem , cmd,
                        EC200_OK,
						EC200_ERR,
                        HTTP_CONFIG_TIMEOUT_MS);
}

int32_t sm_ec200_http_add_header(sm_ec200_t* modem, const char* header) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char cmd[128];
    sprintf(cmd, "AT+QHTTPCFG=\"header\",%s\r\n",header);
    return sm_modem_send_cmd(p_modem , cmd,
    					EC200_OK,
						EC200_ERR,
                        HTTP_COMMON_TIMEOUT_MS);
}

int32_t sm_ec200_http_open(sm_ec200_t *modem, char *host, int port) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char cmd[256];
    sprintf(cmd, "AT+QIOPEN=1,1,\"TCP\",\"%s\",%d,0,0\r\n", host, port);
    return sm_modem_send_cmd(p_modem, cmd,
                        "OK\r\n\r\n+QIOPEN: 1,0",
                        EC200_ERR,
                                HTTP_OPEN_TIMEOUT_MS);
}

int32_t sm_ec200_http_close(sm_ec200_t *modem) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    return sm_modem_send_cmd(p_modem, "AT+QICLOSE=1\r\n",
                        EC200_OK,
                        "ERROR\r\n",
                                HTTP_CLOSE_TIMEOUT_MS);
}

int32_t sm_ec200_http_write(sm_ec200_t *modem, char *data, uint32_t len, int32_t* status_code) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char cmd[128];
    sprintf(cmd, "AT+QHTTPPOST=%lu,80,80\r\n", len);
    int32_t err = sm_modem_send_cmd(p_modem, cmd,
    							EC200_CONNECT,
                                EC200_ERR,
                                HTTP_COMMON_TIMEOUT_MS);
    if (err) return err;

    err = sm_modem_send_cmd(p_modem, data,
                        "OK\r\n\r\n+QHTTPPOST",
                        EC200_ERR,
                            HTTP_WRITE_TIMEOUT_MS);
    char* p_status = strstr(p_modem->buff, "+QHTTPPOST: 0");
    int32_t status = -1;
    if (p_status) {
        sscanf(p_status, "+QHTTPPOST: 0,%d,", &status);
    }
    *status_code = status;
    return (err != 0) ? -1 : 0;
}

int32_t sm_ec200_http_read(sm_ec200_t *modem, char *data) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char buffer[2048];
    memset(buffer, 0, 2048);
    int32_t err = sm_modem_send_recv(p_modem, "AT+QHTTPREAD=80\r\n",
									"+QHTTPREAD: 0\r\n",
									EC200_ERR,
									buffer, 2048,
									HTTP_READ_TIMEOUT_MS);

    if (!err){
        strcpy(data, buffer);
    }
    return err;
}

int32_t sm_ec200_http_get_range(sm_ec200_t* modem, uint32_t start, uint32_t range, int32_t* status_code) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char cmd[64] = {0};
    memset(cmd, 0, 64);
    char buffer[64] = {0};
    memset(buffer, 0, 64);

    sprintf(cmd, "AT+QHTTPGETEX=80,%d,%d\r\n", start, range);
    int32_t err =  sm_modem_send_recv(p_modem, cmd,
                                "+QHTTPGET: 0,20",
                                EC200_ERR,
								buffer, 64,
                                HTTP_COMMON_TIMEOUT_MS);
    char* p_status = strstr(buffer, "+QHTTPGET: 0");
    int32_t status = -1;
    if (p_status) {
        sscanf(p_status, "+QHTTPGET: 0,%d,", &status);
    }
    *status_code = status;
    return (err != 0) ? -1 : 0;
}

int32_t sm_ec200_set_http_url(sm_ec200_t *modem, char *url) {
    if (!modem) return -1;
    sm_modem_t* p_modem = (sm_modem_t*) modem;
    char buff[64] = { 0 };
    memset(buff, 0, 64);
    sprintf(buff, "AT+QHTTPURL=%d,%d\r\n", (int)strlen(url), 60);
    int32_t err = sm_modem_send_cmd (p_modem, buff,
    								EC200_CONNECT,
                                    EC200_ERR,
                                    HTTP_COMMON_TIMEOUT_MS);
    if (err){
        return err;
    }

    return sm_modem_send_cmd(p_modem, url,
                        EC200_OK,
                        EC200_ERR,
                        HTTP_COMMON_TIMEOUT_MS);
}
