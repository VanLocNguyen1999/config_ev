cmake_minimum_required(VERSION 3.22)
project(sdk_common)
set(CMAKE_C_STANDARD 99)

add_library(sdk_common
        interface/sm_mqtt_client.h
        net/mqtt/sm_mqtt_client_ec200.c
        net/http/sm_http_client_ec200.c
        interface/sm_http_client.h
        ble_slave/sm_ble_slave_mdbt42q.c
        modules/at-modem/sm_modem.c
        modules/at-modem/sm_modem.h
)


add_subdirectory(net/mqtt/testing)
add_subdirectory(net/http/testing)
add_subdirectory(net/mqtt/paho_mqtt)
add_subdirectory(storage)

target_link_libraries(sdk_common lte ble_driver sm_hals)
