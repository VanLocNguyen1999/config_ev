#include "hal_data.h"
#include "sm_bsp_pmu.h"
#include "sm_pmu_app.h"
#include "one_write_tx.h"
int64_t get_tick_count();
void SysTick_Handler(void);

volatile int64_t sys_tick = 0;
volatile uint64_t us_tick = 0;
uint8_t one_write_rx_state = 10;
OneWireTx_t g_one_write;
int64_t get_tick_count(){
    return sys_tick;
}
void SysTick_Handler(void){
    sys_tick++;
}
sm_hal_io_t*tx_io = NULL;
void hal_entry(void){

	__disable_irq();
    SystemCoreClockUpdate();  // Ensure correct clock settings
    SysTick_Config(SystemCoreClock / 1000);  // 1ms tick
    NVIC_SetPriority(SysTick_IRQn, 0);  // Set priority
    sm_bsp_pmu_init();
    sm_one_write_init();
    tx_io = sm_bsp_get_one_write_tx();
	__enable_irq();
    while(1){
//    	sm_pmu_app_process();
//    	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    	sm_hal_io_set_value(tx_io,0);
    	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    	sm_hal_io_set_value(tx_io,1);
    	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    }
}

