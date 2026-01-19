cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 99)
project(gps)

add_library(gps
        sm_l76x_gps.h
        sm_l76x_gps.c)

target_link_libraries(gps sdk_utils sm_hals)