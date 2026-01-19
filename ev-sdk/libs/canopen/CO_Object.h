/*
 * CO_Object.h
 *
 *  Created on: Jun 25, 2019
 *      Author: quangnd
 */

#ifndef CO_OBJECT_H_
#define CO_OBJECT_H_

#include "stdint.h"
#include "CO_utils.h"

typedef struct CO_Object_t CO_Object;
typedef struct CO_Sub_Object_t CO_Sub_Object;
typedef struct CO_Storage_Ext_t CO_Storage_Ext;

//------1. CO_Storage_Ext_t-----------------------------------------------------------------------------------
//------ 3. CO_Object_t-----------------------------------------------------------------------------------

struct CO_Object_t
{
	uint16_t		index;				//<< Object Dictionary index
	uint8_t			sub_number;			//<< Number of all sub-object, including sub-object at sub-index 0
	CO_Sub_Object*	subs;
};
//------2. CO_Sub_Object_t-----------------------------------------------------------------------------------

/*
 * Attributes (bit masks) for OD sub-object.
 */

typedef enum {
/**----SDO RWable option--------------------------------------------------------------------*/
    ODA_SDO_R = 0x0001, 			/**< SDO server may read from the variable */
    ODA_SDO_W = 0x0002, 			/**< SDO server may write to the variable */
    ODA_SDO_RW = 0x0003, 			/**< SDO server may read from or write to the variable */

/**----PDO mappable option--------------------------------------------------------------------*/
    ODA_TPDO = 0x0004, 				/**< Variable is mappable into TPDO (can be read) */
    ODA_RPDO = 0x0008, 				/**< (not support) Variable is mappable into RPDO (can be written) */
    ODA_TRPDO = 0x000C, 			/**< (not support) Variable is mappable into TPDO or RPDO */

/**----SRDO mappable option (not support) --------------------------------------------------------------
 * 	-> Function not supported yet! */
    ODA_TSRDO = 0x0010,			 	/**< (not support) Variable is mappable into transmitting SRDO */
    ODA_RSRDO = 0x0020, 			/**< (not support) Variable is mappable into receiving SRDO */
    ODA_TRSRDO = 0x0030, 			/**< (not support) Variable is mappable into tx or rx SRDO */

    ODA_MB = 0x0040, 				/**< (not support) Variable is multi-byte ((u)int16_t to (u)int64_t) */
    ODA_STR = 0x0080, 				/**< (not support) Shorter value, than specified variable size, may be
    								written to the variable. SDO write will fill remaining memory with zeroes.
    								Attribute is used for VISIBLE_STRING and UNICODE_STRING. */

/**----Effective time option--------------------------------------------------------------------*/
	ODA_VWTE_after_reset = 0x0100, 	/**< Value written takes effect after reset.
	 	 	 	 	 	 	 	 	 	 -If you want to the value written remain takes effect after reset,
	 	 	 	 	 	 	 	 	 	 p_data_init in Sub_Object must != NULL)*/
	ODA_VWTE_right_away = 0x0000 	/**< Value written takes effect right away.
										 -This is default mode.
										 -If you want to the value written remain takes effect after reset,
	 	 	 	 	 	 	 	 	 	 p_data_init in Sub_Object must != NULL*/
} CO_Sub_Object_Attr_t;
#if 0
typedef enum {
    CO_EXT_CONFIRM_processing	= 0x01,
    CO_EXT_CONFIRM_abort		= 0x02,
    CO_EXT_CONFIRM_success		= 0x03,
} CO_Sub_Object_Ext_Confirm_Func_t;
#else
typedef enum {
    CO_EXT_CONFIRM_processing			= 0x01,		//<< Process again
    CO_EXT_CONFIRM_abort				= 0x02,		//<< Send abort SDO message to SDOclient
    CO_EXT_CONFIRM_success				= 0x03,		//<< Send success SDO message to SDOclient
	CO_EXT_CONFIRM_copy_and_success		= 0x04		//<< Copy shadow data to main data and send success SDO message to SDOclient
} CO_Sub_Object_Ext_Confirm_Func_t;
#endif

typedef struct
{
    uint8_t* 									p_shadow_data;				//<< Buffer stores data for processing if necessary
    CO_Sub_Object_Ext_Confirm_Func_t 			(*confirm_func) (void);		//<< Last confirmation function when data has been saved in
    											// the case the device is server + download mode or client + upload mode
} CO_Sub_Object_Ext_t;	/* Extension of Sub-object*/

struct CO_Sub_Object_t
{
	void*				p_data;
	uint16_t 			attr;				//<< see CO_Sub_Object_Attr_t
	uint16_t 			len;
	CO_Sub_Object_Ext_t* p_ext;
	void* 				p_temp_data;		//data_init-var bat buoc phai co kieu, do dai giong het bien chinh de tranh bi sai
};




static inline void CO_SUB_OBJ_write(CO_Object *p_obj, const uint8_t sub_index, const void *data,
		const uint8_t len)
{
	uint8_t *src = (uint8_t*) p_obj->subs[sub_index].p_data;
	uint8_t *dst = (uint8_t*) data;
	CO_memcpy(dst, src, len);
}

static inline void CO_SUB_OBJ_read(CO_Object *p_obj, const uint8_t sub_index, void *data,
		const uint8_t len)
{

	uint8_t *src = (uint8_t*) p_obj->subs[sub_index].p_data;
	uint8_t *dst = (uint8_t*) data;
	CO_memcpy(dst, src, len);
}

static inline void* CO_SUB_OBJ_get_data_pointer(CO_Object *p_obj, const uint8_t sub_index)
{
	return p_obj->subs[sub_index].p_data;
}

static inline uint16_t CO_SUB_OBJ_get_data_len(CO_Object *p_obj, const uint8_t sub_index)
{
	return p_obj->subs[sub_index].len;
}

static inline uint16_t CO_SUB_OBJ_get_sub_attr(CO_Object *p_obj, const uint8_t sub_index)
{
	return p_obj->subs[sub_index].attr;
}



#endif /* CO_OBJECT_H_ */
