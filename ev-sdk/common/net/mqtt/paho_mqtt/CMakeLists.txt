cmake_minimum_required(VERSION 3.22)
project(paho-mqtt-client)
set(CMAKE_C_STANDARD 99)

set(PAHO_MQTT_SRC
        ../../../../libs/PahoMqtt/MQTTClient.h
        ../../../../libs/PahoMqtt/MQTTClient.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTConnectClient.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTConnectServer.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTDeserializePublish.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTFormat.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTPacket.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTSerializePublish.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTUnsubscribeClient.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTUnsubscribeServer.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTSubscribeClient.c
        ../../../../libs/PahoMqtt/MQTTPacket/src/MQTTSubscribeServer.c
)

add_library(paho_mqtt
        paho_mqtt_timer.h
        paho_mqtt_timer.c
        paho_mqtt_client.c
        ../../../../platforms/linux/net/paho_mqtt_network.c
        ../../../../platforms/linux/net/paho_mqtt_network.h
        ${PAHO_MQTT_SRC}
)

target_link_libraries(paho_mqtt linux_utils)