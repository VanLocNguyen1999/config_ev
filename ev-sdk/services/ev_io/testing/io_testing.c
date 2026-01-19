//
// Created by vnbk on 20/01/2025.
//
#include "sm_bsp_pmu.h"
#include "sm_sv_bt.h"
#include "sm_sv_io.h"
#include "sm_core_sch.h"
#include "sm_logger.h"

#define TESTING_TAG     "IO_TESTING"

static sm_sch_t* g_sch = NULL;

void test_force_blink_io(void* _arg);
void test_force_cancel_blink_io(void* _arg);
void test_force_io(void* _arg){
    static uint8_t g_io_id = IO_LEFT_SIGNAL;
    static uint8_t g_force = true;
    static uint8_t g_time = 0;

    sm_sv_io_force(_arg, g_io_id, g_force);
    g_io_id++;
    if(g_io_id >= IO_NUMBER){
        g_io_id = IO_LEFT_SIGNAL;
        g_force = !g_force;
        g_time++;
    }

    if(g_time >= 5){
        for(int index = 0; index < IO_NUMBER; index++){
            sm_sv_io_release(_arg, g_io_id);
        }
        sm_sch_cancel_taskz(g_sch, test_force_io);
        sm_sch_start_task(g_sch, 1000, SM_SCH_REPEAT_FOREVER, test_force_blink_io, _arg);
    }
}

void test_force_blink_io(void* _arg){
    static uint8_t g_io_id = IO_LEFT_SIGNAL;

    sm_sv_io_blink(_arg, g_io_id, 10000, 200);
    g_io_id++;
    if(g_io_id >= IO_NUMBER){
        sm_sch_cancel_taskz(g_sch, test_force_blink_io);
        sm_sch_start_task(g_sch, 1000, SM_SCH_REPEAT_FOREVER, test_force_cancel_blink_io, _arg);
    }
}

void test_force_cancel_blink_io(void* _arg){
    static uint8_t g_io_id = IO_LEFT_SIGNAL;

    sm_sv_io_cancel_blink(_arg, g_io_id);

    g_io_id++;
    if(g_io_id >= IO_NUMBER){
        sm_sch_cancel_taskz(g_sch, test_force_cancel_blink_io);
    }
}


static sm_sv_io_if_t g_io_interface = {
        .in_key_fn_t = sm_bsp_pmu_io_get_key,
        .in_parking_fn_t = sm_bsp_pmu_io_get_parking_bt,
        .in_drive_mode_fn_t = sm_bsp_pmu_io_get_drive_mode_bt,
        .in_emergency_fn_t = sm_bsp_pmu_io_get_emergency_bt,
        .in_left_signal_fn_t = sm_bsp_pmu_io_get_left_light_sw,
        .in_right_signal_fn_t = sm_bsp_pmu_io_get_right_light_sw,
        .in_phase_light_fn_t = sm_bsp_pmu_io_get_phase_light_sw,
        .in_cos_light_fn_t = sm_bsp_pmu_io_get_cos_light_sw,
        .in_head_light_fn_t = sm_bsp_pmu_io_get_head_light_sw,
        .in_horn_fn_t = sm_bsp_pmu_io_get_horn_bt,

        .out_left_light_fn_t = sm_bsp_pmu_io_set_left_light,
        .out_right_light_fn_t = sm_bsp_pmu_io_set_right_light,
        .out_phase_light_fn_t = sm_bsp_pmu_io_set_high_beam,
        .out_cos_light_fn_t = NULL,
        .out_head_light_fn_t = NULL,
        .out_emergency_fn_t = sm_bsp_pmu_io_set_emergency,
        .out_horn_fn_t = sm_bsp_pmu_io_set_horn
};

static void sm_sv_io_event_cb(uint8_t _id, uint8_t _value, void* _arg){
    (void)(_arg);
    LOG_INF(TESTING_TAG, "Event %s data: %d", sm_sv_io_event_to_string(_id), _value);
}

int main(){
    sm_bsp_pmu_init();

    g_sch = sm_sch_create_default();

    sm_sv_io_t* ev_io_service = sm_sv_io_create(&g_io_interface);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_KEY_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_LEFT_SIGNAL_LIGHT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_LEFT_SIGNAL_SW_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_RIGHT_SIGNAL_LIGHT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_RIGHT_SIGNAL_SW_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_EMERGENCY_BT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_EMERGENCY_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_PHASE_LIGHT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_PHASE_LIGHT_SW_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_COS_LIGHT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_COS_LIGHT_SW_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_HEAD_LIGHT_SW_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_HEAD_LIGHT_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_HORN_BT_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_HORN_CHANGED, sm_sv_io_event_cb, NULL);

    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_PARKING_CHANGED, sm_sv_io_event_cb, NULL);
    sm_sv_io_reg_event(ev_io_service, SM_SV_IO_EVENT_DRIVE_MODE_CHANGED, sm_sv_io_event_cb, NULL);

/*	sm_sv_bt_t* bt_service = sm_sv_bt_create(NULL);
	g_bt_parking_id = sm_sv_bt_add_bt_if(bt_service, sm_bt_parking_if);
	g_bt_mode_id = sm_sv_bt_add_bt_if(bt_service, sm_bt_mode_if);

	sm_sv_bt_reg_event(bt_service,
						g_bt_parking_id,
						sm_sv_bt_event_cb, NULL);

	sm_sv_bt_reg_event(bt_service,
						g_bt_mode_id,
						sm_sv_bt_event_cb, NULL);*/

    sm_sch_start_task(g_sch, 0, SM_SCH_REPEAT_FOREVER, sm_sv_io_process, ev_io_service);

    sm_sch_start_task(g_sch, 1000, SM_SCH_REPEAT_FOREVER, test_force_io, ev_io_service);

    while(1){
        sm_sch_process(g_sch);
    }

    return 0;
}