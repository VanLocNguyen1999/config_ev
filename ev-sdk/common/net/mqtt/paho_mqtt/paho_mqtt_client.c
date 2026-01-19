//
// Created by vnbk on 19/09/2024.
//
#include <string.h>
#include "sm_mqtt_client.h"
#include "sm_logger.h"

#include "MQTTClient.h"
#include "net/paho_mqtt_network.h"
#include "paho_mqtt_timer.h"

#define TAG "SM_PAHO_MQTT_CLIENT"

#define PAHO_MQTT_RX_BUFFER_LEN     4096
#define PAHO_MQTT_TX_BUFFER_LEN     4096

#define PAHO_MQTT_CLIENT_ID_DEFAULT             "selex_12345"
#define PAHO_MQTT_CLIENT_ID_PREFIX              "selex_"
#define PAHO_MQTT_CLIENT_USERNAME_DEFAULT       "selex"
#define PAHO_MQTT_CLIENT_PASSWORD_DEFAULT       "selex_123"

#define PAHO_MQTT_CLIENT_TIMEOUT_DEFAULT        800
#define PAHO_MQTT_CLIENT_CMD_TIMEOUT_DEFAULT    800

#define PAHO_MQTT_CLIENT_KEEPALIVE_DEFAULT      10

#define MQTT_CLIENT_WILL_TOPIC      "/topic/online"
#define MQTT_CLIENT_WILL_PAYLOAD    "{offline}"

#define _impl(p) ((paho_mqtt_client_impl_t*)(p))

typedef struct paho_mqtt_client{
    mqtt_network_if_t* m_network;

    MQTTClient  m_client;
    int32_t     m_cmd_timeout;
    int32_t     m_timeout;
    uint8_t     m_txBuf[PAHO_MQTT_TX_BUFFER_LEN];
    int32_t 	m_txLen;
    uint8_t     m_rxBuf[PAHO_MQTT_RX_BUFFER_LEN];
    int32_t	    m_rxLen;

//    const char* m_client_id;

    void (*on_message)(sm_mqtt_client_t *, sm_mqtt_msg_t *, void *);
    void* m_arg;
}paho_mqtt_client_impl_t;

static paho_mqtt_client_impl_t g_mqtt_client_default = {
        .m_network = NULL,
        .m_cmd_timeout = PAHO_MQTT_CLIENT_CMD_TIMEOUT_DEFAULT,
        .m_timeout = PAHO_MQTT_CLIENT_TIMEOUT_DEFAULT,
        .m_client = DefaultClient,
};


static void messageArrived(MessageData* md) {
    LOG_INF(TAG, "Paho MQTT have received message");
    paho_mqtt_client_impl_t * mqtt_obj = (paho_mqtt_client_impl_t*)md->userData;
    if (mqtt_obj && _impl(mqtt_obj)->on_message) {
        sm_mqtt_msg_t message;
        if (md && md->topicName) {
            if (md->topicName->cstring) {
                message.m_payload =  md->message->payload;
                message.m_payload_len = md->message->payloadlen;
                message.m_qos =  md->message->qos;
                message.m_topic = md->topicName->cstring;
            } else if (md->topicName->lenstring.data) {
                message.m_payload = md->message->payload;
                message.m_payload_len = md->message->payloadlen;
                message.m_qos = md->message->qos;
                message.m_topic = md->topicName->lenstring.data;
                message.m_topic_length = md->topicName->lenstring.len;
            }

            _impl(mqtt_obj)->on_message(mqtt_obj, &message, _impl(mqtt_obj)->m_arg);
        }
    }
}

sm_mqtt_client_t* sm_mqtt_client_create(void* _net_if){
    if(!_net_if){
        return NULL;
    }
    g_mqtt_client_default.m_network = _net_if;

    g_mqtt_client_default.m_txLen = PAHO_MQTT_TX_BUFFER_LEN;
    g_mqtt_client_default.m_rxLen = PAHO_MQTT_RX_BUFFER_LEN;

    return &g_mqtt_client_default;
}

int32_t sm_mqtt_init(sm_mqtt_client_t* _this){
    if(!_this){
        return -1;
    }

    MQTTClientInit(& _impl(_this)->m_client,
                   _impl(_this)->m_network,
                   _impl(_this)->m_cmd_timeout,
                   _impl(_this)->m_txBuf,
                   _impl(_this)->m_txLen,
                   _impl(_this)->m_rxBuf,
                   _impl(_this)->m_rxLen);

    return 0;
}

/**
 *
 * @param _this
 * @param on_message
 * @param _arg
 * @return
 */
