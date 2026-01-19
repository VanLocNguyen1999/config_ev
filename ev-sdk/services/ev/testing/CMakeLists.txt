cmake_minimum_required(VERSION 3.22)
project(ev-service)

set(CMAKE_C_STANDARD 99)

add_executable(testing_ev_service
        ev_main.c
        ../../../platforms/linux/storage/sm_hmi_storage.c
        ../../../libs/json-maker/json-maker.c
        ../../../libs/tiny-json/tiny-json.c
)

target_link_libraries(testing_ev_service sm_ev_service linux_co_if linux_platform linux_storage) #m is math for round function

