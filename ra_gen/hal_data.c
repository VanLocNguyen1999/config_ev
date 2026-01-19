/* generated HAL source file - do not edit */
#include "hal_data.h"

flash_hp_instance_ctrl_t g_flash_ctrl;
const flash_cfg_t g_flash_cfg = { .data_flash_bgo = false, .p_callback =
		ISR_Flash, .p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
		.irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
		.err_irq = FSP_INVALID_VECTOR,
#endif
		.err_ipl = (11), .ipl = (11), };
/* Instance structure to use this module. */
const flash_instance_t g_flash = { .p_ctrl = &g_flash_ctrl, .p_cfg =
		&g_flash_cfg, .p_api = &g_flash_on_flash_hp };
sci_uart_instance_ctrl_t g_io_uart_ctrl;

baud_setting_t g_io_uart_baud_setting = {
/* Baud rate calculated with 0.160% error. */.semr_baudrate_bits_b.abcse = 0,
		.semr_baudrate_bits_b.abcs = 0, .semr_baudrate_bits_b.bgdm = 0,
		.cks = 0, .brr = 194, .mddr = (uint8_t) 256,
		.semr_baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_uart_extended_cfg_t g_io_uart_cfg_extend = { .clock =
		SCI_UART_CLOCK_INT, .rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE,
		.noise_cancel = SCI_UART_NOISE_CANCELLATION_DISABLE, .rx_fifo_trigger =
				SCI_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting =
				&g_io_uart_baud_setting, .flow_control =
				SCI_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
		.flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
		.rs485_setting = { .enable = SCI_UART_RS485_DISABLE, .polarity =
				SCI_UART_RS485_DE_POLARITY_HIGH,
#if 0xFF != 0xFF
                    .de_control_pin = BSP_IO_PORT_FF_PIN_0xFF,
                #else
				.de_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
				}, .irda_setting = { .ircr_bits_b.ire = 0,
				.ircr_bits_b.irrxinv = 0, .ircr_bits_b.irtxinv = 0, }, };

/** UART interface configuration */
const uart_cfg_t g_io_uart_cfg = { .channel = 4, .data_bits = UART_DATA_BITS_8,
		.parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
				uart_io_callback, .p_context = NULL, .p_extend =
				&g_io_uart_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
		.p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
		.rxi_ipl = (2), .txi_ipl = (2), .tei_ipl = (2), .eri_ipl = (2),
#if defined(VECTOR_NUMBER_SCI4_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI4_RXI,
#else
		.rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI4_TXI,
#else
		.txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI4_TEI,
#else
		.tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI4_ERI,
#else
		.eri_irq = FSP_INVALID_VECTOR,
#endif
		};

/* Instance structure to use this module. */
const uart_instance_t g_io_uart = { .p_ctrl = &g_io_uart_ctrl, .p_cfg =
		&g_io_uart_cfg, .p_api = &g_uart_on_sci };
#ifndef CAN0_BAUD_SETTINGS_OVERRIDE
#define CAN0_BAUD_SETTINGS_OVERRIDE  (0)
#endif
#if CAN0_BAUD_SETTINGS_OVERRIDE
can_bit_timing_cfg_t g_can0_bit_timing_cfg =
{
    .baud_rate_prescaler = 1,
    .time_segment_1 = 5,
    .time_segment_2 = 3,
    .synchronization_jump_width = 1
};
#else
can_bit_timing_cfg_t g_can0_bit_timing_cfg =
		{
				/* Actual bitrate: 500000 Hz. Actual Bit Time Ratio: 75 %. */.baud_rate_prescaler =
						1 + 2 /* Division value of baud rate prescaler */,
				.time_segment_1 = 14, .time_segment_2 = 5,
				.synchronization_jump_width = 4, };
#endif

uint32_t g_can0_mailbox_mask[CAN_NO_OF_MAILBOXES_g_can0 / 4] = { 0x1FFFFFFF,
#if CAN_NO_OF_MAILBOXES_g_can0 > 4
0x1FFFFFFF,
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 8
0x1FFFFFFF,
0x1FFFFFFF,
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 16
0x1FFFFFFF,
0x1FFFFFFF,
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 24
0x1FFFFFFF,
0x1FFFFFFF,
#endif
		};

can_mailbox_t g_can0_mailbox[CAN_NO_OF_MAILBOXES_g_can0] = { { .mailbox_id = 0,
		.id_mode = CAN_ID_MODE_STANDARD, .mailbox_type = CAN_MAILBOX_TRANSMIT,
		.frame_type = CAN_FRAME_TYPE_REMOTE }, { .mailbox_id = 1, .id_mode =
		CAN_ID_MODE_STANDARD, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA }, { .mailbox_id = 2, .id_mode =
		CAN_ID_MODE_STANDARD, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA, }, { .mailbox_id = 3, .id_mode =
		CAN_ID_MODE_STANDARD, .mailbox_type = CAN_MAILBOX_RECEIVE, .frame_type =
		CAN_FRAME_TYPE_DATA },
#if CAN_NO_OF_MAILBOXES_g_can0 > 4
    {
        .mailbox_id              =  4,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  5,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  6,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  7,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 8
    {
        .mailbox_id              =  8,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  9,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  10,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  11,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  12,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  13,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  14,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  15,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 16
    {
        .mailbox_id              =  16,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },

    {
        .mailbox_id              =  17,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  18,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  19,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  20,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  21,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  22,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA,
    },
    {
        .mailbox_id              =  23,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
#endif
#if CAN_NO_OF_MAILBOXES_g_can0 > 24
    {
        .mailbox_id              =  24,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  25,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  26,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  27,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  28,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  29,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  30,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
    {
        .mailbox_id              =  31,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    }
#endif
		};

#if CAN_CFG_FIFO_SUPPORT
const can_fifo_interrupt_cfg_t g_can0_fifo_int_cfg =
{
    .fifo_int_mode           = (can_fifo_interrupt_mode_t) (CAN_FIFO_INTERRUPT_MODE_RX_EVERY_FRAME | CAN_FIFO_INTERRUPT_MODE_TX_EVERY_FRAME),
    .rx_fifo_irq             = VECTOR_NUMBER_CAN0_FIFO_RX,
    .tx_fifo_irq             = VECTOR_NUMBER_CAN0_FIFO_TX,
};

can_rx_fifo_cfg_t g_can0_rx_fifo_cfg =
{
    .rx_fifo_mask1 = 0,
    .rx_fifo_mask2 = 0,

    .rx_fifo_id1 =
    {
        .mailbox_id              =  0x82,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },

    .rx_fifo_id2 =
    {
        .mailbox_id              =  0,
        .id_mode                 =  CAN_ID_MODE_STANDARD,
        .mailbox_type            =  CAN_MAILBOX_RECEIVE,
        .frame_type              =  CAN_FRAME_TYPE_DATA
    },
};
#endif

const can_extended_cfg_t g_can0_extended_cfg = { .clock_source =
		CAN_CLOCK_SOURCE_PCLKB, .p_mailbox_mask = g_can0_mailbox_mask,
		.p_mailbox = g_can0_mailbox, .global_id_mode =
				CAN_GLOBAL_ID_MODE_STANDARD, .mailbox_count =
				CAN_NO_OF_MAILBOXES_g_can0, .message_mode =
				CAN_MESSAGE_MODE_OVERWRITE,
#if CAN_CFG_FIFO_SUPPORT
    .p_fifo_int_cfg         = &g_can0_fifo_int_cfg,
    .p_rx_fifo_cfg          = &g_can0_rx_fifo_cfg,
#else
		.p_fifo_int_cfg = NULL, .p_rx_fifo_cfg = NULL,
#endif
		};

can_instance_ctrl_t g_can0_ctrl;
const can_cfg_t g_can0_cfg = { .channel = 0, .p_bit_timing =
		&g_can0_bit_timing_cfg, .p_callback = can_callback, .p_extend =
		&g_can0_extended_cfg, .p_context = NULL, .ipl = (1),
#if defined(VECTOR_NUMBER_CAN0_MAILBOX_TX)
    .tx_irq             = VECTOR_NUMBER_CAN0_MAILBOX_TX,
#else
		.tx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_MAILBOX_RX)
    .rx_irq             = VECTOR_NUMBER_CAN0_MAILBOX_RX,
#else
		.rx_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_CAN0_ERROR)
    .error_irq             = VECTOR_NUMBER_CAN0_ERROR,
#else
		.error_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const can_instance_t g_can0 = { .p_ctrl = &g_can0_ctrl, .p_cfg = &g_can0_cfg,
		.p_api = &g_can_on_can };
agt_instance_ctrl_t g_timer4_ctrl;
const agt_extended_cfg_t g_timer4_extend = { .count_source = AGT_CLOCK_PCLKB,
		.agto = AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtoa =
				AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtob =
				AGT_PIN_CFG_DISABLED, .measurement_mode = AGT_MEASURE_DISABLED,
		.agtio_filter = AGT_AGTIO_FILTER_NONE, .enable_pin =
				AGT_ENABLE_PIN_NOT_USED,
		.trigger_edge = AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_timer4_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.01 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x927c, .duty_cycle_counts = 0x493e, .source_div =
		(timer_source_div_t) 3, .channel = 4, .p_callback = timer4_ISR,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer4_extend, .cycle_end_ipl = (7),
#if defined(VECTOR_NUMBER_AGT4_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT4_INT,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer4 = { .p_ctrl = &g_timer4_ctrl, .p_cfg =
		&g_timer4_cfg, .p_api = &g_timer_on_agt };
agt_instance_ctrl_t g_timer2_ctrl;
const agt_extended_cfg_t g_timer2_extend = { .count_source = AGT_CLOCK_PCLKB,
		.agto = AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtoa =
				AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtob =
				AGT_PIN_CFG_DISABLED, .measurement_mode = AGT_MEASURE_DISABLED,
		.agtio_filter = AGT_AGTIO_FILTER_NONE, .enable_pin =
				AGT_ENABLE_PIN_NOT_USED,
		.trigger_edge = AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_timer2_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.001 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x7530, .duty_cycle_counts = 0x3a98, .source_div =
		(timer_source_div_t) 0, .channel = 2, .p_callback = timer2_ISR,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer2_extend, .cycle_end_ipl = (2),
#if defined(VECTOR_NUMBER_AGT2_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT2_INT,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer2 = { .p_ctrl = &g_timer2_ctrl, .p_cfg =
		&g_timer2_cfg, .p_api = &g_timer_on_agt };
agt_instance_ctrl_t g_timer1_ctrl;
const agt_extended_cfg_t g_timer1_extend = { .count_source = AGT_CLOCK_PCLKB,
		.agto = AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtoa =
				AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtob =
				AGT_PIN_CFG_DISABLED, .measurement_mode = AGT_MEASURE_DISABLED,
		.agtio_filter = AGT_AGTIO_FILTER_NONE, .enable_pin =
				AGT_ENABLE_PIN_NOT_USED,
		.trigger_edge = AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_timer1_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.001 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x7530, .duty_cycle_counts = 0x3a98, .source_div =
		(timer_source_div_t) 0, .channel = 1, .p_callback = timer1_ISR,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer1_extend, .cycle_end_ipl = (1),
#if defined(VECTOR_NUMBER_AGT1_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT1_INT,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer1 = { .p_ctrl = &g_timer1_ctrl, .p_cfg =
		&g_timer1_cfg, .p_api = &g_timer_on_agt };
agt_instance_ctrl_t g_timer0_ctrl;
const agt_extended_cfg_t g_timer0_extend = { .count_source = AGT_CLOCK_PCLKB,
		.agto = AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtoa =
				AGT_PIN_CFG_DISABLED, .agtoab_settings_b.agtob =
				AGT_PIN_CFG_DISABLED, .measurement_mode = AGT_MEASURE_DISABLED,
		.agtio_filter = AGT_AGTIO_FILTER_NONE, .enable_pin =
				AGT_ENABLE_PIN_NOT_USED,
		.trigger_edge = AGT_TRIGGER_EDGE_RISING, .counter_bit_width =
				AGT_COUNTER_BIT_WIDTH_16, };
const timer_cfg_t g_timer0_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.01 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0x927c, .duty_cycle_counts = 0x493e, .source_div =
		(timer_source_div_t) 3, .channel = 1, .p_callback = timer0_ISR,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer0_extend, .cycle_end_ipl = (4),
#if defined(VECTOR_NUMBER_AGT1_INT)
    .cycle_end_irq       = VECTOR_NUMBER_AGT1_INT,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer0 = { .p_ctrl = &g_timer0_ctrl, .p_cfg =
		&g_timer0_cfg, .p_api = &g_timer_on_agt };
gpt_instance_ctrl_t g_timer3_ctrl;
#if 0
const gpt_extended_pwm_cfg_t g_timer3_pwm_extend =
{
    .trough_ipl          = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT0_COUNTER_UNDERFLOW)
    .trough_irq          = VECTOR_NUMBER_GPT0_COUNTER_UNDERFLOW,
#else
    .trough_irq          = FSP_INVALID_VECTOR,
#endif
    .poeg_link           = GPT_POEG_LINK_POEG0,
    .output_disable      = (gpt_output_disable_t) ( GPT_OUTPUT_DISABLE_NONE),
    .adc_trigger         = (gpt_adc_trigger_t) ( GPT_ADC_TRIGGER_NONE),
    .dead_time_count_up  = 0,
    .dead_time_count_down = 0,
    .adc_a_compare_match = 0,
    .adc_b_compare_match = 0,
    .interrupt_skip_source = GPT_INTERRUPT_SKIP_SOURCE_NONE,
    .interrupt_skip_count  = GPT_INTERRUPT_SKIP_COUNT_0,
    .interrupt_skip_adc    = GPT_INTERRUPT_SKIP_ADC_NONE,
    .gtioca_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
    .gtiocb_disable_setting = GPT_GTIOC_DISABLE_PROHIBITED,
};
#endif
const gpt_extended_cfg_t g_timer3_extend =
		{
				.gtioca = { .output_enabled = false, .stop_level =
						GPT_PIN_LEVEL_LOW }, .gtiocb =
						{ .output_enabled = false, .stop_level =
								GPT_PIN_LEVEL_LOW }, .start_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .stop_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .clear_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .count_up_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .count_down_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .capture_a_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .capture_b_source =
						(gpt_source_t)(GPT_SOURCE_NONE), .capture_a_ipl =
						(BSP_IRQ_DISABLED), .capture_b_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A)
    .capture_a_irq       = VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A,
#else
				.capture_a_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_B)
    .capture_b_irq       = VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_B,
#else
				.capture_b_irq = FSP_INVALID_VECTOR,
#endif
				.compare_match_value = { /* CMP_A */0x0, /* CMP_B */0x0 },
				.compare_match_status = (0U << 1U) | 0U,
				.capture_filter_gtioca = GPT_CAPTURE_FILTER_NONE,
				.capture_filter_gtiocb = GPT_CAPTURE_FILTER_NONE,
#if 0
    .p_pwm_cfg                   = &g_timer3_pwm_extend,
#else
				.p_pwm_cfg = NULL,
#endif
#if 0
    .gtior_setting.gtior_b.gtioa  = (0U << 4U) | (0U << 2U) | (0U << 0U),
    .gtior_setting.gtior_b.oadflt = (uint32_t) GPT_PIN_LEVEL_LOW,
    .gtior_setting.gtior_b.oahld  = 0U,
    .gtior_setting.gtior_b.oae    = (uint32_t) false,
    .gtior_setting.gtior_b.oadf   = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED,
    .gtior_setting.gtior_b.nfaen  = ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
    .gtior_setting.gtior_b.nfcsa  = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U),
    .gtior_setting.gtior_b.gtiob  = (0U << 4U) | (0U << 2U) | (0U << 0U),
    .gtior_setting.gtior_b.obdflt = (uint32_t) GPT_PIN_LEVEL_LOW,
    .gtior_setting.gtior_b.obhld  = 0U,
    .gtior_setting.gtior_b.obe    = (uint32_t) false,
    .gtior_setting.gtior_b.obdf   = (uint32_t) GPT_GTIOC_DISABLE_PROHIBITED,
    .gtior_setting.gtior_b.nfben  = ((uint32_t) GPT_CAPTURE_FILTER_NONE & 1U),
    .gtior_setting.gtior_b.nfcsb  = ((uint32_t) GPT_CAPTURE_FILTER_NONE >> 1U),
#else
				.gtior_setting.gtior = 0U,
#endif
		};

const timer_cfg_t g_timer3_cfg = { .mode = TIMER_MODE_PERIODIC,
/* Actual period: 0.001 seconds. Actual duty: 50%. */.period_counts =
		(uint32_t) 0xea60, .duty_cycle_counts = 0x7530, .source_div =
		(timer_source_div_t) 0, .channel = 0, .p_callback = timer3_ISR,
/** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
		.p_context = &NULL,
#endif
		.p_extend = &g_timer3_extend, .cycle_end_ipl = (3),
#if defined(VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW)
    .cycle_end_irq       = VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW,
#else
		.cycle_end_irq = FSP_INVALID_VECTOR,
#endif
		};
/* Instance structure to use this module. */
const timer_instance_t g_timer3 = { .p_ctrl = &g_timer3_ctrl, .p_cfg =
		&g_timer3_cfg, .p_api = &g_timer_on_gpt };
void g_hal_init(void) {
	g_common_init();
}
