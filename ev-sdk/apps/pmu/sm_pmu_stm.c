//
// Created by vnbk on 27/05/2024.
//
#include "sm_pmu_stm.h"
#include "sm_pmu_app.h"
#include "sm_bp_retain.h"
#include "sm_logger.h"

#include "sm_stm.h"

#define TAG "SM_PMU_STM"

#define TIMOUT_TO_SLEEP_MODE								7200000
#define UPGRADING_STATE_TIMEOUT								120000 // 120s

void sm_pmu_app_stm_proc(void* _arg){

	sm_pmu_app_t* pmu_app = _arg;
	sm_stm_process(pmu_app->m_pmu_stm);
}

int32_t sm_pmu_app_get_stm(void){
	return (sm_stm_get_current_state(g_pmu_app->m_pmu_stm));
}
/*
 * PMU State machine
 */
static int32_t sm_pmu_stm_startup_entry(void* _arg);
static int32_t sm_pmu_stm_startup_proc(void* _arg);
static int32_t sm_pmu_stm_startup_exit(void* _arg);

static int32_t sm_pmu_stm_starting_entry(void* _arg);
static int32_t sm_pmu_stm_starting_proc(void* _arg);
static int32_t sm_pmu_stm_starting_exit(void* _arg);

static int32_t sm_pmu_stm_idle_entry(void* _arg);
static int32_t sm_pmu_stm_idle_proc(void* _arg);
static int32_t sm_pmu_stm_idle_exit(void* _arg);

static int32_t sm_pmu_stm_running_entry(void* _arg);
static int32_t sm_pmu_stm_running_proc(void* _arg);
static int32_t sm_pmu_stm_running_exit(void* _arg);

static int32_t sm_pmu_stm_power_limit_entry(void* _arg);
static int32_t sm_pmu_stm_power_limit_proc(void* _arg);
static int32_t sm_pmu_stm_power_limit_exit(void* _arg);

static int32_t sm_pmu_stm_charging_entry(void* _arg);
static int32_t sm_pmu_stm_charging_proc(void* _arg);
static int32_t sm_pmu_stm_charging_exit(void* _arg);

static int32_t sm_pmu_stm_sleep_entry(void* _arg);
static int32_t sm_pmu_stm_sleep_proc(void* _arg);
static int32_t sm_pmu_stm_sleep_exit(void* _arg);

static int32_t sm_pmu_stm_upgrading_entry(void* _arg);
static int32_t sm_pmu_stm_upgrading_proc(void* _arg);
static int32_t sm_pmu_stm_upgrading_exit(void* _arg);

static int32_t sm_pmu_enter_startting_event_handle(void* _arg);
static int32_t sm_pmu_sleep_key_on_event_handle(void* _arg);
static int32_t sm_pmu_key_off_event_handle(void* _arg);
static int32_t sm_pmu_first_bp_assigned_event_handle(void* _arg);
static int32_t sm_pmu_enter_running_mode_event_handle(void* _arg);
static int32_t sm_pmu_enter_power_limit_mode_event_handle(void* _arg);
static int32_t sm_pmu_enter_idle_event_handle(void* _arg);
static int32_t sm_pmu_plug_charge_event_handle(void* _arg);
static int32_t sm_pmu_unplug_charge_event_handle(void* _arg);
static int32_t sm_pmu_charge_full_event_handle(void* _arg);
static int32_t sm_pmu_motor_stop_event_handle(void* _arg);
static int32_t sm_pmu_timeout_elapsed_event_handle(void* _arg);
static int32_t sm_pmu_enter_upgrading_mode_event_handle(void* _arg);
static int32_t sm_pmu_exit_upgrading_mode_event_handle(void* _arg);

/// Configure startup state
static int32_t sm_pmu_stm_startup_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_STARTUP,
            .entry = sm_pmu_stm_startup_entry,
            .process = sm_pmu_stm_startup_proc,
            .exit = sm_pmu_stm_startup_exit,
    };
    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_ENTER_STARTING;
    event.m_handler = sm_pmu_enter_startting_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_STARTUP, &event);
    return 0;
}

