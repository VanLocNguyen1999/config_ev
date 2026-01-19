/*
 * sm_ev_pmu_protect.c
 *
 *  Created on: 7 Nov 2025
 *      Author: My PC
 */
#include "sm_pmu_ev_protect.h"
#include "sm_ev_mc_module.h"
#include "sm_pmu_app.h"

#include "sm_co_od_common.h"
#include "ev_io_define.h"

typedef struct pmu_ev_protect_ipml pmu_ev_protect_ipml_t;

#define _impl(x)			((pmu_ev_protect_ipml_t*)(x))

enum{

	EV_ANTI_INACTIVE = 0,
	EV_ANTI_IDLE,
	EV_ANTI_ACTIVE
};

struct pmu_ev_protect_ipml{

	sm_pmu_storage_t* 	m_pmu_storage;
	sm_ev_module_t* m_mc_module;

	pmu_config  config_resquet;
	uint8_t ev_anthi_st;

	sm_co_t* m_co;
	elapsed_timer_t m_timeout;

	void* _arg;
};

static pmu_config  config_resquet_default = {
        .anti_st = EV_UNBLOCK_STATE,
        .block_st = EV_UNBLOCK_STATE,
        .lock_st = EV_UNLOCK_STATE,
        .m_pmu_purpose = PURPOSE_FOR_VEHICLE_MARKET
};
static pmu_ev_protect_ipml_t g_pmu_ev_protect = {

		.m_mc_module = NULL,
		.m_pmu_storage = NULL,
		.m_co = NULL,
		.ev_anthi_st = EV_ANTI_INACTIVE,
};
sm_pmu_ev_protect_t* sm_pmu_ev_protect_create(sm_co_t *_co,
		sm_pmu_storage_t *_pmu_storage, sm_ev_module_t *_mc_module, void *_arg) {
	if(!_pmu_storage || ! _mc_module || !_co) return NULL;

	g_pmu_ev_protect.m_co = _co;
	g_pmu_ev_protect.m_mc_module = _mc_module;
	g_pmu_ev_protect.m_pmu_storage = _pmu_storage;
	g_pmu_ev_protect._arg = _arg;

	g_pmu_ev_protect.config_resquet = config_resquet_default;
	return (sm_pmu_ev_protect_t*)&g_pmu_ev_protect;
}
static void sm_pmu_ev_run_warning_anti_theft(pmu_ev_protect_ipml_t* _this) {

	sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, OP_STATE_FORCE_BLINK,_this->_arg);
}
static void sm_pmu_ev_stop_warning_anti_theft(pmu_ev_protect_ipml_t* _this) {
	sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, OP_STATE_NORMAL, _this->_arg);
}

static void sm_co_ev_anthi_theft_sdo_cb(SM_SDO_STATUS_t _status,
		int32_t _tx_err, int32_t _rx_err, void *_arg) {

	(void) _rx_err;
	(void) _tx_err;
	pmu_config *pmu_config_data = NULL;
	pmu_ev_protect_ipml_t *ev_protect = (pmu_ev_protect_ipml_t*) (_arg);
	if (!ev_protect) {
		return;
	}

	if (_status == SM_SDO_ST_SUCCESS) {
		pmu_config_data = sm_pmu_storage_get_config(ev_protect->m_pmu_storage);
		pmu_config_data->anti_st = EV_LOCK_STATE;
	}

}
uint8_t anti_cmd[1];
static int32_t sdo_cmd_mc_wrrite_anti_theft(pmu_ev_protect_ipml_t* _this, uint8_t cmd){
	anti_cmd[0] = cmd;
    return sm_co_sdo_client_send(_this->m_co,
    							SDO_MC_ANTI_THEFT_ST_INDEX,
								SDO_MC_ANTI_THEFT_ST_SUB_INDEX,
								 MC_NODE_ID_DEFAULT,
								 anti_cmd,
                                 1,
								 SDO_TIMEOUT_DEFAULT,
								 sm_co_ev_anthi_theft_sdo_cb,
                                 _this);
}

