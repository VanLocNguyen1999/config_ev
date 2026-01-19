cmake_minimum_required(VERSION 3.22)
project(sv_file_receiver)

set(CMAKE_C_STANDARD 99)

add_library(sv_file_receiver STATIC
        sm_sv_file_receiver.c
        sm_sv_file_receiver.h
)

add_subdirectory(output_if)

target_link_libraries(sv_file_receiver file_receiver_output_if sm_core_co sdk_utils linux_co_if)


add_executable(file_recv_exam file_recv_exam.c)
target_link_libraries(file_recv_exam sv_file_receiver ex_flash sdk_utils sm_ev_storage)