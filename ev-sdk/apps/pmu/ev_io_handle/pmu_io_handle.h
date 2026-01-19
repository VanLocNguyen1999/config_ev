/*
 * pmu_io_handle.h
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */

#ifndef EV_IO_HANDLE_PMU_IO_HANDLE_H_
#define EV_IO_HANDLE_PMU_IO_HANDLE_H_

#ifdef __cplusplus
extern "C"{
#endif
#include "sm_sv_io.h"
#include "sm_types.h"

sm_sv_io_if_t* sm_pmu_get_io_if();

void sm_pmu_io_changed(uint8_t _event, uint8_t _value, void* _arg);
int8_t sm_pmu_io_set_event(uint8_t _cmd, uint8_t _value,void *_arg);
#ifdef __cplusplus
};
#endif

#endif /* EV_IO_HANDLE_PMU_IO_HANDLE_H_ */
