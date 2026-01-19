cmake_minimum_required(VERSION 3.22)
project(ota-service-testing)

set(CMAKE_C_STANDARD 99)

add_executable(testing_ota
        testing_ota.c
)

target_link_libraries(testing_ota sdk_utils sm_ota_service linux_net linux_storage linux_co_if)