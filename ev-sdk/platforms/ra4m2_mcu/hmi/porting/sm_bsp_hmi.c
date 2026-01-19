/*
 * sm_bsp_hmi.c
 *
 *  Created on: Oct 1, 2024
 *      Author: vuonglk
 */
#include "sm_bsp_hmi.h"
#include "hal_data.h"
#include "sm_logger.h"

sm_hal_io_t *g_lte_rst;
sm_hal_io_t *g_exfl_cs;
sm_hal_io_t *g_can_stb;
sm_hal_io_t *g_lcd_pow;
sm_hal_io_t *g_left_signal;
sm_hal_io_t *g_right_signal;
sm_hal_io_t *g_ble_rst;
sm_hal_io_t *g_ble_ind;
sm_hal_io_t *g_ble_wk;
sm_hal_io_t *g_gps_rst;

sm_hal_adc_t *g_input_vol;

sm_hal_can_t *g_hmi_can;

sm_hal_uart_t *g_lte_uart;
sm_hal_uart_t *g_ble_uart;
sm_hal_uart_t *g_gps_uart;

sm_hal_i2c_t *g_lcd_i2c;

sm_hal_spi_t *g_exfl_spi;

sm_hal_timer_t *g_timer_1ms;

sm_hal_flash_t *g_inter_fl;

sm_hal_rtc_t *g_realtime_clock;

volatile uint8_t db_tx_empty = 0;

int32_t sm_bsp_hmi_sys_init(){
    return 0;
}
int32_t sm_bsp_hmi_sys_deinit(){
	return 0;
}
int32_t sm_bsp_hmi_sys_get_tick(){
	return 0;
}

int32_t sm_bsp_hmi_system_reset(){
    NVIC_SystemReset();
    return 0;
}

sm_hal_io_t* sm_bsp_hmi_get_lte_power(){
	return g_lte_rst;
}
sm_hal_uart_t* sm_bsp_hmi_get_lte_uart(){
	return g_lte_uart;
}

sm_hal_io_t* sm_bsp_hmi_get_left_signal(){
	return g_left_signal;
}
sm_hal_io_t* sm_bsp_hmi_get_right_signal(){
	return g_right_signal;
}

sm_hal_io_t* sm_bsp_hmi_get_lcd_power(){
	return g_lcd_pow;
}
sm_hal_i2c_t* sm_bsp_hmi_get_lcd_i2c(){
	return g_lcd_i2c;
}

sm_hal_io_t* sm_bsp_hmi_get_ble_reset(){
	return g_ble_rst;
}

sm_hal_io_t* sm_bsp_hmi_get_ble_indicator(){
    return g_ble_ind;
}

sm_hal_io_t* sm_bsp_hmi_get_ble_wakeup(){
    return g_ble_wk;
}

sm_hal_io_t* sm_bsp_hmi_get_ext_mem_cs(){
    return g_exfl_cs;
}
sm_hal_uart_t* sm_bsp_hmi_get_ble_uart(){
	return g_ble_uart;
}

sm_hal_io_t* sm_bsp_hmi_get_gps_reset(){
	return g_gps_rst;
}
sm_hal_uart_t* sm_bsp_hmi_get_gps_uart(){
	return g_gps_uart;
}

sm_hal_adc_t* sm_bsp_hmi_get_input_vol(){
	return g_input_vol;
}

sm_hal_can_t* sm_bsp_hmi_get_can_port(){
	return g_hmi_can;
}

sm_hal_spi_t* sm_bsp_hmi_get_ext_mem(){
	return g_exfl_spi;
}

sm_hal_flash_t* sm_bsp_hmi_get_data_flash(){
	return g_inter_fl;
}

