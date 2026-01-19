#include "CO_TPDO.h"

static void CO_TPDO_send(CO_TPDO* p_tpdo);

void CO_TPDO_init(CO_TPDO *p_tpdo,
		CO_SYNC* p_sync,
		CO_OD_PDO_Map_Para_Object_t *p_pdo_map_parameter,
		CO_OD* p_od_list,
		CO_OD_TPDO_Comm_Para_Obj_t *p_tpdo_comm_parameter,
		CO_TPDO_Builder_Data_Manually_Interface build_data_manually_impl)
{
	CO_PDO_init((CO_PDO*)p_tpdo, p_pdo_map_parameter, p_od_list);

	p_tpdo->comm_parameter.id.cob_id = p_tpdo_comm_parameter->id.cob_id;
	p_tpdo->comm_parameter.transmission_type = p_tpdo_comm_parameter->transmission_type;
	p_tpdo->comm_parameter.max_inhibit_time_ms = p_tpdo_comm_parameter->max_inhibit_time_ms;
	p_tpdo->comm_parameter.max_event_time_ms = p_tpdo_comm_parameter->max_event_time_ms;
	p_tpdo->comm_parameter.sync_mask_reg = p_tpdo_comm_parameter->sync_mask_reg;
	p_tpdo->comm_parameter.sync_start_value = p_tpdo_comm_parameter->sync_start_value;
	p_tpdo->comm_parameter.build_data_option = p_tpdo_comm_parameter->build_data_option;

	p_tpdo->base.msg.id.cob_id = p_tpdo->comm_parameter.id.cob_id;
	p_tpdo->base.msg.data_len = 8;
	p_tpdo->base.msg.is_new = false;

	p_tpdo->p_sync = p_sync;

	p_tpdo->event_timer_ms = p_tpdo->comm_parameter.max_event_time_ms;
	p_tpdo->inhibit_timer_ms = p_tpdo->comm_parameter.max_inhibit_time_ms;
	p_tpdo->send_request = false;

	p_tpdo->build_data_manually = build_data_manually_impl;
	/* Check error */
	if(CO_TPDO_build_data_manually == p_tpdo->comm_parameter.build_data_option)
    {
       	if(NULL == build_data_manually_impl) CO_detect_error();
    }
    else
    {
      	if(NULL == p_tpdo_comm_parameter) CO_detect_error();
    }

}

static void CO_TPDO_send(CO_TPDO* p_tpdo)
{
	if(p_tpdo->comm_parameter.build_data_option == CO_TPDO_build_data_manually)
	{
		p_tpdo->build_data_manually(p_tpdo->base.msg.data);
		CO_CAN_send(&p_tpdo->base.msg);
	}
	else
	{
		/* Build data according to Mapping application parameter */
		for(uint8_t i = 0; i < 8; i++)
		{
			*(p_tpdo->base.msg.data + i) = **(p_tpdo->base.p_mapping_data + i); //check!!!
		}
		/* Send message */
		CO_CAN_send(&p_tpdo->base.msg);
	}
	p_tpdo->send_request = false;
}




/*
 * Send TPDO message according to TPDO communication parameters
 *
 * */
void CO_TPDO_process(CO_TPDO* p_tpdo,
		uint16_t time_difference_ms,
		bool sync_was)
{
	(void) time_difference_ms;
	if(CO_PDO_VALID == p_tpdo->base.msg.id.valid)
	{
		if(CO_TPDO_TRANSM_TYPE_EVENT_LO == p_tpdo->comm_parameter.transmission_type)
		{
#if CO_TPDO_USE_EVENT_TIMER
            /* event timer */
			if(0 != p_tpdo->comm_parameter.max_event_time_ms)
			{
				p_tpdo->event_timer_ms = (p_tpdo->event_timer_ms > time_difference_ms)
						? (p_tpdo->event_timer_ms - time_difference_ms) : 0;
				if(0 == p_tpdo->event_timer_ms)
				{
					p_tpdo->send_request = true;
				}
			}
#endif
#if CO_TPDO_USE_INHIBIT_TIMER
			/* inhibit timer*/
			p_tpdo->inhibit_timer_ms = (p_tpdo->inhibit_timer_ms > time_difference_ms)
					? (p_tpdo->inhibit_timer_ms - time_difference_ms) : 0;
			/* send */
            if(p_tpdo->send_request && 0 == p_tpdo->inhibit_timer_ms)
            {
            	CO_TPDO_send(p_tpdo);
            }
#else
            if(p_tpdo->send_request)
            {
            	CO_TPDO_send(p_tpdo);
            }
#endif
		}
		else if(sync_was)
		{
			/* send synchronous cyclic TPDO */
			if(p_tpdo->p_sync->sync_counter == p_tpdo->comm_parameter.transmission_type
					&&CO_SYNC_is_match_sync_mask_reg(p_tpdo->p_sync, p_tpdo->comm_parameter.sync_mask_reg))
			{
				CO_TPDO_send(p_tpdo);
			}
			/* send synchronous acyclic TPDO */
			else if(CO_TPDO_TRANSM_TYPE_SYNC_ACYCLIC == p_tpdo->comm_parameter.transmission_type)
			{
				if(true == p_tpdo->send_request)
				{
					CO_TPDO_send(p_tpdo);
				}
			}

		}
	}
}

