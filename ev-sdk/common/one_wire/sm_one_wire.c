/*
 * sm_one_write.c
 *
 *  Created on: 1 Jul 2026
 *      Author: My PC
 */
#include "sm_one_wire.h"
#include "sm_counter_utils.h"

#define _impl(x)			((OneWire_t*)(x))
#define DELAY_US(x)			R_BSP_SoftwareDelay(x, BSP_DELAY_UNITS_MICROSECONDS)

typedef struct OneWireRx {
	/* Private */
	OneWireRx_State_t state;
	OneWireRx_Frame_t frame;

	uint32_t falling_time;
	uint32_t frame_count;
	uint32_t error_count;
} OneWireRx_t;

typedef struct OneWireTx {
	/* private */
	sm_hal_io_t *m_port;
	/* stats */
	uint32_t tx_count;
	uint32_t tx_error;
} OneWireTx_t;

typedef struct {
	/* Private */
	OneWireRx_t m_rx;
	OneWireTx_t m_tx;
	OneWire_Para_t m_para;
	/* Callback khi nhận xong frame */
	OneWireRx_callback_t m_cb;
	void *m_arg;
} OneWire_t;

static OneWire_t g_one_write;
static void tx_set_high(OneWireTx_t *_this);
static void tx_set_low(OneWireTx_t *_this);

static OWStatus OneWireTx_Init(OneWireTx_t *_this, sm_hal_io_t *port) {
	if (!_this || !port)
		return OW_ERR_NULL;

	memset(_this, 0, sizeof(OneWireTx_t));
	_this->m_port = port;
	tx_set_high(_this);
	return OW_OK;
}

static void OneWireRx_Init(OneWireRx_t *_this){

    memset(_this, 0, sizeof(OneWireRx_t));
    _this->state             = OneWireRx_State_Idle;
}

sm_one_wire_t* sm_one_wire_create(sm_hal_io_t *port, OneWire_Para_t *_para,
		OneWireRx_callback_t cb, void *arg) {
	if(port){

		OneWireTx_Init(&g_one_write.m_tx, port);
	}
	OneWireRx_Init(&g_one_write.m_rx);
	g_one_write.m_cb = cb;
	memcpy(&g_one_write.m_para,_para, sizeof(OneWire_Para_t));
	g_one_write.m_arg = arg;
	return &g_one_write;
}
/* ===================== Private tx functions ===================== */
static void tx_set_low(OneWireTx_t *_this) {
	sm_hal_io_set_value(_this->m_port, 1);
}
static void tx_set_high(OneWireTx_t *_this) {
	sm_hal_io_set_value(_this->m_port, 0);
}

static void tx_sync(OneWire_t *_this) {
	tx_set_low(&_this->m_tx);
	DELAY_US(_this->m_para.m_time_syns_low);
	tx_set_high(&_this->m_tx);
	DELAY_US(_this->m_para.m_time_syns_high);
}

static void tx_set_bit(OneWire_t *_this, uint8_t b) {

	if (b) {
		tx_set_low(&_this->m_tx);
		DELAY_US(_this->m_para.m_time_bit1_low);
		tx_set_high(&_this->m_tx);
		DELAY_US(_this->m_para.m_time_bit1_high);
	} else {
		tx_set_low(&_this->m_tx);
	DELAY_US(_this->m_para.m_time_bit0_low);
	tx_set_high(&_this->m_tx);
	DELAY_US(_this->m_para.m_time_bit0_high);
	}
}

static void tx_set_byte(OneWire_t *_this, uint8_t v) {
	for (uint8_t i = 0; i < 8; i++)
		tx_set_bit(_this, (v >> i) & 1); /* LSB first */
}

static void tx_stop(OneWire_t *_this){

	tx_set_low(&_this->m_tx);
    DELAY_US(_this->m_para.m_time_stop_low);
    tx_set_high(&_this->m_tx);
    DELAY_US(_this->m_para.m_time_stop_high);
}
/* ===================== Private rx functions ===================== */
static void rx_push_bit(OneWireRx_Frame_t *frame, uint8_t bit){

    if (frame->bit_count >= OW_MAX_BITS)
    	return;
    uint8_t byte_idx = frame->bit_count / 8;
    uint8_t bit_idx  = frame->bit_count % 8; // LSB first

    if (bit)
        frame->data[byte_idx] |= (uint8_t)(1 << bit_idx);
    else
        frame->data[byte_idx] &= (uint8_t)~(1 << bit_idx);

    frame->bit_count++;
    frame->byte_count = (uint8_t)((frame->bit_count + 7) / 8);
}
static void rx_reset_frame(OneWireRx_Frame_t *frame){

    memset(frame->data, 0, sizeof(frame->data));
    frame->bit_count  = 0;
    frame->byte_count = 0;
    frame->is_valid   = false;
}

