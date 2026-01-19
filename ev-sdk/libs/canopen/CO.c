#include "CO.h"
#include "stdlib.h"
#include "stdint.h"

static inline void CO_process_tpdo(CO *p_co, uint16_t time_diff_ms, bool sync_was);

/* Init function without tpdo, sdo_client, sdo_server, sync, emcy error handle, flash, storage*/
void CO_init_basic(CO *p_co, const uint32_t x1000_deviceType, CO_OD_Identity_t *x1018_identity,
		CO_OD *p_od)
{
	p_co->device_type = x1000_deviceType;
	p_co->status = 0;
//	p_co->guard_time =;
//	p_co->life_time_factor =;
//	p_co->high_resolution_timestamp=;
	p_co->p_identity = x1018_identity;
	p_co->p_od = p_od;
//	p_co->node_id = node_id;
	CO_EMCY_init(&p_co->emcy);
}
/**/
void CO_process(CO* p_co, uint16_t time_diff_ms)
{
    if(p_co->sync.msg.id.valid == CO_SYNC_VALID)
    {
        CO_SYNC_process(&p_co->sync, true, time_diff_ms);
        if(p_co->sync.index_in_cycle >= 2)
        {
            CO_SDOserver_process(&p_co->sdo_server, time_diff_ms);
            CO_SDOclient_process(&p_co->sdo_client, time_diff_ms);
        }
        else if(p_co->sync.index_in_cycle == 1)
        {
            CO_process_tpdo(p_co, time_diff_ms, true);
        }
    }
    else
    {
        CO_SYNC_process(&p_co->sync, true, time_diff_ms);
        CO_SDOserver_process(&p_co->sdo_server, time_diff_ms);
        CO_SDOclient_process(&p_co->sdo_client, time_diff_ms);
        CO_process_tpdo(p_co, time_diff_ms, true);
    }
}

static inline void CO_process_tpdo(CO *p_co, uint16_t time_diff_ms, bool sync_was)
{
    for (uint8_t i = 0; i < TPDO_NUMBER; i++)
    {
        CO_TPDO_process(&p_co->tpdos[i],time_diff_ms, sync_was);
    }
}

void CO_set_node_id(CO* p_co, uint8_t node_id)
{
	/* set node-id of SDOserver, TPDO, EMCY */
	CO_SDOserver_set_node_id(&p_co->sdo_server, node_id);
	for(uint8_t i = 0; i< TPDO_NUMBER; i++)
	{
		CO_TPDO_set_node_id(&p_co->tpdos[i], node_id);
	}
}

void CO_disable_sync_mode(CO* p_co)
{
	p_co->sync.msg.id.valid = CO_SYNC_NOT_VALID;

	p_co->sdo_client.valid = false;
	p_co->sdo_server.valid = false;
	p_co->sdo_client.trans_type = CO_SDO_CLIENT_TRANSM_TYPE_ALWAYS;
	p_co->sdo_server.trans_type = CO_SDO_SERVER_TRANSM_TYPE_ALWAYS;
	uint8_t i = 0;
	for(;i<TPDO_NUMBER; i++)
	{
		uint8_t temp_valid = p_co->tpdos[i].base.msg.id.valid;
		p_co->tpdos[i].base.msg.id.valid = CO_PDO_NOT_VALID;
//		p_co->tpdos[i].comm_parameter.transmission_type = CO_TPDO_TRANSM_TYPE_EVENT_LO;
		p_co->tpdos[i].base.msg.id.valid = temp_valid;
	}
	p_co->sdo_client.valid = true;
	p_co->sdo_server.valid = true;
}

void CO_recover_sync_mode(CO* p_co, OD_Temp_Comm_Profile_Para_t* p_comm_prof_init_data)
{
	p_co->sync.msg.id.valid = p_comm_prof_init_data->x1005_cob_id_sync_message.valid;

	p_co->sdo_client.valid = false;
	p_co->sdo_server.valid = false;
	p_co->sdo_client.trans_type = p_comm_prof_init_data->x1032_sdo_client_comm_para.trans_type;
	p_co->sdo_server.trans_type = p_comm_prof_init_data->x1031_sdo_server_comm_para.trans_type;
	uint8_t i = 0;
	for(;i<TPDO_NUMBER; i++)
	{
		p_co->tpdos[i].base.msg.id.valid = CO_PDO_NOT_VALID;
		p_co->tpdos[i].comm_parameter.transmission_type = p_comm_prof_init_data->x180x_tpdo_comm_para[i].transmission_type;
		p_co->tpdos[i].base.msg.id.valid = p_comm_prof_init_data->x180x_tpdo_comm_para[i].id.valid;
	}
	p_co->sdo_client.valid = true;
	p_co->sdo_server.valid = true;
}








//==========================================================================
#if 0
uint8_t SOM[]={0x55,0xAA};