void tpdo_build_data_impl_default(uint8_t* buffer)
{
	buffer[7] = 0;
	buffer[6] = 0;
	buffer[5] = 0;
	buffer[4] = 0;
	buffer[3] = 0;
	buffer[2] = 0;
	buffer[1] = 0;
	buffer[0] = 0;

}

void CO_TPDO_set_node_id(CO_TPDO* p_tpdo, uint8_t node_id)
{
	bool valid = p_tpdo->base.msg.id.valid;
	p_tpdo->base.msg.id.valid = CO_PDO_NOT_VALID;
	p_tpdo->base.msg.id.node_id = node_id;
	p_tpdo->base.msg.id.valid = valid;
}

void CO_TPDO_set_transmission_type(CO_TPDO *p_tpdo, uint16_t type)
{
	bool valid = p_tpdo->base.msg.id.valid;
	p_tpdo->base.msg.id.valid = CO_PDO_NOT_VALID;
	p_tpdo->comm_parameter.transmission_type = type;
	p_tpdo->base.msg.id.valid = valid;
}

void CO_TPDO_set_sync_mask_reg(CO_TPDO *p_tpdo, uint32_t mask)
{
	bool valid = p_tpdo->base.msg.id.valid;
	p_tpdo->base.msg.id.valid = CO_PDO_NOT_VALID;
	p_tpdo->comm_parameter.sync_mask_reg = mask;
	p_tpdo->base.msg.id.valid = valid;
}


#if 0



//static void CO_TPDO_send(CO_TPDO* p_tpdo)
//{
//	if(p_tpdo->build_data_option == CO_TPDO_build_data_manually)
//	{
//		p_tpdo->build_data_manually(p_tpdo->base.msg.data);
//		CO_CAN_send(&p_tpdo->base.msg);
//	}
//	else
//	{
//		/* Build data according to Mapping application parameter */
//		uint8_t* obj_data;
//	    uint8_t mapped_obj_id=0;
//	    uint8_t byte_index=0;
//		for (mapped_obj_id = 0; mapped_obj_id < p_tpdo->mapped_object_number; mapped_obj_id++)
//		{
//			obj_data = (uint8_t*)p_tpdo->base.mapped_objects[mapped_obj_id].p_data;
//            for(byte_index = 0; byte_index < p_tpdo->base.mapped_objects[mapped_obj_id].reg.len; byte_index++)
//            {
//            	p_tpdo->base.msg.data[byte_index] = *obj_data++;
//            }
//		}
//		/* Send message */
//		CO_CAN_send(&p_tpdo->base.msg);
//	}
//}

void tpdo_init(CO_TPDO* p_tpdo,const uint16_t cob_id){
        pdo_init((CO_PDO*)p_tpdo,cob_id);
        pdo_set_handle((CO_PDO*)p_tpdo,tpdo_process);
}


static void tpdo_process(CO_PDO* p_tpdo){

        CAN_Msg* p_msg= pdo_get_msg((CO_PDO*)p_tpdo);
        uint8_t* msg_data= (uint8_t*) p_msg->data;

        uint8_t* obj_data;
        uint8_t mapped_obj_id=0;
        uint8_t byte_index=0;
        for(mapped_obj_id=0;mapped_obj_id<p_tpdo->mapped_object_no; mapped_obj_id++)
        {
                obj_data= (uint8_t*)p_tpdo->mapped_objects[mapped_obj_id].p_data;
                for(byte_index=0;byte_index<p_tpdo->mapped_objects[mapped_obj_id].len;byte_index++)
                {
                        *msg_data++ = *obj_data++;
                }
        }
}

#endif
