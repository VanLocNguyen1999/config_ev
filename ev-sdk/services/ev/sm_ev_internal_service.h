//
// Created by vnbk on 30/08/2024.
//

#ifndef EV_SDK_SM_EV_INTERNAL_SERVICE_H
#define EV_SDK_SM_EV_INTERNAL_SERVICE_H

#include "sm_ev_cmd.h"
#include "sm_ev_service.h"
#ifdef __cplusplus
extern  "C"{
#endif
int32_t sm_ev_service_notify_ev_event(sm_sv_ev_t* _this, int32_t _event, int32_t _value);
int32_t sm_ev_service_notify_bp_event(sm_sv_ev_t* _this, int32_t _port, int32_t _event, int32_t _value);
#ifdef __cplusplus
};
#endif
#endif //EV_SDK_SM_EV_INTERNAL_SERVICE_H
