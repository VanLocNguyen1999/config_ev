cmake_minimum_required(VERSION 3.16)
project(test_ev_upgrading.c)

set(CMAKE_C_STANDARD 99)

add_executable(test_ev_upgrade test_ev_upgrading.c)
target_link_libraries(test_ev_upgrade ev_upgrade sdk_utils -lpthread sm_core_co linux_co_if ex_flash)