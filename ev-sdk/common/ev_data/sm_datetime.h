//
// Created by vnbk on 19/04/2023.
//

#ifndef DATETIME_H
#define DATETIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef union datetime{
    uint8_t arr_datetime[6];
    struct {
        uint8_t year;
        uint8_t mon;
        uint8_t day;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
    };
}sys_datetime_t;

typedef union time{
    uint8_t arr_time[3];
    struct {
        uint8_t hour;
        uint8_t min;
        uint8_t sec;
    };
}sm_sys_time_t;

int32_t get_timestamp();

const char* get_timestamp_string();

const char* get_current_time();

#ifdef __cplusplus
}
#endif

#endif //DATETIME_H
