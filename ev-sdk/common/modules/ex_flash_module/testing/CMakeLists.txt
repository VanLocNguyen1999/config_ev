cmake_minimum_required(VERSION 3.22)
project(ex_flash_test)
set(CMAKE_C_STANDARD 99)

add_executable(ex_flash_test
        test_ex_flash.c
)

target_link_libraries(ex_flash_test ex_flash sdk_utils sm_ev_storage)