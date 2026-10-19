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
uint8_t id_index = 0;
static int32_t sm_bp_nodeid_select(int32_t _id){
    if(_id == 0){
        return sm_bsp_pmu_io_set_node_id1(SM_HAL_IO_ON);
    }else if(_id == 1){
        return sm_bsp_pmu_io_set_node_id2(SM_HAL_IO_ON);
    }else if(_id == 2){
        return sm_bsp_pmu_io_set_node_id3(SM_HAL_IO_ON);
    }else{
        return -1;
    }
}

static int32_t sm_bp_nodeid_deselect(int32_t _id){
    if(_id == 0){
        return sm_bsp_pmu_io_set_node_id1(SM_HAL_IO_OFF);
    }else if(_id == 1){
        return sm_bsp_pmu_io_set_node_id2(SM_HAL_IO_OFF);
    }else if(_id == 2){
        return sm_bsp_pmu_io_set_node_id3(SM_HAL_IO_OFF);
    }else{
        return -1;
    }
}

int64_t get_tick_count(){
    return sys_tick;
}
void SysTick_Handler(void){
    sys_tick++;
}

void hal_entry(void){

	__disable_irq();
    SystemCoreClockUpdate();  // Ensure correct clock settings
    SysTick_Config(SystemCoreClock / 1000);  // 1ms tick
    NVIC_SetPriority(SysTick_IRQn, 0);  // Set priority
    sm_bsp_pmu_init();
    sm_counter_init();
    sm_one_write_init();
	__enable_irq();
    while(1){
//    	sm_pmu_app_process();
    	R_BSP_SoftwareDelay(30, BSP_DELAY_UNITS_SECONDS);
    	sm_bp_nodeid_select(id_index);
    	R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    	sm_bp_nodeid_deselect(id_index);
    	id_index ++;
    	if(id_index >= 3) id_index = 0;
    }
}

