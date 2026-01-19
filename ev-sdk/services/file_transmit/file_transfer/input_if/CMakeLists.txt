cmake_minimum_required(VERSION 3.22)
project(file_transfer_input_if)

set(CMAKE_C_STANDARD 99)

add_library(file_transfer_input_if STATIC
        sm_sv_file_transfer_input_file.c
        sm_sv_file_transfer_input_if.h
)

target_link_libraries(file_transfer_input_if sdk_utils)