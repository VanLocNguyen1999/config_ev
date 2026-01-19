cmake_minimum_required(VERSION 3.16)
project(linux_storage)

set(CMAKE_C_STANDARD 99)

set(LINUX_STORAGE_SRC
#        sm_linux_file_storage.c
        sm_linux_file.c
)

add_library(linux_storage
        ${LINUX_STORAGE_SRC}
)
target_link_libraries(linux_storage sdk_utils linux_hal)