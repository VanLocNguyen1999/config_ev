cmake_minimum_required(VERSION 3.22)
project(ble_driver)

add_library(ble_driver
    sm_mdbt42q_ble.c
    sm_mdbt42q_ble.h
)

target_link_libraries(ble_driver sdk_utils sm_hals sdk_common)