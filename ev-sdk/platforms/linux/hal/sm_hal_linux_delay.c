//
// Created by vnbk on 12/09/2024.
//
#include <sm_hal_delay.h>
#include <unistd.h>

void sm_hal_delay_ms(uint32_t time_ms){
    usleep(1000*time_ms);
}
void sm_hal_delay_us(uint32_t time_us){
    usleep(time_us);
}