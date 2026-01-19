cmake_minimum_required(VERSION 3.22)
project(mqtt_client_testing)
set(CMAKE_C_STANDARD 99)

add_executable(mqtt_client_testing
        testing.c
)

target_link_libraries(mqtt_client_testing lte sdk_utils  sm_hals sdk_common linux_utils)