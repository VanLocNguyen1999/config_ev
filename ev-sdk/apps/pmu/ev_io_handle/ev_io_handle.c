/*
 * ev_io_handle.c
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */
#include "ev_io_handle.h"
#include "sm_memory.h"
#include "sm_pmu_app.h"
#include "sm_ev_pmu_module.h"
#include "sm_ev_mc_module.h"
#include "ev_io_define.h"

#define TIMEOUT_5M		300000


typedef struct sm_ev_io_get_state_if_t {

	IO_ST (* drive_mode_get_state) (sm_sv_ev_io_t*) ;
	IO_ST (* m_backward_mode_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_uphill_mode_get_state)(sm_sv_ev_io_t*) ;

	IO_ST (* m_charger_mode_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_purpose_mode_get_state)(sm_sv_ev_io_t*) ;

	IO_ST (* m_key_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_parking_get_state)(sm_sv_ev_io_t*) ;

	IO_ST (* m_12v_det_get_state)(sm_sv_ev_io_t*) ;

	IO_ST (* m_left_light_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_right_light_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_emergency_get_state)(sm_sv_ev_io_t*) ;

	IO_ST (* m_low_beam_light_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_high_beam_light_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_pos_light_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_brake_get_state)(sm_sv_ev_io_t*) ;
	IO_ST (* m_horn_get_state)(sm_sv_ev_io_t*) ;
}sm_ev_io_get_state_if;

typedef struct sm_ev_io_on_if_t {

	void (* drive_mode_on) (sm_sv_ev_io_t*) ;
	void (* m_backward_mode_on)(sm_sv_ev_io_t*) ;
	void (* m_uphill_mode_on)(sm_sv_ev_io_t*) ;

	void (* m_charger_mode_on)(sm_sv_ev_io_t*) ;
	void (* m_purpose_mode_on)(sm_sv_ev_io_t*) ;

	void (* m_key_on)(sm_sv_ev_io_t*) ;
	void (* m_parking_on)(sm_sv_ev_io_t*) ;

	void (* m_12v_on)(sm_sv_ev_io_t*) ;

	void (* m_left_light_on)(sm_sv_ev_io_t*) ;
	void (* m_right_light_on)(sm_sv_ev_io_t*) ;
	void (* m_emergency_on)(sm_sv_ev_io_t*) ;

	void (* m_low_beam_light_on)(sm_sv_ev_io_t*) ;
	void (* m_high_beam_light_on)(sm_sv_ev_io_t*) ;
	void (* m_pos_light_on)(sm_sv_ev_io_t*) ;
	void (* m_brake_on)(sm_sv_ev_io_t*) ;
	void (* m_horn_on)(sm_sv_ev_io_t*) ;
}sm_ev_io_on_if;

typedef struct sm_ev_io_off_if_t {

	void (* drive_mode_off) (sm_sv_ev_io_t*) ;
	void (* m_backward_mode_off)(sm_sv_ev_io_t*) ;
	void (* m_uphill_mode_off)(sm_sv_ev_io_t*) ;

	void (* m_charger_mode_off)(sm_sv_ev_io_t*) ;
	void (* m_purpose_mode_off)(sm_sv_ev_io_t*) ;

	void (* m_key_off)(sm_sv_ev_io_t*) ;
	void (* m_parking_off)(sm_sv_ev_io_t*) ;

	void (* m_12v_off)(sm_sv_ev_io_t*) ;

    void (* m_left_light_off)(sm_sv_ev_io_t*) ;
    void (* m_right_light_off)(sm_sv_ev_io_t*) ;
    void (* m_emergency_off)(sm_sv_ev_io_t*) ;

    void (* m_low_beam_light_off)(sm_sv_ev_io_t*) ;
    void (* m_high_beam_light_off)(sm_sv_ev_io_t*) ;
    void (* m_pos_light_off)(sm_sv_ev_io_t*) ;
    void (* m_brake_off)(sm_sv_ev_io_t*) ;
    void (* m_horn_off)(sm_sv_ev_io_t*) ;
}sm_ev_io_off_if;

static sm_sv_ev_io_t g_ev_io_sv_default = {

		.m_pmu_io_service = NULL,
		._arg = NULL
};

sm_sv_ev_io_t* g_ev_io_sv = NULL;

static void sm_io_set_state (sm_sv_ev_io_t*_this, IO_ST state){

	_this->ev_io_state = state;
}

static void sm_io_update_state (io_data_t*_this){

	IO_ST old_state = _this->state;
	if(!_this->get_state) return;
	IO_ST new_state = _this->get_state(g_ev_io_sv);

	if(old_state != new_state){
		_this->state =new_state;
		_this->is_changed = true;
		elapsed_timer_reset(&_this->m_timeout);

		if(_this->state == ACTIVE){
		    if(_this->ev_io_on){
	            _this->ev_io_on(g_ev_io_sv);
		    }
		}else {
            if(_this->ev_io_off){
                _this->ev_io_off(g_ev_io_sv);
            }
		}
	}
}

