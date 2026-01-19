/*
 * CO_slave.h
 *
 *  Created on: Jul 15, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SLAVE_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SLAVE_H_


#include "CO_config.h"

typedef enum CO_SLAVE_NET_STATE{
	CO_SLAVE_CON_ST_DISCONNECT	=	0,
	CO_SLAVE_CON_ST_ASSIGNING	=	1,
	CO_SLAVE_CON_ST_AUTHORIZING	=	2,
	CO_SLAVE_CON_ST_CONNECTED	=	3

}CO_SLAVE_NET_STATE;

typedef struct CO_Slave_t CO_Slave;
struct CO_Slave_t{
	uint32_t inactive_time_ms;
	uint32_t node_id;
	uint32_t sdo_server_address;
	uint8_t  sn[32];
	CO_SLAVE_NET_STATE con_state;
	bool     is_reconnect;
	void     (*disconnect_handle)(CO_Slave*);
    void     (*reconnect_handle)(CO_Slave*);
};

typedef struct CO_Slave_List_t CO_Slave_List;
struct CO_Slave_List_t{
	CO_Slave* data;
	CO_Slave_List* next;
};

static inline void co_slave_set_con_state(CO_Slave* p_sl, CO_SLAVE_NET_STATE state){
        p_sl->con_state=state;
}

static inline void co_slave_reset_inactive_timer(CO_Slave* p_slave){
    p_slave->inactive_time_ms = 0;
}

static inline void co_slave_update_inactive_timer(CO_Slave* p_slave, const uint32_t timeout){
    if(p_slave->con_state != CO_SLAVE_CON_ST_CONNECTED) return;

    p_slave->inactive_time_ms++;
    if(p_slave->inactive_time_ms > timeout){

        p_slave->disconnect_handle(p_slave);
        co_slave_reset_inactive_timer(p_slave);
        p_slave->con_state = CO_SLAVE_CON_ST_DISCONNECT;
    }
}

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_SLAVE_H_ */
