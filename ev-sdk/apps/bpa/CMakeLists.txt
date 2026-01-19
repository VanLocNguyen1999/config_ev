cmake_minimum_required(VERSION 3.16)
project(selex_bpa)

set(CMAKE_C_STANDARD 99)

set(MQTT_IMPL_SRC
        ../../common/interface/sm_host_api.h
        ../../common/interface/sm_mqtt_client.h
        ../../common/net/mqtt/paho_mqtt/paho_mqtt_timer.c
        ../../common/net/mqtt/paho_mqtt/paho_mqtt_client.c
)

set(BPA_DRIVER
        ../../drivers/winbond/sm_w25qxx.c
        ../../drivers/winbond/sm_w25qxx.h
        ../../drivers/lte/sm_ec200.c
        ../../drivers/lte/sm_ec200_http.c
        ../../drivers/lte/sm_ec200_http.c
)

set(PORTING_SRC
        ../../platforms/ra4m2_mcu/hal/ra_hal_can.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_io.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_delay.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_flash.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_i2c.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_uart.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_spi.c

        ../../platforms/ra4m2_mcu/bpa/sm_bpa_flash_config.h
        ../../platforms/ra4m2_mcu/bpa/sm_co_ra4m2_if.c
)

add_executable(sm_bpa_app
        sm_bpa_app.c
        sm_bpa_app.h
        ../modules/sm_bpm_handle.c
        ../modules/sm_bpm_handle.h

        ${PORTING_SRC}
        ${MQTT_IMPL_SRC}
        ${BPA_DRIVER}
)

target_link_libraries(sm_bpa_app
        sm_core_sch
        linux_platform
        sm_bp_management
        sm_sv_charger
        sm_sv_pms
)
