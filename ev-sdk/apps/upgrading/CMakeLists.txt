cmake_minimum_required(VERSION 3.22)
project(upgrading_app)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 99)

add_executable(SFUT
        upgrading_app.cpp)

target_link_libraries(SFUT mb_master sm_sv_boot_master sdk_utils -lpthread)

add_executable(debug_app
        debug_app.cpp)

#target_link_libraries(debug_app mb_master sm_sv_boot_master sdk_utils -lpthread)
