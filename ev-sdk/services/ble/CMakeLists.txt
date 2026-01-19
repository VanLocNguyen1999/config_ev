cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 99)
project(ble_sv)

add_library(ble_sv
        sm_sv_ble.c
        sm_sv_ble.h
        sm_sv_ble_define.h
        ../../libs/json-maker/json-maker.c
        ../../libs/tiny-json/tiny-json.c
)

target_link_libraries(ble_sv sdk_utils sdk_common linux_utils sm_hals )

add_subdirectory(ble_testing)