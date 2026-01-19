#include "sm_hal_delay.h"
#include "sm_time_utils.h"


#ifdef __RTOS
#include "FreeRTOS.h"
#include "task.h"

void sm_hal_delay_ms(uint32_t _ms){
//    R_BSP_SoftwareDelay(_ms, 1000);
//    delayMs(_ms);
    vTaskDelay(_ms);
}

void sm_hal_delay_us(uint32_t _us){
    vTaskDelay(_us);
}
#else
void sm_hal_delay_ms(uint32_t _ms){
    int64_t timeout = get_tick_count() + _ms;
    while (timeout > get_tick_count());
}

void sm_hal_delay_us(uint32_t _us){
//	R_BSP_SoftwareDelay(_us, 1);
    (void) _us;
}
#endif