static void rx_decode_T1(OneWire_t *_this, uint32_t t1_us) {

	OneWireRx_t *p_rx = &_this->m_rx;
	if(!p_rx || !_this)
		return;
	if (t1_us >= _this->m_para.m_time_syns_min
			&& t1_us <= _this->m_para.m_time_syns_max) {

		rx_reset_frame(&p_rx->frame);
		p_rx->state = OneWireRx_State_Synced;
	} else if (p_rx->state == OneWireRx_State_Idle) {
		return;
	} else if (t1_us >= _this->m_para.m_time_bit0_min
			&& t1_us <= _this->m_para.m_time_bit0_max){

		rx_push_bit(&p_rx->frame, 0);
		p_rx->state = OneWireRx_State_Receiving;
	} else if (t1_us >= _this->m_para.m_time_bit1_min
			&& t1_us < _this->m_para.m_time_bit1_max){

		rx_push_bit(&p_rx->frame, 1);
		p_rx->state = OneWireRx_State_Receiving;
	} else if (t1_us >= _this->m_para.m_time_stop_min){

		p_rx->frame.is_valid = true;
		p_rx->state = OneWireRx_State_Complete;
		p_rx->frame_count++;

		if (_this->m_cb.on_frame_complete)
			_this->m_cb.on_frame_complete(&p_rx->frame,_this->m_arg);

		p_rx->state = OneWireRx_State_Idle; /* Sẵn sàng nhận frame tiếp */
	} else /* Không hợp lệ */{

		p_rx->error_count++;
		p_rx->state = OneWireRx_State_Error;
		if (_this->m_cb.on_error)
			_this->m_cb.on_error(OneWireRx_State_Error,_this->m_arg);

		OneWireRx_Reset(&p_rx->frame);
	}
}
/* ===================== Public functions ===================== */
void OneWireRx_OnFallingEdge(sm_one_wire_t *_this){
    _impl(_this)->m_rx.falling_time = sm_counter_get(); /* Ghi nhận thời điểm bắt đầu T1 */
}

void OneWireRx_OnRisingEdge(sm_one_wire_t *_this){
    uint32_t t1_us = sm_counter_elapsed_us(_impl(_this)->m_rx.falling_time);
    rx_decode_T1(_this, t1_us);
}

void OneWireRx_Reset(sm_one_wire_t *_this){
	rx_reset_frame(&_impl(_this)->m_rx.frame);
    _impl(_this)->m_rx.state        = OneWireRx_State_Idle;
    _impl(_this)->m_rx.falling_time = 0;
}

OneWireRx_State_t OneWireRx_GetState(const sm_one_wire_t *_this){
    return _impl(_this)->m_rx.state;
}

uint32_t OneWireRx_GetFrameCount(const sm_one_wire_t *_this){
    return _impl(_this)->m_rx.frame_count;
}

uint32_t OneWireRx_GetErrorCount(const sm_one_wire_t *_this){
    return _impl(_this)->m_rx.error_count;
}

OWStatus OneWireTx_send(sm_one_wire_t *_this, const uint8_t *data, uint8_t len) {

	if (!_this)
		return OW_ERR_NOT_INIT;
	if (!data || len == 0 || len > OW_MAX_BYTES)
		return OW_ERR_LEN;
	/* Build frame */
	uint8_t buf[OW_MAX_BYTES + 4];
	uint8_t index = 0;

	buf[index++] = _impl(_this)->m_para.m_id;
	buf[index++] = _impl(_this)->m_para.m_version;
	buf[index++] = len;
	for (uint8_t i = 0; i < len; i++) {
		buf[index++] = data[i];
	}

	uint8_t checksum = _check_sum(buf, index);
	buf[index++] = checksum;
	/* Transmit */
	tx_sync(_this);
	for (uint8_t i = 0; i < index; i++) {
		tx_set_byte(_this, buf[i]);
	}

	tx_stop(_this);
	_impl(_this)->m_tx.tx_count ++;
	return OW_OK;
}

OWStatus OneWireTx_send_impl(sm_one_wire_t *_this, uint8_t _id, uint8_t _ver,
		const uint8_t *data, uint8_t len) {

	if (!_this)
		return OW_ERR_NOT_INIT;
	if (!data || len == 0 || len > OW_MAX_BYTES)
		return OW_ERR_LEN;
	/* Build frame */
	uint8_t buf[OW_MAX_BYTES + 4];
	uint8_t index = 0;

	buf[index++] = _id;
	buf[index++] = _ver;
	buf[index++] = len;
	for (uint8_t i = 0; i < len; i++) {
		buf[index++] = data[i];
	}

	uint8_t checksum = _check_sum(buf, index);
	buf[index++] = checksum;
	/* Transmit */
	tx_sync(_this);
	for (uint8_t i = 0; i < index; i++) {
		tx_set_byte(_this, buf[i]);
	}

	tx_stop(_this);
	_impl(_this)->m_tx.tx_count ++;
	return OW_OK;
}

