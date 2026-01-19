/*
 * CO_EMCY.h
 *
 *  Created on: Jul 4, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_EMCY_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_EMCY_H_

#include "CO_config.h"

typedef union
{
	uint32_t w;
	struct
	{
		uint8_t std	;		//<< standard error register (0x1001)
		uint16_t manu;		//<< manufacturer error register
	};
}error_reg_t;

typedef union
{
	uint32_t w;
	struct
	{
		uint16_t std;		//<< standard error code
		uint16_t manu;		//<< manufacturer error code
	};
}error_code_t;

typedef union
{
	uint8_t data[8];
	struct
	{
		uint16_t std_err_code;	//<< standard error code
		uint8_t std_err_reg;	//<< standard error register (0x1001)
		uint16_t manu_err_code; //<< manufacturer error code
		uint16_t manu_err_reg;	//<< manufacturer error register
	};
}emcy_data_t;

#define ERR_HISTORY_SIZE					8
typedef struct{
	uint8_t error_no;
	error_code_t error_code[ERR_HISTORY_SIZE + 1];
}CO_Error_List_t;

typedef void (*CO_Error_Handle)(void);

typedef struct CO_EMCY_t CO_EMCY;
struct CO_EMCY_t
{
	emcy_data_t error_data;
	error_code_t error_code;
	error_reg_t error_reg;
	CO_Error_List_t predefined_error_fields;	//0x1003

	CO_Error_Handle error_handle;
};

void CO_EMCY_init(CO_EMCY *p_em);

static inline void CO_EMCY_set_error_code(CO_EMCY* p_em,const uint32_t error_code)
{
	int8_t err_index = p_em->predefined_error_fields.error_no;
	/*
	 * shift all error records to right
	 * so the error record at index 0 always is newest error
	 */
	for (; err_index > 0 ; err_index--)
	{
		p_em->predefined_error_fields.error_code[err_index] =
				p_em->predefined_error_fields.error_code[err_index - 1];
	}
	p_em->predefined_error_fields.error_no =
			(p_em->predefined_error_fields.error_no >= ERR_HISTORY_SIZE) ?
					ERR_HISTORY_SIZE : (p_em->predefined_error_fields.error_no + 1);
	p_em->predefined_error_fields.error_code[0].w = error_code;
	p_em->error_code.w = error_code;
	p_em->error_data.std_err_code = p_em->error_code.std;
	p_em->error_data.manu_err_code = p_em->error_code.manu;
}

static inline void CO_EMCY_set_bit_of_error_register(CO_EMCY* p_em,const uint32_t bit)
{
	p_em->error_reg.w |= bit;
	p_em->error_data.manu_err_reg = p_em->error_reg.manu;
	p_em->error_data.std_err_reg = p_em->error_reg.std;
}

static inline void CO_EMCY_set_bit_of_std_error_register(CO_EMCY* p_em,const uint8_t bit)
{
	p_em->error_reg.std |= bit;
	p_em->error_data.std_err_reg = p_em->error_reg.std;
}

static inline void CO_EMCY_set_bit_of_manu_error_register(CO_EMCY* p_em,const uint16_t bit)
{
	p_em->error_reg.manu |= bit;
	p_em->error_data.manu_err_reg = p_em->error_reg.manu;
}

static inline void CO_EMCY_reset_bit_of_error_register(CO_EMCY* p_em,const uint32_t bit)
{
	p_em->error_reg.w &= (~bit);
	p_em->error_data.manu_err_reg = p_em->error_reg.manu;
	p_em->error_data.std_err_reg = p_em->error_reg.std;
}

static inline void CO_EMCY_reset_bit_of_std_error_register(CO_EMCY* p_em,const uint8_t bit)
{
	p_em->error_reg.std &= (~bit);
	p_em->error_data.std_err_reg = p_em->error_reg.std;
}

static inline void CO_EMCY_reset_bit_of_manu_error_register(CO_EMCY* p_em,const uint16_t bit)
{
	p_em->error_reg.manu &= (~bit);
	p_em->error_data.manu_err_reg = p_em->error_reg.manu;
}

static inline void CO_EMCY_set_error_handle(CO_EMCY* p_em, CO_Error_Handle handle)
{
	p_em->error_handle = handle;
}

static inline void CO_EMCY_handle_error(CO_EMCY* p_em)
{
	p_em->error_handle();
}

static inline uint8_t* CO_EMCY_get_last_error_data_pt(CO_EMCY *p_em)
{
	return p_em->error_data.data;
}

static inline uint32_t CO_EMCY_get_last_error_code(CO_EMCY *p_em)
{
	return p_em->error_code.w;
}

static inline uint16_t CO_EMCY_get_last_manu_error_code(CO_EMCY *p_em)
{
	return p_em->error_data.manu_err_code;
}

static inline uint16_t CO_EMCY_get_last_std_error_code(CO_EMCY *p_em)
{
	return p_em->error_data.std_err_code;
}

static inline uint32_t CO_EMCY_get_last_error_reg(CO_EMCY *p_em)
{
	return p_em->error_reg.w;
}

static inline uint16_t CO_EMCY_get_last_manu_error_reg(CO_EMCY *p_em)
{
	return p_em->error_data.manu_err_reg;
}

static inline uint8_t CO_EMCY_get_last_std_error_reg(CO_EMCY *p_em)
{
	return p_em->error_data.std_err_reg;
}
#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_EMCY_H_ */
