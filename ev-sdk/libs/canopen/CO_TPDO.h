#ifndef CO_TPDO_H_
#define CO_TPDO_H_
#include "CO_PDO.h"
#include "CO_SYNC.h"
#include "CO_config.h"


/**
 * TPDO transmission Types
 */
#define CO_TPDO_TRANSM_TYPE_SYNC_ACYCLIC			(0)		/**< synchronous (acyclic) */
#define CO_TPDO_TRANSM_TYPE_SYNC_CYCLIC(x)			(x)		/**< synchronous (cyclic every sync) [0-30000]*/
#define CO_TPDO_TRANSM_TYPE_EVENT_LO				(0xFFFF)/**< event-driven, lower value (manufacturer specific),  */

/**/
typedef enum
{
	CO_TPDO_build_data_map		= 0x01,
	CO_TPDO_build_data_manually	= 0x02
}CO_TPDO_Build_Data_option_t;



/**/
typedef void (*CO_TPDO_Builder_Data_Manually_Interface)(uint8_t* buffer);
typedef struct CO_TPDO_t CO_TPDO;
struct CO_TPDO_t
{
	CO_PDO base;

	CO_SYNC* p_sync;
	/* Communication parameter */
	CO_OD_TPDO_Comm_Para_Obj_t comm_parameter;
	/* */
	uint16_t event_timer_ms;
	uint16_t inhibit_timer_ms;
	bool send_request;

	/* Build data manually*/
	CO_TPDO_Builder_Data_Manually_Interface 	build_data_manually;
};

void CO_TPDO_init(CO_TPDO *p_tpdo,
		CO_SYNC* p_sync,
		CO_OD_PDO_Map_Para_Object_t *p_pdo_map_parameter,
		CO_OD* p_od_list,
		CO_OD_TPDO_Comm_Para_Obj_t *p_tpdo_comm_parameter,
		CO_TPDO_Builder_Data_Manually_Interface build_data_manually_impl);

void CO_TPDO_process(CO_TPDO* p_tpdo,
		uint16_t time_difference_ms,
		bool sync_was);

/*
 * Example:
 *
 * tpdo1_build_data_impl(uint8_t* buffer)
 * {
 * 		buffer[7] = ...
 * 		buffer[6] = ...
 * 		buffer[5] = ...
 * 		buffer[4] = ...
 * 		buffer[3] = ...
 * 		buffer[2] = ...
 * 		buffer[1] = ...
 * 		buffer[0] = ...
 *
 * }
 *
 *
 *
 * */
static inline void CO_TPDO_set_build_data_manually_interface(CO_TPDO* p_tpdo,
		CO_TPDO_Builder_Data_Manually_Interface build_data_manually_impl)
{
	p_tpdo->build_data_manually = build_data_manually_impl;
}

static inline void CO_TPDO_send_request(CO_TPDO* p_tpdo)
{
	p_tpdo->send_request = true;
}

void CO_TPDO_set_node_id(CO_TPDO* p_tpdo, uint8_t node_id);
void CO_TPDO_set_transmission_type(CO_TPDO *p_tpdo, uint16_t type);
void CO_TPDO_set_sync_mask_reg(CO_TPDO *p_tpdo, uint32_t mask);


void tpdo_build_data_impl_default(uint8_t* buffer);

#endif
