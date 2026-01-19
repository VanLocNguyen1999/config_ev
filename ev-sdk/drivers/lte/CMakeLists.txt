cmake_minimum_required(VERSION 3.22)
project(lte)
set(CMAKE_C_STANDARD 99)

set (LTE_SRC
        ../../common/modules/at-modem/sm_modem.c
        ../../common/modules/at-modem/sm_modem.h
        sm_ec200.c
        sm_ec200.h
        sm_ec200_mqtt.h
        sm_ec200_mqtt.c
        sm_ec200_http.h
        sm_ec200_http.c
)

add_subdirectory(testing)

add_library(lte ${LTE_SRC})

target_link_libraries(lte sdk_utils )