/// Configure starting state
static int32_t sm_pmu_stm_starting_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_STARTING,
            .entry = sm_pmu_stm_starting_entry,
            .process = sm_pmu_stm_starting_proc,
            .exit = sm_pmu_stm_starting_exit,
    };
    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_FIRST_BP_ASSIGNED;
    event.m_handler = sm_pmu_first_bp_assigned_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_STARTING, &event);
    return 0;
}

static int32_t sm_pmu_stm_idle_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_IDLE,
            .entry = sm_pmu_stm_idle_entry,
            .process = sm_pmu_stm_idle_proc,
            .exit = sm_pmu_stm_idle_exit,
    };

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_PLUG_CHARGER;
    event.m_handler = sm_pmu_plug_charge_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_IDLE, &event);

    event.m_id = PMU_STM_EVENT_ENTER_RUNNING_MODE;
    event.m_handler = sm_pmu_enter_running_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_IDLE, &event);

    event.m_id = PMU_STM_EVENT_ENTER_POWER_LIMIT_MODE;
    event.m_handler = sm_pmu_enter_power_limit_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_IDLE, &event);

    event.m_id = PMU_STM_EVENT_TIMEOUT;
    event.m_handler = sm_pmu_timeout_elapsed_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_IDLE, &event);

    sm_stm_add_state(_app->m_pmu_stm, &state);
    return 0;
}

static int32_t sm_pmu_stm_running_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_RUNNING,
            .entry = sm_pmu_stm_running_entry,
            .process = sm_pmu_stm_running_proc,
            .exit = sm_pmu_stm_running_exit,
    };
    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_MOTOR_STOP;
    event.m_handler = sm_pmu_motor_stop_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_RUNNING, &event);

    event.m_id = PMU_STM_EVENT_ENTER_IDLE;
    event.m_handler = sm_pmu_enter_idle_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_RUNNING, &event);

    event.m_id = PMU_STM_EVENT_TURN_OFF_KEY;
    event.m_handler = sm_pmu_key_off_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_RUNNING, &event);

    event.m_id = PMU_STM_EVENT_ENTER_POWER_LIMIT_MODE;
    event.m_handler = sm_pmu_enter_power_limit_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_RUNNING, &event);

    event.m_id = PMU_STM_EVENT_ENTER_UPGRADING_MODE;
    event.m_handler = sm_pmu_enter_upgrading_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_RUNNING, &event);
    return 0;
}

static int32_t sm_pmu_stm_power_limit_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_POWER_LIMIT,
            .entry = sm_pmu_stm_power_limit_entry,
            .process = sm_pmu_stm_power_limit_proc,
            .exit = sm_pmu_stm_power_limit_exit,
    };
    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_MOTOR_STOP;
    event.m_handler = sm_pmu_motor_stop_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_POWER_LIMIT, &event);

    event.m_id = PMU_STM_EVENT_ENTER_IDLE;
    event.m_handler = sm_pmu_enter_idle_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_POWER_LIMIT, &event);

    event.m_id = PMU_STM_EVENT_TURN_OFF_KEY;
    event.m_handler = sm_pmu_key_off_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_POWER_LIMIT, &event);

    event.m_id = PMU_STM_EVENT_ENTER_RUNNING_MODE;
    event.m_handler = sm_pmu_enter_running_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_POWER_LIMIT, &event);

    event.m_id = PMU_STM_EVENT_ENTER_UPGRADING_MODE;
    event.m_handler = sm_pmu_enter_upgrading_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_POWER_LIMIT, &event);

    return 0;
}

static int32_t sm_pmu_stm_charging_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_CHARGING,
            .entry = sm_pmu_stm_charging_entry,
            .process = sm_pmu_stm_charging_proc,
            .exit = sm_pmu_stm_charging_exit,
    };

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_UNPLUG_CHARGER;
    event.m_handler = sm_pmu_unplug_charge_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_CHARGING, &event);

    event.m_id = PMU_STM_EVENT_CHARGER_FULL;
    event.m_handler = sm_pmu_charge_full_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_CHARGING, &event);

    event.m_id = PMU_STM_EVENT_ENTER_UPGRADING_MODE;
    event.m_handler = sm_pmu_enter_upgrading_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_CHARGING, &event);

    sm_stm_add_state(_app->m_pmu_stm, &state);
    return 0;
}

