cmake_minimum_required(VERSION 3.16)
project(linux_platform)

set(CMAKE_C_STANDARD 99)

set(UNIX_UTILS_SRC
        utils/linux_serial.c
        utils/linux_serial.h
        utils/sm_datetime_unix.c
        utils/tcp_client.c
        utils/tcp_client.h
        utils/tcp_server.c
        utils/tcp_server.h
        utils/threading/thpool.h
        utils/threading/thread_utils.h
        utils/threading/thpool_posix.c
)

add_library(linux_utils STATIC
        ${UNIX_UTILS_SRC})
target_link_libraries(linux_utils sdk_utils pthread)

add_library(linux_platform
        bsp/sm_bsp_linux_sys.c
        bsp/sm_bsp_sys_linux.h)

target_link_libraries(linux_platform linux_utils sdk_utils)

add_library(sm_bsp_hmi_linux
        bsp/sm_bsp_hmi_linux.c
)
target_link_libraries(sm_bsp_hmi_linux sdk_utils linux_hal)

add_subdirectory(hal)
add_subdirectory(canopen)
add_subdirectory(net)
add_subdirectory(storage)