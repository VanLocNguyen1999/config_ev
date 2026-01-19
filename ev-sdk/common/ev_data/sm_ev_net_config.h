//
// Created by vnbk on 27/09/2024.
//

#ifndef EV_SDK_SM_EV_NET_CONFIG_H
#define EV_SDK_SM_EV_NET_CONFIG_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"

#define SM_NET_CONFIG_HOST_IOT_LENGTH   32
#define SM_NET_CONFIG_USER_IOT_LENGTH   16
#define SM_NET_CONFIG_PASS_IOT_LENGTH   16

#define SM_NET_CONFIG_LTE_APN_LENGTH    32
#define SM_NET_CONFIG_LTE_USER_LENGTH   8
#define SM_NET_CONFIG_LTE_PASS_LENGTH   8

#define SM_IOT_MQTT_HOST_DEFAULT        "hub.selex.vn"
#define SM_IOT_MQTT_PORT_DEFAULT        (1883)
#define SM_IOT_MQTT_USERNAME_DEFAULT    "selex"
#define SM_IOT_MQTT_PASSWORD_DEFAULT    "selex@123"

#define SM_LTE_APN_DEFAULT              "m3-world"
#define SM_LTE_USERNAME_DEFAULT         "mms"
#define SM_LTE_PASSWORD_DEFAULT         "mms"

#define SM_NET_CONFIG_SIZE_OF                        (sizeof(sm_ev_net_config_t))

typedef struct {
    uint8_t     m_host[SM_NET_CONFIG_HOST_IOT_LENGTH];
    uint16_t    m_port;
    uint8_t     m_user[SM_NET_CONFIG_USER_IOT_LENGTH];
    uint8_t     m_pass[SM_NET_CONFIG_PASS_IOT_LENGTH];

    uint8_t m_lte_apn[SM_NET_CONFIG_LTE_APN_LENGTH];
    uint8_t m_lte_user[SM_NET_CONFIG_LTE_USER_LENGTH];
    uint8_t m_lte_pass[SM_NET_CONFIG_LTE_PASS_LENGTH];
}sm_ev_net_config_t;

static inline void sm_ev_net_config_clone(const sm_ev_net_config_t* _src, sm_ev_net_config_t* _dest){
    memcpy(_dest->m_host, _src->m_host, SM_NET_CONFIG_HOST_IOT_LENGTH);
    _dest->m_port =  _src->m_port;
    memcpy(_dest->m_user, _src->m_user, SM_NET_CONFIG_USER_IOT_LENGTH);
    memcpy(_dest->m_pass, _src->m_pass, SM_NET_CONFIG_PASS_IOT_LENGTH);
    memcpy(_dest->m_lte_apn, _src->m_lte_apn, SM_NET_CONFIG_LTE_APN_LENGTH);
    memcpy(_dest->m_lte_user, _src->m_lte_user, SM_NET_CONFIG_LTE_USER_LENGTH);
    memcpy(_dest->m_lte_pass, _src->m_lte_pass, SM_NET_CONFIG_LTE_PASS_LENGTH);
}

static inline void sm_ev_net_config_default(sm_ev_net_config_t* _default){
    memset(_default->m_host, '\0', SM_NET_CONFIG_HOST_IOT_LENGTH);
    memcpy(_default->m_host, SM_IOT_MQTT_HOST_DEFAULT, strlen(SM_IOT_MQTT_HOST_DEFAULT));

    _default->m_port = SM_IOT_MQTT_PORT_DEFAULT;

    memset(_default->m_user, '\0', SM_NET_CONFIG_USER_IOT_LENGTH);
    memcpy(_default->m_user, SM_IOT_MQTT_USERNAME_DEFAULT, strlen(SM_IOT_MQTT_USERNAME_DEFAULT));

    memset(_default->m_pass, '\0', SM_NET_CONFIG_PASS_IOT_LENGTH);
    memcpy(_default->m_pass, SM_IOT_MQTT_PASSWORD_DEFAULT, strlen(SM_IOT_MQTT_PASSWORD_DEFAULT));

    memset(_default->m_lte_apn, '\0', SM_NET_CONFIG_LTE_APN_LENGTH);
    memcpy(_default->m_lte_apn, SM_LTE_APN_DEFAULT, strlen(SM_LTE_APN_DEFAULT));

    memset(_default->m_lte_user, '\0', SM_NET_CONFIG_LTE_USER_LENGTH);
    memcpy(_default->m_lte_user, SM_LTE_USERNAME_DEFAULT, strlen(SM_LTE_USERNAME_DEFAULT));

    memset(_default->m_lte_pass, '\0', SM_NET_CONFIG_LTE_PASS_LENGTH);
    memcpy(_default->m_lte_pass, SM_LTE_PASSWORD_DEFAULT, strlen(SM_LTE_PASSWORD_DEFAULT));
}

#ifdef __cplusplus
};
#endif


#endif //EV_SDK_SM_EV_NET_CONFIG_H
