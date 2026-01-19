cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 99)

project(ble_testing)

add_executable(ble_testing
    ble_testing.c
)

target_link_libraries(ble_testing
        ble_sv
        linux_platform
        linux_hal
        sm_hals)