int32_t sm_mqtt_set_incoming_data_cb(sm_mqtt_client_t *_this,
                                     void (*on_message)(sm_mqtt_client_t *, sm_mqtt_msg_t *, void *),
                                     void *_arg){
    if(!_this){
        return -1;
    }

    _impl(_this)->on_message = on_message;
    _impl(_this)->m_arg = _arg;
    return 0;
}

/**
 * @brief
 * @param client
 * @param name
 * @param user
 * @param pass
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_connect(sm_mqtt_client_t *_client,
                        const char *_host,
                        int32_t _port,
                        char *_client_id,
                        const char *_user,
                        const char *_pass,
                        void (*on_connected)(int, void *),
                        void *_arg){
    if(!_client){
        return -1;
    }
    int ret = _impl(_client)->m_network->connect(_impl(_client)->m_network, _host, _port);
    if (ret >= 0) {
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.cleansession       = true;
        data.MQTTVersion        = 4;
        data.clientID.cstring   = _client_id;
        data.keepAliveInterval  = PAHO_MQTT_CLIENT_KEEPALIVE_DEFAULT;
        data.username.cstring   = (_user) ? _user : PAHO_MQTT_CLIENT_USERNAME_DEFAULT;
        data.password.cstring   = (_user) ? _pass : PAHO_MQTT_CLIENT_PASSWORD_DEFAULT;
        data.willFlag           = true;
        if (data.willFlag) {
            data.will.message.cstring   = MQTT_CLIENT_WILL_PAYLOAD;
            data.will.topicName.cstring = MQTT_CLIENT_WILL_TOPIC;
            data.will.retained          = false;
            data.will.qos               = QOS0;
        }
        ret = MQTTConnect(&_impl(_client)->m_client, &data);
        if(ret){
            LOG_ERR(TAG, "MQTT connect FAILED. %d", ret);
        }else{
            LOG_INF(TAG, "MQTT connect SUCCESS.");
        }
    }
    if (on_connected) {
        on_connected(ret, _arg);
    }
}

/**
 * @brief
 * @param client
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_disconnect(sm_mqtt_client_t *_client,
                           void (*on_disconnected)(int, void *),
                           void *_arg){
    if(!_client){
        return -1;
    }
    int ret = -1;
    ret = MQTTDisconnect(&_impl(_client)->m_client);
    _impl(_client)->m_network->disconnect(_impl(_client)->m_network);

    if(on_disconnected){
        on_disconnected(ret, _arg);
    }

    return ret;
}

/**
 * @brief
 * @param client
 * @param topic
 * @param data
 * @param len
 * @param qos
 * @param retain
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_publish(sm_mqtt_client_t *_client,
                        const char *_topic,
                        const char *_data,
                        uint8_t _qos,
                        uint8_t _retain,
                        void (*on_published)(int32_t, void *),
                        void *_arg){
    if(!_client){
        return -1;
    }
    int ret = -1;
    MQTTMessage msg;
    msg.qos = (enum QoS)_qos;
    msg.retained = _retain;
    msg.dup = 1;
    msg.id = 0;
    msg.payloadlen = (_data) ? strlen(_data) : 0;
    msg.payload = (void*)_data;

    ret = MQTTPublish(&_impl(_client)->m_client, _topic, &msg);

    LOG_DBG(TAG, "\r\n> publish topic \"%s\" with ret = %d", _topic, ret);
    if (on_published) {
        on_published( ret, _arg);
    }
    return ret;
}

/**
 * @brief
 * @param client
 * @param topic
 * @param qos
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_subscribe(sm_mqtt_client_t *_client,
                          const char *_topic,
                          uint8_t _qos,
                          void (*on_subscribed)(int32_t, void *),
                          void *_arg){
    if(!_client){
        return -1;
    }
    int ret = -1;
    ret = MQTTSubscribe(&_impl(_client)->m_client, _topic, (enum QoS) _qos, messageArrived, _client);
    LOG_DBG(TAG, "Subscribe %s ret = %d", _topic, ret);
    if (on_subscribed) {
        on_subscribed(ret, _arg);
    }
    return ret;
}

/**
 * @brief
 * @param client
 * @param topic
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_unsubscribes(sm_mqtt_client_t *_client,
                             const char *_topic,
                             void (*on_unsubscribed)(int32_t, void *),
                             void *_arg){
    if(!_client){
        return -1;
    }
    int ret = MQTTUnsubscribe(&_impl(_client)->m_client, _topic);
    LOG_DBG(TAG, "Unsubscribe %s ret = %d", _topic, ret);
    if (on_unsubscribed) {
        on_unsubscribed(ret, _arg);
    }
    return ret;
}

/**
 * @brief
 * @param client
 */
int32_t sm_mqtt_client_process(sm_mqtt_client_t *_client){
    if(!_client){
        return -1;
    }

    return MQTTYield(&_impl(_client)->m_client, _impl(_client)->m_timeout);
}