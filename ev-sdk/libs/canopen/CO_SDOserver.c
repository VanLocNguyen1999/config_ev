/*
 * CO_SDOserver.c
 *
 *  Created on: Jun 23, 2022
 *      Author: Dakaka
 */

#include "CO_SDOserver.h"

static CO_SDO_return_t CO_SDOserver_download_initiate_response_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOserver_upload_initiate_response_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOserver_download_segment_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOserver_upload_segment_handle(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOserver_process_first_msg_and_set_state(CO_SDO *p_sdo);
static CO_Sub_Object* CO_SDOserver_get_sub_object_pointer(CO_SDO *p_sdo);
static bool CO_SDOserver_is_in_sync_window_len(CO_SDO* p_sdo);
static void CO_SDOserver_process_init_transfer_msg(CO_SDO *p_sdo);
static CO_SDO_return_t CO_SDOserver_process_first_msg_and_set_state(CO_SDO *p_sdo);
static inline CO_SDO_return_t CO_SDOserver_process_(CO_SDO *p_sdo, const uint16_t time_diff_ms);
static void co_sdoserver_copy_shadow_data_to_operating_data(CO_SDO *p_sdo);

#if CO_SDO_SERVER_INIT_TYPE_1
void CO_SDOserver_init(CO_SDO *p_sdo, uint8_t const server_node_id, const uint16_t server_trans_type,
		const uint16_t allowed_timeout_ms, CO_SYNC*	p_sync, CO_OD* p_od)
{
	p_sdo->valid 				= true;
	p_sdo->tx_msg.id.can_id 	= CO_CAN_ID_SDO_SRV + server_node_id;
	p_sdo->tx_msg.data_len 		= 8;
	p_sdo->tx_msg.is_new		= false;
	p_sdo->rx_msg.id.can_id 	= CO_CAN_ID_SDO_CLI + server_node_id;
	p_sdo->rx_msg.data_len 		= 0;
	p_sdo->rx_msg.is_new		= false;

	p_sdo->buffer_offset		= 0;
	p_sdo->trans_data_len		= 0;
	p_sdo->toggle				= 0;

	p_sdo->trans_type 			= server_trans_type;

	p_sdo->object_no			= 0;
	p_sdo->p_od					= p_od;
	p_sdo->index				= 0;
	p_sdo->sub_index			= 0;

	p_sdo->p_sub_obj				= NULL;
	p_sdo->p_data_itf			= NULL;

	CO_SDOtimeout_reset_and_inactivate(p_sdo);
	p_sdo->allowed_timeout_ms 	= allowed_timeout_ms;

	p_sdo->p_sync				= p_sync;
	p_sdo->is_internal_processing = false;
	p_sdo->tx_abort_code		= CO_SDO_AB_NONE;
	p_sdo->rx_abort_code		= CO_SDO_AB_NONE;

	p_sdo->state				= CO_SDO_ST_IDLE;
	p_sdo->status				= CO_SDO_RT_idle;
}

#else
void CO_SDOserver_init(CO_SDO *p_sdo,
		OD_Temp_Comm_Profile_Para_t *p_od_comm_prof_init_data, CO_SYNC *p_sync,
		CO_OD *p_od)
{
	p_sdo->valid 				= true;
	p_sdo->tx_msg.id.can_id 	= CO_CAN_ID_SDO_SRV + p_od_comm_prof_init_data->x1031_sdo_server_comm_para.node_id;
	p_sdo->tx_msg.data_len 		= 8;
	p_sdo->tx_msg.is_new		= false;
	p_sdo->rx_msg.id.can_id 	= CO_CAN_ID_SDO_CLI + p_od_comm_prof_init_data->x1031_sdo_server_comm_para.node_id;
	p_sdo->rx_msg.data_len 		= 0;
	p_sdo->rx_msg.is_new		= false;

	p_sdo->buffer_offset		= 0;
	p_sdo->trans_data_len		= 0;
	p_sdo->toggle				= 0;

	p_sdo->trans_type 			= p_od_comm_prof_init_data->x1031_sdo_server_comm_para.trans_type;

	p_sdo->object_no			= 0;
	p_sdo->p_od					= p_od;
	p_sdo->index				= 0;
	p_sdo->sub_index			= 0;

	p_sdo->p_sub_obj			= NULL;
	p_sdo->p_data_itf			= NULL;

	CO_SDOtimeout_reset_and_inactivate(p_sdo);
	p_sdo->allowed_timeout_ms 	= p_od_comm_prof_init_data->x1031_sdo_server_comm_para.allow_timeout_ms;

	p_sdo->p_sync				= p_sync;
	p_sdo->is_internal_processing = false;
	p_sdo->tx_abort_code		= CO_SDO_AB_NONE;
	p_sdo->rx_abort_code		= CO_SDO_AB_NONE;

	p_sdo->state				= CO_SDO_ST_IDLE;
	p_sdo->status				= CO_SDO_RT_idle;
}
#endif
void CO_SDOserver_process(CO_SDO *p_sdo, const uint16_t time_diff_ms)
{
	p_sdo->status = CO_SDOserver_process_(p_sdo, time_diff_ms);
}

static inline CO_SDO_return_t CO_SDOserver_process_(CO_SDO *p_sdo, const uint16_t time_diff_ms)
{
//	//test
//	volatile uint32_t test_timer = p_sdo->p_sync->time_in_cycle_ms;
//	volatile uint32_t test_sync_cter = p_sdo->p_sync->sync_counter;
//	//
	if(!p_sdo->valid) return CO_SDO_RT_busy;

	if(CO_SDOtimeout_is_activated(p_sdo))
	{
		CO_SDOtimeout_increase(p_sdo, time_diff_ms);
		if(CO_SDOtimeout_is_timeout(p_sdo))
		{
			CO_SDO_abort(p_sdo, CO_SDO_AB_TIMEOUT);
			return CO_SDO_RT_abort;
		}
	}

#if CO_SDO_USE_SYNC_WINDOW_LENGTH
	if(!CO_SDOserver_is_in_sync_window_len(p_sdo)) return CO_SDO_RT_busy;
#endif


	CO_SDO_return_t ret = CO_SDO_RT_busy;
	if (p_sdo->rx_msg.is_new || p_sdo->is_internal_processing)
	{
		if(p_sdo->rx_msg.is_new)
		{
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			if ((p_sdo->rx_msg.data[0] >> 5) == CCS_ABORT)
			{
				/* Receive abort msg from client */
				p_sdo->rx_abort_code 	= CO_getUint32(&p_sdo->rx_msg.data[4]);
				p_sdo->rx_msg.is_new	= 0;
				p_sdo->state = CO_SDO_ST_IDLE;
				return CO_SDO_RT_abort;
			}
			if(p_sdo->state == CO_SDO_ST_IDLE)
			{
				if(CO_SDO_RT_abort == CO_SDOserver_process_first_msg_and_set_state(p_sdo))
				{
					return CO_SDO_RT_abort;
				}
			}
		}

		switch (p_sdo->state)
		{

		/* Download state */
		case CO_SDO_ST_DOWNLOAD_INITIATE_RSP:
		{
			ret = CO_SDOserver_download_initiate_response_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_DOWNLOAD_SEGMENTED:
		{
			ret = CO_SDOserver_download_segment_handle(p_sdo);
		}
			break;

		/* Upload state */
		case CO_SDO_ST_UPLOAD_INITIATE_RSP:
		{
			ret = CO_SDOserver_upload_initiate_response_handle(p_sdo);
		}
			break;
		case CO_SDO_ST_UPLOAD_SEGMENTED:
		{
			ret = CO_SDOserver_upload_segment_handle(p_sdo);
		}
			break;
		default:
			break;
		}
		p_sdo->rx_msg.is_new	= 0;
	}
	if(p_sdo->state == CO_SDO_ST_IDLE) ret = CO_SDO_RT_idle;
	return ret;

}

void CO_SDOserver_set_node_id(CO_SDO* p_sdo, uint8_t server_node_id)
{
	bool valid = p_sdo->valid;
	p_sdo->valid = false;
	p_sdo->tx_msg.id.node_id = server_node_id;
	p_sdo->rx_msg.id.node_id = server_node_id;
	p_sdo->valid = valid;
}

static inline CO_Sub_Object* CO_SDOserver_get_sub_object_pointer(CO_SDO *p_sdo)
{
	p_sdo->index = CO_getUint16(&p_sdo->rx_msg.data[1]);
	p_sdo->sub_index = p_sdo->rx_msg.data[3];

	p_sdo->object_no = CO_OD_get_no(p_sdo->p_od, p_sdo->index);
	if (p_sdo->object_no == OD_UNSUPPORT_ENTRY)
	{
		return NULL;
	}
	/* Get and verify sub object*/
	CO_Sub_Object *p_sub_obj = CO_OD_get_sub_object_pointer(p_sdo->p_od, p_sdo->object_no, p_sdo->sub_index);
	if(p_sub_obj == NULL)
	{
		return NULL;
	}

	return p_sub_obj;
}

static inline CO_SDO_return_t CO_SDOserver_process_first_msg_and_set_state(CO_SDO *p_sdo)
{
	uint8_t ccs = (p_sdo->rx_msg.data[0] >> 5);
	/* Check CCS */
	if ((ccs != CCS_DOWNLOAD_INITIATE) && (ccs != CCS_UPLOAD_INITIATE))
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_CMD);
		return CO_SDO_RT_abort;
	}
	/* Get sub object according to MUX */
	p_sdo->p_sub_obj = CO_SDOserver_get_sub_object_pointer(p_sdo);
	if (p_sdo->p_sub_obj == NULL)
	{
		CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_SUB_UNKNOWN);
		return CO_SDO_RT_abort;
	}
	/* Verify access attribute of object and set state */
	if(CCS_DOWNLOAD_INITIATE == ccs)
	{
		if((p_sdo->p_sub_obj->attr & ODA_SDO_W) == 0)
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_UNSUPPORTED_ACCESS);
			return CO_SDO_RT_abort;
		}
