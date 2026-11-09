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
//    sm_sch_process(app->m_sch_task);
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
	sm_stm_set_event(PMU_STM_EVENT_ENTER_STARTING);
//	sm_pmu_bpm_first_bp_auth(pmu);
	__enable_irq();
    while(1){

    	R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
//        sm_pmu_uart_process(pmu);
    }
}

static void receive_byte(char c){

    static char buf[64];
    static unsigned int i = 0;
    static int overflow = 0;

    if (c == '\n') {
        buf[i] = '\0';
        if (!overflow &&
            sscanf(buf, "%lu,%lu,", &g_pmu_app->speed_rpm, &g_pmu_app->hall_position_counter) == 2) {
        }
        i = 0;
        overflow = 0;
    } else if (!overflow) {
        if (i < sizeof(buf) - 1)
            buf[i++] = c;
        else
            overflow = 1;
    }
}

void uart_io_callback(uart_callback_args_t *p_args){

    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
//            sm_hal_uart_tx_irq(g_pmu_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
        	receive_byte((char)p_args->data);
            break;
        case UART_EVENT_ERR_PARITY:     ///< Parity error event
        case UART_EVENT_ERR_FRAMING:    ///< Mode fault error event
        case UART_EVENT_ERR_OVERFLOW:   ///< FIFO Overflow error event
        case UART_EVENT_BREAK_DETECT:   ///< Break detect error event
        case UART_EVENT_TX_DATA_EMPTY:  ///< Last byte is transmitting, ready for more data
            break;
    }
}
