cmake_minimum_required(VERSION 3.22)
project(ev-storage)
set(CMAKE_C_STANDARD 99)

add_library(sm_ev_storage
        sm_ev_storage.h
        sm_ev_storage.c
        sm_ev_manu_storage.c
        sm_ev_manu_storage.h
        sm_ev_sec_storage.c
        sm_ev_sec_storage.h
        sm_ev_opt_storage.c
        sm_ev_opt_storage.h
        sm_ev_odo_storage.c
        ssl_file/sm_ssl_file_storage_ex_flash_impl.c
        ssl_file/sm_ssl_file_storage_impl.h
)

target_link_libraries(sm_ev_storage sdk_utils)