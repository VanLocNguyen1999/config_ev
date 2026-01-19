cmake_minimum_required(VERSION 3.22)
project(hmi-service)

set(CMAKE_C_STANDARD 99)

#add_subdirectory(testing)

add_library(sm_hmi_service STATIC
        sm_sv_hmi.h
        sm_sv_hmi.c
)

target_link_libraries(sm_hmi_service sdk_utils)
