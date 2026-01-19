/*
 * CO_OD_sign.h
 *
 *  Created on: Oct 5, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_CO_OD_SIGN_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_CO_OD_SIGN_H_

#include "CO_utils.h"

/*
 * Function code sign map:
 * - bit 0-1: command for communication profile parameter storage
 * 			00B: no action (<=>restore communication parameter from flash when startup)
 * 			01B: store all communication parameter to flash
 * 			10B: restore default communication parameter
 * - bit 2-3: command for application profile parameter storage
 * 			00B: no action (<=>restore application parameter from flash when startup)
 * 			01B: store all application parameter to flash
 * 			10B: restore default application parameter
 * - bit 4-7: reverse
 * */

/* Function code command type */
typedef uint8_t CO_Func_Code_Command_t;
#define CO_OD_SIGN__FUNC_CODE_COMMAND__NO_ACTION					(0x00)
#define CO_OD_SIGN__FUNC_CODE_COMMAND__STORE_PARA					(0x01)
#define CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA					(0x02)

#define OD_1010_VALID_STORE_COMMAND 			(0x73617665)		//"save" signature
#define OD_1011_VALID_RESTORE_COMMAND			(0x6C6F6164)		//"load" signature

typedef enum{
	CO_FCG__COMM_PARA			= 0x02,
	CO_FCG__APP_PARA			= 0x03
}CO_Func_Code_Group_t;

typedef struct{
	uint8_t func;			//<< function code
	uint32_t sw_ver;		//<< software version of canopen device firmware
	uint16_t crc_16_comm;	//<< crc code 16bit of communication parameter
	uint16_t crc_16_app;	//<< crc code 16bit of application parameter
}CO_OD_Sign;

/* Set function*/
static inline void CO_OD_SIGN_set_crc16_comm_para(CO_OD_Sign* _this, uint16_t crc16)
{
	_this->crc_16_comm = crc16;
}
static inline void CO_OD_SIGN_set_crc16_app_para(CO_OD_Sign* _this, uint16_t crc16)
{
	_this->crc_16_app = crc16;
}
static inline void CO_OD_SIGN_set_sw_ver(CO_OD_Sign* _this, uint32_t sw_ver)
{
	_this->sw_ver = sw_ver;
}
void CO_OD_SIGN_set_func(CO_OD_Sign* _this, CO_Func_Code_Group_t group, uint8_t command);

/* Get function*/
static inline uint16_t CO_OD_SIGN_get_crc16_comm_para(CO_OD_Sign* _this)
{
	return _this->crc_16_comm;
}
static inline uint16_t CO_OD_SIGN_get_crc16_app_para(CO_OD_Sign* _this)
{
	return _this->crc_16_app;
}
static inline uint32_t CO_OD_SIGN_get_sw_ver(CO_OD_Sign* _this)
{
	return _this->sw_ver;
}
CO_Func_Code_Command_t CO_OD_SIGN_get_func(CO_OD_Sign* _this, CO_Func_Code_Group_t group);

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_CO_OD_SIGN_H_ */
