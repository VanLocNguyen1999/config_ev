cmake_minimum_required(VERSION 3.22)
project(http_client_testing)
set(CMAKE_C_STANDARD 99)

add_executable(http_client_testing
        http_client_testing.c
)

target_link_libraries(http_client_testing lte sdk_utils  sm_hals linux_utils sdk_common)