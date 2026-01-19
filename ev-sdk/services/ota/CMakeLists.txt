cmake_minimum_required(VERSION 3.22)
project(ota-service)

set(CMAKE_C_STANDARD 99)

add_subdirectory(testing)

add_library(sm_ota_service STATIC
        sm_sv_ota.c
        sm_sv_ota.h
        sm_ota_download.h
        sm_ota_http_download.c
        sm_ota_ble_download.c
)

target_link_libraries(sm_ota_service sdk_utils ev_upgrade ex_flash)
