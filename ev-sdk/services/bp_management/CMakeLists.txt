cmake_minimum_required(VERSION 3.22)
project(bp)

set(CMAKE_C_STANDARD 99)

add_library(sm_bp_management STATIC
        sm_bp.h
        ../../common/ev_data/sm_bp_data.h
        sm_sv_bp.h
        sm_bp_auth.c
        sm_bp_auth.h
        sm_bp_co.c
        sm_bp_co.h
        sm_sv_bp.c
        sm_bp_co.h
)

target_link_libraries(sm_bp_management sdk_utils sm_core_co)