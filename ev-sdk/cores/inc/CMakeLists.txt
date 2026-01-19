cmake_minimum_required(VERSION 3.22)
project(sm-core-canopen)

set(CMAKE_C_STANDARD 99)

add_library(sm_core_co STATIC
        sm_core_co.c
        sm_core_co.h
        sm_co_od_common.h
)

target_link_libraries(sm_core_co co sm_co_od)