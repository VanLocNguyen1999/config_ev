cmake_minimum_required(VERSION 3.16)
project(ev-hmi-app)

set(CMAKE_C_STANDARD 99)

set(LTE_DRIVER_SRC
        ../../common/modules/at-modem/sm_modem.h
        ../../common/modules/at-modem/sm_modem.c
        ../../drivers/lte/sm_ec200.c
        ../../drivers/lte/sm_ec200.h
        ../../drivers/lte/sm_ec200_mqtt.h
        ../../drivers/lte/sm_ec200_mqtt.c
)

set(MQTT_IMPL_SRC
        ../../common/interface/sm_host_api.h
        ../../common/interface/sm_mqtt_client.h
        ../../common/net/mqtt/paho_mqtt/paho_mqtt_timer.c
        ../../common/net/mqtt/paho_mqtt/paho_mqtt_client.c
)

set(HMI_DRIVER
        ../../drivers/winbond/sm_w25qxx.c
        ../../drivers/winbond/sm_w25qxx.h
        ../../drivers/gps/sm_l76x_gps.c
        ../../drivers/gps/sm_l76x_gps.h
        ../../drivers/ble/sm_mdbt42q_ble.h
        ../../drivers/ble/sm_mdbt42q_ble.c
        ../../drivers/lcd/sm_drv_lcd.h
        ../../drivers/lcd/sm_drv_lcd.c
        ../../drivers/lcd/uc1676_lcd.h
        ../../drivers/lcd/uc1676_lcd.c
        ../../drivers/lcd/segment_driver.c
        ../../drivers/lcd/segment_driver.h
)

set(PORTING_SRC
        ../../platforms/ra4m2_mcu/hal/ra_hal_can.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_io.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_delay.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_flash.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_i2c.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_uart.c
        ../../platforms/ra4m2_mcu/hal/ra_hal_spi.c

        ../../platforms/ra4m2_mcu/hmi/porting/sm_hmi_flash_config.h
        ../../platforms/ra4m2_mcu/hmi/porting/sm_co_ra4m2_if.c
)

set(MODULES_SRC
        ../../common/gps/sm_gps_l76x.c
)

add_executable(app_hmi
#        main.c
        sm_app_hmi.h
        sm_app_hmi.c
        sm_ev_ota.c
        sm_ev_ota.h
        sm_ev_handle.c
        sm_ev_handle.h
        sm_ev_ble.c
        sm_ev_ble.h

        ${LTE_DRIVER_SRC}
        ${HMI_DRIVER}
        ${MODULES_SRC}
        ${MQTT_IMPL_SRC}
        ${PORTING_SRC}

        sm_hmi_io.h
        sm_net_monitor.c
        sm_ev_inactive_mode.c
        sm_sync_time.c
        sm_sync_time.h
        sm_auth_bp.c
        sm_auth_bp.h
)

target_link_libraries(app_hmi
        sdk_utils
        sm_core_co
        sm_bp_management
        sm_ev_service
        sm_hmi_service
        sm_iot_service
        sm_ota_service
        sm_core_sch
        sm_ev_storage
)

if(UNIX)
    target_link_libraries(app_hmi sm_bsp_hmi_linux paho_mqtt linux_co_if linux_net linux_storage linux_platform)
endif (UNIX)
