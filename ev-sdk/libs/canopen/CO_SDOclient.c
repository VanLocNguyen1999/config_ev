/*
 * CO_SDOclien.c
 *
 *  Created on: Jun 23, 2022
 *      Author: Dakaka
 */


#include "CO_SDOclient.h"

static CO_SDO_return_t CO_SDOclient_download_initiate_request_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOclient_download_initiate_response_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOclient_upload_initiate_request_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOclient_upload_initiate_response_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOclient_download_segment_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOclient_upload_segment_handle(CO_SDO *p_sdo);
static bool CO_SDOclient_is_in_sync_window_len(CO_SDO* p_sdo);
static void CO_SDOclient_process_init_transfer_msg(CO_SDO *p_sdo);
static inline CO_SDO_return_t CO_SDOclient_process_(CO_SDO *p_sdo, const uint16_t time_diff_ms);

#if CO_SDO_CLIENT_INIT_TYPE_1
void CO_SDOclient_init(CO_SDO *p_sdo, const uint16_t client_trans_type, CO_SYNC *p_sync)
{
	p_sdo->valid 				= true;
	p_sdo->tx_msg.id.can_id		= CO_CAN_ID_SDO_CLI;	//<< + server node id
	p_sdo->tx_msg.data_len 		= 8;
	p_sdo->tx_msg.is_new		= false;
	p_sdo->rx_msg.id.can_id 	= CO_CAN_ID_SDO_SRV;	//<< + server node id
	p_sdo->rx_msg.is_new		= false;

	p_sdo->buffer_offset		= 0;
	p_sdo->trans_data_len		= 0;
	p_sdo->toggle				= 0;

	p_sdo->trans_type 			= client_trans_type;

	p_sdo->index				= 0;
	p_sdo->sub_index			= 0;

	p_sdo->p_sub_obj				= NULL;
	p_sdo->p_data_itf			= NULL;

	CO_SDOtimeout_reset_and_inactivate(p_sdo);
	p_sdo->allowed_timeout_ms 	= 0;
	p_sdo->p_sync				= p_sync;
	p_sdo->is_internal_processing = false;
	p_sdo->tx_abort_code		= CO_SDO_AB_NONE;
	p_sdo->rx_abort_code		= CO_SDO_AB_NONE;

	p_sdo->state				= CO_SDO_ST_IDLE;
	p_sdo->status				= CO_SDO_RT_idle;
}
#else
void CO_SDOclient_init(CO_SDO *p_sdo, OD_Temp_Comm_Profile_Para_t *p_od_comm_prof_init_data, CO_SYNC *p_sync)
{
	p_sdo->valid 				= true;
	p_sdo->tx_msg.id.can_id		= CO_CAN_ID_SDO_CLI;	//<< + server node id
	p_sdo->tx_msg.data_len 		= 8;
	p_sdo->tx_msg.is_new		= false;
	p_sdo->rx_msg.id.can_id 	= CO_CAN_ID_SDO_SRV;	//<< + server node id
	p_sdo->rx_msg.is_new		= false;

	p_sdo->buffer_offset		= 0;
	p_sdo->trans_data_len		= 0;
	p_sdo->toggle				= 0;

	p_sdo->trans_type 			= p_od_comm_prof_init_data->x1032_sdo_client_comm_para.trans_type;
	p_sdo->sync_mask_reg		= p_od_comm_prof_init_data->x1032_sdo_client_comm_para.sync_mask_reg;

	p_sdo->index				= 0;
	p_sdo->sub_index			= 0;

	p_sdo->p_sub_obj			= NULL;
	p_sdo->p_data_itf			= NULL;

	CO_SDOtimeout_reset_and_inactivate(p_sdo);
	p_sdo->allowed_timeout_ms 	= 0;
	p_sdo->p_sync				= p_sync;
	p_sdo->is_internal_processing = false;
	p_sdo->tx_abort_code		= CO_SDO_AB_NONE;
	p_sdo->rx_abort_code		= CO_SDO_AB_NONE;

	p_sdo->state				= CO_SDO_ST_IDLE;
	p_sdo->status				= CO_SDO_RT_idle;
}
#endif