#if 0
		p_sdo->p_data_itf =
				(p_sdo->p_sub_obj->p_ext == NULL) ?
						(p_sdo->p_sub_obj->p_data) : (p_sdo->p_sub_obj->p_ext->p_shadow_data);
		p_sdo->state = CO_SDO_ST_DOWNLOAD_INITIATE_RSP;
#else
		p_sdo->p_data_itf =
				(p_sdo->p_sub_obj->p_ext != NULL) ?
						(p_sdo->p_sub_obj->p_ext->p_shadow_data) :
						((p_sdo->p_sub_obj->p_temp_data != NULL) ?
								p_sdo->p_sub_obj->p_temp_data :
								p_sdo->p_sub_obj->p_data
						);

		p_sdo->state = CO_SDO_ST_DOWNLOAD_INITIATE_RSP;
#endif
	}
	else //upload init
	{
		if((p_sdo->p_sub_obj->attr & ODA_SDO_R) == 0)
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_UNSUPPORTED_ACCESS);
			return CO_SDO_RT_abort;
		}
		p_sdo->state = CO_SDO_ST_UPLOAD_INITIATE_RSP;
	}
	return CO_SDO_RT_busy;
}

static inline CO_SDO_return_t CO_SDOserver_download_initiate_response_handle(CO_SDO *p_sdo)
{
	if(p_sdo->is_internal_processing)
	{
		CO_SDO_reset_tx_abort_code(p_sdo);
		CO_Sub_Object_Ext_Confirm_Func_t ret = p_sdo->p_sub_obj->p_ext->confirm_func();
		if(CO_EXT_CONFIRM_copy_and_success == ret)
		{
			co_sdoserver_copy_shadow_data_to_operating_data(p_sdo);
			ret = CO_EXT_CONFIRM_success;
		}
		if(CO_EXT_CONFIRM_success == ret)
		{
			p_sdo->is_internal_processing = false;
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			/* Response and return success */
			CO_CAN_send(&p_sdo->tx_msg);
			p_sdo->state = CO_SDO_ST_IDLE;
			return CO_SDO_RT_success;
		}
		if(CO_EXT_CONFIRM_abort == ret)
		{
			p_sdo->is_internal_processing = false;
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			/* Response and return abort */
			if(CO_SDO_AB_NONE == p_sdo->tx_abort_code)
			{
				CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_DATA_LOC_CTRL);
			}
			else
			{
				CO_SDO_abort_and_response(p_sdo, p_sdo->tx_abort_code);
			}

			return CO_SDO_RT_abort;
		}
		return CO_SDO_RT_busy;
	}
	else
	{
		CO_SDOserver_process_init_transfer_msg(p_sdo);

		const uint8_t rx_cs = p_sdo->rx_msg.data[0];
		if ((rx_cs & SDO_MASK_INIT_ES_BIT) == SDO_MASK_INIT_ES_BIT) 	/* Expedited transfer, has size */
		{
			/* Get data length and write data to object */
			p_sdo->trans_data_len = 4 - ((rx_cs & SDO_MASK_INIT_N_BIT) >> 2);
	#if CO_SDO_CHECK_MATCH_DATA_LENGTH
			if (p_sdo->p_sub_obj->len < p_sdo->trans_data_len)
			{
				CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_OUT_OF_MEM);
				return CO_SDO_RT_abort;
			}
	#endif
			CO_memcpy(p_sdo->p_data_itf, &p_sdo->rx_msg.data[4],
					p_sdo->trans_data_len);
			p_sdo->buffer_offset = p_sdo->trans_data_len;
			/* Build CS */
			p_sdo->tx_msg.data[0] = (SCS_DOWNLOAD_INITIATED << 5);

			if(p_sdo->p_sub_obj->p_ext == NULL)
			{
				/**/
				if (p_sdo->p_sub_obj->p_temp_data != NULL
						&& (!(p_sdo->p_sub_obj->attr & ODA_VWTE_after_reset)))
				{
					CO_memcpy(p_sdo->p_sub_obj->p_data, p_sdo->p_sub_obj->p_temp_data, p_sdo->p_sub_obj->len);
				}
				/* Response and return success */
				CO_CAN_send(&p_sdo->tx_msg);
				p_sdo->state = CO_SDO_ST_IDLE;
				return CO_SDO_RT_success;
			}
			else
			{
				/* Switch to extension handler */
				CO_SDOtimeout_reset_and_activate(p_sdo);
				p_sdo->is_internal_processing = true;
				return CO_SDO_RT_busy;
			}
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
			p_sdo->tx_msg.data[0] = (SCS_DOWNLOAD_INITIATED << 5);
			/* Response */
			CO_SDOtimeout_reset_and_activate(p_sdo);
			CO_CAN_send(&p_sdo->tx_msg);

			p_sdo->state = CO_SDO_ST_DOWNLOAD_SEGMENTED;
			return CO_SDO_RT_busy;
		}
		else	/* Not support case of no size */
		{
			CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_TYPE_MISMATCH);
			return CO_SDO_RT_abort;
		}
	}
}
static inline CO_SDO_return_t CO_SDOserver_download_segment_handle(CO_SDO *p_sdo)
{
	if(p_sdo->is_internal_processing)
	{
		CO_SDO_reset_tx_abort_code(p_sdo);
		CO_Sub_Object_Ext_Confirm_Func_t ret = p_sdo->p_sub_obj->p_ext->confirm_func();
		if(CO_EXT_CONFIRM_copy_and_success == ret)
		{
			co_sdoserver_copy_shadow_data_to_operating_data(p_sdo);
			ret = CO_EXT_CONFIRM_success;
		}
		if(CO_EXT_CONFIRM_success == ret)
		{
			p_sdo->is_internal_processing = false;
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			/* Response and return success */
			CO_CAN_send(&p_sdo->tx_msg);
			p_sdo->state = CO_SDO_ST_IDLE;
			return CO_SDO_RT_success;
		}
		if(CO_EXT_CONFIRM_abort == ret)
		{
			p_sdo->is_internal_processing = false;
			CO_SDOtimeout_reset_and_inactivate(p_sdo);
			/* Response and return abort */
			if(CO_SDO_AB_NONE == p_sdo->tx_abort_code)
			{
				CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_DATA_LOC_CTRL);
			}
			else
			{
				CO_SDO_abort_and_response(p_sdo, p_sdo->tx_abort_code);
			}

			return CO_SDO_RT_abort;
		}
		return CO_SDO_RT_busy;
	}
	else
	{
		/* Verify CCS */
		const uint8_t rx_cs = p_sdo->rx_msg.data[0];
		if( CCS_DOWNLOAD_SEGMENT != (rx_cs >> 5))
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
		/* Write data to object */
		uint8_t* p_dst = p_sdo->p_data_itf + p_sdo->buffer_offset;
		if (p_sdo->trans_data_len <= p_sdo->buffer_offset + 7)
		{
	#if CO_SDO_CHECK_C_BIT_OF_CS
			if ((rx_cs & SDO_MASK_SEG_C_BIT) != 1)
			{
				CO_SDO_abort_and_response(p_sdo, CO_SDO_AB_GENERAL);
				return CO_SDO_RT_abort;
			}
	#endif
			CO_memcpy(p_dst, &p_sdo->rx_msg.data[1],
					p_sdo->trans_data_len - p_sdo->buffer_offset);
			p_sdo->buffer_offset = p_sdo->trans_data_len;
			/* Build CS */
			uint8_t tx_cs = (SCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle;
			p_sdo->tx_msg.data[0] = tx_cs;

			if(p_sdo->p_sub_obj->p_ext == NULL)
			{
				/**/
				if (p_sdo->p_sub_obj->p_temp_data != NULL
						&& (!(p_sdo->p_sub_obj->attr & ODA_VWTE_after_reset)))
				{
					CO_memcpy(p_sdo->p_sub_obj->p_data, p_sdo->p_sub_obj->p_temp_data, p_sdo->p_sub_obj->len);
				}
				/* Response and return success */
				CO_CAN_send(&p_sdo->tx_msg);
				p_sdo->state = CO_SDO_ST_IDLE;
				return CO_SDO_RT_success;
			}
			else
			{
				/* Switch to extension handler */
				CO_SDOtimeout_reset_and_activate(p_sdo);
				p_sdo->is_internal_processing = true;
				return CO_SDO_RT_busy;
			}
		}
		else
		{
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
			uint8_t tx_cs = (SCS_DOWNLOAD_SEGMENT << 5)
					| p_sdo->toggle;
			p_sdo->tx_msg.data[0] = tx_cs;
			/* Response */
			CO_SDOtimeout_reset_and_activate(p_sdo);
			CO_CAN_send(&p_sdo->tx_msg);
			p_sdo->toggle ^= (1UL << 4);
			return CO_SDO_RT_busy;
		}
	}
}

