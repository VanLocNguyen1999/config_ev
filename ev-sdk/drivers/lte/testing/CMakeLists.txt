cmake_minimum_required(VERSION 3.22)
project(lte_testing)
set(CMAKE_C_STANDARD 99)

add_executable(lte_testing
#    testing.c
        http_testing.c
)

target_link_libraries(lte_testing lte sdk_utils  sm_hals linux_utils)