static void sm_pmu_ev_write_inactive_anthi_theft(pmu_ev_protect_ipml_t* _this){

	pmu_config *pmu_config_data = NULL;
	sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, OP_STATE_NORMAL, _this->_arg);
	/*delay 2000ms*/
	if(elapsed_timer_get_remain(&_this->m_timeout)){
		return;
	}
	pmu_config_data = sm_pmu_storage_get_config(_this->m_pmu_storage);
	pmu_config_data->anti_st = EV_UNLOCK_STATE;
}

static void sm_pmu_ev_write_active_anthi_theft(pmu_ev_protect_ipml_t *_this) {
	sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,OP_STATE_FORCE, _this->_arg);

	if(!_this->m_co) return ;
	/*delay 2000ms, wait MC startup*/
	if (!elapsed_timer_get_remain(&_this->m_timeout)) {
		sdo_cmd_mc_wrrite_anti_theft(_this,1);
	}
}

static void sm_pmu_ev_lock_state(pmu_ev_protect_ipml_t* _this){

	pmu_config *pmu_config_data = NULL;
	pmu_config_data = sm_pmu_storage_get_config(_this->m_pmu_storage);
	sm_mc_data_t *mc_data = sm_mc_get_data(g_pmu_app->m_mc_module);

	if (pmu_config_data->lock_st == EV_UNLOCK_STATE) {

		_this->config_resquet.lock_st = EV_UNLOCK_STATE;
	} else if (pmu_config_data->lock_st  == EV_LOCK_STATE
			&& _this->config_resquet.lock_st != EV_LOCK_STATE) {

		if (mc_data->m_speed_rpm > 1) {

			_this->config_resquet.lock_st = EV_PRE_LOCK_STATE;
		} else {

			_this->config_resquet.lock_st = EV_LOCK_STATE;
		}
	}
}
static void sm_pmu_ev_block_state(pmu_ev_protect_ipml_t* _this){

	pmu_config *pmu_config_data = NULL;
	pmu_config_data = sm_pmu_storage_get_config(_this->m_pmu_storage);
	sm_mc_data_t *mc_data = sm_mc_get_data(g_pmu_app->m_mc_module);

	if (pmu_config_data->block_st == EV_UNLOCK_STATE) {

		_this->config_resquet.block_st = EV_UNLOCK_STATE;
	} else if (pmu_config_data->block_st  == EV_LOCK_STATE
			&& _this->config_resquet.block_st != EV_LOCK_STATE) {

		if (mc_data->m_speed_rpm > 1) {

			_this->config_resquet.block_st = EV_PRE_LOCK_STATE;
		} else {

			_this->config_resquet.block_st = EV_LOCK_STATE;
		}
	}
}
static void sm_pmu_ev_anti_state(pmu_ev_protect_ipml_t* _this){

	pmu_config *pmu_config_data = NULL;
	pmu_config_data = sm_pmu_storage_get_config(_this->m_pmu_storage);
	sm_mc_data_t *mc_data = sm_mc_get_data(g_pmu_app->m_mc_module);

	if (pmu_config_data->anti_st == EV_UNLOCK_STATE) {

		elapsed_timer_resetz(&_impl(_this)->m_timeout, 100);
		_this->config_resquet.anti_st = EV_UNLOCK_STATE;
	} else if (pmu_config_data->anti_st == EV_LOCK_STATE
			&& _this->config_resquet.anti_st != EV_LOCK_STATE) {

		elapsed_timer_resetz(&_impl(_this)->m_timeout, 2000);
		if (mc_data->m_speed_rpm > 1) {

			_this->config_resquet.anti_st = EV_PRE_LOCK_STATE;
		} else {

			_this->config_resquet.anti_st = EV_LOCK_STATE;
		}
	}
}
//
static inline int32_t sm_pmu_ev_lock_process(pmu_ev_protect_ipml_t* _this){

	sm_pmu_ev_lock_state(_this);
	if(_impl(_this)->config_resquet.lock_st == EV_LOCK_STATE){

		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,OP_STATE_NORMAL, _this->_arg);
		return -1;
	}
	return 0;
}
static inline int32_t sm_pmu_ev_block_process(pmu_ev_protect_ipml_t* _this){

	sm_pmu_ev_block_state(_this);
	if(_impl(_this)->config_resquet.block_st == EV_LOCK_STATE){

		sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED,OP_STATE_NORMAL, _this->_arg);
		return -1;
	}
	return 0;
}

