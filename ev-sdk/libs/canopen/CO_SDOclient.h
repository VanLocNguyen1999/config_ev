/*
 * CO_SDOclient.h
 *
 *  Created on: Jun 23, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOCLIENT_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOCLIENT_H_

#include "CO_SDO.h"

#define CO_SDO_CLIENT_INIT_TYPE_1			0

/**
 * SDO client transmission type
 */
#define CO_SDO_CLIENT_TRANSM_TYPE_ALWAYS				(0)		/**< always processing, no synchronous (acyclic) */
#define CO_SDO_CLIENT_TRANSM_TYPE_SYNC_CYCLIC(x)		(x)		/**< synchronous: only processing when sync_counter = x and in window length
 	 	 	 	 	 	 	 	 	 	 	 	 	 		Range of x : [0-30000]*/

#if CO_SDO_CLIENT_INIT_TYPE_1
void CO_SDOclient_init(CO_SDO *p_sdo, const uint16_t client_trans_type, CO_SYNC *p_sync);
#else
void CO_SDOclient_init(CO_SDO *p_sdo, OD_Temp_Comm_Profile_Para_t *p_od_comm_prof_init_data, CO_SYNC *p_sync);
#endif
void CO_SDOclient_start_download(CO_SDO *p_sdo, const uint32_t server_node_id,
		const uint16_t index, const uint8_t sub_index, CO_Sub_Object *p_tx_obj,
		const uint16_t allowed_timeout_ms);

void CO_SDOclient_start_upload(CO_SDO *p_sdo, const uint32_t server_node_id,
		const uint16_t index, const uint8_t sub_index, CO_Sub_Object *p_rx_obj,
		const uint16_t allowed_timeout_ms);

void CO_SDOclient_process(CO_SDO *p_sdo, const uint16_t time_diff_ms);

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SDOCLIENT_H_ */
