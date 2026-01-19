//
// Created by vnbk on 02/10/2024.
//

#ifndef EV_SDK_SM_SYNC_TIME_H
#define EV_SDK_SM_SYNC_TIME_H

#include "time.h"
#include "sm_types.h"
#include "sm_elapsed_timer.h"
#include "sm_ec200.h"
#include "sm_module_gps.h"
#include "sm_datetime.h"
#include "sm_hal.h"

#define SM_TIME_ZONE_DEFAULT            (+7)

typedef struct {
    int8_t      m_timezone;
    uint8_t      m_sync;
    sm_gps_t     *m_module_gps;
    sm_hal_rtc_t *m_module_rtc;
}sm_sync_time_t;

sm_sync_time_t* sm_sync_time_create(sm_gps_t* _gps, sm_hal_rtc_t* _rtc, int8_t _timezone);

int32_t sm_sync_time_set_time(sm_sync_time_t *_this, const sys_datetime_t * _time, bool _from_ntp);
int32_t sm_sync_time_get_time(sm_sync_time_t *_this, sys_datetime_t * _time);

#endif //EV_SDK_SM_SYNC_TIME_H
