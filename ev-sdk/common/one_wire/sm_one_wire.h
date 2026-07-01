/*
 * sm_one_write.h
 *
 *  Created on: 1 Jul 2026
 *      Author: My PC
 */

#ifndef SM_ONE_WIRE_H_
#define SM_ONE_WIRE_H_
#include "one_wire_define.h"
#include "sm_hal.h"
#include "sm_types.h"

typedef void sm_one_wire_t;

sm_one_wire_t* sm_one_wire_create(sm_hal_io_t *port, OneWire_Para_t *_para,
		OneWireRx_callback_t cb, void *arg);

void OneWireRx_OnFallingEdge(sm_one_wire_t *_this);
void OneWireRx_OnRisingEdge(sm_one_wire_t *_this);
void OneWireRx_Reset(sm_one_wire_t *_this);
OneWireRx_State_t OneWireRx_GetState(const sm_one_wire_t *_this);
uint32_t OneWireRx_GetFrameCount(const sm_one_wire_t *_this);
uint32_t OneWireRx_GetErrorCount(const sm_one_wire_t *_this);

OWStatus OneWireTx_send(sm_one_wire_t *_this, const uint8_t *data, uint8_t len);
OWStatus OneWireTx_send_impl(sm_one_wire_t *_this, uint8_t _id, uint8_t _ver,
		const uint8_t *data, uint8_t len);
#endif /* SM_ONE_WIRE_H_ */