static inline CAN_Msg* co_get_rx_msg(CO* p_dev);
static inline CAN_Msg* co_get_rx_msg_buffer_from_pool(CO* p_dev);
static inline void CO_SDO_process(CO* p_dev,CAN_Msg* p_msg);
static inline CAN_Msg* get_msg_from_buffer(CAN_Msg* buffer,
		const uint8_t buffer_size,const bool is_new);

CO* CO_create(const uint8_t node_id){

	CO* p_co=(CO*)malloc(sizeof(CO));
	while(p_co==NULL);
	p_co->node_id=node_id;
	return p_co;
}


void CO_set_od(CO* p_co,CO_OD* p_od){

	p_co->od=p_od;
}
void CO_process(CO* p_co){
       
       CAN_Msg* p_msg=co_get_rx_msg(p_co);
       while(p_msg !=NULL){

                CO_SDO_process(p_co,p_msg);

                 /* if msg is handled by sdos */
                if(p_msg->is_new==false){
                        p_msg=co_get_rx_msg(p_co);
                        continue;
                }
        
                /* message is unsupport */
                if(p_msg->is_new==true){
                        p_msg->is_new=false;
                        p_msg=co_get_rx_msg(p_co);
                }
       }
}

void CO_process_tpdo(CO* p_co){

        uint8_t pdo_id=0;
        for(pdo_id=0;pdo_id< TPDO_NUMBER;pdo_id++){
                pdo_handle((CO_PDO*)&p_co->tpdos[pdo_id]);
        }
}

void CO_process_rpdo(CO* p_co){

        uint8_t pdo_id=0;
        for(pdo_id=0;pdo_id< RPDO_NUMBER;pdo_id++){
                pdo_handle((CO_PDO*)&p_co->rpdos[pdo_id]);
        }
}

void CO_set_send_handle(CO* p_dev,CO_Sends sends){
	p_dev->sends=sends;
}

static inline void CO_SDO_process(CO* p_dev,CAN_Msg* p_msg){
       
       int sdo_id=0; 
       CO_SDO* p_sdo=NULL;
       for(sdo_id=0;sdo_id<SDO_NUMBER;sdo_id++){
                p_sdo= &p_dev->sdos[sdo_id];
                CO_SDOserver_process(p_sdo,p_dev->od,0);
                co_sends(p_dev,&p_sdo->tx_msg);
                p_sdo->rx_msg.is_new=false;
       }
}

static inline CAN_Msg* co_get_rx_msg(CO* p_dev){
       
	return get_msg_from_buffer(&p_dev->rx_msgs[0],RX_MSG_BUFFER_SIZE,true);
}

static inline CAN_Msg* co_get_rx_msg_buffer_from_pool(CO* p_dev){
	return get_msg_from_buffer(&p_dev->rx_msgs[0],RX_MSG_BUFFER_SIZE,false);
}

static inline CAN_Msg* get_msg_from_buffer(CAN_Msg* buffer,const uint8_t buffer_size,const bool is_new){

	CAN_Msg* p_msg=NULL;
       int msg_id=0;
        for(msg_id=0;msg_id<buffer_size;msg_id++){
                p_msg=&buffer[msg_id];
                if(p_msg->is_new==is_new) return p_msg;
        }
        return NULL;
}

void co_receive_msg(CO* p_dev,CAN_Msg* p_msg){

	CAN_Msg* msg_slot= co_get_rx_msg_buffer_from_pool(p_dev);
	if(msg_slot==NULL) return;
	msg_slot->cob_id=p_msg->cob_id;
	msg_slot->data_len=p_msg->data_len;
	msg_slot->data[0]=p_msg->data[0];
	msg_slot->data[1]=p_msg->data[1];
	msg_slot->data[2]=p_msg->data[2];
	msg_slot->data[3]=p_msg->data[3];
	msg_slot->data[4]=p_msg->data[4];
	msg_slot->data[5]=p_msg->data[5];
	msg_slot->data[6]=p_msg->data[6];
	msg_slot->data[7]=p_msg->data[7];
	msg_slot->is_new=true;

	for(int i=0;i<RPDO_NUMBER;i++){
		if(p_dev->rpdos[i].base.cob_id==msg_slot->cob_id){
			pdo_set_msg((CO_PDO*)&p_dev->rpdos[i],msg_slot);
			return;
		}
	}

	for(int i=0;i<TPDO_NUMBER;i++){
		if(p_dev->tpdos[i].base.cob_id==msg_slot->cob_id){
			pdo_set_msg((CO_PDO*)&p_dev->tpdos[i],msg_slot);
			return;
		}
	}

	for(int i=0;i<SDO_NUMBER;i++){
		if(p_dev->sdos[i].client_server_id==msg_slot->cob_id){
			sdo_set_msg(&p_dev->sdos[i],msg_slot);
			return;
		}
	}


}

void co_send_boost_msg(CO* p_co){
}
#endif
