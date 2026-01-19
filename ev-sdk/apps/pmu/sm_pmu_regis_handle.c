/*
 * sm_pmu_regis_handle.c
 *
 *  Created on: 8 Nov 2025
 *      Author: My PC
 */

#include "sm_pmu_regis_handle.h"
#include "sm_ev_mc_module.h"
#include "sm_ev_pmu_module.h"
#include "sm_pmu_app.h"
#include "ev_io_define.h"

typedef struct sm_pmu_regis_impl sm_pmu_regis_impl_t;

#define _impl(x)			((sm_pmu_regis_impl_t*)(x))
typedef IO_ST REGISTRATION_ST;

struct sm_pmu_regis_impl {

	sm_ev_module_t* m_mc_module;
	sm_ev_module_t* m_pmu_module;
	sm_pmu_storage_t* m_storage;

	REGISTRATION_ST pre_state;

	ev_purpose m_purpose;
	bool data_syn_flag;

	elapsed_timer_t m_timeout;

	int32_t m_in_process;

	uint32_t m_counter;
	void* _arg;
};

static sm_pmu_regis_impl_t g_pmu_regis = {

		.m_mc_module = NULL,
		.m_pmu_module = NULL,
		.m_storage = NULL,
		._arg = NULL,
		.pre_state = INACTIVE,
		.m_purpose =  PURPOSE_FOR_VEHICLE_MARKET,
		.data_syn_flag = false,
		.m_in_process = -1,
		.m_counter = 0,
};

sm_pmu_regis_t* sm_pmu_regis_create(sm_ev_module_t *_mc_module,sm_ev_module_t* _pmu_module,
										sm_pmu_storage_t *_storage, void* _arg){

	if(!_mc_module || !_storage) return NULL;

	g_pmu_regis.m_mc_module = _mc_module;
	g_pmu_regis.m_pmu_module = _pmu_module;
	g_pmu_regis.m_storage = _storage;
	g_pmu_regis._arg = _arg;

	pmu_config pmu_config_data = *sm_pmu_storage_get_config(g_pmu_regis.m_storage);
	g_pmu_regis.m_purpose = pmu_config_data.m_pmu_purpose;
	elapsed_timer_resetz(&g_pmu_regis.m_timeout, 2000);
	return (sm_pmu_regis_t*) &g_pmu_regis;
}

static int32_t sm_pmu_regis_update_state(sm_pmu_regis_impl_t* _this){

	sm_pmu_app_t *pmu_app = (sm_pmu_app_t*) (_this->_arg);
	REGISTRATION_ST cur_state = pmu_app->m_ev_io_service->m_purpose_mode.state;

	if(_this->m_in_process == 2) return -2;

	if (_this->m_in_process == PURPOSE_FOR_VEHICLE_INSPECTION
			|| _this->m_in_process == PURPOSE_FOR_VEHICLE_MARKET) {
		return _this->m_in_process;
	}

	if(_this->pre_state == cur_state) return -1;
	_this->pre_state = (uint8_t) pmu_app->m_ev_io_service->m_purpose_mode.state;
	elapsed_timer_resetz(&_this->m_timeout, 1000);
//	_this->pre_state = cur_state;
	if(_this->m_purpose == PURPOSE_FOR_VEHICLE_MARKET){

		_this->m_purpose = PURPOSE_FOR_VEHICLE_INSPECTION;
		_this->m_in_process = PURPOSE_FOR_VEHICLE_INSPECTION;
		return _this->m_in_process;
	}
	_this->m_purpose = PURPOSE_FOR_VEHICLE_MARKET;
	_this->m_in_process = PURPOSE_FOR_VEHICLE_MARKET;
	return _this->m_in_process;;
}
static inline int32_t sm_pmu_set_market_mode(sm_pmu_regis_t *_this) {

	if (!_this)
		return -1;
	sm_pmu_data_t *pmu_data = sm_pmu_get_data(_impl(_this)->m_pmu_module);
	sm_mc_data_t *mc_data = sm_mc_get_data(_impl(_this)->m_mc_module);
	pmu_config pmu_config_data = *sm_pmu_storage_get_config(
			_impl(_this)->m_storage);
	sm_pmu_app_t *pmu_app = (sm_pmu_app_t*) (_impl(_this)->_arg);

	if (!_impl(_this)->data_syn_flag) {

		/*timeout to set config MC*/
		elapsed_timer_resetz(&_impl(_this)->m_timeout, 3000);
	}
	_impl(_this)->data_syn_flag = true;
	pmu_data->m_purpose_state =
			(uint8_t) pmu_app->m_ev_io_service->m_purpose_mode.state;

	if (!elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {

		_impl(_this)->data_syn_flag = false;
		_impl(_this)->m_in_process = -1;
		return -3;
	}

	if (mc_data->m_mc_purpose == PURPOSE_FOR_VEHICLE_MARKET)
		_impl(_this)->m_counter++;
	else
		_impl(_this)->m_counter = 0;
	if (_impl(_this)->m_counter > 100) {

		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, OP_STATE_NORMAL,
				_impl(_this)->_arg);
		pmu_app->m_ev_io_service->m_parking.state = ACTIVE;
		pmu_config_data.m_pmu_purpose = _impl(_this)->m_purpose;
		_impl(_this)->data_syn_flag = false;
		_impl(_this)->m_in_process = -1;
        return (sm_pmu_store_config(_impl(_this)->m_storage, pmu_config_data) < 0)
                ? -4    /* store error */
                : 1;     /* store OK */
	}
	return 0;
}

