#include "hal_data.h"
#include "sm_bsp_pmu.h"
#include "sm_pmu_app.h"
#include "sm_counter_utils.h"
int64_t get_tick_count();
void SysTick_Handler(void);

volatile int64_t sys_tick = 0;
volatile uint32_t time_start = 0;
volatile uint32_t time_calc = 0;
uint8_t one_write_rx_state = 10;


int64_t get_tick_count(){
    return sys_tick;
}
void SysTick_Handler(void){
    sys_tick++;
}
sm_hal_io_t* g_wire_tx = NULL;
void hal_entry(void){

	__disable_irq();
    SystemCoreClockUpdate();  // Ensure correct clock settings
    SysTick_Config(SystemCoreClock / 1000);  // 1ms tick
    NVIC_SetPriority(SysTick_IRQn, 0);  // Set priority
    sm_bsp_pmu_init();
    sm_counter_init();
    sm_one_write_init();
    g_wire_tx =  sm_bsp_get_one_write_tx();
	__enable_irq();
    while(1){
    	sm_pmu_app_process();
//    	sm_hal_io_set_value(g_wire_tx, 0);
//    	R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MICROSECONDS);
//    	sm_hal_io_set_value(g_wire_tx, 1);
    	R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

