cmake_minimum_required(VERSION 3.22)
project(sv_upgrading)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

add_library(sm_sv_upgrading STATIC
        sm_sv_upgrading.h
        sm_sv_upgrading.c)

target_link_libraries(sm_sv_upgrading sm_boot_core sm_boot_impl sdk_utils)


