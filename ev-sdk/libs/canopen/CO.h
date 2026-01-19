#ifndef CO_H_
#define CO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CO_CAN_Msg.h"
#include "CO_Object.h"
#include "CO_TPDO.h"
#include "CO_SDOclient.h"
#include "CO_SDOserver.h"
#include "CO_SYNC.h"
#include "CO_EMCY.h"
#include "CO_slave.h"
#include "CO_FLASH.h"
#include "CO_OD_storage.h"

#define LIB_USED_RAMFUNC                0
#define LIB_USED_OPTIMIZE_SPEED_ON            0
#define LIB_FORCE_INLINE    inline __attribute__((always_inline))

#if LIB_USED_OPTIMIZE_SPEED_ON
#define OPTIMIZE_SPEED_ON   __attribute__((optimize("O3")))
#else
#define LIB_OPTIMIZE_SPEED_ON
#endif

#if LIB_USED_RAMFUNC
#define LIB_RAM_FUNC  __attribute__((section(".data,\"aw\",%progbits @")))
#else
#define LIB_RAM_FUNC
#endif

#define LIB_OPTIMIZE_ATTR  LIB_FORCE_INLINE LIB_RAM_FUNC LIB_OPTIMIZE_SPEED_ON


#define RPDO_NUMBER                        4
#define TPDO_NUMBER                        4
#define SDO_CLIENT_NUMBER                   1
#define SDO_SERVER_NUMBER                   1

typedef struct CO_t CO;

struct CO_t {
    uint32_t device_type;            //0x1000
    uint32_t status;                    //0x1002
//	uint32_t 	comm_cycle_ms;			//0x1006 Communication cycle period
//	uint32_t 	sync_window_len_ms;		//0x1007 Synchronous window length
    uint16_t guard_time;                //0x100C
    uint8_t life_time_factor;        //0x100D
    uint32_t high_resolution_timestamp;    //0x1013
//	uint32_t 	consumer_heartbeat_time;	//0x1016
//	uint32_t 	producer_heartbeat_time;	//0x1017
    volatile CO_OD_Identity_t *p_identity;            //0x1018

    CO_OD *p_od;
//	uint8_t 	node_id;				/**< True in un-configured LSS slave */

//	CO_RPDO rpdos[RPDO_NUMBER];
    CO_TPDO tpdos[TPDO_NUMBER];
    CO_SDO sdo_client;
    CO_SDO sdo_server;
    CO_SYNC sync;
    CO_EMCY emcy;
    CO_FLASH flash;
    CO_OD_Storage storage;
//    CO_CAN_Msg rx_msgs[RX_MSG_BUFFER_SIZE];

//    CO_Sends sends;
};

void CO_init_basic(CO *p_co, const uint32_t x1000_deviceType, CO_OD_Identity_t *x1018_identity,
                   CO_OD *p_od);

void CO_process(CO *p_co, uint16_t time_diff_ms);

void CO_set_node_id(CO *p_co, uint8_t node_id);

static inline void CO_handle_error(CO *p_dev) {
    CO_EMCY_handle_error(&p_dev->emcy);
}

static inline bool CO_can_receive_basic_handle(CO *p_dev, const uint32_t can_id, uint8_t *data) {
    if (can_id == p_dev->sync.msg.id.can_id) {
        if (CO_SYNC_NOT_GEN == p_dev->sync.msg.id.bit_30) {
            CO_memcpy(p_dev->sync.msg.data, data, p_dev->sync.msg.data_len);
            p_dev->sync.msg.is_new = true;
        }
        return 1;
    } else if (can_id == p_dev->sdo_client.rx_msg.id.can_id
               && p_dev->sdo_client.state != CO_SDO_ST_IDLE) {
        CO_memcpy(p_dev->sdo_client.rx_msg.data, data, 8);
        p_dev->sdo_client.rx_msg.is_new = true;
        return 1;
    } else if (can_id == p_dev->sdo_server.rx_msg.id.can_id) {
        CO_memcpy(p_dev->sdo_server.rx_msg.data, data, 8);
        p_dev->sdo_server.rx_msg.is_new = true;
        return 1;
    }
    return 0;
}

