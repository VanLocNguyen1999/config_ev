/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (15)
#endif
/* ISR prototypes */
void gpt_counter_overflow_isr(void);
void agt_int_isr(void);
void can_error_isr(void);
void can_rx_isr(void);
void can_tx_isr(void);
void sci_uart_rxi_isr(void);
void sci_uart_txi_isr(void);
void sci_uart_tei_isr(void);
void sci_uart_eri_isr(void);
void fcu_frdyi_isr(void);
void fcu_fiferr_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 0) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 0) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define VECTOR_NUMBER_AGT1_INT ((IRQn_Type) 1) /* AGT1 INT (AGT interrupt) */
#define AGT1_INT_IRQn          ((IRQn_Type) 1) /* AGT1 INT (AGT interrupt) */
#define VECTOR_NUMBER_AGT2_INT ((IRQn_Type) 2) /* AGT2 INT (AGT interrupt) */
#define AGT2_INT_IRQn          ((IRQn_Type) 2) /* AGT2 INT (AGT interrupt) */
#define VECTOR_NUMBER_AGT4_INT ((IRQn_Type) 3) /* AGT4 INT (AGT interrupt) */
#define AGT4_INT_IRQn          ((IRQn_Type) 3) /* AGT4 INT (AGT interrupt) */
#define VECTOR_NUMBER_CAN0_ERROR ((IRQn_Type) 4) /* CAN0 ERROR (Error interrupt) */
#define CAN0_ERROR_IRQn          ((IRQn_Type) 4) /* CAN0 ERROR (Error interrupt) */
#define VECTOR_NUMBER_CAN0_MAILBOX_RX ((IRQn_Type) 5) /* CAN0 MAILBOX RX (Reception complete interrupt) */
#define CAN0_MAILBOX_RX_IRQn          ((IRQn_Type) 5) /* CAN0 MAILBOX RX (Reception complete interrupt) */
#define VECTOR_NUMBER_CAN0_MAILBOX_TX ((IRQn_Type) 6) /* CAN0 MAILBOX TX (Transmission complete interrupt) */
#define CAN0_MAILBOX_TX_IRQn          ((IRQn_Type) 6) /* CAN0 MAILBOX TX (Transmission complete interrupt) */
#define VECTOR_NUMBER_CAN0_FIFO_RX ((IRQn_Type) 7) /* CAN0 FIFO RX (Receive FIFO interrupt) */
#define CAN0_FIFO_RX_IRQn          ((IRQn_Type) 7) /* CAN0 FIFO RX (Receive FIFO interrupt) */
#define VECTOR_NUMBER_CAN0_FIFO_TX ((IRQn_Type) 8) /* CAN0 FIFO TX (Transmit FIFO interrupt) */
#define CAN0_FIFO_TX_IRQn          ((IRQn_Type) 8) /* CAN0 FIFO TX (Transmit FIFO interrupt) */
#define VECTOR_NUMBER_SCI4_RXI ((IRQn_Type) 9) /* SCI4 RXI (Receive data full) */
#define SCI4_RXI_IRQn          ((IRQn_Type) 9) /* SCI4 RXI (Receive data full) */
#define VECTOR_NUMBER_SCI4_TXI ((IRQn_Type) 10) /* SCI4 TXI (Transmit data empty) */
#define SCI4_TXI_IRQn          ((IRQn_Type) 10) /* SCI4 TXI (Transmit data empty) */
#define VECTOR_NUMBER_SCI4_TEI ((IRQn_Type) 11) /* SCI4 TEI (Transmit end) */
#define SCI4_TEI_IRQn          ((IRQn_Type) 11) /* SCI4 TEI (Transmit end) */
#define VECTOR_NUMBER_SCI4_ERI ((IRQn_Type) 12) /* SCI4 ERI (Receive error) */
#define SCI4_ERI_IRQn          ((IRQn_Type) 12) /* SCI4 ERI (Receive error) */
#define VECTOR_NUMBER_FCU_FRDYI ((IRQn_Type) 13) /* FCU FRDYI (Flash ready interrupt) */
#define FCU_FRDYI_IRQn          ((IRQn_Type) 13) /* FCU FRDYI (Flash ready interrupt) */
#define VECTOR_NUMBER_FCU_FIFERR ((IRQn_Type) 14) /* FCU FIFERR (Flash access error interrupt) */
#define FCU_FIFERR_IRQn          ((IRQn_Type) 14) /* FCU FIFERR (Flash access error interrupt) */
#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
