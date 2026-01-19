/*
 * sm_bsp_bpa.c
 *
 *  Created on: Oct 1, 2024
 *      Author: thuannq
 */
#include "sm_bsp_bpa.h"
#include "hal_data.h"

sm_hal_io_t *g_lte_rst;
sm_hal_io_t *g_lte_power;
sm_hal_io_t *g_lte_wakeup;
sm_hal_io_t *g_exfl_cs;
sm_hal_io_t *g_can_stb;
sm_hal_io_t *g_gps_rst;
sm_hal_io_t *g_nodeid_0;
sm_hal_io_t *g_nodeid_1;
sm_hal_io_t *g_nodeid_2;
sm_hal_io_t *g_charger_sw;
sm_hal_io_t *g_lm5116_en;

sm_hal_adc_t *g_charger_vol;

sm_hal_can_t *g_can_port;

sm_hal_uart_t *g_bsp_lte_uart;
sm_hal_uart_t *g_bsp_gps_uart;

sm_hal_spi_t *g_exfl_spi;

sm_hal_timer_t *g_co_proc;
sm_hal_timer_t *g_iot_proc;

sm_hal_flash_t *g_inter_fl = NULL;
sm_hal_rtc_t *g_realtime_clock = NULL;

int32_t sm_bsp_sys_init(){
    return 0;
}
int32_t sm_bsp_sys_deinit(){
	return 0;
}
int32_t sm_bsp_sys_get_tick(){
	return 0;
}

int32_t sm_bsp_system_reset(){
    NVIC_SystemReset();
    return 0;
}

sm_hal_io_t* sm_bsp_bpa_get_lte_power(){
	return g_lte_rst;
}
sm_hal_uart_t* sm_bsp_bpa_get_lte_uart(){
	return g_bsp_lte_uart;
}

sm_hal_io_t* sm_bsp_bpa_get_lte_power_control(){
    return g_lte_power;
}
sm_hal_io_t* sm_bsp_bpa_get_lte_wakeup(){
    return g_lte_wakeup;
}

sm_hal_io_t* sm_bsp_bpa_get_ext_mem_cs(){
    return g_exfl_cs;
}

sm_hal_io_t* sm_bsp_bpa_get_gps_reset(){
	return g_gps_rst;
}
sm_hal_uart_t* sm_bsp_bpa_get_gps_uart(){
	return g_bsp_gps_uart;
}

sm_hal_can_t* sm_bsp_bpa_get_can_port(){
	return g_can_port;
}

sm_hal_spi_t* sm_bsp_bpa_get_ext_mem(){
	return g_exfl_spi;
}

sm_hal_flash_t* sm_bsp_bpa_get_data_flash(){
	return g_inter_fl;
}

sm_hal_rtc_t* sm_bsp_bpa_get_rtc(){
    return g_realtime_clock;
}

sm_hal_io_t* sm_bsp_bpa_get_node_id0(){
    return g_nodeid_0;
}

sm_hal_io_t* sm_bsp_bpa_get_node_id1(){
    return g_nodeid_1;
}

sm_hal_io_t* sm_bsp_bpa_get_node_id2(){
    return g_nodeid_2;
}

sm_hal_io_t* sm_bsp_bpa_get_charger_sw(){
    return g_charger_sw;
}

sm_hal_adc_t* sm_bsp_bpa_get_charger_vol(){
    return g_charger_vol;
}

sm_hal_timer_t* sm_bsp_bpa_get_timer(int32_t _timer){
    switch (_timer) {
        case SM_HAL_TIMER_1:
            return g_iot_proc;
        case SM_HAL_TIMER_3:
            return g_co_proc;
        default:
            return NULL;
    }
}

static void bsp_io_init();
static void bsp_uart_init();
static void bsp_can_init();
static void bsp_timer_init();
static void bsp_spi_init();
static void bsp_i2c_init();
static void bsp_rtc_init();
static void bsp_flash_init();
static void bsp_adc_init();

void sm_bsp_bpa_init(){
    sm_bsp_disable_irq();
    bsp_io_init();
    bsp_uart_init();
    bsp_can_init();
    bsp_timer_init();
    bsp_spi_init();
//    bsp_i2c_init();
//    bsp_rtc_init();
    bsp_flash_init();
    bsp_adc_init();
    sm_bsp_enable_irq();
}

void sm_bsp_disable_irq(){
    __disable_irq();
//    __disable_fault_irq();
}

void sm_bsp_enable_irq(){
    __enable_irq();
//    __enable_fault_irq();
}