CO_Sub_Object temp_sdo_cli_sub_obj;

/* Example:
uint8_t exam_var	= 1234;
CO_Sub_Object exam_tx_obj =
{
		.p_data = &exam_var,	//<< Address variable receiving data
		.attr	= ODA_SDO_RW,	//<< [skip] set ODA_SDO_RW
		.len	= 10,			//<< data size
		.p_ext	= NULL		//<< [skip] set NULL
};
CO_SDOclient_start_download(___, ____,
		index, sub_index, 10,
		&exam_tx_obj, 500);
*/
void CO_SDOclient_start_download(CO_SDO *p_sdo, const uint32_t server_node_id,
		const uint16_t index, const uint8_t sub_index, CO_Sub_Object *p_tx_obj,
		const uint16_t allowed_timeout_ms)
{
	p_sdo->status = CO_SDO_RT_busy;
	/* Set communication parameter of server */
	p_sdo->tx_msg.id.node_id	= server_node_id;
	p_sdo->rx_msg.id.node_id	= server_node_id;
	p_sdo->index				= index;
	p_sdo->sub_index			= sub_index;
	p_sdo->allowed_timeout_ms	= allowed_timeout_ms;

	temp_sdo_cli_sub_obj.attr 	= p_tx_obj->attr;
	temp_sdo_cli_sub_obj.len 	= p_tx_obj->len;
	temp_sdo_cli_sub_obj.p_data = p_tx_obj->p_data;
	temp_sdo_cli_sub_obj.p_ext 	= p_tx_obj->p_ext;

	p_sdo->p_sub_obj				= &temp_sdo_cli_sub_obj;
	/* Switch to SDOclient_process for further processing */
	p_sdo->is_internal_processing = true;
	p_sdo->state = CO_SDO_ST_DOWNLOAD_INITIATE_REQ;
}

/* Example:
uint8_t* p_exam_var	= CO_NULL;
CO_Sub_Object exam_rx_obj =
{
		.p_data = p_exam_var,	//<< Address variable receiving data
		.attr	= ODA_SDO_RW,	//<< [skip] set ODA_SDO_RW
		.len	= 10,			//<< Maximum data size that can be received
		.p_ext	= NULL		//<< [option], set NULL if not used
};
CO_SDOclient_start_upload(___, ____,
		index, sub_index, 10,
		&exam_rx_obj, 500);
*/
void CO_SDOclient_start_upload(CO_SDO *p_sdo, const uint32_t server_node_id,
		const uint16_t index, const uint8_t sub_index, CO_Sub_Object *p_rx_obj,
		const uint16_t allowed_timeout_ms)
{
	p_sdo->status = CO_SDO_RT_busy;
	/* Set communication parameter of server */
	p_sdo->tx_msg.id.node_id	= server_node_id;
	p_sdo->rx_msg.id.node_id	= server_node_id;
	p_sdo->index				= index;
	p_sdo->sub_index			= sub_index;
	p_sdo->allowed_timeout_ms	= allowed_timeout_ms;

	temp_sdo_cli_sub_obj.attr 	= p_rx_obj->attr;
	temp_sdo_cli_sub_obj.len 	= p_rx_obj->len;
	temp_sdo_cli_sub_obj.p_data = p_rx_obj->p_data;
	temp_sdo_cli_sub_obj.p_ext 	= p_rx_obj->p_ext;

	p_sdo->p_sub_obj				= &temp_sdo_cli_sub_obj;
	/* Switch to SDOclient_process for further processing */
	p_sdo->is_internal_processing = true;
	p_sdo->state = CO_SDO_ST_UPLOAD_INITIATE_REQ;
}

void CO_SDOclient_process(CO_SDO *p_sdo, const uint16_t time_diff_ms)
{
	CO_SDO_return_t ret = CO_SDOclient_process_(p_sdo, time_diff_ms);
	if(CO_SDO_RT_idle != ret)
	{
		p_sdo->status = ret;
	}
}