void CO_disable_sync_mode(CO *p_co);

void CO_recover_sync_mode(CO *p_co, OD_Temp_Comm_Profile_Para_t *p_comm_prof_init_data);

#if 0

//==================================================================================
typedef struct CO_t CO;
typedef void (*CO_Sends)(CO* p_dev,const CAN_Msg* p_msg);
typedef void (*CO_Error_Handle)(CO* p_dev);
extern uint8_t SOM[2];

#define CO_TX_BUFFER_SIZE		128
#define CO_RX_BUFFER_SIZE		128

#define SDO_CLIENT_TO_SERVER            600UL
#define SDO_SERVER_TO_CLIENT            580UL
#define NODE_ID                         3U

#define FW_INDEX			2000UL
#define FW_SUB_INDEX			2U

#define RX_MSG_BUFFER_SIZE			16


typedef enum CO_STATE_t CO_STATE;

///**
// * Default CANopen identifiers.
// *
// * Default CANopen identifiers for CANopen communication objects. Same as
// * 11-bit addresses of CAN messages. These are default identifiers and
// * can be changed in CANopen. Especially PDO identifiers are confgured
// * in PDO linking phase of the CANopen network configuration.
// */
//typedef enum{
//     CO_CAN_ID_NMT_SERVICE       = 0x000,   /**< 0x000, Network management */
//     CO_CAN_ID_SYNC              = 0x080,   /**< 0x080, Synchronous message */
//     CO_CAN_ID_EMERGENCY         = 0x080,   /**< 0x080, Emergency messages (+nodeID) */
//     CO_CAN_ID_TIME_STAMP        = 0x100,   /**< 0x100, Time stamp message */
//     CO_CAN_ID_TPDO_1            = 0x180,   /**< 0x180, Default TPDO1 (+nodeID) */
//     CO_CAN_ID_RPDO_1            = 0x200,   /**< 0x200, Default RPDO1 (+nodeID) */
//     CO_CAN_ID_TPDO_2            = 0x280,   /**< 0x280, Default TPDO2 (+nodeID) */
//     CO_CAN_ID_RPDO_2            = 0x300,   /**< 0x300, Default RPDO2 (+nodeID) */
//     CO_CAN_ID_TPDO_3            = 0x380,   /**< 0x380, Default TPDO3 (+nodeID) */
//     CO_CAN_ID_RPDO_3            = 0x400,   /**< 0x400, Default RPDO3 (+nodeID) */
//     CO_CAN_ID_TPDO_4            = 0x480,   /**< 0x480, Default TPDO4 (+nodeID) */
//     CO_CAN_ID_RPDO_4            = 0x500,   /**< 0x500, Default RPDO5 (+nodeID) */
//     CO_CAN_ID_TSDO              = 0x580,   /**< 0x580, SDO response from server (+nodeID) */
//     CO_CAN_ID_RSDO              = 0x600,   /**< 0x600, SDO request from client (+nodeID) */
//     CO_CAN_ID_HEARTBEAT         = 0x700    /**< 0x700, Heartbeat message */
//}CO_Default_CAN_ID_t;

#define CO_ERR_REG_GENERIC_BIT			(1<<0)
#define CO_ERR_REG_CURRENT_BIT			(1<<1)
#define CO_ERR_REG_VOLTAGE_BIT			(1<<2)
#define CO_ERR_REG_TEMPERATURE_BIT		(1<<3)
#define CO_ERR_REG_COMMUNICATION_BIT		(1<<4)
#define CO_ERR_REG_DEVICE_PROFILE_BIT		(1<<5)
#define CO_ERR_REG_SELEX_SPECIFIC_BIT		(1<<7)

#define ERR_HISTORY_SIZE			8

typedef struct CO_Interface_t CO_Interface;
struct CO_Interface_t{
        void (*co_start)(CO* p_dev);        
};

