cmake_minimum_required(VERSION 3.22)
project(iot_testing)
set(CMAKE_C_STANDARD 99)

add_executable(iot_testing
        testing.c
)

target_link_libraries(iot_testing
        lte
        sdk_utils
        sm_hals
        sdk_common
        linux_utils
        sm_iot_service
        linux_platform)