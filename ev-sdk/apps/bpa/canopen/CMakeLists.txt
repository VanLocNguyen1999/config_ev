cmake_minimum_required(VERSION 3.22)
project(sm-canopen-od)

add_library(sm_co_od
        od.h
        od.c
        manu_od/manu_od.h
        manu_od/manu_od.c
        manu_od/manu_od_default_value.c
        manu_od/manu_od_default_value.h
        comm_od/comm_od.c
        comm_od/comm_od.h
        comm_od/comm_od_default_value.c
        comm_od/comm_od_default_value.h
)

target_link_libraries(sm_co_od sm_core_co)