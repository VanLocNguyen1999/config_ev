//
// Created by Ng Huy Hai on 9/5/2024.
//
#include "sm_mqtt_client.h"
#include "sm_ec200_mqtt.h"
#include "sm_fifo.h"
#include "sm_logger.h"

#include "sm_hal_delay.h"

#define SM_MQTT_CLIENT_EVENT_CB_MAX     8
#define SM_MQTT_RETRY_CONFIG_MAX        5

#define SM_MQTT_TOPIC_SIZE              128
#define SM_MQTT_PAYLOAD_SIZE            2048

static const char* TAG = "MQTT_CLIENT_EC200";

#define _impl(x)    ((sm_mqtt_client_impl_t*)(x))

enum {
    FAILURE = -1,
    SUCCESS = 0,
};

typedef struct {
    sm_ec200_t*         m_if;

    char m_topic_buffer[SM_MQTT_TOPIC_SIZE];
    char m_payload_buffer[SM_MQTT_PAYLOAD_SIZE];

    uint8_t             m_event_cb_num;
    struct event_cb {
        void        (*on_message)(sm_mqtt_client_t*, sm_mqtt_msg_t*, void*);
        void*       m_arg;
    }m_event_cbs[SM_MQTT_CLIENT_EVENT_CB_MAX];
}sm_mqtt_client_impl_t;

static sm_mqtt_client_impl_t g_mqtt_client = {
    .m_if = NULL,
    .m_event_cb_num = 0,
};

sm_mqtt_client_t* sm_mqtt_client_create(void* _net_if){
    if (!_net_if) {
        return NULL;
    }

    g_mqtt_client.m_if = _net_if;
    memset(g_mqtt_client.m_topic_buffer, '\0',SM_MQTT_TOPIC_SIZE);
    memset(g_mqtt_client.m_payload_buffer, '\0', SM_MQTT_PAYLOAD_SIZE);

    for(int index = 0; index < SM_MQTT_CLIENT_EVENT_CB_MAX; index++){
        g_mqtt_client.m_event_cbs[index].on_message = NULL;
        g_mqtt_client.m_event_cbs[index].m_arg = NULL;
    }
    g_mqtt_client.m_event_cb_num = 0;

    return &g_mqtt_client;
}

int32_t sm_mqtt_init(sm_mqtt_client_t* _this) {
    if (!_this) {
        return -1;
    }

    uint8_t retry = 0;
    while (sm_ec200_config_mqtt(_impl(_this)->m_if)) {
        retry++;
        LOG_DBG(TAG, "ec200 retry config mqtt");
        if (retry > SM_MQTT_RETRY_CONFIG_MAX) {
            return -1;
        }
        sm_hal_delay_ms(500);
    }

    return 0;
}

int32_t sm_mqtt_set_incoming_data_cb(sm_mqtt_client_t* _client,  void (*on_message)(sm_mqtt_client_t*, sm_mqtt_msg_t*, void*), void* _arg) {
    if (!_client || !on_message) {
        return -1;
    }
    if (_impl(_client)->m_event_cb_num >= SM_MQTT_CLIENT_EVENT_CB_MAX) {
        return -1;
    }

    _impl(_client)->m_event_cbs[_impl(_client)->m_event_cb_num].on_message = on_message;
    _impl(_client)->m_event_cbs[_impl(_client)->m_event_cb_num].m_arg = _arg;
    _impl(_client)->m_event_cb_num++;

    return 0;
}

int32_t sm_mqtt_connect(sm_mqtt_client_t *_client,
                        const char* _host,
                        int32_t _port,
                        char *_client_id,
                        const char *_user,
                        const char *_pass,
                        void (*on_connected)(int32_t, void*),
                        void* _arg) {
    if (!_client || !_host || !_client_id) {
        return -1;
    }
    sm_ec200_close_mqtt(_impl(_client)->m_if);
    int32_t ret = sm_ec200_open_mqtt(_impl(_client)->m_if, _host, _port);

    if (ret != 0) {
        if(on_connected){
            on_connected(FAILURE, _arg);
        }
        return -1;
    }

    ret = sm_ec200_connect_mqtt(_impl(_client)->m_if, _client_id, _user, _pass);

    if(on_connected){
        if(ret == 0){
            on_connected(SUCCESS, _arg);
        }else{
            on_connected(FAILURE, _arg);
        }
    }

    return (ret == 0) ? ret : -1;
}

