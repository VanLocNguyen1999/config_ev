//
// Created by vnbk on 01/04/2023.
//

#ifndef PAHO_MQTT_NETWORK_H
#define PAHO_MQTT_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mqtt_network_if mqtt_network_if_t;

struct mqtt_network_if{
    /**
     * @brief   Dispose pointer
     * @param   mqtt_network_if_t*
     */
    void (*free)(mqtt_network_if_t*);
    /**
     * @brief   initialize object
     * @param   mqtt_network_if_t*
     * @return  int     (0)     success
                        (-1)    fail
     */
    int (*init)(mqtt_network_if_t*);
    /**
     * @brief       connect to host name & port
     * @param[IN]   mqtt_network_if_t*  object
     * @param[IN]   char*           host name
     * @param[IN]   unsigned short  port number
     * @return      int     (0)     success
                            (-1)    fail
     */
    int (*connect)(mqtt_network_if_t*, const char*, unsigned short);

    /**
     * @brief       close connection
     * @param[IN]   mqtt_network_if_t*  object
     */
    void (*disconnect)(mqtt_network_if_t*);
    /**
     * @brief       read data
     * @param[IN]   mqtt_network_if_t*  object
     * @param[IN]   unsigned char*  buffer
     * @param[IN]   int             length
     * @param[IN]   int             timeout
     * @return      int     (>=0)   length
                            (-1)    fail
     */
    int (*mqttread)(mqtt_network_if_t*, unsigned char*, int, int);
    /**
     * @brief       write data
     * @param[IN]   mqtt_network_if_t*  object
     * @param[IN]   unsigned char*  buffer
     * @param[IN]   int             length
     * @param[IN]   int             timeout
     * @return      int     (>=0)   length
                            (-1)    fail
     */
    int (*mqttwrite)(mqtt_network_if_t*, unsigned char*, int, int);

    /**
     * @brief       set port number
     * @param[IN]   mqtt_network_if_t*  object
     * @param[IN]   unsigned short  port number
     */
    void (*set_port)(mqtt_network_if_t*, unsigned short);

    /**
     * @brief       get port number
     * @param[IN]   mqtt_network_if_t*  object
     * @return      unsigned int    port number
     */
    unsigned short (*get_port)(mqtt_network_if_t*);
};

mqtt_network_if_t* mqtt_network_create();

typedef mqtt_network_if_t Network;

#ifdef __cplusplus
};
#endif

#endif //PAHO_MQTT_NETWORK_H