static inline CO_SDO_return_t CO_SDOclient_process_(CO_SDO *p_sdo, const uint16_t time_diff_ms)
{
//	//test
//	volatile uint32_t test_timer = p_sdo->p_sync->time_in_cycle_ms;
//	volatile uint32_t test_sync_cter = p_sdo->p_sync->sync_counter;
//	//
	if(!p_sdo->valid)
	{
		return CO_SDO_RT_busy;
	}
	else if (p_sdo->state == CO_SDO_ST_IDLE)
	{
		return CO_SDO_RT_idle;
	}

	if(CO_SDOtimeout_is_activated(p_sdo))
	{
		CO_SDOtimeout_increase(p_sdo, time_diff_ms);
		if(CO_SDOtimeout_is_timeout(p_sdo))
		{
			CO_SDO_abort(p_sdo, CO_SDO_AB_TIMEOUT);
			return CO_SDO_RT_abort;
		}
	}

	if(!CO_SDOclient_is_in_sync_window_len(p_sdo)) return CO_SDO_RT_busy;

	CO_SDO_return_t ret = CO_SDO_RT_busy;
	if (p_sdo->rx_msg.is_new || p_sdo->is_internal_processing)
	{
		if(p_sdo->rx_msg.is_new)
		{
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			if ((p_sdo->rx_msg.data[0] >> 5) == SCS_ABORT)
			{
				/* Receive abort msg from server */
				p_sdo->rx_abort_code 	= CO_getUint32(&p_sdo->rx_msg.data[4]);
				p_sdo->rx_msg.is_new	= 0;
				p_sdo->state = CO_SDO_ST_IDLE;
				return CO_SDO_RT_abort;
			}
		}

		switch (p_sdo->state)
		{

		/* Download state */
		case CO_SDO_ST_DOWNLOAD_INITIATE_REQ:
		{
			ret = CO_SDOclient_download_initiate_request_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_DOWNLOAD_INITIATE_RSP:
		{
			ret = CO_SDOclient_download_initiate_response_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_DOWNLOAD_SEGMENTED:
		{
			ret = CO_SDOclient_download_segment_handle(p_sdo);
		}
			break;

		/* Upload state */
		case CO_SDO_ST_UPLOAD_INITIATE_REQ:
		{
			ret = CO_SDOclient_upload_initiate_request_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_UPLOAD_INITIATE_RSP:
		{
			ret = CO_SDOclient_upload_initiate_response_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_UPLOAD_SEGMENTED:
		{
			ret = CO_SDOclient_upload_segment_handle(p_sdo);
		}
			break;
		default:
			break;
		}
		p_sdo->rx_msg.is_new	= 0;
	}
	return ret;
}

static inline CO_SDO_return_t CO_SDOclient_download_initiate_request_handle(CO_SDO *p_sdo)
{
	CO_SDOclient_process_init_transfer_msg(p_sdo);
	p_sdo->trans_data_len = p_sdo->p_sub_obj->len;
	if(p_sdo->trans_data_len <= 4) /* Expedited transfer, has size */
	{
		/* Build CS and data*/
		uint8_t tx_cs = (CCS_DOWNLOAD_INITIATE << 5)
				| (((4 - p_sdo->trans_data_len) << 2) & SDO_MASK_INIT_N_BIT)
				| SDO_MASK_INIT_E_BIT
				| SDO_MASK_INIT_S_BIT;
		p_sdo->tx_msg.data[0] = tx_cs;

		CO_memcpy(&p_sdo->tx_msg.data[4], p_sdo->p_sub_obj->p_data, p_sdo->trans_data_len);
	}
	else	/* Normal transfer, has size */
	{
		/* Build CS and data*/
		uint8_t tx_cs = (CCS_DOWNLOAD_INITIATE << 5)
				|SDO_MASK_INIT_S_BIT;
		p_sdo->tx_msg.data[0] = tx_cs;

		CO_setUint32(&p_sdo->tx_msg.data[4], p_sdo->trans_data_len);
	}
	/* Request */
	CO_CAN_send(&p_sdo->tx_msg);

	CO_SDOtimeout_reset_and_activate(p_sdo);
	p_sdo->is_internal_processing = false;
	p_sdo->state = CO_SDO_ST_DOWNLOAD_INITIATE_RSP;
	return CO_SDO_RT_busy;
}
static inline CO_SDO_return_t CO_SDOclient_download_initiate_response_handle(CO_SDO *p_sdo)
{
//	int8_t ret = CO_SDO_RT_ok;
	/* Verify CCS */
	const uint8_t rx_cs = p_sdo->rx_msg.data[0];
	if (SCS_DOWNLOAD_INITIATED != (rx_cs >> 5))
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_NO_RESOURCE);
		return CO_SDO_RT_abort;
	}

	/* Verify MUX */
    uint16_t index = CO_getUint16(&p_sdo->rx_msg.data[1]);
    uint8_t subindex = p_sdo->rx_msg.data[3];
    if (index != p_sdo->index || subindex != p_sdo->sub_index)
    {
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_PRAM_INCOMPAT);
		return CO_SDO_RT_abort;
    }

    /* Transfer */
	if(p_sdo->trans_data_len <= 4) /* Expedited transfer */
	{
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else							/* Normal transfer */
	{
		/* Build data and CS */
		uint8_t *p_src = p_sdo->p_sub_obj->p_data;
		if (p_sdo->trans_data_len <= (p_sdo->buffer_offset + 7))
		{	//data
			uint8_t cur_trans_data_len = p_sdo->trans_data_len;
			CO_memcpy(&p_sdo->tx_msg.data[1], p_src, cur_trans_data_len);
			p_sdo->buffer_offset = p_sdo->trans_data_len;
			//cs
			uint8_t tx_cs = (CCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle
					| ((7 - cur_trans_data_len) << 1)
					| SDO_MASK_SEG_C_BIT;
			p_sdo->tx_msg.data[0] = tx_cs;
		}
		else
		{	//data
			CO_memcpy(&p_sdo->tx_msg.data[1], p_src, 7);
			p_sdo->buffer_offset = 7;
			//cs
			uint8_t tx_cs = (CCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle;
			p_sdo->tx_msg.data[0] = tx_cs;
		}
		/* Request */
		CO_SDOtimeout_reset_and_activate(p_sdo);
		CO_CAN_send(&p_sdo->tx_msg);
		p_sdo->state = CO_SDO_ST_DOWNLOAD_SEGMENTED;
		return CO_SDO_RT_busy;
	}
}

static inline CO_SDO_return_t CO_SDOclient_download_segment_handle(CO_SDO *p_sdo)
{
	/* Verify CCS */
	const uint8_t rx_cs = p_sdo->rx_msg.data[0];
	if (SCS_DOWNLOAD_SEGMENT != (rx_cs >> 5))
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_NO_RESOURCE);
		return CO_SDO_RT_abort;
	}

	/* Check toggle bit */
	if ((rx_cs & SDO_MASK_SEG_T_BIT) != p_sdo->toggle)
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_TOGGLE_BIT);
		return CO_SDO_RT_abort;
	}
	p_sdo->toggle ^= (1UL << 4);

	if(p_sdo->trans_data_len <= p_sdo->buffer_offset)	//success
	{
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else	// transfer
	{
		/* Build data and CS */
		uint8_t *p_src = p_sdo->p_sub_obj->p_data + p_sdo->buffer_offset;
		if (p_sdo->trans_data_len <= (p_sdo->buffer_offset + 7))
		{	//data
			uint8_t cur_trans_data_len = p_sdo->trans_data_len - p_sdo->buffer_offset;
			CO_memcpy(&p_sdo->tx_msg.data[1], p_src, cur_trans_data_len);
			p_sdo->buffer_offset =  p_sdo->trans_data_len;
			//cs
			uint8_t tx_cs = (CCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle
					| ((7 - cur_trans_data_len) << 1)
					| SDO_MASK_SEG_C_BIT;
			p_sdo->tx_msg.data[0] = tx_cs;
		}
		else
		{	//data
			CO_memcpy(&p_sdo->tx_msg.data[1], p_src, 7);
			p_sdo->buffer_offset += 7;
			//cs
			uint8_t tx_cs = (CCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle;
			p_sdo->tx_msg.data[0] = tx_cs;
		}
		/* Request */
		CO_SDOtimeout_reset_and_activate(p_sdo);
		CO_CAN_send(&p_sdo->tx_msg);
		p_sdo->state = CO_SDO_ST_DOWNLOAD_SEGMENTED;
		return CO_SDO_RT_busy;
	}
}



static inline CO_SDO_return_t CO_SDOclient_upload_initiate_request_handle(CO_SDO *p_sdo)
{
	CO_SDOclient_process_init_transfer_msg(p_sdo);
	/* Build CS */
	uint8_t tx_cs = CCS_UPLOAD_INITIATE << 5;
	p_sdo->tx_msg.data[0] = tx_cs;

	/* Request */
	CO_CAN_send(&p_sdo->tx_msg);

	CO_SDOtimeout_reset_and_activate(p_sdo);
	p_sdo->is_internal_processing = false;
	p_sdo->state = CO_SDO_ST_UPLOAD_INITIATE_RSP;
	return CO_SDO_RT_busy;
}
static inline CO_SDO_return_t CO_SDOclient_upload_initiate_response_handle(CO_SDO *p_sdo)
{
	/* Verify CCS */
	const uint8_t rx_cs = p_sdo->rx_msg.data[0];
	if (SCS_UPLOAD_INITIATE != (rx_cs >> 5))
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_NO_RESOURCE);
		return CO_SDO_RT_abort;
	}

	/* Verify MUX */
    uint16_t index = CO_getUint16(&p_sdo->rx_msg.data[1]);
    uint8_t subindex = p_sdo->rx_msg.data[3];
    if (index != p_sdo->index || subindex != p_sdo->sub_index)
    {
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_PRAM_INCOMPAT);
		return CO_SDO_RT_abort;
    }
    /* Main processing */
	if ((rx_cs & SDO_MASK_INIT_ES_BIT) == SDO_MASK_INIT_ES_BIT) 	/* Expedited transfer, has size */
	{
		/* Get data length and write data to object */
		p_sdo->trans_data_len = 4 - ((rx_cs & SDO_MASK_INIT_N_BIT) >> 2);
#if CO_SDO_CHECK_MATCH_DATA_LENGTH
		if (p_sdo->p_sub_obj->len < p_sdo->trans_data_len)
		{
			CO_SDO_abort(p_sdo, CO_SDO_AB_OUT_OF_MEM);
			return CO_SDO_RT_abort;
		}
#endif
		CO_memcpy(p_sdo->p_sub_obj->p_data, &p_sdo->rx_msg.data[4],
				p_sdo->trans_data_len);
		/* Success */
		p_sdo->buffer_offset = p_sdo->trans_data_len;
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else if ((rx_cs & SDO_MASK_INIT_ES_BIT) == SDO_MASK_INIT_S_BIT) 	/* Normal transfer, has size */
	{
		/* Get data length*/
		p_sdo->trans_data_len = CO_getUint32(&(p_sdo->rx_msg.data[4]));
#if CO_SDO_CHECK_MATCH_DATA_LENGTH
		if (p_sdo->p_sub_obj->len < p_sdo->trans_data_len)
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_OUT_OF_MEM);
			return CO_SDO_RT_abort;
		}
#endif
		p_sdo->buffer_offset = 0;
		/* Build CS */
		p_sdo->tx_msg.data[0] = (CCS_UPLOAD_SEGMENT << 5);
		/* Response */
		CO_SDOtimeout_reset_and_activate(p_sdo);
		CO_CAN_send(&p_sdo->tx_msg);
		p_sdo->state = CO_SDO_ST_UPLOAD_SEGMENTED;
		return CO_SDO_RT_busy;
	}
	else	/* Not support case of no size */
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_TYPE_MISMATCH);
		return CO_SDO_RT_abort;
	}
}

