/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define LED1 (BSP_IO_PORT_00_PIN_00)
#define OUT_12V_ENABLE (BSP_IO_PORT_01_PIN_00)
#define IN_12V_DETECTOR (BSP_IO_PORT_01_PIN_01)
#define NodeID1 (BSP_IO_PORT_01_PIN_04)
#define one_write_tx (BSP_IO_PORT_01_PIN_09)
#define one_write_rx (BSP_IO_PORT_01_PIN_10)
#define NodeID3 (BSP_IO_PORT_01_PIN_11)
#define NodeID2 (BSP_IO_PORT_01_PIN_12)
#define CAN_MODE (BSP_IO_PORT_02_PIN_01)
#define LED3 (BSP_IO_PORT_04_PIN_02)
extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA4M2AB3CFL.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
