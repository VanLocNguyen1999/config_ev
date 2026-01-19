cmake_minimum_required(VERSION 3.22)
project(ev-service)

set(CMAKE_C_STANDARD 99)

add_library(sm_ev_service STATIC
        sm_ev_service.h
        sm_ev_service.c
        sm_ev_internal_service.h
        modules/sm_ev_module.c
        modules/sm_ev_module.h
        modules/sm_ev_pmu_module.c
        modules/sm_ev_pmu_module.h
        modules/sm_ev_mc_module.c
        modules/sm_ev_mc_module.h
)

target_link_libraries(sm_ev_service sdk_utils m sm_bp_management) #m is math for round function

add_subdirectory(testing)