/*
 * sm_bsp_hmi.c
 *
 *  Created on: Oct 1, 2024
 *      Author: vuonglk
 */
#include "sm_bsp_pmu.h"
#include "hal_data.h"
#include "sm_logger.h"


sm_hal_io_t *g_io_in_12v_det;

sm_hal_io_t *g_io_out_12v_en;
sm_hal_io_t *g_io_out_can_mode;
sm_hal_io_t *g_io_out_nodeid_1;
sm_hal_io_t *g_io_out_nodeid_2;
sm_hal_io_t *g_io_out_nodeid_3;

sm_hal_io_t *g_led_1;
sm_hal_io_t *g_led_2;
sm_hal_io_t *g_led_3;

sm_hal_can_t *g_pmu_can;

sm_hal_timer_t *g_pmu_timer3;

sm_hal_timer_agt_t *g_pmu_timer0;
sm_hal_timer_agt_t *g_pmu_timer1;
sm_hal_timer_agt_t *g_pmu_timer2;
sm_hal_timer_agt_t *g_pmu_timer4;

sm_hal_flash_t *g_pmu_flash;

sm_hal_uart_t *g_pmu_uart;


sm_hal_io_t* sm_bsp_pmu_get_node_id1(){
    return g_io_out_nodeid_1;
}

sm_hal_io_t* sm_bsp_pmu_get_node_id2(){
    return g_io_out_nodeid_2;
}

sm_hal_io_t* sm_bsp_pmu_get_node_id3(){
    return g_io_out_nodeid_3;
}

sm_hal_timer_agt_t* sm_bsp_pmu_get_timer_agt(int32_t _timer){
    switch (_timer) {
        case SM_HAL_TIMER_0:
            return g_pmu_timer0;
        case SM_HAL_TIMER_1:
            return g_pmu_timer1;
        case SM_HAL_TIMER_2:
            return g_pmu_timer2;
        case SM_HAL_TIMER_4:
            return g_pmu_timer4;
        default:
            return NULL;
    }
}

sm_hal_timer_t* sm_bsp_pmu_get_timer(int32_t _timer){
    switch (_timer) {
        case SM_HAL_TIMER_3:
            return g_pmu_timer3;
        default:
            return NULL;
    }
}

static void bsp_io_init();
static void bsp_can_init();
static inline void bsp_timer_init();
static void bsp_flash_init();
static void bsp_uart_init();


int32_t sm_bsp_pmu_init(void){
    __disable_irq();
    bsp_io_init();
    bsp_can_init();
    bsp_timer_init();
    bsp_flash_init();
    bsp_uart_init();
    __enable_irq();
    return 0;
}


static void bsp_io_init(){
    /* IN PUT */
    g_io_in_12v_det	= sm_hal_io_init(IN_12V_DETECTOR);
    sm_hal_io_open(g_io_in_12v_det, SM_HAL_IO_INPUT);

    /* OUTPUT */

    g_led_1   = sm_hal_io_init(LED1);
    sm_hal_io_open(g_led_1, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_led_1, 0);

    g_led_2   = sm_hal_io_init(LED2);
    sm_hal_io_open(g_led_2, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_led_2, 0);

    g_led_3   = sm_hal_io_init(LED3);
    sm_hal_io_open(g_led_3, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_led_3, 0);

    g_io_out_nodeid_1   = sm_hal_io_init(NodeID1);
    sm_hal_io_open(g_io_out_nodeid_1, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_io_out_nodeid_1, 0);

    g_io_out_nodeid_2   = sm_hal_io_init(NodeID2);
    sm_hal_io_open(g_io_out_nodeid_2, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_io_out_nodeid_2, 0);

    g_io_out_nodeid_3   = sm_hal_io_init(NodeID3);
    sm_hal_io_open(g_io_out_nodeid_3, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_io_out_nodeid_3, 0);
//
    g_io_out_12v_en = sm_hal_io_init(OUT_12V_ENABLE);
    sm_hal_io_open(g_io_out_12v_en, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_io_out_12v_en, 1);
//    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_00, 1);

    g_io_out_can_mode = sm_hal_io_init(CAN_MODE);
    sm_hal_io_open(g_io_out_can_mode, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_io_out_can_mode, 0);
}

static void bsp_can_init(){
    g_pmu_can = sm_hal_can_init(&g_can0, 500000);
    sm_hal_can_open(g_pmu_can);
    sm_hal_io_set_value(g_io_out_can_mode, 0);
}

static inline void bsp_timer_init(){

    g_pmu_timer1 = sm_hal_timer_agt_init(&g_timer1, 0);
    sm_hal_timer_agt_start(g_pmu_timer1);

    g_pmu_timer2 = sm_hal_timer_agt_init(&g_timer2, 0);
    sm_hal_timer_agt_start(g_pmu_timer2);

    g_pmu_timer4 = sm_hal_timer_agt_init(&g_timer4, 0);
    sm_hal_timer_agt_start(g_pmu_timer4);
}

static void bsp_flash_init(){
	g_pmu_flash = sm_hal_flash_init(&g_flash);
}

static void bsp_uart_init(){

    g_pmu_uart = sm_hal_uart_init(&g_io_uart, 115200, 1, 8);
    sm_hal_uart_open(g_pmu_uart);
}

sm_hal_can_t* sm_bsp_pmu_get_can_port(){
	return g_pmu_can;
}

