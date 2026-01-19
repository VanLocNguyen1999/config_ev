#ifndef PORTING_INCLUDE_SM_PORTING_H_
#define PORTING_INCLUDE_SM_PORTING_H_

// #include "sm_hal_adc.h"
// #include "sm_hal_can.h"
#include "sm_hal_delay.h"
// #include "sm_hal_flash.h"
// #include "sm_hal_i2c.h"
#include "sm_hal_io.h"
// #include "sm_hal_pwm.h"
// #include "sm_hal_spi.h"
// #include "sm_hal_timer.h"
#include "sm_hal_uart.h"

// extern sm_hal_adc_proc_t adc_func;
// extern sm_hal_can_proc_t can_func;
// extern sm_hal_flash_proc_t flash_func;
extern sm_hal_io_proc_t io_func;
// extern sm_hal_iic_proc_t iic_func;
// extern sm_hal_pwm_proc_t pwm_func;
// extern sm_hal_spi_proc_t spi_func;
// extern sm_hal_timer_proc_t timer_func;
extern sm_hal_uart_proc_t uart_func;


#endif /* PORTING_INCLUDE_SM_PORTING_H_ */
