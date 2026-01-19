/*
 * sm_pmu_regis_handle.h
 *
 *  Created on: 8 Nov 2025
 *      Author: My PC
 */

#ifndef SM_PMU_REGIS_HANDLE_H_
#define SM_PMU_REGIS_HANDLE_H_
#include "sm_types.h"
#include "ev_io_handle.h"
#include "sm_pmu_storage.h"
#include "sm_ev_module.h"


typedef void sm_pmu_regis_t;


sm_pmu_regis_t* sm_pmu_regis_create(sm_ev_module_t *_mc_module,sm_ev_module_t* _pmu_module,
									sm_pmu_storage_t *_storage, void* _arg);

int32_t sm_pmu_regis_process(sm_pmu_regis_t *_this);
int32_t sm_pmu_regis_sync_process(sm_pmu_regis_t *_this);
int32_t sm_pmu_get_regis_mode(sm_pmu_regis_t *_this);
int32_t sm_pmu_is_regis_process(sm_pmu_regis_t *_this);
#endif /* SM_PMU_REGIS_HANDLE_H_ */
