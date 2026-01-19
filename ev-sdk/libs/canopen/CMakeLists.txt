cmake_minimum_required(VERSION 3.22)
project(libcanopen)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(co STATIC
        CO.c
        CO_CAN_Msg.c
        CO_CRC.c
        CO_EMCY.c
        CO_FLASH.c
        CO_NMT.c
        CO_OD.c
        CO_OD_sign.c
        CO_OD_storage.c
        CO_Object.c
        CO_PDO.c
        CO_RPDO.c
        CO_SDO.c
        CO_SDOclient.c
        CO_SDOserver.c
        CO_SYNC.c
        CO_TPDO.c

        CO.h
        CO_CAN_Msg.h
        CO_CRC.h
        CO_EMCY.h
        CO_FLASH.h
        CO_NMT.h
        CO_OD.h
        CO_OD_sign.h
        CO_OD_storage.h
        CO_Object.h
        CO_PDO.h
        CO_RPDO.h
        CO_SDO.h
        CO_SDOclient.h
        CO_SDOserver.h
        CO_SYNC.h
        CO_TPDO.h
        CO_config.h
        CO_slave.h
        CO_utils.h)