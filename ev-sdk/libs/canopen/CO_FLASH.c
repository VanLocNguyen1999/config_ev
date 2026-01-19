/*
 * CO_FLASH.c
 *
 *  Created on: Sep 24, 2022
 *      Author: Dakaka
 */


#include "CO_FLASH.h"
#include "CO_utils.h"

/*
 * Attention:
 * - "erase_and_write_to_flash_impl": return success(0), fail(-1)
 */
void CO_FLASH_init(CO_FLASH* p_flash,
		CO_Earse_And_Write_Data_To_Flash erase_and_write_data_to_flash_impl,
		CO_FLASH_Error_Handle error_handle_impl)
{
	p_flash->status = CO_FLASH_STATUS__idle;
	p_flash->erase_and_write = erase_and_write_data_to_flash_impl;

	p_flash->pending_cm.addr_dst = 0;
	p_flash->pending_cm.addr_src = 0;
	p_flash->pending_cm.len = 0;

	p_flash->error_handle = error_handle_impl;
}

//-------------------------------------------------------------------------------
#if CO_FLASH_USE_WITHOUT_DELAY
/*
 * Attention: Function has "delay"
 * Suggest: Call in main().while(1)
 */
void CO_FLASH_process(CO_FLASH* p_flash)
{
	if(CO_FLASH_STATUS__busy == p_flash->status)
	{
		int8_t result = p_flash->erase_and_write(p_flash->pending_cm.addr_dst,
				p_flash->pending_cm.addr_src, p_flash->pending_cm.len);
		if(0 == result)
		{
			p_flash->status = CO_FLASH_STATUS__success;
		}
		else
		{
			p_flash->status = CO_FLASH_STATUS__fail;
		}
	}
};

/*
 * Return:
 *  - Success (0): Started save "buff" to flash
 *  - Fail (-1): "CO_FLASH_process" function is busy
 * Attention: "buff" must be address of global variable
 * Suggest: Sau khi goi ham nay. Neu "FLASH_process" ranh, se bat dau qua trinh luu "buff" vao flash trong ham CO_FLASH_process. Ket qua cua qua trinh
 * luu se duoc lay tu ham "CO_FLASH_get_status"
 */
int8_t CO_FLASH_start_erase_and_write_to_flash(CO_FLASH* p_flash, uint32_t addr_dst, uint32_t addr_src, uint32_t len)
{
	if(CO_FLASH_STATUS__idle == p_flash->status)
	{
		p_flash->status = CO_FLASH_STATUS__busy;
		p_flash->pending_cm.addr_dst = addr_dst;
		p_flash->pending_cm.addr_src = addr_src;
		p_flash->pending_cm.len = len;
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * Return:
 * - success(0)
 * - fail(-1): "CO_FLASH_process" function is busy
 * */
int8_t CO_FLASH_read(CO_FLASH* p_flash, uint32_t addr_dst, uint32_t addr_src, uint32_t size)
{
	if(p_flash->status == CO_FLASH_STATUS__idle)
	{
		memcpy(addr_dst, addr_src, size);
		return 0;
	}
	return -1;
}
#else
/*
 * Return:
 * 	- Busy (1) : "CO_FLASH_erase_and_write_to_flash" function is busy
 *  - Success (0): Save "buff" to flash successfully
 *  - Fail (-1): Save "buff" to flash fail
 * Attention: This function has "delay" (estimate 20-50ms ....)
 * Suggest: Can call this function (use while) until return != busy
 */
int8_t CO_FLASH_erase_and_write_to_flash(CO_FLASH* p_flash, uint32_t addr_dst, uint32_t addr_src, uint32_t len)
{
	int8_t rt = 1;
	if(CO_FLASH_STATUS__idle == p_flash->status)
	{
		p_flash->status = CO_FLASH_STATUS__busy;
		if(p_flash->erase_and_write(addr_dst, addr_src, len) == 0)
		{
			rt = (CO_memcmp((uint8_t*)addr_dst, (uint8_t*)addr_src, len) == 1) ? 0 : -1;
		}
		else
		{
			rt = -1;
		}
		p_flash->status = CO_FLASH_STATUS__idle;
	}
	return rt;
}

/*
 * Return:
 * - success(0)
 * - busy(1): "CO_FLASH_process" function is busy
 * */
int8_t CO_FLASH_read(CO_FLASH* p_flash, uint32_t addr_dst, uint32_t addr_src, uint32_t size)
{
	if(p_flash->status == CO_FLASH_STATUS__idle)
	{
		CO_memcpy((uint8_t*)addr_dst, (uint8_t*)addr_src, size);
		return 0;
	}
	return 1;
}
#endif
//---------------------------------------------------------------------------------------------------
