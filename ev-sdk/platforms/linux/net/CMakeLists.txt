cmake_minimum_required(VERSION 3.16)
project(linux_net)

set(CMAKE_C_STANDARD 99)

set(LINUX_NET_SRC
        http_client_curl.c
)

add_library(linux_net
        ${LINUX_NET_SRC}
)
target_link_libraries(linux_net sdk_utils linux_hal -lcurl)