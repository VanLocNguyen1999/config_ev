/*
 * error_handle.h
 *
 *  Created on: 14 Nov 2025
 *      Author: My PC
 */

#ifndef EV_ERROR_HANDLE_H_
#define EV_ERROR_HANDLE_H_
#include "sm_types.h"

typedef void ev_error_t;

ev_error_t* ev_err_create(void);

void ev_err_set(ev_error_t* _this, uint32_t code);
uint32_t ev_err_get(const ev_error_t* _this);

#endif /* EV_ERROR_HANDLE_H_ */
