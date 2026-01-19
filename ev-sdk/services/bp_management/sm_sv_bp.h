#ifndef SM_SV_BP_H
#define SM_SV_BP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_sv_bp_event.h"
#include "sm_co_if.h"
#include "sm_bp_auth.h"
#include "sm_core_co.h"

#define SM_SV_BP_NUMBER_DEFAULT SM_BP_NUMBER_DEFAULT

#define LOWEST_BP_BUILD_VER_SUPPORT_FORCE_DISCHARGING      18

typedef void sm_sv_bp_t;

sm_sv_bp_t* sm_sv_bp_create(int32_t _bp_num, sm_co_t* _co, bool _auth_master, void* _auth_master_if);

int32_t sm_sv_bp_destroy(sm_sv_bp_t* _this);

int32_t sm_sv_bp_reg_event(sm_sv_bp_t* _this, const sm_sv_bp_event_cb_t* _event_cb_fn, void* _arg);

int32_t sm_sv_bp_get_number(sm_sv_bp_t* _this);

int32_t sm_sv_bp_force_get_assigned_dev(sm_sv_bp_t* _this, int32_t _id);

const sm_bp_data_t* sm_sv_bp_get_data(sm_sv_bp_t* _this, int32_t _id);
sm_bp_t* sm_sv_bp_get_obj(sm_sv_bp_t* _this, int32_t _id);
void* sm_sv_bp_retain_get_obj(sm_sv_bp_t* _this);

int32_t sm_sv_bp_reset(sm_sv_bp_t* _this, int32_t _id);

int32_t sm_sv_bp_auth(sm_sv_bp_t* _this, 
                        int32_t _id,
                        sm_bp_auth_event_fn_t _cb,
                        void* _arg);
int32_t sm_sv_first_bp_auth(sm_sv_bp_t* _this,
                        int32_t _id,
                        sm_bp_auth_event_fn_t _cb,
                        void* _arg);
int32_t sm_sv_bp_is_authenticating(sm_sv_bp_t* _this,
                                int32_t _id);

int32_t sm_sv_bp_is_connected(sm_sv_bp_t* _this, int32_t _id);

int32_t sm_sv_bp_get_bp_numbs_assign(sm_sv_bp_t *_this);

int32_t sm_sv_bp_set_cmd(sm_sv_bp_t* _this,
                        int32_t _id, 
                        SM_BP_CMD _cmd, 
                        void* _data,
                        sm_bp_on_cmd_fn_t _cmd_cb,
                        void* _arg);

int32_t sm_sv_bp_set_off_all(sm_sv_bp_t* _this);

int32_t sm_sv_bp_reset_current_cmd(sm_sv_bp_t* _this);

int32_t sm_sv_bp_enter_low_data_mode(sm_sv_bp_t* _this);

int32_t sm_sv_bp_exit_low_data_mode(sm_sv_bp_t* _this);

int32_t sm_sv_bp_remove_all_cmd(sm_sv_bp_t* _this);

int32_t sm_sv_bp_process(sm_sv_bp_t* _this);

int32_t sm_sv_bp_check_pre_force_discharging_condition(sm_sv_bp_t* _this, int32_t _id);

void sm_bp_co_finish_cmd_callback(const sm_bp_cmd_t* _cmd, int32_t _status, void* _arg);

void sm_sv_bp_update_connect_state(int32_t _id, void *_arg);
#ifdef __cplusplus
}
#endif


#endif
