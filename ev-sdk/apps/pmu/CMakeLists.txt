cmake_minimum_required(VERSION 3.16)
project(selex_pmu)

set(CMAKE_C_STANDARD 99)

add_executable(pmu_app
        main.c
        sm_pmu_app.c
        sm_pmu_app.h
        sm_pmu_stm.h
        sm_pmu_stm.c
)

target_link_libraries(pmu_app
        sm_core_sch
        linux_platform
        sm_bp_management
        sm_sv_charger
        sm_sv_ev_io
        sm_sv_pms
        sm_stm
)
