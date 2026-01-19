//
// Created by vnbk on 01/10/2024.
//

#ifndef EV_SDK_SM_INACTIVE_MODE_H
#define EV_SDK_SM_INACTIVE_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_storage.h"
#include "sm_ev_data.h"
#include "sm_ev_service.h"

#define SM_INACTIVE_MODE_KM_WARNING_DEFAULT     20 // km
#define SM_INACTIVE_MODE_KM_EXECUTED_DEFAULT    30 //km
#define SM_INACTIVE_MODE_DEFAULT                EV_ACTIVE_STATE

#define SM_INACTIVE_MODE_KM_WARNING_MAX         50
#define SM_INACTIVE_MODE_KM_EXECUTED_MAX        200

typedef void sm_inactive_mode_t;

enum {
    SM_EV_INACTIVE_RESET,
    SM_EV_INACTIVE_WARNING,
    SM_EV_INACTIVE_EXECUTED,
};

typedef void (*sm_inactive_mode_on_event_fn_t)(int32_t, void*);

sm_inactive_mode_t* sm_inactive_mode_create(sm_sv_ev_t* _ev_service,
                                            sm_storage_t* _storage,
                                            sm_ev_inactive_mode_config_t* _config,
                                            sm_inactive_mode_on_event_fn_t _cb,
                                            void* _arg);

int32_t sm_inactive_mode_destroy(sm_inactive_mode_t* _this);

int32_t sm_inactive_mode_update_net_state(sm_inactive_mode_t* _this, int32_t _net_state);

int32_t sm_inactive_mode_reset(sm_inactive_mode_t* _this);

int32_t sm_inactive_mode_process(sm_inactive_mode_t* _this);


#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_INACTIVE_MODE_H
