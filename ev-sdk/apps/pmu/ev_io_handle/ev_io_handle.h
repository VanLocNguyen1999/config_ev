/*
 * ev_io_handle.h
 *
 *  Created on: Nov 9, 2025
 *      Author: Admin
 */

#ifndef EV_IO_HANDLE_EV_IO_HANDLE_H_
#define EV_IO_HANDLE_EV_IO_HANDLE_H_
#include "sm_types.h"
#include "sm_ev_data.h"
#include "sm_elapsed_timer.h"
#include "pmu_io_handle.h"
#include "ioc_io_handle.h"

typedef struct io_data io_data_t;
typedef struct sm_sv_ev_io_impl sm_sv_ev_io_t;

typedef enum {
    INACTIVE = 0,
	ACTIVE,
}IO_ST;

typedef enum{
	SPORT = 0,
	ECO,
} DRIVE_MODE;

typedef enum {
	MARKET = 0,
	INSPECTION,
} VEHICLE_PURPOSE;

struct io_data{
	IO_ST 			state;
	IO_ST			(*get_state)(sm_sv_ev_io_t* );
	void 			(*ev_io_on)(sm_sv_ev_io_t* );
	void 			(*ev_io_off)(sm_sv_ev_io_t* );

	bool 			latch;
	bool			is_changed;
	elapsed_timer_t	m_timeout;
};

struct sm_sv_ev_io_impl{

	IO_ST ev_io_state;

	io_data_t m_drive_mode;
	io_data_t m_backward_mode;
	io_data_t m_uphill_mode;

	io_data_t m_charger_mode;
	io_data_t m_purpose_mode;

	io_data_t m_key;
	io_data_t m_parking;

	io_data_t m_12v_det;

	io_data_t m_left_signal;
	io_data_t m_right_signal;
	io_data_t m_emergency_signal;

	io_data_t m_low_beam_state;
	io_data_t m_high_beam_state;
	io_data_t m_pos_light_state;
	io_data_t m_brake;
	io_data_t m_horn;

	sm_ioc_data_output_t	m_ioc_output;
	sm_ioc_data_input_t		m_ioc_input;
	sm_sv_io_t*				m_pmu_io_service;
	void* 					_arg;

	elapsed_timer_t	m_timeout;
};

//uint8_t hmi_tx_buff[8] = {0};
sm_sv_ev_io_t* sm_ev_io_sv_create(sm_sv_io_t* _pmu_io_service,void* _arg);
void sm_ev_io_sv_process(sm_sv_ev_io_t* _this);

static inline IO_ST get_ev_io_state(io_data_t* _io){

	return _io->state;
}
#endif /* EV_IO_HANDLE_EV_IO_HANDLE_H_ */
