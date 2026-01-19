/*
 * error_handle.c
 *
 *  Created on: 14 Nov 2025
 *      Author: My PC
 */
#include "ev_error_handle.h"
#include "sm_fifo.h"

#define _impl(x)      ((ev_error_impl_t*)(x))

#define MAX_ERR_BUFFER				5
#define MAX_ERR_SIZE				4
#define MIN_SEND_COUNTER			5

typedef struct ev_error_impl {

	sm_fifo_handle_t m_fifo;

	uint32_t 	m_error;
	uint8_t 	m_send_count;
    uint8_t 	m_min_send_count;

    bool 		m_is_send;
}ev_error_impl_t;

static ev_error_impl_t g_ev_error = {
		.m_min_send_count = MIN_SEND_COUNTER,
		.m_error = 0,
		.m_send_count = 0,
		.m_is_send = false
};

ev_error_t* ev_err_create(void){

	ev_error_impl_t* _this = &g_ev_error;
	if(!_this) return NULL;

	sm_fifo_init(&_impl(_this)->m_fifo, MAX_ERR_BUFFER, MAX_ERR_SIZE);

	return _this;
}

void ev_err_set(ev_error_t* _this, uint32_t code){

	if(!_this || code == 0) return;

	sm_fifo_push(&_impl(_this)->m_fifo, &code);
}
uint32_t ev_err_get(const ev_error_t* _this){

	if ((_impl(_this)->m_is_send == true)
			&& (_impl(_this)->m_send_count <= _impl(_this)->m_min_send_count - 1)) {

		_impl(_this)->m_send_count++;
		return _impl(_this)->m_error;
	}

	_impl(_this)->m_send_count = 0;
	_impl(_this)->m_is_send = false;
	_impl(_this)->m_error = 0;
	sm_fifo_pop(&_impl(_this)->m_fifo, &_impl(_this)->m_error);

	if (_impl(_this)->m_error == 0) {

		return 0;
	} else {

		_impl(_this)->m_is_send = true;
		_impl(_this)->m_send_count++;
		return _impl(_this)->m_error;
	}

}