enum CO_STATE_t{

    CO_ST_NMT_INIT		=0,
    CO_ST_PRE_OPERATIONAL   =1,
    CO_ST_READY		=2,
    CO_ST_OPERATIONAL	=3,
        CO_ST_STOPPED           =4
};

//typedef struct CO_Identity_t{
//         uint32_t vendor_id;
//         uint32_t product_code;
//         uint32_t revisionNumber;
//         uint32_t serial_number;
//}CO_Identity;

typedef struct CO_Error_List_t{
    uint8_t error_no;
    uint32_t error_code[ERR_HISTORY_SIZE];
}CO_Error_List;

typedef struct CO_Program_Data_t{
    uint8_t program_no;
    void* program_addr[8];
}CO_Program_Data;

struct CO_t{
    uint32_t	device_type;		//0x1000
    uint8_t 	error_reg;			//0x1001
    uint16_t	manu_error_reg;
    uint32_t 	status;				//0x1002
    CO_Error_List predefined_error_fields;	//0x1003
    uint32_t 	comm_cycle;			//0x1006 Communication cycle period
    uint32_t 	sync_window_len;	//0x1007 Synchronous window length
    uint16_t 	guard_time;			//0x100C
    uint8_t		life_time_factor;	//0x100D
    uint32_t 	high_resolution_timestamp;	//0x1013
//	uint32_t 	consumer_heartbeat_time;	//0x1016
//	uint32_t 	producer_heartbeat_time;	//0x1017
    CO_Identity identity;			//0x1018

    uint32_t 	error_code;		//byte01: standard error code
                                    //byte23: manufacturer error code
    CO_OD* od;

    uint8_t node_id;

    CO_RPDO rpdos[RPDO_NUMBER];
    CO_TPDO tpdos[TPDO_NUMBER];
    CO_SDO sdos[SDO_NUMBER];
    CO_CAN_Msg rx_msgs[RX_MSG_BUFFER_SIZE];

    CO_Sends sends;
    CO_Error_Handle error_handle;
};






/**/
void co_send_boost_msg(CO* p_co);

static LIB_OPTIMIZE_ATTR uint32_t CO_get_last_error(const CO* p_dev){
    return p_dev->error_code;
}

static LIB_OPTIMIZE_ATTR void co_set_error(CO* p_dev,const uint32_t code){

    uint8_t error_index=0;
    /*
     * shift all error records to right
     * so the error record at index 0 always is newest error
     */
    for(error_index=0;error_index<p_dev->predefined_error_fields.error_no-1;
            error_index++){

        p_dev->predefined_error_fields.errors[error_index+1]=
                p_dev->predefined_error_fields.errors[error_index];
    }

    p_dev->predefined_error_fields.error_no+=1;
    if(p_dev->predefined_error_fields.error_no>ERR_HISTORY_SIZE){
        p_dev->predefined_error_fields.error_no=ERR_HISTORY_SIZE;
    }

    p_dev->predefined_error_fields.errors[0]=code;
    p_dev->error_code=code;
}

CO* CO_create(const uint8_t node_id);
void CO_set_od(CO* p_co,CO_OD* p_od);
void CO_process(CO* p_co);
void CO_process_tpdo(CO* p_co);
void CO_process_rpdo(CO* p_co);
void CO_process_sdo(CO* p_co);
void CO_set_send_handle(CO* p_dev,CO_Sends sends);
void co_receive_msg(CO* p_dev,CAN_Msg* p_msg);

static inline void co_sends(CO* p_co,const CAN_Msg* p_msg){

        p_co->sends(p_co,p_msg);
}

static inline void CO_set_error_handle(CO* p_dev,CO_Error_Handle handle){

    p_dev->error_handle=handle;
}

static LIB_OPTIMIZE_ATTR void CO_handle_error(CO* p_dev){
    p_dev->error_handle(p_dev);
}
#endif

#ifdef __cplusplus
}
#endif

#endif
