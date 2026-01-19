//
// Created by vnbk on 06/02/2025.
//
#include "sm_pmu_bpm_handle.h"
#include "sm_logger.h"

#include "sm_bsp_pmu.h"
#include "sm_co_od_common.h"
#include "sm_pmu_stm.h"
#include "sm_pmu_app.h"
#include "sm_bp_retain.h"

#define TAG "SM_PMU_BP_HANDLE"

#define _impl(x)    ((sm_pmu_bpm_handle_t*)(x))

#define WAITING_FIRST_BP_TIMEOUT    3000
#define AUTH_BP_ID_INITIALIZED      (-1)
#define AUTH_BP_ID_FULL             (-2)

static int32_t sm_bp_nodeid_select(int32_t _id){
    if(_id == 0){
        return sm_bsp_pmu_io_set_node_id1(SM_HAL_IO_ON);
    }else if(_id == 1){
        return sm_bsp_pmu_io_set_node_id2(SM_HAL_IO_ON);
    }else if(_id == 2){
        return sm_bsp_pmu_io_set_node_id3(SM_HAL_IO_ON);
    }else{
        return -1;
    }
}

static int32_t sm_bp_nodeid_deselect(int32_t _id){
    if(_id == 0){
        return sm_bsp_pmu_io_set_node_id1(SM_HAL_IO_OFF);
    }else if(_id == 1){
        return sm_bsp_pmu_io_set_node_id2(SM_HAL_IO_OFF);
    }else if(_id == 2){
        return sm_bsp_pmu_io_set_node_id3(SM_HAL_IO_OFF);
    }else{
        return -1;
    }
}

static sm_bp_node_id_controller_t g_node_id_controller = {
        .sm_bp_node_id_select = sm_bp_nodeid_select,
        .sm_bp_node_id_deselect = sm_bp_nodeid_deselect
};

sm_bp_node_id_controller_t* sm_pmu_get_node_id_if(){
    return &g_node_id_controller;
}

typedef struct sm_pmu_bpm_handle{
    sm_sv_bp_t* m_bpm;
    sm_pmu_bpm_config_t* m_config;

    bool m_first_bp_found;
    int32_t m_first_bp_id;
    elapsed_timer_t m_timeout;

    int32_t m_auth_id_in_process;
    int32_t m_pause;
    uint32_t m_retry;
}sm_pmu_bpm_handle_t;

static sm_pmu_bpm_config_t g_bpm_config_default[SM_BP_NUMBER_DEFAULT] = {
        {
            .m_enable = true,
        },
        {
            .m_enable = true,
        },
        {
            .m_enable = true,
        }
};

static sm_pmu_bpm_handle_t g_pmu_bpm_handle = {
        .m_bpm = NULL,
        .m_config = NULL,
        .m_auth_id_in_process = AUTH_BP_ID_INITIALIZED,
        .m_retry = 0,
        .m_first_bp_found = false,
        .m_first_bp_id = -1
};

static int32_t sm_pmu_bpm_find_next_bp_auth(sm_pmu_bpm_handle_t* _this){
    if(_this->m_auth_id_in_process == AUTH_BP_ID_INITIALIZED){
        _this->m_auth_id_in_process = 0;
        if(_this->m_config[_this->m_auth_id_in_process].m_enable){
        	return _this->m_auth_id_in_process;
        }
    }

    if(_this->m_auth_id_in_process == AUTH_BP_ID_FULL){
    	 return _this->m_auth_id_in_process;
    }

    for(uint8_t index = 0; index < SM_BP_NUMBER_DEFAULT; index++){
        _this->m_auth_id_in_process++;
        if(_this->m_auth_id_in_process >= SM_BP_NUMBER_DEFAULT){
            _this->m_auth_id_in_process = 0;
        }

        if(!_this->m_config[_this->m_auth_id_in_process].m_enable){
            continue;
        }

        if(!sm_sv_bp_is_connected(_this->m_bpm, _this->m_auth_id_in_process)){
            return _this->m_auth_id_in_process;
        }
    }
    _this->m_auth_id_in_process = AUTH_BP_ID_FULL;
    return _this->m_auth_id_in_process;
}

