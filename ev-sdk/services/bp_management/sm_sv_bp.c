#include "sm_sv_bp.h"
#include "sm_bp.h"
#include "sm_bp_auth.h"
#include "sm_bp_co.h"
#include "sm_bp_retain.h"
#include "sm_elapsed_timer.h"
#include "sm_logger.h"

#define _impl(p)    ((sm_sv_bp_impl_t*)(p))

#define TAG "SM_SV_BP"

#define SM_SV_BP_EVENT_CB_MAX       				10
#define SM_SV_POLLING_ASSIGN_FIRST_BP_PERIOD		1000
#define SM_SV_POLLING_ASSIGN_BP_PERIOD				500

#define SM_BP_BUILD_VER_POS     0
#define SM_SV_BP_DELAY_ASSIGN						4000

typedef struct {
    int32_t m_numb;
    sm_bp_t m_bps[SM_SV_BP_NUMBER_DEFAULT];

    sm_co_if_t* m_co_if;

    sm_bp_co_t* m_bp_co;

    sm_bp_auth_t* m_auth;

    sm_bp_retain_t* m_bp_retain;

    sm_bp_cmd_t m_cmds[BP_CMD_QUEUE_SIZE];
    uint8_t m_cmd_head;
    uint8_t m_cmd_tail;
    sm_bp_cmd_t* m_current_cmd;

    bool m_active_low_data_mode;

    sm_bp_auth_event_fn_t m_force_auth_event; 
    void* m_force_auth_arg;
    int8_t m_force_auth_id;

    struct {
        const sm_sv_bp_event_cb_t* m_event_cb;
        void* m_event_arg;
    }m_event_handlers[SM_SV_BP_EVENT_CB_MAX];

    uint8_t m_temp[BP_DEVICE_SN_SIZE];

}sm_sv_bp_impl_t;


static sm_sv_bp_impl_t g_sv_bp_default = {
    .m_numb = SM_SV_BP_NUMBER_DEFAULT,
    .m_event_handlers ={
                         { .m_event_cb = NULL,
                           .m_event_arg = NULL }
                        },
    .m_co_if = NULL,
    .m_auth = NULL,
	.m_bp_retain = NULL,
    .m_cmd_head = 0,
    .m_cmd_tail = 0,
    .m_current_cmd = NULL,
    .m_bp_co = NULL
};

static void sm_bp_reset_force_assigning(sm_sv_bp_impl_t* _this){
    _this->m_force_auth_arg = NULL;
    _this->m_force_auth_id = -1;
    _this->m_force_auth_event = NULL;
}

static void sm_bp_on_sync_info(int32_t _id, SM_BP_CMD _cmd, int32_t _success, void* _cmd_data, void* _arg){

    (void)_cmd_data;
    sm_sv_bp_impl_t* _this = (sm_sv_bp_impl_t*)_arg;
    char temp[BP_DEVICE_SN_SIZE];

    if(_success == SM_BP_CMD_FAILURE){
        if(!_this->m_bps[_id].m_is_connected){
            sm_sv_bp_reset_current_cmd(_this);
            return;
        }
        if(_cmd == BP_CMD_READ_SN){
            sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_SN, _this->m_temp, sm_bp_on_sync_info, _this);
        }else if(_cmd == BP_CMD_READ_VERSION){
            sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_VERSION, _this->m_temp, sm_bp_on_sync_info, _this);
        }else if(_cmd == BP_CMD_READ_ASSIGNED_DEV){
            sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_ASSIGNED_DEV, _this->m_temp, sm_bp_on_sync_info, _this);
        }
    }else{
        if(_cmd == BP_CMD_READ_SN){
            memset(temp, '\0', BP_DEVICE_SN_SIZE);
            sm_bp_verify_sn((const char*)_this->m_temp, temp);
            sm_bp_reverse_sn(temp, _this->m_bps[_id].m_data.m_sn);
            for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
                if(_this->m_event_handlers[index].m_event_cb && _this->m_event_handlers[index].m_event_cb->on_bp_connected){
                        _this->m_event_handlers[index].m_event_cb->on_bp_connected(_id,
                                                                                   _this->m_bps[_id].m_data.m_sn,
                                                                                   _this->m_bps[_id].m_data.m_soc,
                                                                                   _this->m_event_handlers[index].m_event_arg);
                 }
           }
        }else if(_cmd == BP_CMD_READ_VERSION){
            memcpy(_this->m_bps[_id].m_data.m_version, _this->m_temp, BP_DEVICE_SN_SIZE);
        }else if(_cmd == BP_CMD_READ_ASSIGNED_DEV){
            memcpy(_this->m_bps[_id].m_data.m_assignedSn, _this->m_temp, BP_DEVICE_SN_SIZE);
        }
    }
    sm_sv_bp_reset_current_cmd(_this);
}

