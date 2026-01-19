/*
 * ioc_io_handle.h
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */

#ifndef EV_IO_HANDLE_IOC_IO_HANDLE_H_
#define EV_IO_HANDLE_IOC_IO_HANDLE_H_
#include "sm_types.h"

int8_t sm_ioc_io_set_event( uint8_t _cmd,uint8_t _value, void *_arg);

void sm_ioc_io_on_charger(uint8_t value,void *_arg);
void sm_ioc_io_err_charger(int32_t value, void *_arg);
#endif /* EV_IO_HANDLE_IOC_IO_HANDLE_H_ */