static inline CO_SDO_return_t CO_SDOclient_upload_segment_handle(CO_SDO *p_sdo)
{
	/* Verify CCS */
	const uint8_t rx_cs = p_sdo->rx_msg.data[0];
	if( SCS_UPLOAD_SEGMENT != (rx_cs >> 5))
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_NO_RESOURCE);
		return CO_SDO_RT_abort;
	}

	/* Check toggle bit */
	if ((rx_cs & SDO_MASK_SEG_T_BIT) != p_sdo->toggle)
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_TOGGLE_BIT);
		return CO_SDO_RT_abort;
	}
	p_sdo->toggle ^= (1UL << 4);

    /* Main processing */
	uint8_t* p_dst = p_sdo->p_sub_obj->p_data + p_sdo->buffer_offset;
	if (p_sdo->trans_data_len <= p_sdo->buffer_offset + 7)
	{	/* Write data to object */
#if CO_SDO_CHECK_C_BIT_OF_CS
		if ((rx_cs & SDO_MASK_SEG_C_BIT) != 1)
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_GENERAL);
			return CO_SDO_RT_abort;
		}
#endif
		CO_memcpy(p_dst, &p_sdo->rx_msg.data[1],
				p_sdo->trans_data_len - p_sdo->buffer_offset);
		/* Success */
		p_sdo->buffer_offset = p_sdo->trans_data_len;
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else
	{	/* Write data to object */
#if CO_SDO_CHECK_C_BIT_OF_CS
		if ((rx_cs & SDO_MASK_SEG_C_BIT) == 1)
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_GENERAL);
			return CO_SDO_RT_abort;
		}