static void sm_bp_sync_info(sm_sv_bp_impl_t* _this, int32_t _id){
    sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_SN, _this->m_temp, sm_bp_on_sync_info, _this);
    sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_VERSION, _this->m_temp, sm_bp_on_sync_info, _this);
    sm_sv_bp_set_cmd(_this, _id, BP_CMD_READ_ASSIGNED_DEV, _this->m_temp, sm_bp_on_sync_info, _this);
}

 static void sm_bp_auth_event_handle(int32_t _id, SM_BP_AUTH_EVENT _event, const char* _sn, int32_t _soc, void* _arg){
    sm_sv_bp_impl_t* bp_service = (sm_sv_bp_impl_t*)(_arg);
    if(!bp_service){
        return;
    }
    sm_bp_t* bp = &bp_service->m_bps[_id];
    if(_event == BP_AUTH_SUCCESS){
        bp->m_is_connected = 1;
        elapsed_timer_reset(&bp->m_timeout);

        for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
            if(bp_service->m_event_handlers[index].m_event_cb && bp_service->m_event_handlers[index].m_event_cb->on_bp_connected){
                bp_service->m_event_handlers[index].m_event_cb->on_bp_connected(_id,
                                                                                _sn,
                                                                                _soc,
                                                                                bp_service->m_event_handlers[index].m_event_arg);
            }
        }
        sm_sv_bp_set_cmd(bp_service, _id, BP_CMD_READ_VERSION, bp_service->m_temp, sm_bp_on_sync_info, bp_service);
        sm_sv_bp_set_cmd(bp_service, _id, BP_CMD_READ_ASSIGNED_DEV, bp_service->m_temp, sm_bp_on_sync_info, bp_service);
    }
    else if (_event == BP_AUTH_FAILURE) {

        for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
            if(bp_service->m_event_handlers[index].m_event_cb && bp_service->m_event_handlers[index].m_event_cb->on_bp_connected){
                bp_service->m_event_handlers[index].m_event_cb->bp_assgin_fail(_id,bp_service->m_event_handlers[index].m_event_arg);
            }
        }
	}

    if(_id == bp_service->m_force_auth_id && bp_service->m_force_auth_event){
        bp_service->m_force_auth_event(_id,
                                        _event,
                                        _sn,
                                        _soc, 
                                        bp_service->m_force_auth_arg);

        sm_bp_reset_force_assigning(bp_service);
    }
 }

void sm_bp_co_finish_cmd_callback(const sm_bp_cmd_t* _cmd, int32_t _status, void* _arg){

    (void) _status;
    (void)_cmd;
    sm_sv_bp_impl_t* bp_service = (sm_sv_bp_impl_t*)(_arg);
    if(!bp_service){
        return;
    }
    LOG_INF(TAG, "Finish current cmd %d", _cmd->m_cmd);
    sm_sv_bp_reset_current_cmd(bp_service);
}

static void sm_bp_co_update_data_callback(int32_t _id, const sm_bp_data_t* _data, void* _arg){
    sm_sv_bp_impl_t* bp_service = (sm_sv_bp_impl_t*)(_arg);
    if(!bp_service){
        return;
    }

    if(!bp_service->m_auth){ // NOT master
        if(!bp_service->m_bps[_id].m_is_connected && bp_service->m_bps[_id].m_data.m_soc >= 0){
            bp_service->m_bps[_id].m_is_connected = 1;
            elapsed_timer_reset(&bp_service->m_bps[_id].m_timeout);
            for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
                if(bp_service->m_event_handlers[index].m_event_cb && bp_service->m_event_handlers[index].m_event_cb->on_bp_connected){
                    bp_service->m_event_handlers[index].m_event_cb->on_bp_connected(_id,
                                                                                    NULL,
                                                                                    _data->m_soc,
                                                                                    bp_service->m_event_handlers[index].m_event_arg);
                }
            }
            if(!bp_service->m_active_low_data_mode){
                sm_bp_sync_info(bp_service, _id);
            }
        }
    }else{	// Master
        /// TODO: Handle after
    }

	if (!bp_service->m_bps[_id].m_is_connected
			&& bp_service->m_bps[_id].m_data.m_soc >= 0) {
		bp_service->m_bps[_id].m_is_connected = 1;
		elapsed_timer_reset(&bp_service->m_bps[_id].m_timeout);
		for (int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++) {
			if (bp_service->m_event_handlers[index].m_event_cb
					&& bp_service->m_event_handlers[index].m_event_cb->on_bp_update_data) {
				bp_service->m_event_handlers[index].m_event_cb->on_bp_update_data(
						_id, _data,
						bp_service->m_event_handlers[index].m_event_arg);
			}
		}
	}

}

