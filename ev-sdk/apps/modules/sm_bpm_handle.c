//
// Created by vnbk on 06/02/2025.
//
#include "sm_bpm_handle.h"
#include "sm_logger.h"

#include "sm_bsp_bpa.h"
#include "sm_co_od_common.h"

#define TAG "SM_BPM_HANDLE"

#define _impl(x)    ((sm_bpm_handle_impl_t*)(x))

#define BP_STABLE_WAITING_TIME      3000

#define AUTH_BP_ID_INITIALIZED      (-1)
#define AUTH_BP_ID_FULL             (-2)

typedef struct sm_bpm_handle{
    sm_sv_bp_t* m_bpm;
    sm_bpm_config_t* m_config;

    bool m_first_bp_found;
    int32_t m_first_bp_id;
    elapsed_timer_t m_timeout;

    int32_t m_auth_id_in_process;
    uint8_t m_retry;
}sm_bpm_handle_impl_t;

static sm_bpm_config_t g_bpm_config_default[SM_BP_NUMBER_DEFAULT] = {
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

static sm_bpm_handle_impl_t g_bpm_handle = {
        .m_bpm = NULL,
        .m_config = NULL,
        .m_auth_id_in_process = AUTH_BP_ID_INITIALIZED,
        .m_retry = 0,
        .m_first_bp_found = false,
        .m_first_bp_id = -1
};

static int32_t sm_bpm_find_next_bp_auth(sm_bpm_handle_impl_t* _this){
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

void sm_bpm_handle_on_re_config_node_id(int32_t _id, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg){
    (void)_data;
    sm_bpm_handle_impl_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }
    if(_cmd != BP_CMD_RECONFIG_ID){
        return;
    }

    if(_success == SM_BP_CMD_SUCCESS){
        bpm->m_first_bp_found = true;
        bpm->m_first_bp_id = -1;
        LOG_DBG(TAG, "Found first BP SUCCESS", _id);
    }else{
        if(bpm->m_retry < 2){
            LOG_ERR(TAG, "Retry Re-config NODE ID: %d", _id);
            sm_sv_bp_set_cmd(bpm->m_bpm,
                             _id,
                             BP_CMD_RECONFIG_ID,
                             &bpm->m_first_bp_id,
                             sm_bpm_handle_on_re_config_node_id,
                             bpm);
            bpm->m_retry++;
        }
    }
}

void sm_bpm_handle_on_connected(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    (void)_sn;
    (void)_soc;
    sm_bpm_handle_impl_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }

    if(!bpm->m_first_bp_found){
        bpm->m_first_bp_id = _id + BP_NODE_ID_OFFSET;
        bpm->m_retry = 0;
        LOG_DBG(TAG, "Re-config NODE ID: %d", _id);
        sm_sv_bp_set_cmd(bpm->m_bpm,
                         _id,
                         BP_CMD_RECONFIG_ID,
                         &bpm->m_first_bp_id,
                         sm_bpm_handle_on_re_config_node_id,
                         bpm);
    }
}

void sm_bpm_on_disconnected(int32_t _id, const char * _sn, void *_arg){
    (void)_sn;
    sm_bpm_handle_impl_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }

    if(bpm->m_auth_id_in_process == AUTH_BP_ID_FULL){
        bpm->m_auth_id_in_process = _id;
    }
}

void sm_bpm_on_update_data(int32_t _id, const sm_bp_data_t* _bp_data, void* _arg){
    sm_bpm_handle_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }
}

void sm_bpm_auth_handle(int32_t _id, SM_BP_AUTH_EVENT _event, const char* _sn, int32_t _soc, void* _arg){
    (void)_sn;
    (void)_soc;
    sm_bpm_handle_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }

   /* if(_id != bpm->m_auth_id_in_process){
        bpm->m_auth_id_in_process = 0;
        return;
    }*/

    if(_event == BP_AUTH_FAILURE){
//        bpm->m_auth_id_in_process = sm_bpm_find_next_bp_auth(bpm);
    }
}

static sm_sv_bp_event_cb_t g_bp_event_cb = {
        .on_bp_connected = sm_bpm_handle_on_connected,
        .on_bp_disconnected = sm_bpm_on_disconnected,
        .on_bp_update_data = sm_bpm_on_update_data,
};

sm_bpm_handle_t* sm_bpm_handle_create(sm_sv_bp_t* _bp_service, sm_bpm_config_t* _config){
    if(!_bp_service){
        return NULL;
    }
    sm_bpm_handle_impl_t* bpm = &g_bpm_handle;

    bpm->m_bpm = _bp_service;
    sm_sv_bp_reg_event(_bp_service, &g_bp_event_cb, &g_bpm_handle);

    if(!_config){
        bpm->m_config = &g_bpm_config_default[0];
    }

    elapsed_timer_resetz(&bpm->m_timeout, BP_STABLE_WAITING_TIME);

    return bpm;
}

int32_t sm_bpm_set_config(sm_bpm_handle_t* _this, uint8_t _port, uint8_t _enable){
    sm_bpm_handle_impl_t* bpm = _impl(_this);
    if(!bpm || _port >= SM_BP_NUMBER_DEFAULT){
        return -1;
    }
    bpm->m_config[_port].m_enable = _enable;
    return 0;
}

void sm_bpm_handle_process(void* _arg){
    sm_bpm_handle_impl_t* bpm = _impl(_arg);
    if(!bpm){
        return;
    }

    if(!bpm->m_first_bp_found && elapsed_timer_get_remain(&bpm->m_timeout)){
        return;
    }

    if(bpm->m_auth_id_in_process >= 0 && sm_sv_bp_is_authenticating(bpm->m_bpm, bpm->m_auth_id_in_process)){
        return;
    }

    if(bpm->m_first_bp_id > 0){
        return;
    }

    if(sm_bpm_find_next_bp_auth(bpm) < 0){
        return;
    }

    if(!bpm->m_first_bp_found){
        bpm->m_retry++;
    	if(bpm->m_retry >= SM_BP_NUMBER_DEFAULT){
            bpm->m_first_bp_found = true;
    	}
        sm_sv_first_bp_auth(bpm->m_bpm, bpm->m_auth_id_in_process, sm_bpm_auth_handle, bpm);
    }else{
        sm_sv_bp_auth(bpm->m_bpm, bpm->m_auth_id_in_process, sm_bpm_auth_handle, bpm);
    }
}