#endif
		CO_memcpy(p_dst, &p_sdo->rx_msg.data[1], 7);
		p_sdo->buffer_offset += 7;
		/* Build CS */
		uint8_t tx_cs = (CCS_UPLOAD_SEGMENT << 5)
				| p_sdo->toggle;
		p_sdo->tx_msg.data[0] = tx_cs;
		/* Request */
		CO_CAN_send(&p_sdo->tx_msg);

		CO_SDOtimeout_reset_and_activate(p_sdo);
		return CO_SDO_RT_busy;
	}
}

static inline bool CO_SDOclient_is_in_sync_window_len(CO_SDO* p_sdo)
{
#if CO_SDO_USE_SYNC_WINDOW_LENGTH
	bool in_window = CO_SYNC_is_in_window_length(p_sdo->p_sync)
			&& (p_sdo->p_sync->sync_counter == p_sdo->trans_type)
			&& CO_SYNC_is_match_sync_mask_reg(p_sdo->p_sync, p_sdo->sync_mask_reg);
//	return !((!in_window )&&(p_sdo->trans_type != CO_SDO_TRANSM_TYPE_ALWAYS));
	return ((p_sdo->trans_type == CO_SDO_CLIENT_TRANSM_TYPE_ALWAYS)? 1 : in_window);
#else
	bool in_window = (p_sdo->p_sync->sync_counter == p_sdo->trans_type)
					&& CO_SYNC_is_match_sync_mask_reg(p_sdo->p_sync, p_sdo->sync_mask_reg);
	return ((p_sdo->trans_type == CO_SDO_CLIENT_TRANSM_TYPE_ALWAYS)? 1 : in_window);
#endif
}

static void CO_SDOclient_process_init_transfer_msg(CO_SDO *p_sdo)
{
	p_sdo->toggle		= 0x00;
	p_sdo->buffer_offset = 0;
	/* copy mux - index and sub index data bytes */
	p_sdo->tx_msg.data[1] = (uint8_t)p_sdo->index;
	p_sdo->tx_msg.data[2] = (uint8_t)(p_sdo->index >> 8);
	p_sdo->tx_msg.data[3] = p_sdo->sub_index;
}
