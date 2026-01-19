cmake_minimum_required(VERSION 3.22)
project(ev-upgrade)

set(CMAKE_C_STANDARD 99)

add_subdirectory(sv_upgrading)
add_subdirectory(testing)

add_library(ev_upgrade STATIC
        sm_sv_ev_upgrade.c
        sm_sv_ev_upgrade.h)

target_link_libraries(ev_upgrade sm_sv_upgrading)
