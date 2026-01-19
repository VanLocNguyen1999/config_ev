//
// Created by vnbk on 01/04/2023.
//
#include <stdio.h>
#include "paho_mqtt_timer.h"

void TimerInit(Timer* timer)
{
    elapsed_timer_resetz(&timer->m_timer, 0);
}

char TimerIsExpired(Timer* timer)
{
    return !elapsed_timer_get_remain(&timer->m_timer) ? 1 : 0;
}

void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
    elapsed_timer_resetz(&timer->m_timer, (int32_t)timeout);
}

void TimerCountdown(Timer* timer, unsigned int timeout)
{
    elapsed_timer_resetz(&timer->m_timer, (int32_t)timeout*1000);
}

int TimerLeftMS(Timer* timer)
{
    return elapsed_timer_get_remain(&timer->m_timer);
}



/*
void TimerInit(Timer* timer)
{
    timer->end_time = (struct timeval){0, 0};
}

char TimerIsExpired(Timer* timer)
{
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
    timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout, 0};
    timeradd(&now, &interval, &timer->end_time);
}


int TimerLeftMS(Timer* timer)
{
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    //printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

*/
