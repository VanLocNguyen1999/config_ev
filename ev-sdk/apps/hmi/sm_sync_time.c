/*
 * sm_sync_time.c
 *
 *  Created on: Oct 8, 2024
 *      Author: admin
 */
#include "sm_sync_time.h"

#include "sm_bsp_hmi.h"
#include "sm_logger.h"
#include "stdio.h"

#define TAG "SYNC_TIME"

enum{
    SYNC_TIME_INIT = 0,
    SYNC_TIME_FROM_GPS,
    SYNC_TIME_FROM_CLOUD
};

static sm_sync_time_t g_sync_time = {
                                     .m_module_gps = NULL,
                                     .m_module_rtc = NULL,
};

sm_sync_time_t* sm_sync_time_create(sm_gps_t* _gps, sm_hal_rtc_t* _rtc, int8_t _timezone){
    if(!_gps || !_rtc){
        return NULL;
    }
    sm_sync_time_t* sync_time = &g_sync_time;

    sync_time->m_module_gps = _gps;
    sync_time->m_module_rtc = _rtc;
    sync_time->m_sync = SYNC_TIME_INIT;
    sync_time->m_timezone = _timezone;


    return sync_time;
}

int32_t sm_sync_time_set_time(sm_sync_time_t *_this, const sys_datetime_t * _time, bool _from_ntp){
    if (!_this){
        return -1;
    }
    sm_time_t p_time = {
         .tm_sec = _time->sec, .tm_min = _time->min, .tm_hour = _time->hour,
         .tm_mday = _time->day, .tm_mon = _time->mon, .tm_year = _time->year
    };
    if(_from_ntp){
        _this->m_sync = SYNC_TIME_FROM_CLOUD;
    }else{
        _this->m_sync = SYNC_TIME_FROM_GPS;
    }
    sm_hal_rtc_set_time(_this->m_module_rtc, &p_time);
    return 0;
}

int32_t sm_sync_time_get_time(sm_sync_time_t *_this, sys_datetime_t * _time){
    if (!_this){
        return -1;
    }

    sys_datetime_t p_time;
    int32_t ret = -1;
    if(_this->m_sync == SYNC_TIME_INIT){
        ret = _this->m_module_gps->proc->get_time(_this->m_module_gps, &p_time, _this->m_timezone);
        if (ret <= 0){
            _time->hour = 0;
            _time->min = 0;
           return -1;
        }else if(ret == 1){
            _time->min = p_time.min;
            _time->hour = p_time.hour;
             return 0;
        } else{
            sm_sync_time_set_time(_this, &p_time, false);
            _this->m_sync = SYNC_TIME_FROM_GPS;
        }
    }

    sm_time_t rtc_time;
    sm_hal_rtc_get_time(_this->m_module_rtc, &rtc_time);

    _time->sec = rtc_time.tm_sec ;
    _time->min = rtc_time.tm_min ;
    _time->hour = rtc_time.tm_hour ;
    _time->day = rtc_time.tm_mday ;
    _time->mon = rtc_time.tm_mon ;
    _time->year = rtc_time.tm_year;

    return 0;
}
