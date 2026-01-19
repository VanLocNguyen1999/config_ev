cmake_minimum_required(VERSION 3.16)
project(sm-stm)

set(CMAKE_C_STANDARD 99)

add_library(sm_stm STATIC
        sm_stm.c
        sm_stm.h
)

target_link_libraries(sm_stm sdk_utils)