cmake_minimum_required(VERSION 3.22)

project(ev-sdk)

set(CMAKE_C_STANDARD 99)

include_directories(
        utils
        api/

        platforms/
        platforms/linux/
        platforms/linux/storage/
        platforms/linux/net/
        platforms/linux/bsp/
        platforms/linux/utils

        apps/bpa
        apps/bpa/canopen
        apps/pmu
        apps/upgrading
        apps/canopen/
        apps/canopen/manu_od
        apps/canopen/comm_od
        apps/modules

        utils/
        utils/algorithms
        utils/containers
        utils/timing

        libs/
        libs/canopen
        libs/host
        libs/nanoModbus
        libs/PahoMqtt
        libs/PahoMqtt/MQTTPacket/src

        hals
        platforms
        services/canopen
        services/ev_io
        services/boot_master
        services/boot_master/boot-core
        services/boot_master/boot-core/fw
        services/boot_master/boot-impl
        services/ev_upgrade
        services/ev_upgrade/sv_upgrading
        services/boot_master/boot-core
        services/boot_master/boot-core/fw
        services/boot_master/boot-impl
        services/canopen
        services/button
        services/bp_management/
        services/pms
        services/charger/
        services/ev/
        services/ev/modules
        services/iot
        services/ota
        services/hmi
        services/ble
        services/file_transmit

        services/file_transmit/file_transfer
        services/file_transmit/file_transfer/input_if

        services/file_transmit/file_receiver
        services/file_transmit/file_receiver/output_if

        common/
        common/ev_data
        common/modules
        common/modules/at-modem
        common/gps
        common/interface
        common/modbus
        common/modbus/mb_slave
        common/modbus/mb_master
        common/net/mqtt/
        common/net/mqtt/paho_mqtt
        common/net/http
        common/modules/ex_flash_module
        common/modules/ex_flash_module/interface
        common/ble_slave
        common/storage/
        common/storage/ssl_file

        cores/
        cores/inc
        cores/storage
        cores/sch
        cores/stm

        hals/include
        bsp/

        drivers/
        drivers/lcd/
        drivers/ble/
        drivers/at25xe/
        drivers/gps/
        drivers/lte/
        drivers/winbond

        platforms/
        platforms/linux/
        platforms/linux/utils/
        platforms/linux/canopen/

        platforms/ra4m2_mcu/hmi/porting
        platforms/ra4m2_mcu/hal/

        platforms/ra4m2_mcu/bpa/
)

add_subdirectory(utils)
add_subdirectory(libs/canopen)
add_subdirectory(libs/host)
add_subdirectory(libs/nanoModbus)

add_subdirectory(services/ev_io)
add_subdirectory(services/boot_master)
add_subdirectory(services/bp_management)
add_subdirectory(services/ev)
add_subdirectory(services/button)
add_subdirectory(services/pms)
add_subdirectory(services/charger)
add_subdirectory(services/ev_upgrade)
add_subdirectory(services/iot)
add_subdirectory(services/ota)
add_subdirectory(services/hmi)
add_subdirectory(services/ble)

add_subdirectory(common/modbus/mb_slave)
add_subdirectory(common/modbus/mb_master)
add_subdirectory(common/modules/ex_flash_module)

add_subdirectory(services/file_transmit/file_transfer)
add_subdirectory(services/file_transmit/file_receiver)


add_subdirectory(cores/inc)
add_subdirectory(cores)
add_subdirectory(cores/stm)

add_subdirectory(hals)

add_subdirectory(api)
add_subdirectory(apps/upgrading)
add_subdirectory(apps/hmi)
add_subdirectory(apps/pmu)
add_subdirectory(apps/canopen)
add_subdirectory(apps/bpa)

add_subdirectory(platforms/linux)

add_subdirectory(drivers/lte)

add_subdirectory(drivers/winbond)

add_subdirectory(drivers/gps)
add_subdirectory(drivers/ble)

add_subdirectory(common)