void sm_sv_bp_update_connect_state(int32_t _id, void *_arg) {
	sm_sv_bp_impl_t *bp_service = (sm_sv_bp_impl_t*) (_arg);
	if (!bp_service) {
		return;
	}

	bp_service->m_bps[_id].m_is_connected = 1;

}

sm_sv_bp_t* sm_sv_bp_create(int32_t _bp_num, sm_co_t* _co, bool _auth_master, void* _auth_master_if){
    if(_bp_num < 0 || !_co){
        LOG_ERR(TAG, "Created BP Service FAILURE, Parameter INVALID");
        return NULL;
    }
    g_sv_bp_default.m_numb = _bp_num;
    g_sv_bp_default.m_active_low_data_mode = false;

    for(uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++){
        g_sv_bp_default.m_bps[index].m_id = index;
    	sm_bp_reset(&g_sv_bp_default.m_bps[index]);
    }
    memset(g_sv_bp_default.m_temp, 0, BP_DEVICE_SN_SIZE);

    g_sv_bp_default.m_cmd_head = 0;
    g_sv_bp_default.m_cmd_tail = 0;
    g_sv_bp_default.m_current_cmd = NULL;
    for(uint8_t index = 0; index < BP_CMD_QUEUE_SIZE; index++){
        g_sv_bp_default.m_cmds[index].m_arg = NULL;
        g_sv_bp_default.m_cmds[index].m_cb = NULL;
        g_sv_bp_default.m_cmds[index].m_cmd = BP_CMD_NUMBER;
        g_sv_bp_default.m_cmds[index].m_id = -1;
        g_sv_bp_default.m_cmds[index].m_data = NULL;
    }

    g_sv_bp_default.m_co_if = sm_co_get_if(_co);


    sm_bp_co_cb_t bp_co_cb = {
            .finish_cmd_cb = sm_bp_co_finish_cmd_callback,
            .update_data_cb = sm_bp_co_update_data_callback,
            .arg = &g_sv_bp_default
    };

    g_sv_bp_default.m_bp_co = sm_bp_co_create(_co,
                                            g_sv_bp_default.m_bps,
                                            (uint8_t)g_sv_bp_default.m_numb,
                                            bp_co_cb);

    if(!g_sv_bp_default.m_bp_co){
        LOG_ERR(TAG, "Created BP Service FAILURE, Could NOT create CANOPEN");
        return NULL;
    }

    if(_auth_master){
        g_sv_bp_default.m_auth = sm_bp_auth_create(sm_co_get_if(_co),
        											g_sv_bp_default.m_bp_co,
                                                   _auth_master_if,
                                                   sm_bp_auth_event_handle,
                                                   &g_sv_bp_default);
    }

    g_sv_bp_default.m_bp_retain = sm_bp_retain_create(g_sv_bp_default.m_bps, &g_sv_bp_default);

    if(!g_sv_bp_default.m_bp_retain){
        LOG_ERR(TAG, "Created BP REATAIN DISCHARGER Service FAILURE, Could NOT create CANOPEN");
        return NULL;
    }

    return &g_sv_bp_default;
}

int32_t sm_sv_bp_destroy(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_sv_bp_reg_event(sm_sv_bp_t* _this, const sm_sv_bp_event_cb_t* _event_cb_fn, void* _arg){
    if(!_this){
        return -1;
    }
    for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
        if(_impl(_this)->m_event_handlers[index].m_event_cb == NULL){
            _impl(_this)->m_event_handlers[index].m_event_cb = _event_cb_fn;
            _impl(_this)->m_event_handlers[index].m_event_arg = _arg;
            return 0;
        }
    }
    return -1;
}

int32_t sm_sv_bp_get_number(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    return _impl(_this)->m_numb;
}

int32_t sm_sv_bp_force_get_assigned_dev(sm_sv_bp_t* _this, int32_t _id){
    return sm_sv_bp_set_cmd(_this,
                            _id,
                            BP_CMD_READ_ASSIGNED_DEV,
                            _impl(_this)->m_temp,
                            sm_bp_on_sync_info,
                            _this);
}

const sm_bp_data_t* sm_sv_bp_get_data(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _impl(_this)->m_numb <= _id){
        return NULL;
    }
    return &_impl(_this)->m_bps[_id].m_data;
}