/// Configure upgrading state

static int32_t sm_pmu_stm_upgrading_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_UPGRADING,
            .entry = sm_pmu_stm_upgrading_entry,
            .process = sm_pmu_stm_upgrading_proc,
            .exit = sm_pmu_stm_upgrading_exit,
    };
    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;

    event.m_id = PMU_STM_EVENT_EXIT_UPGRADING_MODE;
    event.m_handler = sm_pmu_exit_upgrading_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_UPGRADING, &event);

    return 0;
}

/// Configure Sleep State
static int32_t sm_pmu_stm_sleep_config(sm_pmu_app_t* _app){
    sm_stm_state_t state = {
            .m_arg = _app,
            .m_state = PMU_STM_SLEEP,
            .entry = sm_pmu_stm_sleep_entry,
            .process = sm_pmu_stm_sleep_proc,
            .exit = sm_pmu_stm_sleep_exit,
    };

    sm_stm_add_state(_app->m_pmu_stm, &state);

    sm_stm_event_t event ;
    event.m_id = PMU_STM_EVENT_TURN_ON_KEY;
    event.m_handler = sm_pmu_sleep_key_on_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_SLEEP, &event);

    event.m_id = PMU_STM_EVENT_ENTER_UPGRADING_MODE;
    event.m_handler = sm_pmu_enter_upgrading_mode_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_SLEEP, &event);

    event.m_id = PMU_STM_EVENT_PLUG_CHARGER;
    event.m_handler = sm_pmu_plug_charge_event_handle;
    event.m_arg = _app;
    sm_stm_config_transition(_app->m_pmu_stm, PMU_STM_SLEEP, &event);
    return 0;
}


int32_t sm_pmu_app_stm_init(void* _app){
    if(!_app){
        return -1;
    }
    sm_pmu_app_t* pmu_app = _app;
    LOG_INF(TAG, "PMU Application is initializing the state machine ....");
    elapsed_timer_resetz(&pmu_app->stm_timeout, TIMOUT_TO_SLEEP_MODE);
    sm_pmu_stm_startup_config(pmu_app);
    sm_pmu_stm_starting_config(pmu_app);
    sm_pmu_stm_idle_config(pmu_app);
    sm_pmu_stm_running_config(pmu_app);
    sm_pmu_stm_power_limit_config(pmu_app);
    sm_pmu_stm_charging_config(pmu_app);
    sm_pmu_stm_upgrading_config(pmu_app);
    sm_pmu_stm_sleep_config(pmu_app);

	sm_stm_set_current_state(pmu_app->m_pmu_stm, PMU_STM_STARTUP);
    return 0;
}

/* ----------------- startup ----------------- */
static int32_t sm_pmu_stm_startup_entry(void* _arg){

	(void)_arg;
//	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
    return 0;
}
static int32_t sm_pmu_stm_startup_proc(void* _arg){

	(void)_arg;
	return 0;
}
static int32_t sm_pmu_stm_startup_exit(void* _arg){

//	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	(void)_arg;
    return 0;
}

