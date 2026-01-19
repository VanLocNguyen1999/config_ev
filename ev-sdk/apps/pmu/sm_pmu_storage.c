/*
 * sm_pmu_storage.c
 *
 *  Created on: 7 Nov 2025
 *      Author: My PC
 */
#include "sm_ev_storage.h"
#include "sm_pmu_storage.h"
#include "sm_bsp_pmu.h"
#include "sm_pmu_flash_config.h"
#include "sm_hal.h"



typedef struct pmu_storage_impl pmu_storage_impl_t;
#define _impl(x)			((pmu_storage_impl_t*)(x))

static const pmu_config config_data = {
                                 .anti_st = EV_UNBLOCK_STATE,
                                 .block_st = EV_UNBLOCK_STATE,
                                 .lock_st = EV_UNLOCK_STATE,
                                 .m_pmu_purpose = PURPOSE_FOR_VEHICLE_MARKET
                                  };
struct pmu_storage_impl{

    struct{
        sm_storage_t* m_manu_storage;
        sm_storage_t* m_config_storage;
        sm_storage_t* m_ev_data;
    }m_storage;

    struct{
        pmu_config  config_data;
        sys_energy  energy_data;
        sm_module_info_t m_manu_info;
    }m_data_store;
};

static pmu_storage_impl_t pmu_storage_default = {
        .m_storage = {
                .m_config_storage = NULL,
                .m_manu_storage = NULL,
                .m_ev_data = NULL
        },
};
//static pmu_storage_impl_t* pmu_storage_default = NULL;

sm_pmu_storage_t* sm_pmu_storage_create(){

    sm_storage_t* storage = NULL;
    sm_hal_flash_t* data_flash = sm_bsp_pmu_get_data_flash();

    storage = sm_ev_config_create (data_flash, SM_STORAGE_PMU_CONFIG_FLASH_ADDR,
                                   sizeof(pmu_storage_default.m_data_store.config_data));
    pmu_storage_default.m_storage.m_config_storage = storage;

    storage = sm_ev_config_create (data_flash, SM_STORAGE_PMU_MANU_INFO_FLASH_ADDR,
                                   sizeof(pmu_storage_default.m_data_store.m_manu_info));
    pmu_storage_default.m_storage.m_manu_storage = storage;

    storage = sm_ev_config_create (data_flash, SM_STORAGE_PMU_EV_DATA_FLASH_ADDR,
                                   sizeof(pmu_storage_default.m_data_store.energy_data));
    pmu_storage_default.m_storage.m_ev_data = storage;
	return (sm_pmu_storage_t*)&pmu_storage_default;
}

int32_t sm_pmu_storage_load(sm_pmu_storage_t* _this){

	if(!_this) return -1;
	int32_t ret = 0;

	ret = _impl(_this)->m_storage.m_manu_storage->m_proc->load(
	_impl(_this)->m_storage.m_manu_storage,
			&_impl(_this)->m_data_store.m_manu_info);
	if (ret == -1) {

		sm_module_reset_data(&_impl(_this)->m_data_store.m_manu_info);
		_impl(_this)->m_storage.m_manu_storage->m_proc->store(
		_impl(_this)->m_storage.m_manu_storage,
				&_impl(_this)->m_data_store.m_manu_info);
//		LOG_ERR(TAG, "Could NOT load pmu infor, now set default value");
	} else if (ret == -2) {

//	        LOG_WRN(TAG, "Config pmu infor INVALID");
	} else {

//	        LOG_INF(TAG, "Load pmu infor SUCCEED");
	}

	ret = _impl(_this)->m_storage.m_ev_data->m_proc->load(
	_impl(_this)->m_storage.m_ev_data,
			&_impl(_this)->m_data_store.energy_data);
	if (ret == -1) {

//	        LOG_ERR(TAG, "Could NOT energy_data, now set default value");
		_impl(_this)->m_data_store.energy_data.elec_energyCharge = 0;
		_impl(_this)->m_data_store.energy_data.elec_energyDischarge = 0;
		_impl(_this)->m_storage.m_ev_data->m_proc->store(
				_impl(_this)->m_storage.m_ev_data,&_impl(_this)->m_data_store.energy_data);
	} else {
//	    	LOG_INF(TAG, "Load ev_data SUCCEED");
	}

	ret = _impl(_this)->m_storage.m_config_storage->m_proc->load(
			_impl(_this)->m_storage.m_config_storage,&_impl(_this)->m_data_store.config_data);
	if (ret < 0) {
//	        LOG_ERR(TAG, "Could NOT config_data or data invalid, now set default value");
		memcpy(&_impl(_this)->m_data_store.config_data, &config_data,sizeof(config_data));
		_impl(_this)->m_storage.m_config_storage->m_proc->store(
				_impl(_this)->m_storage.m_config_storage,&_impl(_this)->m_data_store.config_data);
	} else {

//	    	LOG_INF(TAG, "Load config_data SUCCEED");
	}
	return 0;
}

/*pmu config*/
pmu_config* sm_pmu_storage_get_config(sm_pmu_storage_t *_this){

	if(!_this) return NULL;
	return &_impl(_this)->m_data_store.config_data;
}

int32_t sm_pmu_store_config(sm_pmu_storage_t *_this, pmu_config _data){

	int32_t ret = -1;
	memcpy(&_impl(_this)->m_data_store.config_data, &_data,sizeof(_data));
	ret = _impl(_this)->m_storage.m_config_storage->m_proc->store(
	_impl(_this)->m_storage.m_config_storage, &_impl(_this)->m_data_store.config_data);

	return ret;
}
/*sys_energy*/
sys_energy* sm_pmu_storage_get_sys_energy(sm_pmu_storage_t *_this){

	if(!_this) return NULL;
	return &_impl(_this)->m_data_store.energy_data;
}
int32_t sm_pmu_store_sys_energy(sm_pmu_storage_t *_this, sys_energy _energy){

	int32_t ret = -1;
	memcpy(&_impl(_this)->m_data_store.energy_data, &_energy,sizeof(_energy));
	ret = _impl(_this)->m_storage.m_ev_data->m_proc->store(
			_impl(_this)->m_storage.m_ev_data,&_impl(_this)->m_data_store.energy_data);

	return ret;
}
/*module_info*/
sm_module_info_t* sm_pmu_storage_get_module_info(sm_pmu_storage_t *_this){

	if(!_this) return NULL;
	return &_impl(_this)->m_data_store.m_manu_info;
}
int32_t sm_pmu_store_module_info(sm_pmu_storage_t *_this, sm_module_info_t _info){

	int32_t ret = -1;
	memcpy(&_impl(_this)->m_data_store.m_manu_info, &_info,sizeof(_info));
	ret = _impl(_this)->m_storage.m_manu_storage->m_proc->store(
			_impl(_this)->m_storage.m_manu_storage,&_impl(_this)->m_data_store.m_manu_info);

	return ret;
}