static inline int32_t sm_pmu_set_regis_mode(sm_pmu_regis_t *_this) {

	if (!_this)
		return -1;
	sm_pmu_data_t *pmu_data = sm_pmu_get_data(_impl(_this)->m_pmu_module);
	sm_mc_data_t *mc_data = sm_mc_get_data(_impl(_this)->m_mc_module);
	pmu_config pmu_config_data = *sm_pmu_storage_get_config(_impl(_this)->m_storage);

	sm_pmu_app_t *pmu_app = (sm_pmu_app_t*) (_impl(_this)->_arg);

	/*delay 1000ms, wait MC startup*/
	if (elapsed_timer_get_remain(&_impl(_this)->m_timeout)
			&& (_impl(_this)->data_syn_flag == false))
		return -2;

	if (!_impl(_this)->data_syn_flag) {

		/*timeout to set config MC*/
		elapsed_timer_resetz(&_impl(_this)->m_timeout, 3000);
	}
	_impl(_this)->data_syn_flag = true;
	pmu_data->m_purpose_state =
			(uint8_t) pmu_app->m_ev_io_service->m_purpose_mode.state;

	if (!elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {

		_impl(_this)->data_syn_flag = false;
		_impl(_this)->m_in_process = -1;
		return -3;
	}
	if (mc_data->m_mc_purpose == PURPOSE_FOR_VEHICLE_INSPECTION)
		_impl(_this)->m_counter++;
	else
		_impl(_this)->m_counter = 0;
	if (_impl(_this)->m_counter > 100) {

		pmu_config_data.m_pmu_purpose = _impl(_this)->m_purpose;
		_impl(_this)->data_syn_flag = false;
		_impl(_this)->m_in_process = -1;
        return (sm_pmu_store_config(_impl(_this)->m_storage, pmu_config_data) < 0)
                ? -4    /* store error */
                : 1;     /* store OK */
	}
	return 0;
}

int32_t sm_pmu_is_regis_process(sm_pmu_regis_t *_this){

	if (!_this)
		return -1;

	return _impl(_this)->m_in_process;
}

int32_t sm_pmu_regis_sync_process(sm_pmu_regis_t *_this) {

	if (!_this)
		return -1;

	sm_mc_data_t *mc_data = sm_mc_get_data(_impl(_this)->m_mc_module);
	pmu_config pmu_config_data = *sm_pmu_storage_get_config(_impl(_this)->m_storage);
	sm_pmu_app_t *pmu_app = (sm_pmu_app_t*) (_impl(_this)->_arg);

	if (pmu_app->m_ev_io_service->m_parking.state == ACTIVE
			|| mc_data->m_speed_rpm > 0) return -1;

	if (mc_data->m_mc_purpose != pmu_config_data.m_pmu_purpose) {

		_impl(_this)->m_in_process = 2;
		_impl(_this)->m_counter++;
	} else {
		_impl(_this)->m_counter = 0;
		_impl(_this)->m_in_process = -1;
		return 1;
	}

	if (_impl(_this)->m_counter > 3000) {

		pmu_config_data.m_pmu_purpose = mc_data->m_mc_purpose;
		_impl(_this)->m_purpose = mc_data->m_mc_purpose;
		if (pmu_config_data.m_pmu_purpose == PURPOSE_FOR_VEHICLE_MARKET) {

			sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,
					OP_STATE_NORMAL,
					_impl(_this)->_arg);
			pmu_app->m_ev_io_service->m_parking.state = ACTIVE;
		}

		if (sm_pmu_store_config(_impl(_this)->m_storage, pmu_config_data) < 0) {

			_impl(_this)->m_counter = 0;
			_impl(_this)->m_in_process = -1;
			return -3;
		}
	}
	return 0;
}

int32_t sm_pmu_regis_process(sm_pmu_regis_t *_this) {

	if (!_this)
		return -1;

	int32_t purose = sm_pmu_regis_update_state(_this);
	if (purose < 0)
		return -2;
	else if (purose == PURPOSE_FOR_VEHICLE_INSPECTION) {
		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,OP_STATE_FORCE,_impl(_this)->_arg);
		sm_pmu_set_regis_mode(_this);
	} else if (purose == PURPOSE_FOR_VEHICLE_MARKET) {

		sm_pmu_set_market_mode(_this);
	}
	return 0;
}
