/*
 * CAN_Msg.h
 *
 *  Created on: May 3, 2019
 *      Author: quangnd
 */

#ifndef CO_CAN_MSG_H_
#define CO_CAN_MSG_H_

#include "stdint.h"
#include "stdbool.h"
#include "CO_utils.h"

#define CO_SYNC_GEN					1
#define CO_SYNC_NOT_GEN				0

#define CO_SYNC_VALID				0
#define CO_SYNC_NOT_VALID			1

#define CO_EMCY_VALID				0
#define CO_EMCY_NOT_VALID			1

#define CO_PDO_VALID				0
#define CO_PDO_NOT_VALID			1

#define CO_BIT_RTR_ALLOW			0
#define CO_BIT_RTR_NOT_ALLOW		1

#define CO_BIT_CAN_FRAME_11BIT		0
#define CO_BIT_CAN_FRAME_29BIT		1

typedef union
{
	uint32_t cob_id;
	struct
	{
		uint32_t    node_id		:7;
		uint32_t	func_code	:4;
		uint32_t    reverse1	:18;
		uint32_t	bit_frame	:1;
		uint32_t    bit_30		:1;		//<< SYNC COB-ID: 		generate bit
										//	 EMCY COB-ID: 		reverse bit
										//	 SDO server COB-ID: dyn bit
										//	 SDO client COB-ID: dyn bit
										//	 RPDO COB-ID: 		reverse bit
										//   TPDO COB-ID: 		rtr bit

		uint32_t	valid		:1;		//<< SYNC COB-ID: 		valid bit (enable: 0; disable: 1)
										//	 EMCY COB-ID: 		valid bit (enable: 0; disable: 1)
										//	 SDO server COB-ID: valid bit (enable: 0; disable: 1)
										//	 SDO client COB-ID: valid bit (enable: 0; disable: 1)
										//	 RPDO COB-ID: 		valid bit (enable: 0; disable: 1)
										//   TPDO COB-ID: 		valid bit (enable: 0; disable: 1)
	};
	struct
	{
		uint32_t    can_id		:29;
		uint32_t    reverse2	:3;
	};
} co_id_t;

typedef struct CO_CAN_Msg_t CO_CAN_Msg;

struct CO_CAN_Msg_t{

	co_id_t		id;
	uint8_t		data[8];
	uint8_t		data_len;
	bool		is_new;
};


/**
 * Default CANopen identifiers.
 *
 * Default CANopen identifiers for CANopen communication objects. Same as
 * 11-bit addresses of CAN messages. These are default identifiers and
 * can be changed in CANopen. Especially PDO identifiers are confgured
 * in PDO linking phase of the CANopen network configuration.
 */
typedef enum{
	CO_CAN_ID_NMT_SERVICE       = 0x000,   /**< 0x000, Network management */
	CO_CAN_ID_SYNC              = 0x080,   /**< 0x080, Synchronous message */
	CO_CAN_ID_EMERGENCY         = 0x080,   /**< 0x080, Emergency messages (+nodeID) */
	CO_CAN_ID_TIME_STAMP        = 0x100,   /**< 0x100, Time stamp message */
	CO_CAN_ID_TPDO_1            = 0x180,   /**< 0x180, Default TPDO1 (+nodeID) */
	CO_CAN_ID_RPDO_1            = 0x200,   /**< 0x200, Default RPDO1 (+nodeID) */
	CO_CAN_ID_TPDO_2            = 0x280,   /**< 0x280, Default TPDO2 (+nodeID) */
	CO_CAN_ID_RPDO_2            = 0x300,   /**< 0x300, Default RPDO2 (+nodeID) */
	CO_CAN_ID_TPDO_3            = 0x380,   /**< 0x380, Default TPDO3 (+nodeID) */
	CO_CAN_ID_RPDO_3            = 0x400,   /**< 0x400, Default RPDO3 (+nodeID) */
	CO_CAN_ID_TPDO_4            = 0x480,   /**< 0x480, Default TPDO4 (+nodeID) */
	CO_CAN_ID_RPDO_4            = 0x500,   /**< 0x500, Default RPDO5 (+nodeID) */
	CO_CAN_ID_SDO_SRV           = 0x580,   /**< 0x580, SDO response from server (+nodeID) */
	CO_CAN_ID_SDO_CLI           = 0x600,   /**< 0x600, SDO request from client (+nodeID) */
	CO_CAN_ID_HEARTBEAT         = 0x700,    /**< 0x700, Heartbeat message */
	CO_CAN_ID_LSS_SLV			= 0x7E4,	/**< 0x7E4, LSS response from slave */
	CO_CAN_ID_LSS_MST			= 0x7E5		/**< 0x7E5, LSS request from master */
}CO_Default_CAN_ID_t;


typedef void(*CO_CAN_Send_Interface)(CO_CAN_Msg* p_msg);

extern CO_CAN_Send_Interface co_can_send;

static inline void CO_CAN_set_can_send_interface(CO_CAN_Send_Interface can_send_impl)
{
	co_can_send = can_send_impl;
}

static inline void CO_CAN_send(CO_CAN_Msg* p_msg)
{
	co_can_send(p_msg);
}

#if 0
typedef struct CAN_Msg_t CAN_Msg;

struct CAN_Msg_t{

	uint16_t 	cob_id;		//is function code of COB-ID or COB-ID?
	uint8_t		node_id;
	uint8_t		data[8];
	uint8_t		data_len;
	bool		is_new;
};
//ham phan tich ?? Co nen can thiep sau den muc nay??
//Should adjust that input: cob-id, data 8 byte?
static inline void can_rx_msg_parse(CAN_Msg* p_msg,const uint8_t* p_data,const uint16_t len){

	if(len <11 ) return;
	uint16_t msg_id= CO_getUint16(p_data);
	p_msg->data_len=p_data[2];
	p_msg->node_id= (uint8_t) (msg_id & 0x007FUL); // node id is 7bit LSB 		(0000 0111 1111)
	p_msg->cob_id= msg_id & 0x0780UL;	//is function code of COB-ID or COB-ID?	(0111 1000 0000)
	CO_memcpy(p_msg->data,p_data+3,p_msg->data_len);
}

#endif
#endif /* CAN_MSG_H_ */
