#ifndef COMMON_MQTT_SM_MQTT_CLIENT_H_
#define COMMON_MQTT_SM_MQTT_CLIENT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"

typedef void sm_mqtt_client_t;

typedef struct {
    const char *m_topic;
    int32_t m_topic_length;
    void *m_payload;
    int32_t m_payload_len;
    uint8_t m_qos;
    uint8_t m_retain;
} sm_mqtt_msg_t;

sm_mqtt_client_t* sm_mqtt_client_create(void* _net_if);

/**
 * @brief
 * @param _net_if lte modem, wifi, ethernet, ..
 * @return
 */
int32_t sm_mqtt_init(sm_mqtt_client_t* _this);

/**
 *
 * @param _this
 * @param on_message
 * @param _arg
 * @return
 */
int32_t
sm_mqtt_set_incoming_data_cb(sm_mqtt_client_t *_this, void (*on_message)(sm_mqtt_client_t *, sm_mqtt_msg_t *, void *),
                             void *_arg);

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
                        void (*on_connected)(int32_t, void *),
                        void *_arg);

/**
 * @brief
 * @param client
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_disconnect(sm_mqtt_client_t *_client,
                           void (*on_disconnected)(int32_t, void *),
                           void *_arg);

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
                        void *_arg);

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
                          void *_arg);

/**
 * @brief
 * @param client
 * @param topic
 * @return  0: success | NULL : false
 */
int32_t sm_mqtt_unsubscribes(sm_mqtt_client_t *_client,
                             const char *_topic,
                             void (*on_subscribed)(int32_t, void *),
                             void *_arg);

/**
 * @brief
 * @param client
 */
int32_t sm_mqtt_client_process(sm_mqtt_client_t *client);

#ifdef __cplusplus
};
#endif

#endif