static inline CO_SDO_return_t CO_SDOserver_upload_initiate_response_handle(CO_SDO *p_sdo)
{
	CO_SDOserver_process_init_transfer_msg(p_sdo);
	p_sdo->trans_data_len = p_sdo->p_sub_obj->len;
	if(p_sdo->trans_data_len <= 4) /* Expedited transfer, has size */
	{
		/* Build CS and data*/
		uint8_t tx_cs = (SCS_UPLOAD_INITIATE << 5)
				| (((4 - p_sdo->trans_data_len) << 2) & SDO_MASK_INIT_N_BIT)
				| SDO_MASK_INIT_E_BIT
				| SDO_MASK_INIT_S_BIT;
		p_sdo->tx_msg.data[0] = tx_cs;

		CO_memcpy(&p_sdo->tx_msg.data[4], p_sdo->p_sub_obj->p_data, p_sdo->trans_data_len);
		/* Response and return success */
		CO_CAN_send(&p_sdo->tx_msg);
		p_sdo->buffer_offset = p_sdo->trans_data_len;
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else	/* Normal transfer, has size */
	{
		/* Build CS and data*/
		uint8_t tx_cs = (SCS_UPLOAD_INITIATE << 5)
				|SDO_MASK_INIT_S_BIT;
		p_sdo->tx_msg.data[0] = tx_cs;

		CO_setUint32(&p_sdo->tx_msg.data[4], p_sdo->trans_data_len);
		/* Response */
		CO_CAN_send(&p_sdo->tx_msg);

		CO_SDOtimeout_reset_and_activate(p_sdo);
		p_sdo->state = CO_SDO_ST_UPLOAD_SEGMENTED;
		return CO_SDO_RT_busy;
	}
}

static inline CO_SDO_return_t CO_SDOserver_upload_segment_handle(CO_SDO *p_sdo)
{
	/* Verify CCS */
	const uint8_t rx_cs = p_sdo->rx_msg.data[0];
	if (CCS_UPLOAD_SEGMENT != (rx_cs >> 5))
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
    /* Main processing */
	uint8_t *p_src = p_sdo->p_sub_obj->p_data + p_sdo->buffer_offset;
	if (p_sdo->trans_data_len <= (p_sdo->buffer_offset + 7))
	{	/* Build data */
		uint8_t cur_trans_data_len = p_sdo->trans_data_len - p_sdo->buffer_offset;
		CO_memcpy(&p_sdo->tx_msg.data[1], p_src, cur_trans_data_len);
		p_sdo->buffer_offset =  p_sdo->trans_data_len;
		/* Build CS */
		uint8_t tx_cs = (SCS_UPLOAD_SEGMENT << 5)
				| p_sdo->toggle
				| ((7 - cur_trans_data_len) << 1)
				| SDO_MASK_SEG_C_BIT;
		p_sdo->tx_msg.data[0] = tx_cs;
		/* Response and return success */
		CO_CAN_send(&p_sdo->tx_msg);
		p_sdo->toggle ^= (1UL << 4);
		p_sdo->state = CO_SDO_ST_IDLE;
		return CO_SDO_RT_success;
	}
	else
	{	/* Build data */
		CO_memcpy(&p_sdo->tx_msg.data[1], p_src, 7);
		p_sdo->buffer_offset += 7;
		/* Build CS */
		uint8_t tx_cs = (SCS_UPLOAD_SEGMENT << 5)
				| p_sdo->toggle;
		p_sdo->tx_msg.data[0] = tx_cs;
		/* Response */
		CO_CAN_send(&p_sdo->tx_msg);

		CO_SDOtimeout_reset_and_activate(p_sdo);
		p_sdo->toggle ^= (1UL << 4);
		return CO_SDO_RT_busy;
	}
}

static inline bool CO_SDOserver_is_in_sync_window_len(CO_SDO* p_sdo)
{
	bool in_window = CO_SYNC_is_in_window_length(p_sdo->p_sync);
//	return !((!in_window )&&(p_sdo->trans_type != CO_SDO_TRANSM_TYPE_ALWAYS));
	return ((p_sdo->trans_type == CO_SDO_SERVER_TRANSM_TYPE_ALWAYS)? 1 : in_window);
}

static void CO_SDOserver_process_init_transfer_msg(CO_SDO *p_sdo)
{
	p_sdo->toggle		= 0x00;
	p_sdo->buffer_offset = 0;
	/* copy mux - index and sub index data bytes */
	p_sdo->tx_msg.data[1] = p_sdo->rx_msg.data[1];
	p_sdo->tx_msg.data[2] = p_sdo->rx_msg.data[2];
	p_sdo->tx_msg.data[3] = p_sdo->rx_msg.data[3];
}

static void co_sdoserver_copy_shadow_data_to_operating_data(CO_SDO *p_sdo)
{
	if(p_sdo->p_sub_obj->p_temp_data == NULL)
	{
		CO_memcpy(p_sdo->p_sub_obj->p_data, p_sdo->p_sub_obj->p_ext->p_shadow_data, p_sdo->p_sub_obj->len);
	}
	else
	{
		CO_memcpy(p_sdo->p_sub_obj->p_temp_data, p_sdo->p_sub_obj->p_ext->p_shadow_data, p_sdo->p_sub_obj->len);
		if(!(p_sdo->p_sub_obj->attr & ODA_VWTE_after_reset))
		{
			CO_memcpy(p_sdo->p_sub_obj->p_data, p_sdo->p_sub_obj->p_ext->p_shadow_data, p_sdo->p_sub_obj->len);
		}
	}
}
