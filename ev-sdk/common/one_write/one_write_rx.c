/*
 * one_write_rx.c
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */
#include "one_write_rx.h"

/* ===================== Private functions ===================== */
static void _push_bit(OneWireRx_Frame_t *frame, uint8_t bit)
{
    if (frame->bit_count >= OW_MAX_BITS) return;

    uint8_t byte_idx = frame->bit_count / 8;
    uint8_t bit_idx  = 7 - (frame->bit_count % 8); // MSB first

    if (bit)
        frame->data[byte_idx] |=  (1 << bit_idx);
    else
        frame->data[byte_idx] &= ~(1 << bit_idx);

    frame->bit_count++;
    frame->byte_count = (frame->bit_count + 7) / 8;
}

static void _reset_frame(OneWireRx_Frame_t *frame)
{
    memset(frame->data, 0, sizeof(frame->data));
    frame->bit_count  = 0;
    frame->byte_count = 0;
    frame->is_valid   = false;
}

static void _decode_T1(OneWireRx_t *hbms, uint32_t t1_ms)
{
    if (t1_ms >= OW_SYNC_MIN && t1_ms <= OW_SYNC_MAX) /* Sync ~62ms */
    {
        _reset_frame(&hbms->frame);
        hbms->state = OneWireRx_State_Synced;
    }
    else if (hbms->state == OneWireRx_State_Idle)              /* Chưa sync → bỏ qua */
    {
        return;
    }
    else if (t1_ms >= OW_BIT0_MIN && t1_ms <= OW_BIT0_MAX) /* DATA(0) ~4ms */
    {
        _push_bit(&hbms->frame, 0);
        hbms->state = OneWireRx_State_Receiving;
    }
    else if (t1_ms >= OW_BIT1_MIN && t1_ms < OW_BIT1_MAX)  /* DATA(1) ~2ms */
    {
        _push_bit(&hbms->frame, 1);
        hbms->state = OneWireRx_State_Receiving;
    }
    else if (t1_ms >= OW_STOP_MIN)                           /* Stop ≥20ms */
    {
        hbms->frame.is_valid = true;
        hbms->state          = OneWireRx_State_Complete;
        hbms->frame_count++;

        if (hbms->on_frame_complete)
            hbms->on_frame_complete(&hbms->frame);

        hbms->state = OneWireRx_State_Idle; /* Sẵn sàng nhận frame tiếp */
    }
    else                                                       /* Không hợp lệ */
    {
        hbms->error_count++;
        hbms->state = OneWireRx_State_Error;

        if (hbms->on_error)
            hbms->on_error(OneWireRx_State_Error);

        OneWireRx_Reset(hbms);
    }
}

/* ===================== Public functions ===================== */
void OneWireRx_Init(OneWireRx_t *hbms,
              void (*on_complete)(const OneWireRx_Frame_t *),
              void (*on_error)(OneWireRx_State_t))
{
    memset(hbms, 0, sizeof(OneWireRx_t));
    hbms->state             = OneWireRx_State_Idle;
    hbms->on_frame_complete = on_complete;
    hbms->on_error          = on_error;
}

void OneWireRx_OnFallingEdge(OneWireRx_t *hbms, uint32_t timestamp_ms)
{
    hbms->falling_time = timestamp_ms; /* Ghi nhận thời điểm bắt đầu T1 */
}

void OneWireRx_OnRisingEdge(OneWireRx_t *hbms, uint32_t timestamp_ms)
{
    uint32_t t1_ms = timestamp_ms - hbms->falling_time;
    _decode_T1(hbms, t1_ms);
}

void OneWireRx_Reset(OneWireRx_t *hbms)
{
    _reset_frame(&hbms->frame);
    hbms->state        = OneWireRx_State_Idle;
    hbms->falling_time = 0;
}

OneWireRx_State_t OneWireRx_GetState(const OneWireRx_t *hbms)
{
    return hbms->state;
}

uint32_t OneWireRx_GetFrameCount(const OneWireRx_t *hbms)
{
    return hbms->frame_count;
}

uint32_t OneWireRx_GetErrorCount(const OneWireRx_t *hbms)
{
    return hbms->error_count;
}
