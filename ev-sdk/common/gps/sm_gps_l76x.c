//
// Created by admin on 9/23/2024.
//
#include <time.h>
#include "sm_gps_l76x.h"
#include "sm_datetime.h"
#include "sm_l76x_gps.h"
#include "sm_logger.h"

typedef struct {
    sm_gps_t m_base;
    sm_l76x_t* g_gps_l76x;
}sm_gps_l76x_t;

static int32_t sm_gps_module_init(sm_gps_t* _this);
static int32_t sm_gps_module_free(sm_gps_t* _this);
static int32_t sm_gps_module_reboot(sm_gps_t* _this);
static int32_t sm_gps_module_pw_reset(sm_gps_t* _this);
static float sm_gps_module_get_lat(sm_gps_t*  _this);
static float sm_gps_module_get_lon(sm_gps_t* _this);
static int32_t sm_gps_module_get_time(sm_gps_t* _this, sm_gps_datetime_t* _time, int8_t _timezone);
static int32_t sm_gps_module_get_coordinate(sm_gps_t* _this, sm_gps_coordinate_t* _coordinate);
static bool sm_gps_is_valid(sm_gps_t* _this);
static int32_t sm_gps_module_process(sm_gps_t* _this);

sm_gps_module_proc_t g_gps_module_proc = {
    .init = sm_gps_module_init,
    .free = sm_gps_module_free,
    .reboot = sm_gps_module_reboot,
    .power_reset = sm_gps_module_pw_reset,
    .get_lat = sm_gps_module_get_lat,
    .get_lon = sm_gps_module_get_lon,
    .get_time = sm_gps_module_get_time,
    .get_coordinate = sm_gps_module_get_coordinate,
    .data_is_valid = sm_gps_is_valid,
    .process = sm_gps_module_process
};

static sm_gps_l76x_t g_gps_l76x = {
        .m_base.proc = &g_gps_module_proc,
        .g_gps_l76x = NULL
};

static int32_t sm_gps_module_init(sm_gps_t* _this) {
    if (!_this) {
        return -1;
    }
//    g_gps_l76x.g_gps_l76x = sm_l76x_init(g_gps_uart, g_gps_rst);
    return 0;
}
static int32_t sm_gps_module_free(sm_gps_t* _this){
    if (!_this) {
        return -1;
    }
    return 0;
}
static int32_t sm_gps_module_reboot(sm_gps_t* _this){
    if (!_this) {
        return -1;
    }
    sm_l76x_reboot(g_gps_l76x.g_gps_l76x);
    return 0;
}
static int32_t sm_gps_module_pw_reset(sm_gps_t* _this){
    if (!_this) {
        return -1;
    }
    sm_l76x_reboot(g_gps_l76x.g_gps_l76x);
    return 0;
}
static float sm_gps_module_get_lat(sm_gps_t*  _this){
    if (!_this) {
        return -1;
    }
    sm_l76x_get_lat(g_gps_l76x.g_gps_l76x);
    return 0;
}
static float sm_gps_module_get_lon(sm_gps_t* _this){
    if (!_this) {
        return -1;
    }
    sm_l76x_get_lon(g_gps_l76x.g_gps_l76x);
    return 0;
}
static int32_t sm_gps_module_get_time(sm_gps_t* _this, sm_gps_datetime_t* _time, int8_t _timezone){
    if (!_this) {
        return -1;
    }
    sys_datetime_t raw_time;
    int32_t ret = sm_l76x_get_time(g_gps_l76x.g_gps_l76x, &raw_time);
    if(ret <= 0){
        _time->hour = 0;
        _time->min = 0;
        _time->sec = 0;
        _time->year = 0;
        _time->mon = 1;
        _time->day = 1;
        return -1;
    }

    struct tm p_time = {
        .tm_sec = raw_time.sec,
        .tm_min = raw_time.min,
        .tm_hour = raw_time.hour,
        .tm_mday = raw_time.day,
        .tm_mon = raw_time.mon,
        .tm_year = raw_time.year,
    };

    time_t temp_time;
    temp_time = mktime(&p_time);
    temp_time += _timezone * 3600;

    struct tm *offset_time = gmtime(&temp_time);

    _time->hour = offset_time->tm_hour;
    _time->min = offset_time->tm_min;
    _time->sec = offset_time->tm_sec;
    _time->year = offset_time->tm_year;
    _time->mon = offset_time->tm_mon;
    _time->day = offset_time->tm_mday;

    return ret;
}
static int32_t sm_gps_module_get_coordinate(sm_gps_t* _this, sm_gps_coordinate_t* _coordinate){
    if (!_this) {
        return -1;
    }

    _coordinate->lat = sm_l76x_get_lat(g_gps_l76x.g_gps_l76x);
    _coordinate->lon = sm_l76x_get_lon(g_gps_l76x.g_gps_l76x);
    return 0;
}
static int32_t sm_gps_module_process(sm_gps_t* _this){
    if (!_this) {
        return -1;
    }
    return  sm_l76x_process(g_gps_l76x.g_gps_l76x);
}

static bool sm_gps_is_valid(sm_gps_t* _this){
    if(!_this){
        return 0;
    }
    return g_gps_l76x.g_gps_l76x->data_is_valid;
}

sm_gps_t *sm_gps_create(void* _driver){
    if(!_driver){
        return NULL;
    }
    g_gps_l76x.g_gps_l76x = _driver;

    return &g_gps_l76x.m_base;
}

