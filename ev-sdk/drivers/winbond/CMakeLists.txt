cmake_minimum_required(VERSION 3.22)
project(w25q)
set(CMAKE_C_STANDARD 99)

add_library(w25q STATIC
        sm_w25qxx.c
        sm_w25qxx.h)