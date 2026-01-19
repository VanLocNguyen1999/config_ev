cmake_minimum_required(VERSION 3.22)
project(ex-flash)
set(CMAKE_C_STANDARD 99)

add_library(ex_flash
        sm_ex_flash_storage.c
        sm_ex_flash_storage.h

        interface/sm_ex_flash_storage_if.h
        interface/sm_ex_flash_storage_virtual_flash_interface.c
        interface/sm_ex_flash_storage_w25q_interface.c
)

add_subdirectory(testing)

