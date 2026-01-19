/*
 * CO_FLASH.h
 *
 *  Created on: Sep 24, 2022
 *      Author: Dakaka
 *
 *  Target: Handle the saving data into flash
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_FLASH_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_FLASH_H_

#include "CO_config.h"
#include "string.h"

typedef struct CO_FLASH_t CO_FLASH;
typedef enum
{
	CO_FLASH_STATUS__idle = 0,			/* */
	CO_FLASH_STATUS__busy = 1,			/* Saving data to FLASH*/
	CO_FLASH_STATUS__success = 2,		/* Erase and write data to flash successfully */
	CO_FLASH_STATUS__fail = -1			/* Erase and write data to flash failed */
}CO_FLASH_Status_t;

typedef int8_t (*CO_Earse_And_Write_Data_To_Flash)(uint32_t addr_dst, uint32_t addr_src, uint32_t len); /*Return: success(0), fail(-1)*/
typedef void (*CO_FLASH_Error_Handle)(void);

typedef struct
{
	uint32_t addr_dst;
	uint32_t addr_src;
	uint32_t len;
}CO_Pending_CM_t;

struct CO_FLASH_t
{
	CO_FLASH_Status_t status;
	CO_Pending_CM_t pending_cm;

	CO_Earse_And_Write_Data_To_Flash erase_and_write;
	CO_FLASH_Error_Handle error_handle;
};

void CO_FLASH_init(CO_FLASH* p_flash,
		CO_Earse_And_Write_Data_To_Flash erase_and_write_data_to_flash_impl,
		CO_FLASH_Error_Handle error_handle_impl);

//-------------------------------------------------------------------------------
#if CO_FLASH_USE_WITHOUT_DELAY
void CO_FLASH_process(CO_FLASH* p_flash);

int8_t CO_FLASH_start_erase_and_write_to_flash(CO_FLASH* p_flash, uint32_t addr, uint8_t* buff, uint32_t len);

static inline CO_FLASH_Status_t CO_FLASH_get_status(CO_FLASH* p_flash)
{
	return p_flash->status;
}

void CO_FLASH_reset_status(CO_FLASH* p_flash)
{
	p_flash->status = CO_FLASH_STATUS__idle;
}

int8_t CO_FLASH_read(CO_FLASH* p_flash, uint32_t buff_dst, uint32_t buff_src, uint32_t size);

#else

int8_t CO_FLASH_erase_and_write_to_flash(CO_FLASH* p_flash, uint32_t addr_dst, uint32_t addr_src, uint32_t len);
int8_t CO_FLASH_read(CO_FLASH* p_flash, uint32_t buff_dst, uint32_t buff_src, uint32_t size);

static inline void CO_FLASH_error_handle(CO_FLASH* p_flash)
{
	p_flash->error_handle();
}

#endif
//-------------------------------------------------------------------------------
#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_FLASH_H_ */
