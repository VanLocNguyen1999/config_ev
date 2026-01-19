#ifndef SM_EC200_MQTT_H
#define SM_EC200_MQTT_H

#include "sm_ec200.h"

#define MQTT_URC_QMTSTAT	"+QMTSTAT"
#define MQTT_URC_QMTRECV	"+QMTRECV"
#define TCP_URC_QIDEACT		"+QIDEACT"

typedef enum {
 URC_NULL,
 URC_QMTSTAT,
 URC_QMTRECV,
 URC_QIDEACT,
}URC_MSG;

/**
 *
 * @param modem
 * @return
 */
int32_t sm_ec200_config_mqtt(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @param host
 * @param port
 * @return
 */
int32_t sm_ec200_open_mqtt(sm_ec200_t *modem, const char *host, uint32_t port);
/**
 * @brief
 * @param modem
 * @param device_name
 * @param user
 * @param pass
 * @return
 */
int32_t sm_ec200_connect_mqtt(sm_ec200_t *modem, const char *client_id, const char *user, const char *pass);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_disconnect_mqtt(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_close_mqtt(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @param topic
 * @param qos
 * @return
 */
int32_t sm_ec200_subscribes_mqtt(sm_ec200_t *modem, const char* topic, int qos);
/**
 * @brief
 * @param modem
 * @param topic
 * @return
 */
int32_t sm_ec200_unsubscribes_mqtt(sm_ec200_t *modem, const char* topic);
/**
 * @brief
 * @param modem
 * @param topic
 * @param data
 * @return
 */
int32_t sm_ec200_data_incoming_mqtt(sm_ec200_t *modem, char *topic, char* data);
/**
 * @brief
 * @param modem
 * @param topic
 * @param data
 * @param len
 * @param qos
 * @param retain
 * @return
 */
int32_t sm_ec200_publish_mqtt(sm_ec200_t *modem, const char *topic, const char *data, uint32_t len, int qos, int retain);

/**
 * Reads URC (Unsolicited Result Code) MQTT messages from the modem.
 *
 * @param modem Pointer to the sm_ec200_t modem instance.
 * @return URC_MSG indicating the type of URC message received.
 */
URC_MSG sm_ec200_read_urc_mqtt(sm_ec200_t *modem);

#endif //SM_EC200_MQTT_H