/* ----------------- starting ----------------- */
static int32_t sm_pmu_stm_starting_entry(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_starting_proc(void* _arg){

//	(void)_arg;
	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	int32_t bp_assign_numbs = sm_sv_bp_get_bp_numbs_assign(app->m_bp_service);
	int32_t is_plugging = sm_sv_charger_is_charging(app->m_charger_service);

	if((bp_assign_numbs > 0) && (is_plugging == SM_CHARGER_IS_PLUGGED)){

		sm_stm_set_current_state(app, PMU_STM_CHARGING);
	}else if((bp_assign_numbs > 0) && (is_plugging != SM_CHARGER_IS_PLUGGED)){

		sm_stm_set_current_state(app, PMU_STM_IDLE);
	}
    return 0;
}
static int32_t sm_pmu_stm_starting_exit(void* _arg){

	(void)_arg;
    return 0;
}
/* ----------------- idle ----------------- */
static int32_t sm_pmu_stm_idle_entry(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	elapsed_timer_resetz(&app->stm_timeout, TIMOUT_TO_SLEEP_MODE);
	sm_bp_retain_pause((sm_bp_retain_t*) sm_sv_bp_retain_get_obj(app->m_bp_service));
    return 0;
}
static int32_t sm_pmu_stm_idle_proc(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	if(app->m_ev_io_service->m_key.state == ACTIVE) {

	    elapsed_timer_resetz(&app->stm_timeout, TIMOUT_TO_SLEEP_MODE);
	}

	if (!elapsed_timer_get_remain(&app->stm_timeout)) {

	    sm_stm_set_event(PMU_STM_EVENT_TIMEOUT);
	}

	return 0;
}
static int32_t sm_pmu_stm_idle_exit(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	sm_bp_retain_resume((sm_bp_retain_t*) sm_sv_bp_retain_get_obj(app->m_bp_service));
    return 0;
}

/* ----------------- block ----------------- */
static int32_t sm_pmu_stm_running_entry(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_running_proc(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_running_exit(void* _arg){

	(void)_arg;
    return 0;
}
/* ----------------- power_limit ----------------- */
static int32_t sm_pmu_stm_power_limit_entry(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	sm_sv_discharger_set_sw_state(&app->m_discharger_service, INACTIVE);
    return 0;
}
static int32_t sm_pmu_stm_power_limit_proc(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_power_limit_exit(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	sm_sv_discharger_set_sw_state(&app->m_discharger_service,ACTIVE);
    return 0;
}
/* ----------------- starting ----------------- */
static int32_t sm_pmu_stm_charging_entry(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_charging_proc(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_charging_exit(void* _arg){

	(void)_arg;
    return 0;
}
/* ----------------- sleep ----------------- */
static int32_t sm_pmu_stm_sleep_entry(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_sleep_proc(void* _arg){

	(void)_arg;
    return 0;
}
static int32_t sm_pmu_stm_sleep_exit(void* _arg){

	(void)_arg;
    return 0;
}
/* ----------------- upgrading ----------------- */
static int32_t sm_pmu_stm_upgrading_entry(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	elapsed_timer_resetz(&app->stm_timeout, UPGRADING_STATE_TIMEOUT);
    return 0;
}
static int32_t sm_pmu_stm_upgrading_proc(void* _arg){

	sm_pmu_app_t * app = (sm_pmu_app_t*) _arg;
	if(!elapsed_timer_get_remain(&app->stm_timeout)){

		sm_stm_set_event(PMU_STM_EVENT_EXIT_UPGRADING_MODE);
	}
    return 0;
}
static int32_t sm_pmu_stm_upgrading_exit(void* _arg){

	(void)_arg;
    return 0;
}

/* ----------------- event ----------------- */
static int32_t sm_pmu_enter_startting_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_STARTING;
}
static int32_t sm_pmu_sleep_key_on_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
static int32_t sm_pmu_key_off_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
static int32_t sm_pmu_first_bp_assigned_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}

static int32_t sm_pmu_enter_running_mode_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_RUNNING;
}

static int32_t sm_pmu_enter_power_limit_mode_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_POWER_LIMIT;
}

static int32_t sm_pmu_enter_idle_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}

static int32_t sm_pmu_plug_charge_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_CHARGING;
}
static int32_t sm_pmu_unplug_charge_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
static int32_t sm_pmu_charge_full_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
static int32_t sm_pmu_motor_stop_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
static int32_t sm_pmu_timeout_elapsed_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_SLEEP;
}
static int32_t sm_pmu_enter_upgrading_mode_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_UPGRADING;
}
static int32_t sm_pmu_exit_upgrading_mode_event_handle(void* _arg){

	(void)_arg;
    return PMU_STM_IDLE;
}
