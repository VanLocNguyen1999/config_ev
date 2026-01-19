//
// Created by vnbk on 04/09/2024.
//

#ifndef EV_SDK_SM_SV_IOT_CONFIG_H
#define EV_SDK_SM_SV_IOT_CONFIG_H

#include "sm_types.h"

typedef struct{
    const char* m_host;
    int32_t m_port;

    const char* m_username;
    const char* m_password;
    const char* m_client_id;

    bool m_tls;
    const char* m_ca_cert;
    const char* m_client_cert;
    const char* m_client_key;
}sm_sv_iot_config_t;

#endif //EV_SDK_SM_SV_IOT_CONFIG_H
