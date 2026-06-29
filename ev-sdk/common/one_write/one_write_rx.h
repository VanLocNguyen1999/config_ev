/*
 * one_write_rx.h
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */

#ifndef ONE_WRITE_RX_H_
#define ONE_WRITE_RX_H_
#include "sm_types.h"
#include "sm_hal.h"
/* ===================== Cấu hình ===================== */
#define OW_MAX_BITS        64
#define OW_MAX_BYTES       (OW_MAX_BITS / 8)

#define OW_SYNC_MIN        55
#define OW_SYNC_MAX        68
#define OW_BIT0_MIN        3
#define OW_BIT0_MAX        5
#define OW_BIT1_MIN        1
#define OW_BIT1_MAX        3
#define OW_STOP_MIN        20

/* ===================== Trạng thái ===================== */
typedef enum {
	OneWireRx_State_Idle = 0,
	OneWireRx_State_Synced,
	OneWireRx_State_Receiving,
	OneWireRx_State_Complete,
	OneWireRx_State_Error
} OneWireRx_State_t;

/* ===================== Frame dữ liệu ===================== */
typedef struct {
    uint8_t  data[OW_MAX_BITS];
    uint8_t  bit_count;
    uint8_t  byte_count;
    bool     is_valid;
} OneWireRx_Frame_t;

/* ===================== Object chính ===================== */
typedef struct {
    /* Private */
	OneWireRx_State_t state;
	OneWireRx_Frame_t frame;
    uint32_t    falling_time;
    uint32_t    frame_count;
    uint32_t    error_count;

    /* Callback khi nhận xong frame */
    void (*on_frame_complete)(const OneWireRx_Frame_t *frame);
    void (*on_error)(OneWireRx_State_t error_state);
} OneWireRx_t;

/* ===================== API ===================== */
void OneWireRx_Init     (OneWireRx_t *hbms,
                        void (*on_complete)(const OneWireRx_Frame_t *),
                        void (*on_error)(OneWireRx_State_t));
void OneWireRx_OnFallingEdge (OneWireRx_t *hbms, uint32_t timestamp_ms);
void OneWireRx_OnRisingEdge  (OneWireRx_t *hbms, uint32_t timestamp_ms);
void OneWireRx_Reset         (OneWireRx_t *hbms);
OneWireRx_State_t OneWireRx_GetState  (const OneWireRx_t *hbms);
uint32_t    OneWireRx_GetFrameCount(const OneWireRx_t *hbms);
uint32_t   OneWireRx_GetErrorCount(const OneWireRx_t *hbms);
#endif /* ONE_WRITE_RX_H_ */