static IO_ST sm_ev_io_drive_mode_get_state (sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_backward_mode_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_uphill_mode_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_charger_mode_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_purpose_mode_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_key_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_parking_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_12v_power_get_state(sm_sv_ev_io_t*_this) ;
static IO_ST sm_ev_io_left_light_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_right_light_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_emergency_get_state(sm_sv_ev_io_t *_this);
static IO_ST sm_ev_io_low_beam_light_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_high_beam_light_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_pos_light_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_brake_get_state(sm_sv_ev_io_t*_this);
static IO_ST sm_ev_io_horn_get_state(sm_sv_ev_io_t*_this);

static void sm_ev_io_drive_mode_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_backward_mode_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_uphill_mode_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_charger_mode_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_purpose_mode_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_key_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_parking_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_12v_power_on(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_emergency_on(sm_sv_ev_io_t*_this);
static void sm_ev_io_on(sm_sv_ev_io_t*_this);

static void sm_ev_io_drive_mode_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_backward_mode_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_uphill_mode_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_charger_mode_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_purpose_mode_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_key_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_parking_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_12v_power_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_emergency_off(sm_sv_ev_io_t*_this) ;
static void sm_ev_io_off(sm_sv_ev_io_t*_this);

static void sm_ev_io_data (sm_sv_ev_io_t*_this);
static sm_ev_io_get_state_if g_get_state_if = {

		.drive_mode_get_state = sm_ev_io_drive_mode_get_state,
		.m_backward_mode_get_state = sm_ev_io_backward_mode_get_state,
		.m_charger_mode_get_state = sm_ev_io_charger_mode_get_state,
		.m_key_get_state = sm_ev_io_key_get_state,
		.m_parking_get_state = sm_ev_io_parking_get_state,
		.m_uphill_mode_get_state = sm_ev_io_uphill_mode_get_state,
		.m_purpose_mode_get_state = sm_ev_io_purpose_mode_get_state,
		.m_12v_det_get_state = sm_ev_io_12v_power_get_state,
		.m_left_light_get_state = sm_ev_io_left_light_get_state,
	    .m_right_light_get_state = sm_ev_io_right_light_get_state,
		.m_emergency_get_state = sm_ev_io_emergency_get_state,
	    .m_low_beam_light_get_state = sm_ev_io_low_beam_light_get_state,
	    .m_high_beam_light_get_state = sm_ev_io_high_beam_light_get_state,
	    .m_pos_light_get_state = sm_ev_io_pos_light_get_state,
	    .m_brake_get_state = sm_ev_io_brake_get_state,
	    .m_horn_get_state = sm_ev_io_horn_get_state
};

static sm_ev_io_on_if g_ioc_on_if = {
		.drive_mode_on = sm_ev_io_drive_mode_on,
		.m_backward_mode_on = sm_ev_io_backward_mode_on,
		.m_charger_mode_on = sm_ev_io_charger_mode_on,
		.m_key_on = sm_ev_io_key_on,
		.m_parking_on = sm_ev_io_parking_on,
		.m_purpose_mode_on = sm_ev_io_purpose_mode_on,
		.m_uphill_mode_on = sm_ev_io_uphill_mode_on,
		.m_12v_on = sm_ev_io_12v_power_on,
        .m_left_light_on = sm_ev_io_on,
        .m_right_light_on = sm_ev_io_on,
		.m_emergency_on = sm_ev_io_emergency_on,
        .m_low_beam_light_on = sm_ev_io_on,
        .m_high_beam_light_on = sm_ev_io_on,
        .m_pos_light_on = sm_ev_io_on,
        .m_brake_on = sm_ev_io_on,
        .m_horn_on = sm_ev_io_on
};

static sm_ev_io_off_if g_ioc_off_if = {
		.drive_mode_off = sm_ev_io_drive_mode_off,
		.m_backward_mode_off = sm_ev_io_backward_mode_off,
		.m_charger_mode_off = sm_ev_io_charger_mode_off,
		.m_key_off = sm_ev_io_key_off,
		.m_parking_off = sm_ev_io_parking_off,
		.m_purpose_mode_off = sm_ev_io_purpose_mode_off,
		.m_uphill_mode_off = sm_ev_io_uphill_mode_off,
		.m_12v_off = sm_ev_io_12v_power_off,
	    .m_left_light_off = sm_ev_io_off,
	    .m_right_light_off = sm_ev_io_off,
		.m_emergency_off  = sm_ev_io_emergency_off,
	    .m_low_beam_light_off = sm_ev_io_off,
	    .m_high_beam_light_off = sm_ev_io_off,
	    .m_pos_light_off = sm_ev_io_off,
	    .m_brake_off = sm_ev_io_off,
	    .m_horn_off = sm_ev_io_off
};

static void sm_ev_io_init(io_data_t* _io){


	_io->state = INACTIVE;
	if(!_io->ev_io_off){

		_io->ev_io_off(g_ev_io_sv);
	}
	_io->is_changed = false;
	_io->latch		= false;
}
static void sm_ev_io_data_init(sm_sv_ev_io_t* _this){

	_this->m_key.get_state 			= g_get_state_if.m_key_get_state;
	_this->m_key.ev_io_on 			= g_ioc_on_if.m_key_on;
	_this->m_key.ev_io_off 			= g_ioc_off_if.m_key_off;
	elapsed_timer_resetz(&_this->m_key.m_timeout,100);
	sm_ev_io_init(&_this->m_key);

	_this->m_parking.get_state 		= g_get_state_if.m_parking_get_state;
	_this->m_parking.ev_io_on 		= g_ioc_on_if.m_parking_on;
	_this->m_parking.ev_io_off 		= g_ioc_off_if.m_parking_off;
	sm_ev_io_init(&_this->m_parking);
	_this->m_parking.state 			= ACTIVE;

	_this->m_drive_mode.get_state 	= g_get_state_if.drive_mode_get_state;
	_this->m_drive_mode.ev_io_on 		= g_ioc_on_if.drive_mode_on;
	_this->m_drive_mode.ev_io_off 		= g_ioc_off_if.drive_mode_off;
	sm_ev_io_init(&_this->m_drive_mode);

	_this->m_backward_mode.get_state = g_get_state_if.m_backward_mode_get_state;
	_this->m_backward_mode.ev_io_on 	= g_ioc_on_if.m_backward_mode_on;
	_this->m_backward_mode.ev_io_off 	= g_ioc_off_if.m_backward_mode_off;
	sm_ev_io_init(&_this->m_backward_mode);

	_this->m_uphill_mode.get_state 	= g_get_state_if.m_uphill_mode_get_state;
	_this->m_uphill_mode.ev_io_on 		= g_ioc_on_if.m_uphill_mode_on;
	_this->m_uphill_mode.ev_io_off 	= g_ioc_off_if.m_uphill_mode_off;
	sm_ev_io_init(&_this->m_uphill_mode);

	_this->m_purpose_mode.get_state 	= g_get_state_if.m_purpose_mode_get_state;
	_this->m_purpose_mode.ev_io_on 	= g_ioc_on_if.m_purpose_mode_on;
	_this->m_purpose_mode.ev_io_off 	= g_ioc_off_if.m_purpose_mode_off;
	 elapsed_timer_resetz(&_this->m_purpose_mode.m_timeout,30000);
	sm_ev_io_init(&_this->m_purpose_mode);

	_this->m_charger_mode.get_state 	= g_get_state_if.m_charger_mode_get_state;
	_this->m_charger_mode.ev_io_on 	= g_ioc_on_if.m_charger_mode_on;
	_this->m_charger_mode.ev_io_off 	= g_ioc_off_if.m_charger_mode_off;
	sm_ev_io_init(&_this->m_charger_mode);

    _this->m_12v_det.get_state     = g_get_state_if.m_12v_det_get_state;
    _this->m_12v_det.ev_io_on    = g_ioc_on_if.m_12v_on;
    _this->m_12v_det.ev_io_off   = g_ioc_off_if.m_12v_off;
    elapsed_timer_resetz(&_this->m_12v_det.m_timeout,100);
    sm_ev_io_init(&_this->m_12v_det);

    _this->m_left_signal.get_state     = g_get_state_if.m_left_light_get_state;
    _this->m_left_signal.ev_io_on    =  g_ioc_on_if.m_left_light_on;
    _this->m_left_signal.ev_io_off   =  g_ioc_off_if.m_left_light_off;
    elapsed_timer_resetz(&_this->m_left_signal.m_timeout,515);
    sm_ev_io_init(&_this->m_left_signal);

    _this->m_right_signal.get_state     = g_get_state_if.m_right_light_get_state;
    _this->m_right_signal.ev_io_on    = g_ioc_on_if.m_right_light_on;
    _this->m_right_signal.ev_io_off   = g_ioc_off_if.m_right_light_off;
    elapsed_timer_resetz(&_this->m_right_signal.m_timeout,515);
    sm_ev_io_init(&_this->m_right_signal);

    _this->m_emergency_signal.get_state     = g_get_state_if.m_emergency_get_state;
    _this->m_emergency_signal.ev_io_on    = g_ioc_on_if.m_emergency_on;
    _this->m_emergency_signal.ev_io_off   = g_ioc_off_if.m_emergency_off;
    elapsed_timer_resetz(&_this->m_emergency_signal.m_timeout,500);
    sm_ev_io_init(&_this->m_emergency_signal);

    _this->m_low_beam_state.get_state     = g_get_state_if.m_low_beam_light_get_state;
    _this->m_low_beam_state.ev_io_on    = g_ioc_on_if.m_low_beam_light_on;
    _this->m_low_beam_state.ev_io_off   = g_ioc_off_if.m_low_beam_light_off;
    elapsed_timer_resetz(&_this->m_low_beam_state.m_timeout,50);
    sm_ev_io_init(&_this->m_low_beam_state);

    _this->m_high_beam_state.get_state     = g_get_state_if.m_high_beam_light_get_state;
    _this->m_high_beam_state.ev_io_on    = g_ioc_on_if.m_high_beam_light_on;
    _this->m_high_beam_state.ev_io_off   = g_ioc_off_if.m_high_beam_light_off;
    elapsed_timer_resetz(&_this->m_high_beam_state.m_timeout,50);
    sm_ev_io_init(&_this->m_high_beam_state);

    _this->m_pos_light_state.get_state     = g_get_state_if.m_pos_light_get_state;
    _this->m_pos_light_state.ev_io_on    = g_ioc_on_if.m_pos_light_on;
    _this->m_pos_light_state.ev_io_off   = g_ioc_off_if.m_pos_light_off;
    elapsed_timer_resetz(&_this->m_pos_light_state.m_timeout,50);
    sm_ev_io_init(&_this->m_pos_light_state);

    _this->m_horn.get_state     = g_get_state_if.m_horn_get_state;
    _this->m_horn.ev_io_on    = g_ioc_on_if.m_horn_on;
    _this->m_horn.ev_io_off   = g_ioc_off_if.m_horn_off;
    elapsed_timer_resetz(&_this->m_pos_light_state.m_timeout,50);
    sm_ev_io_init(&_this->m_horn);

    _this->m_brake.get_state     = g_get_state_if.m_brake_get_state;
    _this->m_brake.ev_io_on    = g_ioc_on_if.m_brake_on;
    _this->m_brake.ev_io_off   = g_ioc_off_if.m_brake_off;
    elapsed_timer_resetz(&_this->m_brake.m_timeout,50);
    sm_ev_io_init(&_this->m_brake);
	sm_io_set_state(_this, INACTIVE);
}


sm_sv_ev_io_t* sm_ev_io_sv_create(sm_sv_io_t* _pmu_io_service,void* _arg){

	if(!_arg && !_pmu_io_service) return NULL;

	sm_sv_ev_io_t* ev_io_sv = &g_ev_io_sv_default;
	g_ev_io_sv = &g_ev_io_sv_default;
	ev_io_sv->m_pmu_io_service = _pmu_io_service;
	ev_io_sv->_arg = _arg;
	elapsed_timer_resetz(&ev_io_sv->m_timeout,5000);
	sm_ev_io_data_init(ev_io_sv);

	return ev_io_sv;
}
static void sm_ev_io_state_update(sm_sv_ev_io_t* _this){

	uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();
	if ((pmu_stm == PMU_STM_STARTUP || pmu_stm == PMU_STM_STARTING)
			&& elapsed_timer_get_remain(&_this->m_timeout))
		return;


	sm_io_update_state(&_this->m_key);
	sm_io_update_state(&_this->m_parking);
	sm_io_update_state(&_this->m_drive_mode);
	sm_io_update_state(&_this->m_backward_mode);
	sm_io_update_state(&_this->m_uphill_mode);
	sm_io_update_state(&_this->m_purpose_mode);
	sm_io_update_state(&_this->m_charger_mode);
	sm_io_update_state(&_this->m_12v_det);
    sm_io_update_state(&_this->m_left_signal);
    sm_io_update_state(&_this->m_right_signal);
    sm_io_update_state(&_this->m_emergency_signal);
    sm_io_update_state(&_this->m_low_beam_state);
    sm_io_update_state(&_this->m_high_beam_state);
    sm_io_update_state(&_this->m_pos_light_state);
    sm_io_update_state(&_this->m_brake);
    sm_io_update_state(&_this->m_horn);
	sm_ev_io_data(_this);
}

void sm_ev_io_sv_process(sm_sv_ev_io_t* _this){

	if(!_this) return;
	sm_ev_io_state_update(_this);
	sm_sv_io_process(_this->m_pmu_io_service);
}

static IO_ST sm_ev_io_drive_mode_get_state (sm_sv_ev_io_t*_this) {

	if(_this->m_parking.state == ACTIVE) return INACTIVE;

	if (_this->m_backward_mode.state == ACTIVE) {
		return _this->m_drive_mode.state;
	}

	uint8_t drive_state = _this->m_drive_mode.state;
    if (_this->m_ioc_input.m_drive_mode_signal == 1) {
//        eco_sport_cnt++;
    } else {
    	elapsed_timer_resetz(&_this->m_drive_mode.m_timeout,50);
    	_this->m_drive_mode.latch = false;
    }

    if ((!elapsed_timer_get_remain(&_this->m_drive_mode.m_timeout))
    		&& !_this->m_drive_mode.latch) {

        drive_state = (drive_state == ACTIVE) ? INACTIVE : ACTIVE;
        _this->m_drive_mode.latch = true;
    }

	return drive_state;
}

bool reverse_st = true;
static IO_ST sm_ev_io_backward_mode_get_state(sm_sv_ev_io_t *_this)
{
	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
    sm_mc_data_t *mc_data = sm_mc_get_data(pmu_app->m_mc_module);
    /* ====== BƯỚC 1: Kiểm tra điều kiện kích hoạt chế độ Reverse ====== */
    if(_this->m_parking.state == ACTIVE){

        _this->m_backward_mode.state = INACTIVE;
        reverse_st = true;
        _this->m_backward_mode.latch = false;
        return INACTIVE;
    }

    if ((_this->m_backward_mode.state == INACTIVE) &&
        (mc_data->m_speed_rpm == 0) &&
        (_this->m_ioc_input.m_emergency == 1) &&
        (_this->m_key.state == ACTIVE)){
        /* Nếu đang nhấn giữ mode */
        if (_this->m_ioc_input.m_drive_mode_signal == 1){
            /* Nếu timer chưa chạy thì khởi động đếm 1.5s */
            if (!_this->m_backward_mode.latch){
                _this->m_backward_mode.latch = true;
                elapsed_timer_resetz(&_this->m_backward_mode.m_timeout, 1500);
            }
        }else{
            /* Thả nút mode → reset latch (cho phép nhấn lại) */
            _this->m_backward_mode.latch = false;
        }

        /* Khi giữ đủ 1.5s → ACTIVE */
        if (_this->m_backward_mode.latch &&
            !elapsed_timer_get_remain(&_this->m_backward_mode.m_timeout)) {
            _this->m_backward_mode.state = ACTIVE;
            reverse_st = true; // Khi vừa Active, reverse_st = false
            _this->m_backward_mode.latch = false;
            return ACTIVE;
        }
    }
    /* ====== BƯỚC 2: Khi đã ACTIVE rồi ====== */
    if (_this->m_backward_mode.state == ACTIVE){
        /* Nếu tắt emergency hoặc tắt khóa → thoát chế độ */
        if ((_this->m_ioc_input.m_emergency != 1) || (_this->m_key.state != ACTIVE)){
            _this->m_backward_mode.state = INACTIVE;
            reverse_st = true;
            _this->m_backward_mode.latch = false;
            return INACTIVE;
        }

        /* Nếu đang nhấn giữ mode */
        if (_this->m_ioc_input.m_drive_mode_signal == 1){
            /* Nếu timer chưa chạy thì khởi động đếm 1.5s */
            if (!_this->m_backward_mode.latch)
            {
                _this->m_backward_mode.latch = true;
                elapsed_timer_resetz(&_this->m_backward_mode.m_timeout, 1000);
            }
        }else{
            /* Thả nút mode → reset latch (cho phép đổi lần tiếp theo) */
            _this->m_backward_mode.latch = false;
        }
        /* Khi giữ đủ 1.5s → đổi trạng thái reverse_st */
        if (_this->m_backward_mode.latch &&
            !elapsed_timer_get_remain(&_this->m_backward_mode.m_timeout)){
            reverse_st = !reverse_st;
            _this->m_backward_mode.latch = false;
        }
    }

    return _this->m_backward_mode.state;
}

bool uphill_mode_lath = false;

static IO_ST sm_ev_io_uphill_mode_get_state(sm_sv_ev_io_t* _this)  {

	uint8_t state = _this->m_ioc_input.m_drive_mode_signal;
	/* Hold the MODE button >= 5s, to switch to Uphill mode*/
	if (_this->m_uphill_mode.state == INACTIVE &&
			_this->m_parking.state == INACTIVE ){
		if (state == ACTIVE && uphill_mode_lath == false){
			uphill_mode_lath = true;
			elapsed_timer_resetz(&_this->m_uphill_mode.m_timeout, 5000);
		}
		else if(state == INACTIVE){
			uphill_mode_lath = false;
		}
		if (!elapsed_timer_get_remain(&_this->m_uphill_mode.m_timeout)
				&& uphill_mode_lath == true) {

			uphill_mode_lath = false;
			_this->m_uphill_mode.state = ACTIVE;
		}
	}

	/* When Uphill_mode_st = ON, hold the MODE >= 300ms to exit Uphill mode*/
	if (_this->m_uphill_mode.state == ACTIVE ){
		if (_this->m_parking.state == ACTIVE){
			_this->m_uphill_mode.state = INACTIVE;
		}
		if (state == ACTIVE && uphill_mode_lath == false){
			uphill_mode_lath = true;
			elapsed_timer_resetz(&_this->m_uphill_mode.m_timeout, 300);
		}
		else if(state == INACTIVE){
			uphill_mode_lath = false;
		}
		if (!elapsed_timer_get_remain(&_this->m_uphill_mode.m_timeout)
				&& uphill_mode_lath == true) {

			uphill_mode_lath = false;
			_this->m_uphill_mode.state = INACTIVE;
		}
	}

	return _this->m_uphill_mode.state;

}
static IO_ST sm_ev_io_charger_mode_get_state(sm_sv_ev_io_t* _this)  {

	uint8_t old_state =  _this->m_charger_mode.state;
	uint8_t state =_this->m_ioc_input.m_charger_st;
	if(state == old_state){

		elapsed_timer_resetz(&_this->m_charger_mode.m_timeout,100);
	}

    if (!elapsed_timer_get_remain(&_this->m_charger_mode.m_timeout)) {

        	return (IO_ST) state;
    }
	return (IO_ST) old_state;
}
typedef enum {
    PURPOSE_STEP_NONE = 0,
    PURPOSE_STEP_EMG1,
    PURPOSE_STEP_LEFT1,
    PURPOSE_STEP_EMG2,
    PURPOSE_STEP_RIGHT,
    PURPOSE_STEP_EMG3,
    PURPOSE_STEP_LEFT2
} purpose_step_t;

static purpose_step_t purpose_step = PURPOSE_STEP_NONE;
static bool emergency_lath = false;
static bool left_signal_lath = false;
static bool right_signal_lath = false;
static bool step_timeout_check = false;

static IO_ST sm_ev_io_purpose_mode_get_state(sm_sv_ev_io_t* _this) {
    sm_pmu_app_t* pmu_app = (sm_pmu_app_t*) _this->_arg;
    sm_mc_data_t *mc_data = sm_mc_get_data(pmu_app->m_mc_module);

    if ((_this->m_parking.state == INACTIVE) && (mc_data->m_speed_rpm == 0)
            && (_this->m_key.state == ACTIVE)) {

        bool emg_pressed = (_this->m_ioc_input.m_emergency == ACTIVE);
        bool left_pressed = (_this->m_ioc_input.m_left_signal == ACTIVE);
        bool right_pressed = (_this->m_ioc_input.m_right_signal == ACTIVE);

        // Reset nếu timeout cho bước hiện tại
        if(step_timeout_check && (!elapsed_timer_get_remain(&_this->m_purpose_mode.m_timeout))) {
            purpose_step = PURPOSE_STEP_NONE;
            step_timeout_check = false;
        }

        // ===== Nhấn Emergency =====
        if (emg_pressed && !emergency_lath) {
            emergency_lath = true;
            switch(purpose_step) {
                case PURPOSE_STEP_NONE: purpose_step = PURPOSE_STEP_EMG1; break;
                case PURPOSE_STEP_LEFT1: purpose_step = PURPOSE_STEP_EMG2; break;
                case PURPOSE_STEP_RIGHT: purpose_step = PURPOSE_STEP_EMG3; break;
                default:
                    // Nhấn sai trình tự → reset
                    purpose_step = PURPOSE_STEP_NONE;
                    step_timeout_check = false;
                    break;
            }
            elapsed_timer_reset(&_this->m_purpose_mode.m_timeout);
            step_timeout_check = true;
        } else if (!emg_pressed) {
            emergency_lath = false;
        }

        // ===== Nhấn Left Signal =====
        if (left_pressed && !left_signal_lath) {
            left_signal_lath = true;
            if (purpose_step == PURPOSE_STEP_EMG1) {
                purpose_step = PURPOSE_STEP_LEFT1;
            } else if (purpose_step == PURPOSE_STEP_EMG3) {
                purpose_step = PURPOSE_STEP_LEFT2; // Bước cuối
            } else {
                purpose_step = PURPOSE_STEP_NONE;
                step_timeout_check = false;
            }
            elapsed_timer_reset(&_this->m_purpose_mode.m_timeout);
            step_timeout_check = true;
        } else if (!left_pressed) {
            left_signal_lath = false;
        }

        // ===== Nhấn Right Signal =====
        if (right_pressed && !right_signal_lath) {
            right_signal_lath = true;
            if (purpose_step == PURPOSE_STEP_EMG2) {
                purpose_step = PURPOSE_STEP_RIGHT;
                elapsed_timer_reset(&_this->m_purpose_mode.m_timeout);
                step_timeout_check = true;
            } else {
                purpose_step = PURPOSE_STEP_NONE;
                step_timeout_check = false;
            }
        } else if (!right_pressed) {
            right_signal_lath = false;
        }

        // ===== Kiểm tra hoàn thành chuỗi =====
        if (purpose_step == PURPOSE_STEP_LEFT2 && !left_signal_lath) {
            purpose_step = PURPOSE_STEP_NONE;
            step_timeout_check = false;
            return (_this->m_purpose_mode.state == ACTIVE) ? INACTIVE : ACTIVE;
        }

    } else {
        // Reset toàn bộ nếu xe chạy hoặc key OFF
        purpose_step = PURPOSE_STEP_NONE;
        emergency_lath = false;
        left_signal_lath = false;
        right_signal_lath = false;
        step_timeout_check = false;
    }

    return _this->m_purpose_mode.state;
}



static IO_ST sm_ev_io_key_get_state(sm_sv_ev_io_t*_this)  {

	uint8_t old_state =  _this->m_key.state;
	uint8_t state =_this->m_ioc_input.m_key;
	if(state==old_state){

		elapsed_timer_resetz(&_this->m_key.m_timeout,100);
	}

    if (!elapsed_timer_get_remain(&_this->m_key.m_timeout)) {

    	return (IO_ST)state;
    }
	return (IO_ST) old_state;
}

static uint16_t active_parking_delay_cnt 	= 0;
static uint16_t inactive_parking_delay_cnt 	= 0;
static uint16_t active_parking_cnt 			= 0;
static uint16_t	cnt_parking 				= 0;
static IO_ST sm_ev_io_parking_get_state(sm_sv_ev_io_t*_this)  {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
	uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();
	/***Parking always activates when key off** ✅
	 * */
	if(_this->m_key.state == INACTIVE){

		return (IO_ST) ACTIVE;
	}
	/***"Parking activates when not moving for 5 minutes."** ✅
	 * */
	sm_mc_data_t*mc_data = sm_mc_get_data(pmu_app->m_mc_module);
	if (_this->m_key.state == ACTIVE && _this->m_parking.state == INACTIVE
			&& mc_data->m_speed_rpm == 0) {

	}else {
		elapsed_timer_resetz(&_this->m_parking.m_timeout,TIMEOUT_5M);
	}

	if(!elapsed_timer_get_remain(&_this->m_parking.m_timeout)) {

		return (IO_ST) ACTIVE;
	}
	/***"Auto exit Parking when shifting to reverse."** ✅
	 * */
	if(_this->m_backward_mode.state == ACTIVE){

		return INACTIVE;
	}

	/***"Parking activates."** ✅
		 * */
	if( (active_parking_delay_cnt < 5) && (_this->m_ioc_input.m_parking == 0)&&
			(_this->m_parking.state == INACTIVE)){

		active_parking_delay_cnt++;;
	}
	if( (_this->m_ioc_input.m_parking == 1)&& (active_parking_delay_cnt >= 5)&&
			(mc_data->m_speed_rpm == 0)&& (_this->m_parking.state == INACTIVE)){

		active_parking_cnt++;;
	}
	else active_parking_cnt = 0;

	if (active_parking_cnt > 5
			&& (pmu_stm == PMU_STM_POWER_LIMIT || pmu_stm == PMU_STM_RUNNING
					|| pmu_stm == PMU_STM_STARTING )) {

		active_parking_cnt = 0;
		inactive_parking_delay_cnt = 0;
		return ACTIVE;
	}
	/***"Parking inactivates."** ✅
		 * */
	if( (inactive_parking_delay_cnt < 5)&&(_this->m_ioc_input.m_parking == 0)
			&&(_this->m_parking.state == ACTIVE)){

		inactive_parking_delay_cnt++;;
	}
	if ( (_this->m_ioc_input.m_parking == 1)&&(inactive_parking_delay_cnt >= 5)){
		cnt_parking ++;
	}
	else cnt_parking = 0;

	if( cnt_parking > 5 &&_this->m_parking.state == ACTIVE){

		_this->m_parking.state = INACTIVE;
		active_parking_delay_cnt = 0;
	}
	return _this->m_parking.state;
}

static IO_ST sm_ev_io_12v_power_get_state(sm_sv_ev_io_t*_this)  {

//	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
    uint8_t old_state =  _this->m_12v_det.state;
    uint8_t state = (uint8_t)sm_sv_io_get_state(_this->m_pmu_io_service,IN_12V_DET);
    if(state==old_state){

        elapsed_timer_resetz(&_this->m_12v_det.m_timeout,100);
    }

    if (!elapsed_timer_get_remain(&_this->m_12v_det.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_left_light_get_state(sm_sv_ev_io_t *_this) {

	uint8_t old_state = _this->m_left_signal.state;
	uint8_t state = 0;
	if (_this->m_ioc_input.m_emergency == 1) {
		state = OP_STATE_FORCE_BLINK;
	} else {

		state = _this->m_ioc_output.m_left_light;
	}
	if (state == OP_STATE_NORMAL) {

		return INACTIVE;
	} else if (state == OP_STATE_DEFAULT || state == OP_STATE_FORCE) {
		return ACTIVE;
	} else if (state == OP_STATE_FORCE_BLINK) {

		if (!elapsed_timer_get_remain(&_this->m_left_signal.m_timeout)) {

			return (IO_ST) (!old_state);
		}
	}

	return (IO_ST) old_state;
}
static IO_ST sm_ev_io_right_light_get_state(sm_sv_ev_io_t *_this) {

	uint8_t old_state = _this->m_right_signal.state;
	uint8_t state = 0;
	if (_this->m_ioc_input.m_emergency == 1) {
		state = OP_STATE_FORCE_BLINK;
	} else {

		state = _this->m_ioc_output.m_right_light;
	}
	if (state == OP_STATE_NORMAL) {

		return INACTIVE;
	} else if (state == OP_STATE_DEFAULT || state == OP_STATE_FORCE) {
		return ACTIVE;
	} else if (state == OP_STATE_FORCE_BLINK) {

		if (!elapsed_timer_get_remain(&_this->m_right_signal.m_timeout)) {

			return (IO_ST) (!old_state);
		}
	}
	return (IO_ST) old_state;
}
static IO_ST sm_ev_io_emergency_get_state(sm_sv_ev_io_t *_this) {

    uint8_t old_state =  _this->m_emergency_signal.state;
    uint8_t state = _this->m_ioc_input.m_emergency;

    if(state==old_state){

        elapsed_timer_reset(&_this->m_emergency_signal.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_emergency_signal.m_timeout)) {

    	 return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_low_beam_light_get_state(sm_sv_ev_io_t*_this){

    uint8_t old_state =  _this->m_low_beam_state.state;
    uint8_t state =_this->m_ioc_input.m_low_beam_light_sw;

    if(state==old_state){

        elapsed_timer_reset(&_this->m_low_beam_state.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_low_beam_state.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_high_beam_light_get_state(sm_sv_ev_io_t*_this){

    uint8_t old_state =  _this->m_high_beam_state.state;
    uint8_t state =_this->m_ioc_output.m_high_beam_light;
    if(state==old_state){

        elapsed_timer_reset(&_this->m_high_beam_state.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_high_beam_state.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_pos_light_get_state(sm_sv_ev_io_t*_this){

    uint8_t old_state =  _this->m_pos_light_state.state;
    uint8_t state =_this->m_ioc_input.m_pos_light_sw;
    if(state==old_state){

        elapsed_timer_reset(&_this->m_pos_light_state.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_pos_light_state.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_brake_get_state(sm_sv_ev_io_t*_this){

    uint8_t old_state =  _this->m_brake.state;
    uint8_t state =_this->m_ioc_input.m_brake;
    if(state==old_state){

        elapsed_timer_reset(&_this->m_brake.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_brake.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}
static IO_ST sm_ev_io_horn_get_state(sm_sv_ev_io_t*_this){

    uint8_t old_state =  _this->m_horn.state;
    uint8_t state =_this->m_ioc_output.m_horn;
    if(state==old_state){

        elapsed_timer_reset(&_this->m_horn.m_timeout);
    }

    if (!elapsed_timer_get_remain(&_this->m_horn.m_timeout)) {

        return (IO_ST)state;
    }
    return (IO_ST) old_state;
}

static void sm_ev_io_drive_mode_on(sm_sv_ev_io_t* _this)  {

	(void)_this;
}
static void sm_ev_io_backward_mode_on(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_uphill_mode_on(sm_sv_ev_io_t*_this)   {

	(void)_this;
}
static void sm_ev_io_charger_mode_on(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_purpose_mode_on(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_key_on(sm_sv_ev_io_t*_this)  {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
	uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();
	sm_io_update_state(&_this->m_drive_mode);
	_this->m_drive_mode.is_changed = true;
	sm_pmu_io_set_event(SM_PMU_IO_EVENT_12V_CHANGED, OP_STATE_FORCE, pmu_app);
	sm_stm_set_event(PMU_STM_EVENT_TURN_ON_KEY);
	sm_io_set_state(_this,ACTIVE);
}
static void sm_ev_io_parking_on(sm_sv_ev_io_t*_this)  {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
    sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, OP_STATE_FORCE, pmu_app);
	sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, OP_STATE_NORMAL, pmu_app);
    sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, OP_STATE_NORMAL, pmu_app);

	sm_stm_set_event(PMU_STM_EVENT_ENTER_IDLE);
}
static void sm_ev_io_12v_power_on(sm_sv_ev_io_t*_this)  {

    (void)_this;
}

static void sm_ev_io_emergency_on(sm_sv_ev_io_t*_this)  {

	_this->m_left_signal.state = ACTIVE;
	_this->m_right_signal.state = ACTIVE;
	elapsed_timer_resetz(&_this->m_left_signal.m_timeout,500);
	elapsed_timer_resetz(&_this->m_right_signal.m_timeout,500);
}

static void sm_ev_io_on(sm_sv_ev_io_t*_this)  {

    (void)_this;
}

static void sm_ev_io_drive_mode_off(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_backward_mode_off(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_uphill_mode_off(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_charger_mode_off(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_purpose_mode_off(sm_sv_ev_io_t*_this)  {

	(void)_this;
}
static void sm_ev_io_key_off(sm_sv_ev_io_t* _this)  {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
	sm_pmu_data_t*pmu_data = sm_pmu_get_data(pmu_app->m_pmu_module);
	uint8_t pmu_stm = (uint8_t) sm_pmu_app_get_stm();
	if(pmu_stm != PMU_STM_UPGRADING){

	}
    if(pmu_data->m_anti_theft_status != EV_BLOCK_STATE){

    	sm_pmu_io_set_event(SM_PMU_IO_EVENT_12V_CHANGED, OP_STATE_NORMAL, pmu_app);
    }
	sm_io_set_state(_this,INACTIVE);
	sm_stm_set_event(PMU_STM_EVENT_TURN_OFF_KEY);
}
static void sm_ev_io_parking_off(sm_sv_ev_io_t*_this)  {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
    sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, 	OP_STATE_FORCE, pmu_app);
	sm_ioc_io_set_event(SM_IOC_IO_EVENT_MC_POWER_CHANGED, 	OP_STATE_FORCE, pmu_app);
    sm_ioc_io_set_event(SM_IOC_IO_EVENT_EMERGENCY_CHANGED, 	OP_STATE_NORMAL, pmu_app);

	sm_stm_set_event(PMU_STM_EVENT_ENTER_RUNNING_MODE);
}
static void sm_ev_io_12v_power_off(sm_sv_ev_io_t*_this)  {

    (void)_this;
}

static void sm_ev_io_emergency_off(sm_sv_ev_io_t*_this)  {

	_this->m_left_signal.state = INACTIVE;
	_this->m_right_signal.state = INACTIVE;
	elapsed_timer_resetz(&_this->m_left_signal.m_timeout,500);
	elapsed_timer_resetz(&_this->m_right_signal.m_timeout,500);
}

static void sm_ev_io_off(sm_sv_ev_io_t*_this)  {

    (void)_this;
}
/**/
static void sm_ev_io_data (sm_sv_ev_io_t*_this) {

	sm_pmu_app_t* pmu_app = (sm_pmu_app_t*)	_this->_arg;
    sm_pmu_data_t*pmu_data = sm_pmu_get_data(pmu_app->m_pmu_module);

    pmu_data->m_key                     =   (uint8_t) _this->m_key.state;
    pmu_data->m_parking                 =   (uint8_t) _this->m_parking.state;
    pmu_data->m_drive_mode_signal       =   (uint8_t) _this->m_drive_mode.state;
    if (_this->m_backward_mode.state == INACTIVE){

        pmu_data->m_backward_mode_signal = 0;
    }
    else{
        if (reverse_st == true){
            pmu_data->m_backward_mode_signal = 1;
        }
        else{
            pmu_data->m_backward_mode_signal = 2;
        }
    }
    pmu_data->m_uphill_mode             =   (uint8_t) _this->m_uphill_mode.state;
//    pmu_data->m_purpose_state           =   (uint8_t) _this->m_purpose_mode.state;


	pmu_data->m_right_signal = (uint8_t) _this->m_right_signal.state;
	pmu_data->m_left_signal = (uint8_t) _this->m_left_signal.state;


    pmu_data->m_high_beam_state =(uint8_t) _this->m_high_beam_state.state;
	if (pmu_data->m_high_beam_state == INACTIVE) {

		pmu_data->m_low_beam_state = (uint8_t) _this->m_low_beam_state.state;
	} else {
		pmu_data->m_low_beam_state = 0;
	}

    pmu_data->m_horn = (uint8_t) _this->m_horn.state;
    pmu_data->m_brake                  =   (uint8_t) _this->m_brake.state;
    pmu_data->m_pos_light_state        =   (uint8_t) _this->m_pos_light_state.state;
}