sm_bp_t* sm_sv_bp_get_obj(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _impl(_this)->m_numb <= _id){
        return NULL;
    }
    return &_impl(_this)->m_bps[_id];
}
void* sm_sv_bp_retain_get_obj(sm_sv_bp_t* _this){

    if(!_this){
        return NULL;
    }
    return _impl(_this)->m_bp_retain;
}
int32_t sm_sv_bp_reset(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _impl(_this)->m_numb <= _id){
        return -1;
    }
    LOG_WRN(TAG, "Removed bp %d data", _id);
    sm_bp_reset(&_impl(_this)->m_bps[_id]);
    return 0;
}

int32_t sm_sv_bp_auth(sm_sv_bp_t* _this, 
                      int32_t _id,
                      sm_bp_auth_event_fn_t _cb,
                      void* _arg){
    if(!_this || !_impl(_this)->m_auth){
        return -1;
    }            
    _impl(_this)->m_force_auth_id = (int8_t)_id;
    _impl(_this)->m_force_auth_event = _cb;
    _impl(_this)->m_force_auth_arg = _arg;

    return sm_bp_auth_start_auth(_impl(_this)->m_auth, &_impl(_this)->m_bps[_id]);
}

int32_t auth_bp_id = -1;
int32_t sm_sv_first_bp_auth(sm_sv_bp_t* _this,
                      int32_t _id,
                      sm_bp_auth_event_fn_t _cb,
                      void* _arg){
    if(!_this || !_impl(_this)->m_auth){
        return -1;
    }
    _impl(_this)->m_force_auth_id = (int8_t)_id;
    _impl(_this)->m_force_auth_event = _cb;
    _impl(_this)->m_force_auth_arg = _arg;
    auth_bp_id = _id;
    return sm_bp_auth_first_bp_auth(_impl(_this)->m_auth, &_impl(_this)->m_bps[_id]);
}

int32_t sm_sv_bp_is_authenticating(sm_sv_bp_t* _this,
                                int32_t _id){
    if(!_this || !_impl(_this)->m_auth){
        return 0;
    }
    if(_impl(_this)->m_bps[_id].m_is_connected){
        return 0;
    }
    return _id == sm_bp_auth_get_bp_authenticating(_impl(_this)->m_auth);
}

int32_t sm_sv_bp_is_connected(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _id >= _impl(_this)->m_numb){
        return 0;
    }
    return _impl(_this)->m_bps[_id].m_is_connected;
}
int32_t sm_sv_bp_get_bp_numbs_assign(sm_sv_bp_t *_this) {

	if (!_this)
		return -1;

	int32_t assign_numbs = 0;
	for (uint8_t index = 0; index < _impl(_this)->m_numb; index++) {
		if (!_impl(_this)->m_bps[index].m_is_connected) {
			continue;
		}
		assign_numbs++;

	}
	return assign_numbs;
}
int32_t sm_sv_bp_set_cmd(sm_sv_bp_t* _this,
                        int32_t _id, 
                        SM_BP_CMD _cmd, 
                        void* _data,
                        sm_bp_on_cmd_fn_t _cmd_cb,
                        void* _arg){
    if(!_this || _id >= _impl(_this)->m_numb || !_impl(_this)->m_bps[_id].m_is_connected){
        if(_cmd_cb){
            _cmd_cb(_id, _cmd, SM_BP_CMD_FAILURE, _data, _arg);
        }
        return -1;
    }
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_id = _id;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cmd = _cmd;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_data = _data;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cb = _cmd_cb;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_arg = _arg;

    _impl(_this)->m_cmd_head++;
    if(_impl(_this)->m_cmd_head >= BP_CMD_QUEUE_SIZE){
        _impl(_this)->m_cmd_head = 0;
    }
    return 0;
}

int32_t sm_sv_bp_set_off_all(sm_sv_bp_t* _this){
    for(int index = 0; index < _impl(_this)->m_numb; index++){
        if(!_impl(_this)->m_bps[index].m_is_connected){
            continue;
        }
        _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_id = index;
        _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cmd = BP_CMD_STANDBY;
        _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_data = NULL;
        _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cb = NULL;
        _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_arg = NULL;

        _impl(_this)->m_cmd_head++;
        if(_impl(_this)->m_cmd_head >= BP_CMD_QUEUE_SIZE){
            _impl(_this)->m_cmd_head = 0;
        }
    }
    return 0;
}

int32_t sm_sv_bp_reset_current_cmd(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_current_cmd = NULL;
    return 0;
}

