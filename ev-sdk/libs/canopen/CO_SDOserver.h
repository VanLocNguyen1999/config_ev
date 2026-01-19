/*
 * CO_SDOserver.h
 *
 *  Created on: Jun 23, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOSERVER_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOSERVER_H_

#include "CO_SDO.h"

#define CO_SDO_SERVER_INIT_TYPE_1			0

/**
 * SDO server transmission type
 */
#define CO_SDO_SERVER_TRANSM_TYPE_ALWAYS				(0)		/**< always processing, no synchronous (acyclic) */
#define CO_SDO_SERVER_TRANSM_TYPE_SYNC					(1)		/**< synchronous: only processing in window length */

#if CO_SDO_SERVER_INIT_TYPE_1
void CO_SDOserver_init(CO_SDO *p_sdo, uint8_t const server_node_id, const uint16_t server_trans_type,
		const uint16_t allowed_timeout_ms, CO_SYNC*	p_sync, CO_OD* p_od);
#else
void CO_SDOserver_init(CO_SDO *p_sdo,
		OD_Temp_Comm_Profile_Para_t *p_od_comm_prof_init_data, CO_SYNC *p_sync,
		CO_OD *p_od);
#endif
void CO_SDOserver_process(CO_SDO *p_sdo, const uint16_t time_diff_ms);
void CO_SDOserver_set_node_id(CO_SDO* p_sdo, uint8_t server_node_id);

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOSERVER_H_ */
