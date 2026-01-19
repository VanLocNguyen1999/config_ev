#include "CO_RPDO.h"

#if 0
static void rpdo_process(CO_PDO* p_rpdo);
void rpdo_init(CO_RPDO* p_rpdo,const uint16_t cob_id){
        pdo_init((CO_PDO*)p_rpdo,cob_id);
        pdo_set_handle((CO_PDO*)p_rpdo,rpdo_process);
}

static void rpdo_process(CO_PDO* p_rpdo){
        
		CAN_Msg* p_msg= pdo_get_msg((CO_PDO*)p_rpdo);

        uint8_t* msg_data= (uint8_t*) p_msg->data;

        uint8_t* obj_data;
        uint8_t mapped_obj_id=0;
        uint8_t byte_index=0;
        for(mapped_obj_id=0;mapped_obj_id<p_rpdo->mapped_object_no; mapped_obj_id++){
                obj_data= (uint8_t*)p_rpdo->mapped_objects[mapped_obj_id].p_data;
                for(byte_index=0;byte_index<p_rpdo->mapped_objects[mapped_obj_id].len;byte_index++){
                        *obj_data++ = *msg_data++;
                }
        }
        p_msg->is_new=false;
}

#endif
