/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_flash_hp.h"
#include "r_flash_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "r_can.h"
#include "r_can_api.h"
#include "r_agt.h"
#include "r_timer_api.h"
#include "r_gpt.h"
#include "r_timer_api.h"
FSP_HEADER
/* Flash on Flash HP Instance */
extern const flash_instance_t g_flash;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_flash_ctrl;
extern const flash_cfg_t g_flash_cfg;

#ifndef ISR_Flash
void ISR_Flash(flash_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_io_uart;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t g_io_uart_ctrl;
extern const uart_cfg_t g_io_uart_cfg;
extern const sci_uart_extended_cfg_t g_io_uart_cfg_extend;

#ifndef uart_io_callback
void uart_io_callback(uart_callback_args_t *p_args);
#endif
/** CAN on CAN Instance. */
extern const can_instance_t g_can0;
/** Access the CAN instance using these structures when calling API functions directly (::p_api is not used). */
extern can_instance_ctrl_t g_can0_ctrl;
extern const can_cfg_t g_can0_cfg;
extern const can_extended_cfg_t g_can0_extended_cfg;

#ifndef can_callback
void can_callback(can_callback_args_t *p_args);
#endif
#define CAN_NO_OF_MAILBOXES_g_can0 (24)
/** AGT Timer Instance */
extern const timer_instance_t g_timer4;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer4_ctrl;
extern const timer_cfg_t g_timer4_cfg;

#ifndef timer4_ISR
void timer4_ISR(timer_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_timer2;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer2_ctrl;
extern const timer_cfg_t g_timer2_cfg;

#ifndef timer2_ISR
void timer2_ISR(timer_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_timer1;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer1_ctrl;
extern const timer_cfg_t g_timer1_cfg;

#ifndef timer1_ISR
void timer1_ISR(timer_callback_args_t *p_args);
#endif
/** AGT Timer Instance */
extern const timer_instance_t g_timer0;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t g_timer0_ctrl;
extern const timer_cfg_t g_timer0_cfg;

#ifndef timer0_ISR
void timer0_ISR(timer_callback_args_t *p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer3;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer3_ctrl;
extern const timer_cfg_t g_timer3_cfg;

#ifndef timer3_ISR
void timer3_ISR(timer_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
