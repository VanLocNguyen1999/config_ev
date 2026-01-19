/*
 * sm_pmu_storage.h
 *
 *  Created on: 7 Nov 2025
 *      Author: My PC
 */

#ifndef SM_PMU_STORAGE_H_
#define SM_PMU_STORAGE_H_
#include "sm_types.h"
#include "sm_ev_data.h"

typedef void sm_pmu_storage_t;

sm_pmu_storage_t* sm_pmu_storage_create();
int32_t sm_pmu_storage_load(sm_pmu_storage_t *_this);

pmu_config* sm_pmu_storage_get_config(sm_pmu_storage_t *_this);
int32_t sm_pmu_store_config(sm_pmu_storage_t *_this, pmu_config _data);

sys_energy* sm_pmu_storage_get_sys_energy(sm_pmu_storage_t *_this);
int32_t sm_pmu_store_sys_energy(sm_pmu_storage_t *_this, sys_energy _energy);

sm_module_info_t* sm_pmu_storage_get_module_info(sm_pmu_storage_t *_this);
int32_t sm_pmu_store_module_info(sm_pmu_storage_t *_this, sm_module_info_t _info);
#endif /* SM_PMU_STORAGE_H_ */
