/*
 * pmu_io_handle.c
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */
#include "pmu_io_handle.h"
#include "sm_logger.h"

#include "sm_bsp_pmu.h"
#include "sm_pmu_app.h"
#include "ev_io_define.h"

#define TAG "PMU_IO_HANDLE"

typedef int8_t (*sm_pmu_io_event_handle_fn_t)(uint8_t, void*);
static sm_sv_io_if_t g_pmu_io_if = {
        .in_12v_det_fn_t = sm_bsp_pmu_io_get_12v_det,
        .out_actice_12V_fn_t = sm_bsp_pmu_io_set_12V_power,
};

sm_sv_io_if_t* sm_pmu_get_io_if(){
    return &g_pmu_io_if;
}

static int8_t sm_pmu_io_set_12v_power(uint8_t _value, void* _arg){

    sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    uint8_t cmd =  (_value == OP_STATE_FORCE) ? ACTIVE : INACTIVE;
    if(app->m_ev_io_service->m_12v_det.state == cmd) return -1;
    sm_sv_io_force(&app->m_ev_io_service->m_pmu_io_service, OUT_12V_ACT, cmd);
    return 0;
}

static sm_pmu_io_event_handle_fn_t g_pmu_io_event_handle[SM_PMU_IO_EVENT_NUMBER] = {

		sm_pmu_io_set_12v_power,
};

int8_t sm_pmu_io_set_event(uint8_t _cmd, uint8_t _value,void *_arg) {

    int8_t ret = g_pmu_io_event_handle[_cmd](_value,_arg);
    return ret;
}

void sm_pmu_io_changed(uint8_t _event, uint8_t _value, void *_arg) {

	//    LOG_DBG(TAG, "PMU IO Event %s : %d", sm_sv_io_event_to_string(_event), _value);
	(void) _arg;
	(void) _value;
	switch (_event) {
	case SM_SV_IO_EVENT_12V_DET_CHANGED:

		break;
	case SM_SV_IO_EVENT_ACTIVE_12V_CHANGED:

		break;
	}
}

