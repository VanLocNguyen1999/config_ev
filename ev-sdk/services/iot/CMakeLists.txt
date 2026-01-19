cmake_minimum_required(VERSION 3.22)
project(iot-service)

set(CMAKE_C_STANDARD 99)

add_subdirectory(testing)

add_library(sm_iot_service STATIC
        sm_sv_iot.c
        sm_sv_iot.h
        sm_sv_iot_topic_builder.h
        sm_sv_iot_topic_builder.c
        sm_sv_iot_config.h
        ../../libs/json-maker/json-maker.c
        ../../libs/tiny-json/tiny-json.c
)

target_link_libraries(sm_iot_service sdk_utils)