bool stop_warning_anti_theft_st = false;
static inline int32_t sm_pmu_ev_anti_process(pmu_ev_protect_ipml_t* _this){

	sm_pmu_ev_anti_state(_this);
	sm_mc_data_t *mc_data = sm_mc_get_data(g_pmu_app->m_mc_module);
	/*active anti-theft*/
//
	if (_impl(_this)->config_resquet.anti_st == EV_LOCK_STATE
			&& mc_data->m_anti_theft_st != MC_ANTI_ACTIVATED_IDLE
			&& mc_data->m_anti_theft_st != MC_ANTI_ACTIVATED_RUNNING) {

		sm_pmu_ev_write_active_anthi_theft(_impl(_this));
	} else if (_impl(_this)->config_resquet.anti_st == EV_LOCK_STATE
			&& (mc_data->m_anti_theft_st == MC_ANTI_ACTIVATED_IDLE)) {

		_impl(_this)->ev_anthi_st = EV_ANTI_IDLE;
	}else if (_impl(_this)->config_resquet.anti_st == EV_LOCK_STATE
			&& (mc_data->m_anti_theft_st == MC_ANTI_ACTIVATED_RUNNING)) {

		_impl(_this)->ev_anthi_st = EV_ANTI_ACTIVE;
	}
//	/*inactive anti-theft*/
	else if(_impl(_this)->config_resquet.anti_st == EV_UNLOCK_STATE &&
			mc_data->m_anti_theft_st != MC_ANTI_INACTIVATED &&
			_impl(_this)->ev_anthi_st != EV_ANTI_INACTIVE){
			sm_pmu_ev_write_inactive_anthi_theft(_impl(_this));
			_impl(_this)->ev_anthi_st = EV_ANTI_INACTIVE;
	}
	else{
		elapsed_timer_reset(&_impl(_this)->m_timeout);
	}
//	/********Warning***********/
	if( _impl(_this)->ev_anthi_st == EV_ANTI_ACTIVE){

		sm_pmu_ev_run_warning_anti_theft(_impl(_this));
		stop_warning_anti_theft_st = false;
		return -1;
	}
	else if (( _impl(_this)->ev_anthi_st != EV_ANTI_ACTIVE)
			&& (!stop_warning_anti_theft_st)) {

		sm_pmu_ev_stop_warning_anti_theft(_impl(_this));
		stop_warning_anti_theft_st = true;
	}

	return 0;
}
void sm_pmu_ev_protect_process(sm_pmu_ev_protect_t *_this) {

	if (sm_pmu_ev_anti_process(_impl(_this)) < 0) {

		return;
	}
	if (sm_pmu_ev_block_process(_impl(_this)) < 0) {

		return;
	}
	sm_pmu_ev_lock_process(_impl(_this));
}
void sm_pmu_ev_set_anthi_theft_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd) {

	if(!_this) return ;
	_impl(_this)->config_resquet.anti_st = cmd;
}
void sm_pmu_ev_set_block_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd) {

	if(!_this) return ;
	_impl(_this)->config_resquet.block_st = cmd;
}
void sm_pmu_ev_set_lock_resquet(sm_pmu_ev_protect_t *_this, uint8_t cmd) {

	if(!_this) return ;
	_impl(_this)->config_resquet.lock_st = cmd;
}
