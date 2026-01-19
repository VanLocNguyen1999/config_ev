cmake_minimum_required(VERSION 3.16)
project(ev-core)

set(CMAKE_C_STANDARD 99)

add_library(sm_core_sch STATIC
        sch/sm_core_sch.c
        sch/sm_core_sch.h
)

target_link_libraries(sm_core_sch sdk_utils)

add_library(sm_core_storage STATIC
        storage/sm_core_fs.h
        storage/sm_core_fs.c
        storage/sm_core_param.h
        storage/sm_core_param.c)

target_link_libraries(sm_core_storage sdk_utils)