int32_t sm_mqtt_disconnect(sm_mqtt_client_t *_client,
                           void (*on_disconnected)(int32_t, void*),
                           void* _arg) {
    if (!_client) {
        return -1;
    }
    if (sm_ec200_disconnect_mqtt(_impl(_client)->m_if) != 0) {
        if(on_disconnected){
            on_disconnected(FAILURE,_arg);
        }
        return -1;
    }
    if(on_disconnected){
        on_disconnected(SUCCESS,_arg);
    }
    return 0;
}

int32_t sm_mqtt_publish(sm_mqtt_client_t *_client,
                        const char *_topic,
                        const char *_data,
                        uint8_t _qos,
                        uint8_t _retain,
                        void (*on_published)(int32_t, void*),
                        void* _arg) {
    if (!_client) {
        return -1;
    }
    if (sm_ec200_publish_mqtt(_impl(_client)->m_if, _topic, _data, (uint32_t)strlen(_data), _qos, _retain) != 0) {
        if(on_published){
            on_published(FAILURE, _arg);
        }
        return -1;
    }
    if(on_published){
        on_published(SUCCESS, _arg);
    }

    return 0;
}

int32_t sm_mqtt_subscribe(sm_mqtt_client_t *_client,
                          const char *_topic,
                          uint8_t _qos,
                          void (*on_subscribed)(int32_t, void*),
                          void* _arg) {
    if (!_client) {
        return -1;
    }
    if (sm_ec200_subscribes_mqtt(_impl(_client)->m_if, _topic, _qos)) {
        if(on_subscribed){
            on_subscribed(FAILURE, _arg);
        }
        return -1;
    }
    if(on_subscribed){
        on_subscribed(SUCCESS, _arg);
    }
    return 0;
}

int32_t sm_mqtt_unsubscribes(sm_mqtt_client_t *_client,
                             const char* _topic,
                             void (*on_unsubscribed)(int32_t, void*),
                             void* _arg) {
    if (!_client) {
        return -1;
    }
    if (sm_ec200_unsubscribes_mqtt(_impl(_client)->m_if, _topic) != 0){
        if(on_unsubscribed){
            on_unsubscribed(FAILURE, _arg);
        }
        return -1;
    }
    if(on_unsubscribed){
        on_unsubscribed(SUCCESS, _arg);
    }
    return 0;
}

int32_t sm_mqtt_client_process(sm_mqtt_client_t *_client) {
    if (!_client) {
        return -1;
    }
    if (sm_ec200_read_urc_mqtt(_impl(_client)->m_if) == URC_QMTSTAT)
        return -1;
    if (sm_ec200_data_incoming_mqtt(_impl(_client)->m_if, _impl(_client)->m_topic_buffer,  _impl(_client)->m_payload_buffer) != 0) {
        return 0;
    }

    sm_mqtt_msg_t msg_tmp;
    msg_tmp.m_topic = _impl(_client)->m_topic_buffer;
    msg_tmp.m_topic_length = (int32_t)strlen(_impl(_client)->m_topic_buffer);
    msg_tmp.m_payload =  _impl(_client)->m_payload_buffer;
    msg_tmp.m_payload_len = (int32_t)strlen( _impl(_client)->m_payload_buffer);

    for (int i=0; i< _impl(_client)->m_event_cb_num; i++) {
        if (_impl(_client)->m_event_cbs[i].on_message)
            _impl(_client)->m_event_cbs[i].on_message(_client, &msg_tmp, _impl(_client)->m_event_cbs[i].m_arg);
    }
    return 0;
}
