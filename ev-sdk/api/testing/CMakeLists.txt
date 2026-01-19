cmake_minimum_required(VERSION 3.16)
project(test_pmu_api)

set(CMAKE_C_STANDARD 99)

add_executable(api_testing test_api.c)

target_link_libraries(api_testing
        sdk_utils
#        sm_bp_management
#        sm_sv_charger
        sm_sv_ev_io
#        sm_sv_pms
        pmu_app
        sm_pmu_api
)


add_executable(test_ev_upgrade ../../services/ev_upgrade/testing/test_ev_upgrading.c)
target_link_libraries(test_ev_upgrade ev_upgrade sdk_utils -lpthread sm_core_co)