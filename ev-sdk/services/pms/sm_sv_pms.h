//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_PMS_H
#define EV_SDK_SM_SV_PMS_H

#include "sm_sv_bp.h"
#include "sm_pms_est.h"
#include "sm_pms_controller.h"
#include "sm_sv_charger.h"
#include "sm_sv_discharger.h"

typedef enum {

	PMS_DISCHARGER = 0,
	PMS_CHARGER,
}PMS_MODE;

typedef void sm_sv_pms_t;

sm_sv_pms_t* sm_sv_pms_create(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp, sm_sv_discharger_t *_discharger,
        sm_sv_charger_t *_charger, est_data_t *_est);
int32_t sm_sv_pms_reinit(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp,
		sm_sv_discharger_t *_discharger, sm_sv_charger_t *_charger);
int32_t sm_sv_pms_destroy(sm_sv_pms_t* _this);
int32_t sm_sv_pms_process(sm_sv_pms_t* _this) ;

PMS_MODE sm_sv_pms_get_mode(void);

#endif //EV_SDK_SM_SV_PMS_H
