cmake_minimum_required(VERSION 3.22)
project(sm_hals)
set(CMAKE_C_STANDARD 99)

set(SM_HALS_INC
        sm_hal_adc.h
        sm_hal_can.h
        sm_hal_delay.h
        sm_hal_flash.h
        sm_hal_i2c.h
        sm_hal_io.h
        sm_hal_iwdg.h
        sm_hal_pwm.h
        sm_hal_spi.h
        sm_hal_system.h
        sm_hal_timer.h
        sm_hal_uart.h

        ../platforms/linux/hal/sm_hal_linux_can.c
        ../platforms/linux/hal/sm_hal_linux_delay.c
        ../platforms/linux/hal/sm_hal_linux_i2c.c
        ../platforms/linux/hal/sm_hal_linux_io.c
        ../platforms/linux/hal/sm_hal_linux_spi.c
        ../platforms/linux/hal/sm_hal_linux_uart.c
        ../platforms/linux/hal/sm_hal_linux_adc.c
)

add_library(sm_hals STATIC ${SM_HALS_INC})
