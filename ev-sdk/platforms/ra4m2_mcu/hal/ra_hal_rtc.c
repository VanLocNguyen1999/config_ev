/*
 * ra_hal_rtc.c
 *
 *  Created on: Oct 8, 2024
 *      Author: admin
 */

#include "sm_hal_rtc.h"
#include "hal_data.h"

typedef struct {
    rtc_instance_t* m_rtc;
    alarm_callback_fn m_fn;
    void *m_arg;
}ra_hal_rtc_t;
ra_hal_rtc_t g_rtc;

#define impl(x) ((ra_hal_rtc_t*)(x))

sm_hal_rtc_t* sm_hal_rtc_init(void* handle){
    g_rtc.m_rtc = handle;
    return (sm_hal_rtc_t*)&g_rtc;
}

int32_t sm_hal_rtc_open(sm_hal_rtc_t* _this){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);
    R_RTC_Open(this->m_rtc->p_ctrl, this->m_rtc->p_cfg);
    return 0;
}

int32_t sm_hal_rtc_close(sm_hal_rtc_t* _this){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);
    R_RTC_Close(this->m_rtc->p_ctrl);
    return 0;
}

int32_t sm_hal_rtc_set_time(sm_hal_rtc_t* _this, sm_time_t* _time){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);
    rtc_time_t* p_time = (rtc_time_t*) _time;
    p_time->tm_year += 100;
    R_RTC_CalendarTimeSet(this->m_rtc->p_ctrl, p_time);
    return 0;
}

int32_t sm_hal_rtc_get_time(sm_hal_rtc_t* _this, sm_time_t* _time){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);
    rtc_time_t* p_time = (rtc_time_t*) _time;
    R_RTC_CalendarTimeGet(this->m_rtc->p_ctrl, p_time);
    _time->tm_year -= 100;
    return 0;
}

int32_t sm_hal_rtc_set_alarm(sm_hal_rtc_t* _this, SM_ALARM_MASK _alarm_mask,  sm_time_t* _time, alarm_callback_fn _cb, void* _arg){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);

    this->m_fn = _cb;
    this->m_arg = _arg;

    rtc_alarm_time_t p_alarm = {
        .time.tm_sec =  _time->tm_sec, .time.tm_min = _time->tm_min, .time.tm_hour = _time->tm_hour,
        .time.tm_mday =  _time->tm_mday, .time.tm_mon = _time->tm_mon, .time.tm_year = _time->tm_year,
        .sec_match =    (_alarm_mask >= SM_ALARM_SEC_MATCH) ? true : false,
        .min_match =    (_alarm_mask >= SM_ALARM_MIN_MATCH) ? true : false,
        .hour_match =   (_alarm_mask >= SM_ALARM_HOUR_MATCH) ? true : false,
        .mday_match =    (_alarm_mask >= SM_ALARM_DAY_MATCH) ? true : false,
        .mon_match =    (_alarm_mask >= SM_ALARM_MON_MATCH) ? true : false,
        .year_match =   (_alarm_mask >= SM_ALARM_YEAR_MATCH) ? true : false,
    };

    R_RTC_CalendarAlarmSet(this->m_rtc->p_ctrl, &p_alarm);
    return 0;
}

int32_t sm_hal_rtc_get_alarm(sm_hal_rtc_t* _this, uint8_t* _mask, sm_time_t* _time){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);

    rtc_alarm_time_t p_alarm;
    R_RTC_CalendarAlarmGet(this->m_rtc->p_ctrl, &p_alarm);

    memcpy(_time, &p_alarm.time, sizeof(p_alarm.time));

    if (p_alarm.sec_match) *_mask = SM_ALARM_SEC_MATCH;
    if (p_alarm.min_match) *_mask = SM_ALARM_MIN_MATCH;
    if (p_alarm.hour_match) *_mask = SM_ALARM_HOUR_MATCH;
    if (p_alarm.mday_match) *_mask = SM_ALARM_DAY_MATCH;
    if (p_alarm.mon_match) *_mask = SM_ALARM_MON_MATCH;
    if (p_alarm.year_match) *_mask = SM_ALARM_YEAR_MATCH;

    return 0;
}

int32_t sm_hal_rtc_irq_cb(sm_hal_rtc_t* _this){
    if (!_this){
        return -1;
    }
    ra_hal_rtc_t* this = impl(_this);
    if (this->m_fn){
        this->m_fn(this->m_arg);
    }
    return 0;
}
