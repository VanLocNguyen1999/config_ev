cmake_minimum_required(VERSION 3.16)
project(selex_ev_api)

set(CMAKE_C_STANDARD 99)

add_library(sm_pmu_api STATIC
        sm_pmu_api.h
        sm_pmu_api.c
)

target_link_libraries(sm_pmu_api
        sdk_utils
        sm_bp_management
        sm_sv_charger
        sm_sv_ev_io
        sm_sv_pms
        sm_stm
        pmu_app
)