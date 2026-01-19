/*
 * sm_hal_rtc.h
 *
 *  Created on: Oct 8, 2024
 *      Author: admin
 */

#ifndef INCLUDE_SM_HAL_RTC_H_
#define INCLUDE_SM_HAL_RTC_H_

#include "time.h"
#include "sm_types.h"

typedef void sm_hal_rtc_t;
typedef struct tm sm_time_t;

typedef enum {
    SM_ALARM_NONE_MATCH     = 0,         ///< Disable alarm
    SM_ALARM_SEC_MATCH      = 1,         ///< Enable the alarm based on a match of the seconds field
    SM_ALARM_MIN_MATCH      = 2,         ///< Enable the alarm based on a match of the minutes field
    SM_ALARM_HOUR_MATCH     = 3,         ///< Enable the alarm based on a match of the hours field
    SM_ALARM_DAY_MATCH      = 4,         ///< Enable the alarm based on a match of the days field
    SM_ALARM_MON_MATCH      = 5,         ///< Enable the alarm based on a match of the months field
    SM_ALARM_YEAR_MATCH     = 6,         ///< Enable the alarm based on a match of the years field
    SM_ALARM_ALL_MATCH      = 0xff,
} SM_ALARM_MASK;

typedef void (*alarm_callback_fn)(void* _arg);

sm_hal_rtc_t* sm_hal_rtc_init(void* handle);
int32_t sm_hal_rtc_open(sm_hal_rtc_t*);
int32_t sm_hal_rtc_close(sm_hal_rtc_t*);
int32_t sm_hal_rtc_set_time(sm_hal_rtc_t*, sm_time_t*);
int32_t sm_hal_rtc_get_time(sm_hal_rtc_t*, sm_time_t*);
int32_t sm_hal_rtc_set_alarm(sm_hal_rtc_t*, SM_ALARM_MASK , sm_time_t*, alarm_callback_fn, void*);
int32_t sm_hal_rtc_get_alarm(sm_hal_rtc_t*, uint8_t* _mask, sm_time_t* _time);
int32_t sm_hal_rtc_irq_cb(sm_hal_rtc_t* _this);
#endif /* INCLUDE_SM_HAL_RTC_H_ */
