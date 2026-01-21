#include "hal_data.h"
#include "sm_bsp_pmu.h"
#include "sm_pmu_app.h"
int64_t get_tick_count();
void SysTick_Handler(void);

volatile int64_t sys_tick = 0;

int64_t get_tick_count(){
    return sys_tick;
}
void SysTick_Handler(void){
    sys_tick++;
}

static void sm_hal_timer1_cb(void* _arg){
    sm_pmu_app_t* app = (sm_pmu_app_t*)_arg;
    sm_co_process(app->m_co, 1);
}

static void sm_hal_timer2_cb(void* _arg){

    sm_pmu_app_t* app = (sm_pmu_app_t*)_arg;
    sm_sch_process(app->m_sch_task);
}

static void sm_hal_timer4_cb(void* _arg){

	FSP_PARAMETER_NOT_USED(_arg);
//    sm_pmu_app_t* app = (sm_pmu_app_t*)_arg;
//    sm_pmu_assign_process(app);
}

void hal_entry(void){

	__disable_irq();
    SystemCoreClockUpdate();  // Ensure correct clock settings
    SysTick_Config(SystemCoreClock / 1000);  // 1ms tick
    NVIC_SetPriority(SysTick_IRQn, 0);  // Set priority
    sm_bsp_pmu_init();
    sm_pmu_app_t* pmu = sm_pmu_app_create();
    sm_hal_timer_agt_t * timer1 = sm_bsp_pmu_get_timer_agt(SM_HAL_TIMER_1);
    if(timer1){
        sm_hal_timer_agt_set_callback(timer1, sm_hal_timer1_cb, pmu);
    }
    sm_hal_timer_agt_t * timer2 = sm_bsp_pmu_get_timer_agt(SM_HAL_TIMER_2);
    if(timer2){
        sm_hal_timer_agt_set_callback(timer2, sm_hal_timer2_cb, pmu);
    }
    sm_hal_timer_agt_t * timer4 = sm_bsp_pmu_get_timer_agt(SM_HAL_TIMER_4);
    if(timer4){
        sm_hal_timer_agt_set_callback(timer4, sm_hal_timer4_cb, pmu);
    }
	__enable_irq();
    while(1){

    	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        sm_pmu_uart_process(pmu);
    }
}