//IO
static void bsp_io_init(){
     g_lte_rst = sm_hal_io_init(io_lte_rst);
     sm_hal_io_open(g_lte_rst, SM_HAL_IO_OUTPUT);

     g_lte_power = sm_hal_io_init(io_lte_pwr);
     sm_hal_io_open(g_lte_power, SM_HAL_IO_OUTPUT);
     sm_hal_io_set_value(g_lte_power, 1);

     g_lte_wakeup = sm_hal_io_init(io_lte_wakeup);
     sm_hal_io_open(g_lte_wakeup, SM_HAL_IO_OUTPUT);
     sm_hal_io_set_value(g_lte_wakeup, 0);

     g_exfl_cs = sm_hal_io_init(io_spi_cs);
     sm_hal_io_open(g_exfl_cs, SM_HAL_IO_OUTPUT);
     sm_hal_io_set_value(g_exfl_cs, 1);

     g_can_stb = sm_hal_io_init(io_can_stb);
     sm_hal_io_open(g_can_stb, SM_HAL_IO_OUTPUT);

     g_gps_rst = sm_hal_io_init(io_gps_rst);
     sm_hal_io_open(g_gps_rst, SM_HAL_IO_OUTPUT);

     g_nodeid_0 = sm_hal_io_init(io_node_id1);
    sm_hal_io_open(g_nodeid_0, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_nodeid_0, 0);

    g_nodeid_1 = sm_hal_io_init(io_node_id2);
    sm_hal_io_open(g_nodeid_1, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_nodeid_1, 0);

    g_nodeid_2 = sm_hal_io_init(io_node_id3);
    sm_hal_io_open(g_nodeid_2, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_nodeid_2, 0);

    g_charger_sw = sm_hal_io_init(io_charger_mcu);
    sm_hal_io_open(g_charger_sw, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_charger_sw, 0);

    g_lm5116_en = sm_hal_io_init(io_lm5116_en);
    sm_hal_io_open(g_lm5116_en, SM_HAL_IO_OUTPUT);
    sm_hal_io_set_value(g_lm5116_en, 0);
}
//UART
static void bsp_uart_init(){
	g_bsp_lte_uart = sm_hal_uart_init(&g_lte_uart, 115200, 1, 8);
    sm_hal_uart_open(g_bsp_lte_uart);

    g_bsp_gps_uart = sm_hal_uart_init(&g_gps_uart, 9600, 1, 8);
    sm_hal_uart_open(g_bsp_gps_uart);
}

void lte_callback(uart_callback_args_t *p_args){
    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
            sm_hal_uart_tx_irq(g_bsp_lte_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_bsp_lte_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_ERR_PARITY:     ///< Parity error event
        case UART_EVENT_ERR_FRAMING:    ///< Mode fault error event
        case UART_EVENT_ERR_OVERFLOW:   ///< FIFO Overflow error event
        case UART_EVENT_BREAK_DETECT:   ///< Break detect error event
        case UART_EVENT_TX_DATA_EMPTY:  ///< Last byte is transmitting, ready for more data
            break;
    }
}

void gps_callback(uart_callback_args_t *p_args){
    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
            sm_hal_uart_tx_irq(g_bsp_gps_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_bsp_gps_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_ERR_PARITY:     ///< Parity error event
        case UART_EVENT_ERR_FRAMING:    ///< Mode fault error event
        case UART_EVENT_ERR_OVERFLOW:   ///< FIFO Overflow error event
        case UART_EVENT_BREAK_DETECT:   ///< Break detect error event
        case UART_EVENT_TX_DATA_EMPTY:  ///< Last byte is transmitting, ready for more data
            break;
    }
}

//CAN
static void bsp_can_init(){
    g_can_port = sm_hal_can_init(&g_can0, 500000);
    sm_hal_can_open(g_can_port);
    sm_hal_io_set_value(g_can_stb, 0);
}

static sm_hal_can_msg_t g_can_msg_buff;
void can_callback(can_callback_args_t* p_args){
    switch (p_args->event){
        case CAN_EVENT_RX_COMPLETE:
            g_can_msg_buff.id = p_args->frame.id;
            memcpy(g_can_msg_buff.data, p_args->frame.data, 8);
            g_can_msg_buff.length = p_args->frame.data_length_code;
            sm_hal_can_rx_irq(g_can_port, &g_can_msg_buff);
            return ;
        case CAN_EVENT_TX_COMPLETE:

            return ;
    }
}

//TIMER
static void bsp_timer_init(){
	g_co_proc = sm_hal_timer_init(&g_timer3_1ms, 0);
    sm_hal_timer_start(g_co_proc);

    g_iot_proc = sm_hal_timer_init(&g_timer1_1ms, 0);
    sm_hal_timer_start(g_iot_proc);
}

void timer1_ISR(timer_callback_args_t *p_args){
	sm_hal_timer_irq(g_iot_proc);
}

void timer3_ISR(timer_callback_args_t* p_args){
    sm_hal_timer_irq(g_co_proc);
}

//SPI
static void bsp_spi_init(){
    g_exfl_spi = sm_hal_spi_init(&g_exflash_spi);
    sm_hal_spi_open(g_exfl_spi);
}
void exflash_spi_callback(spi_callback_args_t* p_args){
    (void*)p_args;
}

/* Callback function */
void data_flash_cb(flash_callback_args_t *p_args)
{
  //   TODO: add your own code here
}

/*
static void bsp_rtc_init(){
    g_realtime_clock = sm_hal_rtc_init(&mcu_rtc);
    sm_hal_rtc_open(g_realtime_clock);
}

void rtc_callback(rtc_callback_args_t *p_args){
    sm_hal_rtc_irq_cb(g_realtime_clock);
}
*/
static void bsp_flash_init(){
    g_inter_fl = sm_hal_flash_init(&g_flash0);
}

static void bsp_adc_init(){
    g_charger_vol = sm_hal_adc_init(&g_adc0, ADC_CHANNEL_16, ADC_MASK_CHANNEL_16);
    sm_hal_adc_open(g_charger_vol);
    sm_hal_adc_start(g_charger_vol);
}
