//
// Created by vnbk on 01/04/2023.
//

#ifndef PAHO_MQTT_TIMER_H
#define PAHO_MQTT_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_elapsed_timer.h"

typedef struct Timer Timer;
struct Timer {
    elapsed_timer_t m_timer;
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

#ifdef __cplusplus
};
#endif

#endif //PAHO_MQTT_TIMER_H
