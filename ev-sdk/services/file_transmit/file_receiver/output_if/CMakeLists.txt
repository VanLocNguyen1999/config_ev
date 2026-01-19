cmake_minimum_required(VERSION 3.22)
project(file_receiver_output_if)

set(CMAKE_C_STANDARD 99)

add_library(file_receiver_output_if STATIC
        sm_sv_file_receiver_output_if.h
        sm_sv_file_receiver_output_file.c
        sm_sv_file_receiver_output_flash.c
)

target_link_libraries(file_receiver_output_if sdk_utils)