sm_hal_rtc_t* sm_bsp_hmi_get_rtc(){
    return g_realtime_clock;
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

void sm_bsp_hmi_init(){
    sm_bsp_disable_irq();
    bsp_io_init();
    bsp_uart_init();
    bsp_can_init();
    bsp_timer_init();
    bsp_spi_init();
    bsp_i2c_init();
    bsp_rtc_init();
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
     g_lte_rst = sm_hal_io_init(LTE_RST);
     sm_hal_io_open(g_lte_rst, SM_HAL_IO_OUTPUT);

     g_exfl_cs = sm_hal_io_init(EXFL_CS);
     sm_hal_io_open(g_exfl_cs, SM_HAL_IO_OUTPUT);
     sm_hal_io_set_value(g_exfl_cs, 1);

     g_can_stb = sm_hal_io_init(CAN_STB);
     sm_hal_io_open(g_can_stb, SM_HAL_IO_OUTPUT);

     g_lcd_pow = sm_hal_io_init(LCD_POW);
     sm_hal_io_open(g_lcd_pow, SM_HAL_IO_OUTPUT);
     sm_hal_io_set_value(g_lcd_pow, 0);

     g_left_signal = sm_hal_io_init(LED_LEFT);
     sm_hal_io_open(g_left_signal, SM_HAL_IO_OUTPUT);

  g_right_signal = sm_hal_io_init(LED_RIGHT);
     sm_hal_io_open(g_right_signal, SM_HAL_IO_OUTPUT);

     sm_hal_io_set_value(g_left_signal, 0);
     sm_hal_io_set_value(g_right_signal, 0);

     g_ble_rst = sm_hal_io_init(BLE_RST);
     sm_hal_io_open(g_ble_rst, SM_HAL_IO_OUTPUT);

     g_ble_ind = sm_hal_io_init(BLE_IND);
     sm_hal_io_open(g_ble_ind, SM_HAL_IO_INPUT);

     g_ble_wk = sm_hal_io_init(BLE_WK);
     sm_hal_io_open(g_ble_wk, SM_HAL_IO_OUTPUT);

     g_gps_rst = sm_hal_io_init(GPS_RST);
     sm_hal_io_open(g_gps_rst, SM_HAL_IO_OUTPUT);
}
//UART
static void bsp_uart_init(){
    g_lte_uart = sm_hal_uart_init(&lte_uart, 115200, 1, 8);
    sm_hal_uart_open(g_lte_uart);

    g_ble_uart = sm_hal_uart_init(&ble_uart, 9600, 1, 8);
    sm_hal_uart_open(g_ble_uart);

    g_gps_uart = sm_hal_uart_init(&gps_uart, 9600, 1, 8);
    sm_hal_uart_open(g_gps_uart);
}

void lte_callback(uart_callback_args_t *p_args){
    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
            sm_hal_uart_tx_irq(g_lte_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_lte_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_ERR_PARITY:     ///< Parity error event
        case UART_EVENT_ERR_FRAMING:    ///< Mode fault error event
        case UART_EVENT_ERR_OVERFLOW:   ///< FIFO Overflow error event
        case UART_EVENT_BREAK_DETECT:   ///< Break detect error event
        case UART_EVENT_TX_DATA_EMPTY:  ///< Last byte is transmitting, ready for more data
            break;
    }
}

void ble_callback(uart_callback_args_t *p_args){
    switch (p_args->event){
        case UART_EVENT_RX_COMPLETE:    ///< Receive complete event
            break;
        case UART_EVENT_TX_COMPLETE:    ///< Transmit complete event
            sm_hal_uart_tx_irq(g_ble_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_ble_uart, (uint8_t)p_args->data);
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
            sm_hal_uart_tx_irq(g_gps_uart, (uint8_t)p_args->data);
            break;
        case UART_EVENT_RX_CHAR:        ///< Character received
            sm_hal_uart_rx_irq(g_gps_uart, (uint8_t)p_args->data);
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
    g_hmi_can = sm_hal_can_init(&g_can0, 500000);
    sm_hal_can_open(g_hmi_can);
    sm_hal_io_set_value(g_can_stb, 0);
}

static sm_hal_can_msg_t g_can_msg_buff;
void can_callback(can_callback_args_t* p_args){
    switch (p_args->event){
        case CAN_EVENT_RX_COMPLETE:
            g_can_msg_buff.id = p_args->frame.id;
            memcpy(g_can_msg_buff.data, p_args->frame.data, 8);
            g_can_msg_buff.length = p_args->frame.data_length_code;
            sm_hal_can_rx_irq(g_hmi_can, &g_can_msg_buff);
            return ;
        case CAN_EVENT_TX_COMPLETE:

            return ;
    }
}

//TIMER
static void bsp_timer_init(){
    g_timer_1ms = sm_hal_timer_init(&timer_1ms, 0);
    sm_hal_timer_start(g_timer_1ms);
}

void timer0_1ms(timer_callback_args_t* p_args){
    sm_hal_timer_irq(g_timer_1ms);
}

//SPI
static void bsp_spi_init(){
    g_exfl_spi = sm_hal_spi_init(&exflash_spi);
    sm_hal_spi_open(g_exfl_spi);
}
void exflash_spi_callback(spi_callback_args_t* p_args){
    (void*)p_args;
}


// I2C
static void bsp_i2c_init(){
    g_lcd_i2c = sm_hal_i2c_init(&lcd_i2c, 0x38);
    sm_hal_i2c_open(g_lcd_i2c);
}

void  lcd_i2c_callback(i2c_master_callback_args_t *p_args)
{
    sm_hal_i2c_callback(g_lcd_i2c, p_args->event);

}
/* Callback function */
void mcu_flash_callback(flash_callback_args_t *p_args)
{
    /* TODO: add your own code here */
}


static void bsp_rtc_init(){
    g_realtime_clock = sm_hal_rtc_init(&mcu_rtc);
    sm_hal_rtc_open(g_realtime_clock);
}

void rtc_callback(rtc_callback_args_t *p_args){
    sm_hal_rtc_irq_cb(g_realtime_clock);
}

static void bsp_flash_init(){
    g_inter_fl = sm_hal_flash_init(&mcu_flash);
}

static void bsp_adc_init(){
    g_input_vol = sm_hal_adc_init(&g_adc0, ADC_CHANNEL_12);
    sm_hal_adc_open(g_input_vol);
    sm_hal_adc_start(g_input_vol);
}

