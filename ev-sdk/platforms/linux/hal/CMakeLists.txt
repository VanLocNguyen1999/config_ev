cmake_minimum_required(VERSION 3.16)
project(linux_hal)

set(CMAKE_C_STANDARD 99)

set(LINUX_HAL_SRC
        sm_hal_linux_delay.c
        sm_hal_linux_uart.c
        sm_hal_linux_io.c
        sm_hal_linux_can.c
        sm_hal_linux_i2c.c
        sm_hal_linux_spi.c
)

add_library(linux_hal STATIC
        ${LINUX_HAL_SRC})

target_link_libraries(linux_hal sdk_utils)

