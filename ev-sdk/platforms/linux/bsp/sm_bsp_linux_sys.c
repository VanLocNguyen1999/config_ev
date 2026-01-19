//
// Created by vnbk on 27/02/2024.
//
#include <stdio.h>
#include <time.h>
#include <stdint.h>

int64_t get_tick_count() {
    struct timespec ts;
    int32_t tick = 0U;
    clock_gettime(CLOCK_REALTIME, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

void sm_bsp_linux_logger_put(const char* log){
    printf("%s\n", log);
}