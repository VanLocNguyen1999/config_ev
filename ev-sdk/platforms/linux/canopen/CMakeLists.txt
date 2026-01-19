cmake_minimum_required(VERSION 3.16)
project(co-linux-interface)

set(CMAKE_C_STANDARD 99)

set(CO_SERIAL_INTERFACE ON)
set(CO_ETHERNET_INTERFACE OFF)

if (CO_SERIAL_INTERFACE)
    set(CO_INTERFACE_SRC
            sm_co_host_if.c
    )
endif ()

if (CO_ETHERNET_INTERFACE)
    set(CO_INTERFACE_SRC
            sm_co_eth_if.c
    )
endif ()

add_library(linux_co_if STATIC
        ${CO_INTERFACE_SRC})
target_link_libraries(linux_co_if sdk_utils linux_utils host)
