//
// Created by vnbk on 05/09/2024.
//

#ifndef EV_SDK_SM_BSP_HMI_H
#define EV_SDK_SM_BSP_HMI_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_hal.h"

//#define get_tick_count sm_bsp_sys_get_tick

void sm_bsp_bpa_init();

void sm_bsp_disable_irq();
void sm_bsp_enable_irq();

int32_t sm_bsp_system_reset();

int32_t sm_bsp_sys_init();
int32_t sm_bsp_sys_deinit();
int32_t sm_bsp_sys_get_tick();

sm_hal_io_t* sm_bsp_bpa_get_lte_power();
sm_hal_uart_t* sm_bsp_bpa_get_lte_uart();
sm_hal_io_t* sm_bsp_bpa_get_lte_power_control();
sm_hal_io_t* sm_bsp_bpa_get_lte_wakeup();

sm_hal_io_t* sm_bsp_bpa_get_gps_reset();
sm_hal_uart_t* sm_bsp_bpa_get_gps_uart();

sm_hal_can_t* sm_bsp_bpa_get_can_port();

sm_hal_spi_t* sm_bsp_bpa_get_ext_mem();
sm_hal_io_t* sm_bsp_bpa_get_ext_mem_cs();

sm_hal_flash_t* sm_bsp_bpa_get_data_flash();

sm_hal_io_t* sm_bsp_bpa_get_node_id0();
sm_hal_io_t* sm_bsp_bpa_get_node_id1();
sm_hal_io_t* sm_bsp_bpa_get_node_id2();

sm_hal_rtc_t* sm_bsp_bpa_get_rtc();

sm_hal_io_t* sm_bsp_bpa_get_charger_sw();
sm_hal_adc_t* sm_bsp_bpa_get_charger_vol();

sm_hal_timer_t* sm_bsp_bpa_get_timer(int32_t _timer);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_BSP_HMI_H
