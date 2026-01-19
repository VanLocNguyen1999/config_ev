/*
 * CO_SYNC.h
 *
 *  Created on: Jun 20, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SYNC_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SYNC_H_

#include "CO_config.h"
#include "CO_CAN_Msg.h"
#include "CO_OD.h"

#define CO_SYNC_INIT_TYPE_1			0

typedef enum{
	CO_SYNC_ST_stop					= 0x00,
	CO_SYNC_ST_operational          = 0x01,
	CO_SYNC_ST_fault				= 0x02,
}CO_SYNC_State_t;

typedef struct CO_SYNC_t CO_SYNC;

struct CO_SYNC_t
{
	CO_CAN_Msg msg;
//    bool is_producer;			    /** True, if device is SYNC producer. Calculated from _COB ID SYNC Message_
//    								variable from Object dictionary (index 0x1005). */

	uint16_t cycle_period_ms;
	uint16_t window_length_ms;
	uint16_t max_sync_counter;		//<< Setting range: [1-30000]
	uint8_t max_aux_sync_counter;	//<< Setting range: [0-31]

	uint16_t index_in_cycle;		//<< Increase 1 when running Sync_process, reset when receiving sync pulse
	uint16_t time_in_cycle_ms;
	uint16_t sync_counter;		 	//<< Running range: 1 - max_sync_counter; = 1 when reset; =0 when init
	uint8_t aux_sync_counter;		//<< Running range: 0 - (max_aux_sync_counter-1); = 0 when reset
	uint32_t aux_sync_reg;			//<< Auxiliary sync register
	bool is_inside_window;			//<< state = operational(?)
};




#if CO_SYNC_INIT_TYPE_1
void CO_SYNC_init(CO_SYNC *p_sync, co_id_t x1005_sync_cob_id, uint16_t x1006_cycle_period_ms,
		uint16_t x1007_sync_window_len, uint16_t x1019_sync_cter_over_flow_value,
		uint8_t x1030_sync_bit_pos_over_flow_value);
#else
void CO_SYNC_init(CO_SYNC *p_sync, OD_Temp_Comm_Profile_Para_t* p_od_comm_prof_init_data);
#endif

void CO_SYNC_process(CO_SYNC* p_sync, bool nmt_is_pre_or_operational, uint16_t time_diff_ms);

static inline bool CO_SYNC_is_in_window_length(CO_SYNC* p_sync)
{
	return p_sync->is_inside_window;
}

static inline bool CO_SYNC_is_match_sync_mask_reg(CO_SYNC* p_sync, const uint32_t sync_mask_reg)
{
	return (p_sync->aux_sync_reg & sync_mask_reg);
}

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SYNC_H_ */