void sm_pmu_bpm_handle_on_re_config_node_id(int32_t _id, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg){
    (void)_data;
    sm_pmu_bpm_handle_t* pmu_bpm = _impl(_arg);
    if(!pmu_bpm){
        return;
    }
    if(_cmd != BP_CMD_RECONFIG_ID){
        return;
    }
    if (_success == SM_BP_CMD_SUCCESS)
    {
    	sm_stm_set_event(PMU_STM_EVENT_FIRST_BP_ASSIGNED);
        pmu_bpm->m_first_bp_found = true;
        pmu_bpm->m_first_bp_id = -1;
        LOG_DBG(TAG, "Found first BP SUCCESS", _id);
    }else{
        if(pmu_bpm->m_retry < 2){
            LOG_ERR(TAG, "Retry Re-config NODE ID: %d", _id);
            sm_sv_bp_set_cmd(pmu_bpm->m_bpm,
                             _id,
                             BP_CMD_RECONFIG_ID,
                             &pmu_bpm->m_first_bp_id,
                             sm_pmu_bpm_handle_on_re_config_node_id,
                             pmu_bpm);
            pmu_bpm->m_retry++;
        }
    }
}

void sm_pmu_bpm_handle_on_connected(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    (void)_sn;
    (void)_soc;
    sm_pmu_bpm_handle_t* pmu_bpm = _impl(_arg);
    if(!pmu_bpm){
        return;
    }
    if(!pmu_bpm->m_first_bp_found){
    	pmu_bpm->m_first_bp_id = _id + BP_NODE_ID_OFFSET;
    	pmu_bpm->m_retry = 0;
        LOG_DBG(TAG, "Re-config NODE ID: %d", _id);
        sm_sv_bp_set_cmd(pmu_bpm->m_bpm,
                         _id,
                         BP_CMD_RECONFIG_ID,
                         &pmu_bpm->m_first_bp_id,
						 sm_pmu_bpm_handle_on_re_config_node_id,
						 pmu_bpm);
    }

    int32_t bp_retain = sm_bp_deactive_retain((sm_bp_retain_t*) sm_sv_bp_retain_get_obj(pmu_bpm->m_bpm),
    											(uint8_t) _id) ;
	if( bp_retain == 0){

		sm_sv_discharger_resume(g_pmu_app->m_discharger_service);
		sm_sv_discharger_set_sw_state(g_pmu_app->m_discharger_service,ACTIVE);
	}

	if (bp_retain == 0
			&& (get_ev_io_state(&g_pmu_app->m_ev_io_service->m_key) == ACTIVE)
			&& (g_pmu_app->m_ev_io_service->m_parking.state == INACTIVE)) {

		sm_stm_set_event(PMU_STM_EVENT_ENTER_RUNNING_MODE);
	}
}

void sm_pmu_bpm_on_disconnected(int32_t _id, const char * _sn, void *_arg){
    (void)_sn;
    sm_pmu_bpm_handle_t* pmu_bpm = _impl(_arg);
    if(!pmu_bpm){
        return;
    }

    if(pmu_bpm->m_auth_id_in_process == AUTH_BP_ID_FULL){
        pmu_bpm->m_auth_id_in_process = _id;
    }

    int32_t bp_discharger = sm_sv_get_discharger_bp_num(g_pmu_app->m_discharger_service);
    int32_t bp_retain = sm_bp_active_retain((sm_bp_retain_t*) sm_sv_bp_retain_get_obj(pmu_bpm->m_bpm),
    											(uint8_t) _id) ;
    if (bp_retain > 0) {

        if (bp_discharger == 0) {
        	pmu_bpm->m_first_bp_found = false;
        	sm_sv_discharger_pause(g_pmu_app->m_discharger_service);
        } else {
            sm_sv_discharger_set_sw_state(&g_pmu_app->m_discharger_service,INACTIVE);
        }

        sm_stm_set_event(PMU_STM_EVENT_ENTER_POWER_LIMIT_MODE);
    }

}

void sm_pmu_bpm_on_update_data(int32_t _id, const sm_bp_data_t* _bp_data, void* _arg){

    (void)_id;
    (void)_bp_data;
    sm_pmu_bpm_handle_t* pmu_bpm = _impl(_arg);
    if(!pmu_bpm){
        return;
    }
//    sm_sv_bp_update_connect_state(_id, pmu_bpm->m_bpm);
}

void sm_pmu_bpm_handle_assign_fail(int32_t _id ,void *_arg) {

	(void) _id;
	sm_pmu_bpm_handle_t *pmu_bpm = _impl(_arg);
	if (!pmu_bpm) {
		return;
	}
	int32_t bp_assign_numbs = sm_sv_bp_get_bp_numbs_assign(pmu_bpm->m_bpm);
	if (!bp_assign_numbs) {
//		pmu_bpm->m_auth_id_in_process = sm_pmu_bpm_find_next_bp_auth(pmu_bpm);
//		sm_sv_first_bp_auth(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process,
//				sm_pmu_bpm_auth_handle, pmu_bpm);
	}
}

