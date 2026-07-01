/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
            [1] = agt_int_isr, /* AGT1 INT (AGT interrupt) */
            [2] = agt_int_isr, /* AGT2 INT (AGT interrupt) */
            [3] = agt_int_isr, /* AGT4 INT (AGT interrupt) */
            [4] = can_error_isr, /* CAN0 ERROR (Error interrupt) */
            [5] = can_rx_isr, /* CAN0 MAILBOX RX (Reception complete interrupt) */
            [6] = can_tx_isr, /* CAN0 MAILBOX TX (Transmission complete interrupt) */
            [7] = can_rx_isr, /* CAN0 FIFO RX (Receive FIFO interrupt) */
            [8] = can_tx_isr, /* CAN0 FIFO TX (Transmit FIFO interrupt) */
            [9] = sci_uart_rxi_isr, /* SCI4 RXI (Receive data full) */
            [10] = sci_uart_txi_isr, /* SCI4 TXI (Transmit data empty) */
            [11] = sci_uart_tei_isr, /* SCI4 TEI (Transmit end) */
            [12] = sci_uart_eri_isr, /* SCI4 ERI (Receive error) */
            [13] = fcu_frdyi_isr, /* FCU FRDYI (Flash ready interrupt) */
            [14] = fcu_fiferr_isr, /* FCU FIFERR (Flash access error interrupt) */
            [15] = r_icu_isr, /* ICU IRQ13 (External pin interrupt 13) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_GPT0_COUNTER_OVERFLOW,GROUP0), /* GPT0 COUNTER OVERFLOW (Overflow) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_AGT1_INT,GROUP1), /* AGT1 INT (AGT interrupt) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_AGT2_INT,GROUP2), /* AGT2 INT (AGT interrupt) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_AGT4_INT,GROUP3), /* AGT4 INT (AGT interrupt) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_CAN0_ERROR,GROUP4), /* CAN0 ERROR (Error interrupt) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_CAN0_MAILBOX_RX,GROUP5), /* CAN0 MAILBOX RX (Reception complete interrupt) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_CAN0_MAILBOX_TX,GROUP6), /* CAN0 MAILBOX TX (Transmission complete interrupt) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_CAN0_FIFO_RX,GROUP7), /* CAN0 FIFO RX (Receive FIFO interrupt) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_CAN0_FIFO_TX,GROUP0), /* CAN0 FIFO TX (Transmit FIFO interrupt) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI4_RXI,GROUP1), /* SCI4 RXI (Receive data full) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI4_TXI,GROUP2), /* SCI4 TXI (Transmit data empty) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI4_TEI,GROUP3), /* SCI4 TEI (Transmit end) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_SCI4_ERI,GROUP4), /* SCI4 ERI (Receive error) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_FCU_FRDYI,GROUP5), /* FCU FRDYI (Flash ready interrupt) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_FCU_FIFERR,GROUP6), /* FCU FIFERR (Flash access error interrupt) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ13,GROUP7), /* ICU IRQ13 (External pin interrupt 13) */
        };
        #endif
        #endif
