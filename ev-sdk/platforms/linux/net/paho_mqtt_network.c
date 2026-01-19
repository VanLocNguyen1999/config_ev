//
// Created by vnbk on 01/04/2023.
//
#include <stdio.h>
#include "paho_mqtt_network.h"
#include "tcp_client.h"

#define TAG "paho_mqtt_network"

#define _imp(p) ((mqtt_network_if_impl_t*)(p))

typedef struct {
    mqtt_network_if_t   prototype;

    tcp_client_t* m_client;
    /*string_t* m_host;
    uint16_t m_port;*/
} mqtt_network_if_impl_t;

static void mqtt_network_free(mqtt_network_if_t* _self);
static int mqtt_network_init(mqtt_network_if_t* _self);
static int mqtt_network_connect(mqtt_network_if_t*,const char*, unsigned short);
static void mqtt_network_disconnect(mqtt_network_if_t* _self);
static int mqtt_network_mqttread(mqtt_network_if_t*, unsigned char*, int, int);
static int mqtt_network_mqttwrite(mqtt_network_if_t*, unsigned char*, int, int);
static void mqtt_network_set_port(mqtt_network_if_t*, unsigned short);
static unsigned short mqtt_network_get_port(mqtt_network_if_t* _self);

static mqtt_network_if_impl_t g_mqtt_network_default = {
    .m_client = NULL,
};

mqtt_network_if_t* mqtt_network_create(){
    g_mqtt_network_default.prototype.init = mqtt_network_init;
    g_mqtt_network_default.prototype.free = mqtt_network_free;
    g_mqtt_network_default.prototype.connect = mqtt_network_connect;
    g_mqtt_network_default.prototype.disconnect = mqtt_network_disconnect;
    g_mqtt_network_default.prototype.mqttread = mqtt_network_mqttread;
    g_mqtt_network_default.prototype.mqttwrite = mqtt_network_mqttwrite;
    g_mqtt_network_default.prototype.set_port = mqtt_network_set_port;
    g_mqtt_network_default.prototype.get_port = mqtt_network_get_port;

    g_mqtt_network_default.m_client = tcp_client_create();

    return &g_mqtt_network_default.prototype;
}


static void mqtt_network_free(mqtt_network_if_t* _self){
    if(_self && _imp(_self)->m_client){
        tcp_client_destroy(_imp(_self)->m_client);
        _imp(_self)->m_client = NULL;
    }
}
static int mqtt_network_init(mqtt_network_if_t* _self){
    if(_self){
        if(tcp_client_is_connected(_imp(_self)->m_client)){
            tcp_client_disconnect(_imp(_self)->m_client);
        }
        return 0;
    }
    return -1;
}
static int mqtt_network_connect(mqtt_network_if_t* _self, const char* _host, unsigned short _port){
    return _self ? tcp_client_connect(_imp(_self)->m_client, _host, _port) : -1;
}
static void mqtt_network_disconnect(mqtt_network_if_t* _self){
    tcp_client_disconnect(_imp(_self)->m_client);
}
static int mqtt_network_mqttread(mqtt_network_if_t* _self, unsigned char* _buf, int _max_len, int _timeout){
    return tcp_client_recv(_imp(_self)->m_client, _buf, _max_len, _timeout);
}
static int mqtt_network_mqttwrite(mqtt_network_if_t* _self, unsigned char* _data, int _len, int _timeout){
    return tcp_client_send(_imp(_self)->m_client, _data, _len, _timeout);
}
static void mqtt_network_set_port(mqtt_network_if_t* _self, unsigned short _port){

}
static unsigned short mqtt_network_get_port(mqtt_network_if_t* _self){

}
