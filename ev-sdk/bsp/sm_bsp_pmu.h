//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_BSP_PMU_H
#define EV_SDK_SM_BSP_PMU_H

#include <stdint.h>
#include "sm_hal.h"
enum {
    IO_STATE_OFF = 0,
    IO_STATE_ON
};
int32_t sm_bsp_pmu_init                         (void);
int32_t sm_pmu_bsp_reboot                       ();

/* INTERFACE */
sm_hal_can_t* sm_bsp_pmu_get_can_port           ();
sm_hal_flash_t* sm_bsp_pmu_get_data_flash       ();
sm_hal_uart_t* sm_bsp_pmu_get_uart_port         ();
/* INPUT */
int32_t sm_bsp_pmu_io_get_12v_det               ();

/* OUTPUT */
int32_t sm_bsp_pmu_io_set_12V_power             (uint8_t _value);
int32_t sm_bsp_pmu_io_set_deactice_can_bus      ();
int32_t sm_bsp_pmu_io_set_active_can_bus        ();

int32_t sm_bsp_pmu_io_set_node_id1              (uint8_t _value);
int32_t sm_bsp_pmu_io_set_node_id2              (uint8_t _value);
int32_t sm_bsp_pmu_io_set_node_id3              (uint8_t _value);

int32_t sm_bsp_led_green_set(uint8_t _value);
int32_t sm_bsp_led_red_set(uint8_t _value);
int32_t sm_bsp_led_blue_set(uint8_t _value);

sm_hal_io_t* sm_bsp_pmu_get_node_id1            ();

sm_hal_io_t* sm_bsp_pmu_get_node_id2            ();

sm_hal_io_t* sm_bsp_pmu_get_node_id3            ();

/* CAN */
void sm_bsp_pmu_can_send                        (int32_t _id, const char* _data);

/* TIMER */
void sm_bsp_pmu_timer_0_set_callback            (void* _fn, void* _arg);
void sm_bsp_pmu_timer_1_set_callback            (void* _fn, void* _arg);
void sm_bsp_pmu_timer_2_set_callback            (void* _fn, void* _arg);
void sm_bsp_pmu_timer_3_set_callback            (void* _fn, void* _arg);
void sm_bsp_pmu_timer_4_set_callback            (void* _fn, void* _arg);

sm_hal_timer_agt_t* sm_bsp_pmu_get_timer_agt    (int32_t _timer);
sm_hal_timer_t* sm_bsp_pmu_get_timer            (int32_t _timer);
#endif //EV_SDK_SM_BSP_PMU_H