void sm_pmu_bpm_auth_handle(int32_t _id, SM_BP_AUTH_EVENT _event, const char* _sn, int32_t _soc, void* _arg){
    (void)_sn;
    (void)_soc;
    (void)_id;
    (void)_event;
    sm_pmu_bpm_handle_t* pmu_bpm = _impl(_arg);
    if(!pmu_bpm){
        return;
    }

   /* if(_id != pmu_bpm->m_auth_id_in_process){
        pmu_bpm->m_auth_id_in_process = 0;
        return;
    }*/

//    if(_event == BP_AUTH_FAILURE) ){
//        pmu_bpm->m_auth_id_in_process = sm_pmu_bpm_find_next_bp_auth(pmu_bpm);
//        sm_sv_first_bp_auth(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process, sm_pmu_bpm_auth_handle, pmu_bpm);
//    }
}

static sm_sv_bp_event_cb_t g_bp_event_cb = {
        .on_bp_connected = sm_pmu_bpm_handle_on_connected,
        .on_bp_disconnected = sm_pmu_bpm_on_disconnected,
        .on_bp_update_data = sm_pmu_bpm_on_update_data,
		.bp_assgin_fail = sm_pmu_bpm_handle_assign_fail,
};

sm_pmu_bpm_t* sm_pmu_bpm_create(sm_sv_bp_t* _bp_service, sm_pmu_bpm_config_t* _config){
    if(!_bp_service){
        return NULL;
    }
    sm_pmu_bpm_handle_t* pmu_bpm = &g_pmu_bpm_handle;

    pmu_bpm->m_bpm = _bp_service;
    sm_sv_bp_reg_event(_bp_service, &g_bp_event_cb, &g_pmu_bpm_handle);

    if(!_config){
        pmu_bpm->m_config = &g_bpm_config_default[0];
    }
    elapsed_timer_resetz(&pmu_bpm->m_timeout, WAITING_FIRST_BP_TIMEOUT);

    return pmu_bpm;
}

void sm_pmu_bpm_first_bp_auth(void *_arg) {

	(void)_arg;
	sm_pmu_bpm_handle_t *pmu_bpm = &g_pmu_bpm_handle;
	if (!pmu_bpm) {
		return;
	}
	while(elapsed_timer_get_remain(&pmu_bpm->m_timeout)){


	}
	pmu_bpm->m_auth_id_in_process = 0;
	sm_sv_first_bp_auth(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process,
			sm_pmu_bpm_auth_handle, pmu_bpm);
}

void sm_pmu_bpm_handle_process(void* _arg){

    sm_pmu_app_t* app = (sm_pmu_app_t*) _arg;
    uint8_t pmu_stm = (uint8_t)sm_stm_get_current_state(app->m_pmu_stm);
    sm_pmu_bpm_handle_t* pmu_bpm = &g_pmu_bpm_handle;

    if(!pmu_bpm || (pmu_stm == PMU_STM_SLEEP)){
        return;
    }

    if(!pmu_bpm->m_first_bp_found && elapsed_timer_get_remain(&pmu_bpm->m_timeout)){
        return;
    }
    if(pmu_bpm->m_auth_id_in_process >= 0 && sm_sv_bp_is_authenticating(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process)){
        return;
    }
    if(pmu_bpm->m_first_bp_id > 0){
        return;
    }

    if(sm_pmu_bpm_find_next_bp_auth(pmu_bpm) < 0){
        return;
    }
    int32_t bp_discharger = sm_sv_get_discharger_bp_num(g_pmu_app->m_discharger_service);
    if(!pmu_bpm->m_first_bp_found){
       	pmu_bpm->m_retry++;
        	if(pmu_bpm->m_retry > SM_BP_NUMBER_DEFAULT && bp_discharger != 0){
        		pmu_bpm->m_first_bp_found = true;
        	}
        sm_sv_first_bp_auth(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process, sm_pmu_bpm_auth_handle, pmu_bpm);
    }
    else{
        sm_sv_bp_auth(pmu_bpm->m_bpm, pmu_bpm->m_auth_id_in_process, sm_pmu_bpm_auth_handle, pmu_bpm);
    }
}

