/*
 * delay.c
 *
 *  Created on: Jul 10, 2023
 *      Author: Admin
 */
#include "time.h"

/*Include mcu platform*/

/*End include mcu platform*/

#include "sm_hal_delay.h"
#include <unistd.h>

void sm_hal_delay_ms(uint32_t timems){
    usleep(1000*timems);
}
void sm_hal_delay_us(uint32_t timeus){
    usleep(timeus);
}