int32_t sm_sv_bp_check_pre_force_discharging_condition(sm_sv_bp_t* _this, int32_t _id){
    if(!_this){
        return -1;
    }

    sm_sv_bp_impl_t* this = _this;
    char build_ver = this->m_bps[_id].m_data.m_version[SM_BP_BUILD_VER_POS];

    if(!this->m_bps[_id].m_is_connected){
        return -1;
    }
    if(build_ver < LOWEST_BP_BUILD_VER_SUPPORT_FORCE_DISCHARGING){
        return -1;
    }
    if(this->m_bps[_id].m_data.m_state != BP_STATE_DISCHARGING){
        return -1;
    }
    return 0;
}

int32_t sm_sv_bp_enter_low_data_mode(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    sm_sv_bp_impl_t* this = _this;
    this->m_active_low_data_mode = true;
    LOG_INF(TAG, "Enter low data mode, reset all command remain");
    sm_sv_bp_remove_all_cmd(this);
    return 0;
}

int32_t sm_sv_bp_exit_low_data_mode(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    sm_sv_bp_impl_t* this = _this;
    this->m_active_low_data_mode = false;
    LOG_INF(TAG, "Exit low data mode, reset all command remain");
    sm_sv_bp_remove_all_cmd(this);
    return 0;
}

int32_t sm_sv_bp_remove_all_cmd(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    sm_sv_bp_impl_t* this = _this;
    this->m_cmd_head = this->m_cmd_tail = 0;
    sm_sv_bp_reset_current_cmd(this);
    return 0;
}
static int32_t sm_sv_bp_check_err(sm_sv_bp_impl_t* _this){

    if(!_this){
        return -1;
    }
	uint8_t bp_retain_numbs = (uint8_t)sm_bp_get_active_retain_numbs(_this->m_bp_retain);
	uint8_t bp_dischar_numbs = 0;

    for(uint8_t i = 0; i < _this->m_numb; ++ i) {
        if(!_this->m_bps[i].m_is_connected){
            continue;
        }
        if(_this->m_bps[i].m_data.m_state == BP_STATE_DISCHARGING){
        	bp_dischar_numbs++;
        }
    }

    if(bp_retain_numbs && bp_dischar_numbs){

    	return -1;
    }else if (bp_retain_numbs && !bp_dischar_numbs) {

    	return -2;
	}
	return 0;
}

int32_t sm_sv_bp_process(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    sm_sv_bp_impl_t* this = _this;

    if(_impl(_this)->m_auth){
        sm_bp_auth_process(_impl(_this)->m_auth);
    }
    if(_impl(_this)->m_bp_retain){
        sm_bp_retain_process(this->m_bp_retain);
    }
//    if(_impl(_this)->m_bp_co){
//        sm_bp_co_process(this->m_bp_co);
//    }

    sm_bp_t* bp = NULL;
    for(int32_t index = 0; index < this->m_numb; index++){
        bp = &this->m_bps[index];
        if(!bp->m_is_connected){
            continue;
        }

        if(!elapsed_timer_get_remain(&bp->m_timeout) && !this->m_active_low_data_mode){
        	bp->m_is_connected = 0;
            for(int i = 0; i < SM_SV_BP_EVENT_CB_MAX; i++){
                if(this->m_event_handlers[i].m_event_cb && this->m_event_handlers[i].m_event_cb->on_bp_disconnected){
                    this->m_event_handlers[i].m_event_cb->on_bp_disconnected(bp->m_id, bp->m_data.m_sn,
                                                                                     this->m_event_handlers[i].m_event_arg);
                }
            }

            sm_bp_reset(bp);

            for(int i = 0; i < SM_SV_BP_EVENT_CB_MAX; i++){
                if(this->m_event_handlers[i].m_event_cb && this->m_event_handlers[i].m_event_cb->on_bp_update_data){
                    this->m_event_handlers[i].m_event_cb->on_bp_update_data(bp->m_id, &bp->m_data,
                                                                                    this->m_event_handlers[i].m_event_arg);
                }
            }
        }
    }

    if(!sm_bp_co_is_busy(this->m_bp_co) && !this->m_current_cmd && this->m_cmd_head != this->m_cmd_tail){
        this->m_current_cmd = &this->m_cmds[this->m_cmd_tail];

        this->m_cmd_tail++;
        if(this->m_cmd_tail >= BP_CMD_QUEUE_SIZE){
            this->m_cmd_tail = 0;
        }
        LOG_DBG(TAG, "CURRENT_CMD: ID: %d, CMD: %d",this->m_current_cmd->m_id, this->m_current_cmd->m_cmd);
        /// Process CMD
        sm_bp_co_set_cmd(this->m_bp_co, this->m_current_cmd);
    }

    return sm_sv_bp_check_err(this);
}                   
