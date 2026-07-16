/*
 * one_write_define.h
 *
 *  Created on: 30 Jun 2026
 *      Author: My PC
 */

#ifndef ONE_WIRE_DEFINE_H_
#define ONE_WIRE_DEFINE_H_
#include "sm_types.h"
#define OW_MAX_BITS        256
#define OW_MAX_BYTES       (OW_MAX_BITS / 8)
typedef struct {

	uint8_t m_id;
	uint8_t m_version;
	/*one_write_tx para*/
	uint32_t m_time_bit0_low;
	uint32_t m_time_bit0_high;

	uint32_t m_time_bit1_low;
	uint32_t m_time_bit1_high;

	uint32_t m_time_syns_low;
	uint32_t m_time_syns_high;

	uint32_t m_time_stop_low;
	uint32_t m_time_stop_high;
	/*one_write_rx para*/
	uint32_t m_time_bit0_min;
	uint32_t m_time_bit0_max;

	uint32_t m_time_bit1_min;
	uint32_t m_time_bit1_max;

	uint32_t m_time_syns_min;
	uint32_t m_time_syns_max;

	uint32_t m_time_stop_min;
	uint32_t m_time_stop_max;
}OneWire_Para_t;

typedef enum {
	OneWireRx_State_Idle = 0,
	OneWireRx_State_Synced,
	OneWireRx_State_Receiving,
	OneWireRx_State_Complete,
	OneWireRx_State_Error
} OneWireRx_State_t;

typedef struct {
    uint8_t  data[OW_MAX_BITS];
    uint8_t  bit_count;
    uint8_t  byte_count;
    bool     is_valid;
} OneWireRx_Frame_t;

typedef enum {
    OW_OK = 0,
    OW_ERR_NULL,
    OW_ERR_NOT_INIT,
    OW_ERR_LEN,
} OWStatus;
typedef struct {

    void (*on_frame_complete)(const OneWireRx_Frame_t *frame, void* arg);
    void (*on_error)(OneWireRx_State_t error_state, void* arg);
}OneWireRx_callback_t;
static inline uint8_t _check_sum(const uint8_t *buf, uint8_t len) {

	uint16_t s = 0;
	for (uint8_t i = 0; i < len; i++) {
		s += buf[i];
	}
	return (uint8_t) (s & 0xFF);
}
#endif /* ONE_WIRE_DEFINE_H_ */
