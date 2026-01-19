cmake_minimum_required(VERSION 3.22)
project(sv_file_transfer)

set(CMAKE_C_STANDARD 99)

add_library(sv_file_transfer STATIC
        sm_sv_file_transfer.c
        sm_sv_file_transfer.h
)

add_subdirectory(input_if)

target_link_libraries(sv_file_transfer file_transfer_input_if sm_core_co sdk_utils linux_co_if)


add_executable(file_trans_exam file_trans_exam.c)
target_link_libraries(file_trans_exam sv_file_transfer)