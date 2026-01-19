//
// Created by admin on 1/3/2025.
//

#include "sm_sv_pms.h"
#include "sm_bp_data.h"
#include "sm_logger.h"
#include "sm_math.h"

#define TAG "SM_SV_PMS"

#define _impl(x) ((sm_sv_pms_impl_t*)(x))

typedef struct {
    sm_sv_bp_t              *m_bp;
    sm_pms_ctl_t            *m_ctl;
    est_data_t              *m_est;

    sm_sv_discharger_t      *m_discharger;
    sm_sv_charger_t      	*m_charger;

    PMS_MODE				 m_mode;
} sm_sv_pms_impl_t;

static sm_sv_pms_impl_t g_sv_pms = {
        .m_est = NULL,
        .m_bp = NULL,
        .m_ctl = NULL,
		.m_discharger = NULL,
		.m_charger = NULL,
		.m_mode = PMS_DISCHARGER,
};
/**************************************************************************************/

sm_sv_pms_t* sm_sv_pms_create(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp, sm_sv_discharger_t *_discharger,
        sm_sv_charger_t *_charger, est_data_t *_est) {
    if (!_bp || !_ctl){
        return NULL;
    }
    sm_sv_pms_impl_t *this = &g_sv_pms;

    this->m_bp = _bp;
    this->m_ctl = _ctl;
    this->m_charger = _charger;
    this->m_discharger = _discharger;
    this->m_est = _est;
    return (sm_sv_pms_t*)this;
}

int32_t sm_sv_pms_reinit(sm_pms_ctl_t *_ctl, sm_sv_bp_t *_bp,
		sm_sv_discharger_t *_discharger, sm_sv_charger_t *_charger) {

	if(_ctl){
		g_sv_pms.m_ctl = _ctl;
    }
    if(_bp){
    	g_sv_pms.m_bp = _bp;
    }
    if(_discharger){
    	g_sv_pms.m_discharger = _discharger;
    }
    if(_charger){
    	g_sv_pms.m_charger = _charger;
    }
    return 0;
}

int32_t sm_sv_pms_destroy(sm_sv_pms_t* _this) {
    if(!_this){
        return -1;
    }
    _impl(_this)->m_bp = NULL;
    _impl(_this)->m_ctl = NULL;
    _impl(_this)->m_charger = NULL;
    _impl(_this)->m_discharger = NULL;
    return 0;
}

int32_t sm_sv_pms_process(sm_sv_pms_t *_this) {
	if (!_this) {
		return -1;
	}
	pms_est_process(_impl(_this)->m_est);
	sm_pms_ctl_process(_impl(_this)->m_ctl);
	if(sm_sv_charger_process(_impl(_this)->m_charger) < 0){

		_impl(_this)->m_mode = PMS_DISCHARGER;
	}else {
		_impl(_this)->m_mode = PMS_CHARGER;
	}

	if (_impl(_this)->m_mode == PMS_DISCHARGER) {

		sm_sv_discharger_process(_impl(_this)->m_discharger);
	}
	return sm_sv_charger_is_err(_impl(_this)->m_charger);
}

PMS_MODE sm_sv_pms_get_mode(void) {
    return g_sv_pms.m_mode;
}