sm_hal_flash_t* sm_bsp_pmu_get_data_flash(){
    return g_pmu_flash;
}

sm_hal_uart_t* sm_bsp_pmu_get_uart_port(){
    return g_pmu_uart;
}

/*--------------------------------CAN MODE--------------------------------*/
int32_t sm_bsp_pmu_io_set_deactice_can_bus(){
    return sm_hal_io_set_value(g_io_out_can_mode, SM_HAL_IO_ON);
}
int32_t sm_bsp_pmu_io_set_active_can_bus(){
    return sm_hal_io_set_value(g_io_out_can_mode, SM_HAL_IO_OFF);
}
/*--------------------------------NodeID--------------------------------*/
int32_t sm_bsp_pmu_io_set_node_id1                          (uint8_t _value)
{
    return sm_hal_io_set_value (g_io_out_nodeid_1, _value);
//    return R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_04, _value);
}
int32_t sm_bsp_pmu_io_set_node_id2                          (uint8_t _value)
{
    return sm_hal_io_set_value (g_io_out_nodeid_2, _value);
//    return R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_12, _value);

}
int32_t sm_bsp_pmu_io_set_node_id3                          (uint8_t _value)
{
    return sm_hal_io_set_value (g_io_out_nodeid_3, _value);
//    return R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_11, _value);
}

int32_t sm_bsp_led_green_set(uint8_t _value)
{
	return sm_hal_io_set_value(g_led_1, _value);
}
int32_t sm_bsp_led_red_set(uint8_t _value)
{
	return sm_hal_io_set_value(g_led_2, _value);
}
int32_t sm_bsp_led_blue_set(uint8_t _value)
{
	return sm_hal_io_set_value(g_led_3, _value);
}
/*--------------------------------12V--------------------------------*/
int32_t sm_bsp_pmu_io_get_12v_det(){
	uint8_t ret = 0;
	fsp_err_t err = R_IOPORT_PinRead(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_01,&ret);

    if (err) {
       return 0;
    }
    return ret ? 0: 1;
}

int32_t sm_bsp_pmu_io_set_12V_power                          (uint8_t _value){


	uint8_t cmd = (_value == 1) ? 0 : 1;
	return sm_hal_io_set_value(g_io_out_12v_en, cmd);
//		return R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_00, cmd);
}
/*--------------------------------CAN--------------------------------*/
static sm_hal_can_msg_t g_can_msg_buff;
void can_callback(can_callback_args_t* p_args){

    switch (p_args->event){
        case CAN_EVENT_RX_COMPLETE:
            g_can_msg_buff.id = p_args->frame.id;
            memcpy(g_can_msg_buff.data, p_args->frame.data, 8);
            g_can_msg_buff.length = p_args->frame.data_length_code;
            sm_hal_can_rx_irq(g_pmu_can, &g_can_msg_buff);
            return ;
        case CAN_EVENT_TX_COMPLETE:
            return ;
        default:
            break;
    }
}

/* TIMER */
void sm_bsp_pmu_timer_0_set_callback(void* _fn, void* _arg){
    sm_hal_timer_agt_set_callback(g_pmu_timer0, _fn, _arg);
}

void sm_bsp_pmu_timer_1_set_callback(void* _fn, void* _arg){
    sm_hal_timer_agt_set_callback(g_pmu_timer1, _fn, _arg);
}

void sm_bsp_pmu_timer_2_set_callback(void* _fn, void* _arg){
    sm_hal_timer_agt_set_callback(g_pmu_timer2, _fn, _arg);
}

void sm_bsp_pmu_timer_3_set_callback(void* _fn, void* _arg){
    sm_hal_timer_set_callback(g_pmu_timer3, _fn, _arg);
}
void sm_bsp_pmu_timer_4_set_callback(void* _fn, void* _arg){
    sm_hal_timer_agt_set_callback(g_pmu_timer4, _fn, _arg);
}
/*------------------TIME ISR------------------*/
void timer0_ISR(timer_callback_args_t *p_args){

    (void) p_args;
    sm_hal_timer_irq(g_pmu_timer0);
}

void timer1_ISR(timer_callback_args_t* p_args){

    (void) p_args;
    sm_hal_timer_irq(g_pmu_timer1);
}
void timer2_ISR(timer_callback_args_t *p_args){

    (void) p_args;
    sm_hal_timer_irq(g_pmu_timer2);
}

void timer3_ISR(timer_callback_args_t* p_args){

    (void) p_args;
    sm_hal_timer_irq(g_pmu_timer3);
}
void timer4_ISR(timer_callback_args_t* p_args){

    (void) p_args;
    sm_hal_timer_irq(g_pmu_timer4);
}

void uart_io_callback(uart_callback_args_t *p_args){

    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
            sm_hal_uart_tx_irq(g_pmu_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_pmu_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_ERR_PARITY:     ///< Parity error event
        case UART_EVENT_ERR_FRAMING:    ///< Mode fault error event
        case UART_EVENT_ERR_OVERFLOW:   ///< FIFO Overflow error event
        case UART_EVENT_BREAK_DETECT:   ///< Break detect error event
        case UART_EVENT_TX_DATA_EMPTY:  ///< Last byte is transmitting, ready for more data
            break;
    }
}

void ISR_Flash(flash_callback_args_t *p_args){

    (void) p_args;
}
int32_t sm_pmu_bsp_reboot(){
    __NVIC_SystemReset();
    return 0;
}
