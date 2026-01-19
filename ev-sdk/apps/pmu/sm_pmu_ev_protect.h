/*
 * sm_ev_protect.h
 *
 *  Created on: 7 Nov 2025
 *      Author: My PC
 */

#ifndef SM_PMU_EV_PROTECT_H_
#define SM_PMU_EV_PROTECT_H_
#include "sm_types.h"
#include "sm_pmu_storage.h"
#include "sm_ev_module.h"
#include "ev_io_handle.h"
#include "sm_core_co.h"

typedef void sm_pmu_ev_protect_t;

sm_pmu_ev_protect_t* sm_pmu_ev_protect_create(sm_co_t *_co,
		sm_pmu_storage_t *_pmu_storage, sm_ev_module_t *_mc_module, void *_arg);
void sm_pmu_ev_protect_process(sm_pmu_ev_protect_t *_this);
void sm_pmu_ev_set_anthi_theft_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd);
void sm_pmu_ev_set_block_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd);
void sm_pmu_ev_set_lock_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd);
#endif /* SM_PMU_EV_PROTECT_H_ */
