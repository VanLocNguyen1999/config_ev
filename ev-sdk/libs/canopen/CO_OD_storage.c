/*
 * CO_OD_storage.c
 *
 *  Created on: Jul 15, 2022
 *      Author: Dakaka
 */

#include "CO_OD_storage.h"
#include "CO_FLASH.h"
#include "CO_OD.h"

CO_OD_Sign sign;
CO_OD_Storage co_storage;


/**--- Declare private function --------------------------------------------------------------------------------------------------------*/
static void co_load_sign_from_flash(CO_OD_Storage* this);

static inline void co_load_comm_para_from_flash(CO_OD_Storage* this);
static inline void co_load_app_para_from_flash(CO_OD_Storage* this);

static inline void co_copy_temp_data_to_operating_data
(CO_OD_Storage* this);

/**--- Define private function --------------------------------------------------------------------------------------------------------*/

/*
 * Has "delay"
 * */
static void co_load_sign_from_flash(CO_OD_Storage* this)
{
	while (1 == CO_FLASH_read(this->p_co_flash, (uint32_t)&this->sign, this->image.sign_addr,
					this->image.sign_size));
}

static bool co_check_crc16_from_data_in_flash_is_valid(CO_OD_Storage* this)
{
    uint16_t comm_para_crc16_val = CO_CRC_CalculateCRC16((uint8_t*)this->image.comm_addr, this->image.comm_size);
    uint16_t app_para_crc16_val = CO_CRC_CalculateCRC16((uint8_t*)(this->image.app_addr + this->image.boot_setting_size),
    		this->image.app_size - this->image.boot_setting_size);
    return (CO_OD_SIGN_get_crc16_comm_para(&this->sign) == comm_para_crc16_val
    		&& CO_OD_SIGN_get_crc16_app_para(&this->sign) == app_para_crc16_val);
}

static bool co_check_sign_is_valid(CO_OD_Storage* this, uint32_t cur_ver)
{
#if CO_OD_STORAGE__SAVE_DEFAULT_PARA_INTO_FLASH_WHEN_UPDATE_FW
    return (this->sign.sw_ver == cur_ver
			&& cur_ver != 0
			&& co_check_crc16_from_data_in_flash_is_valid(this));
#else
    (void)cur_ver;
    return (co_check_crc16_from_data_in_flash_is_valid(this));
#endif
}

static void co_process_when_reset(CO_OD_Storage* this, bool sign_is_valid, uint32_t cur_sw_ver, bool is_restore_valid)
{
	bool sign_in_flash_outdated = false;
	int8_t sum_err = 0;
	/**/
	if(sign_is_valid)
	{
		/**/
		uint8_t comm_para_cmd = CO_OD_SIGN_get_func(&this->sign, CO_FCG__COMM_PARA);
		if(CO_OD_SIGN__FUNC_CODE_COMMAND__NO_ACTION == comm_para_cmd || is_restore_valid == false)
		{
			co_load_comm_para_from_flash(this);
		}
		else if(CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA & comm_para_cmd)
		{
			sum_err -= CO_OD_STORAGE_save_comm_para_to_flash(this);
			sign_in_flash_outdated = true;
		}
		/**/
		uint8_t app_para_cmd = CO_OD_SIGN_get_func(&this->sign, CO_FCG__APP_PARA);
		if(CO_OD_SIGN__FUNC_CODE_COMMAND__NO_ACTION == app_para_cmd|| is_restore_valid == false)
		{
			co_load_app_para_from_flash(this);
		}
		else if(CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA & app_para_cmd)
		{
			sum_err -= CO_OD_STORAGE_save_app_para_to_flash(this);
			sign_in_flash_outdated = true;
		}
	}
	else if(is_restore_valid == true)
	{
		/**/
		sum_err -= CO_OD_STORAGE_save_comm_para_to_flash(this);
		sum_err -= CO_OD_STORAGE_save_app_para_to_flash(this);
		sign_in_flash_outdated = true;
	}
	if(sum_err != 0) while(1);
	/**/
	if(sign_in_flash_outdated)
	{
		CO_OD_STORAGE_update_crc16_comm_from_data_in_ram(this);
		CO_OD_STORAGE_update_crc16_app_from_data_in_ram(this);
		CO_OD_SIGN_set_func(&this->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__NO_ACTION);
		CO_OD_SIGN_set_func(&this->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__NO_ACTION);
		CO_OD_SIGN_set_sw_ver(&this->sign, cur_sw_ver);

		sum_err -= CO_OD_STORAGE_save_sign_to_flash(this);
	}
	if(sum_err != 0) while(1);
	/*Copy temporary data to operating data in OD*/
	co_copy_temp_data_to_operating_data(this);

}



static inline void co_load_comm_para_from_flash(CO_OD_Storage* this)
{
	while (1 == CO_FLASH_read(this->p_co_flash, (uint32_t)this->comm_para.p_data, this->image.comm_addr, this->comm_para.size));
}
static inline void co_load_app_para_from_flash(CO_OD_Storage* this)
{
	while (1 == CO_FLASH_read(this->p_co_flash, (uint32_t)this->app_para.p_data, this->image.app_addr, this->app_para.size));
}


static inline void co_copy_temp_data_to_operating_data(CO_OD_Storage* this)
{
	uint16_t obj_nb = CO_OD_get_number(this->p_od);
	uint16_t od_list_no = 0;
	uint8_t sub_no = 0;

	for (od_list_no = 0; od_list_no < obj_nb; od_list_no++)
	{
		uint8_t sub_nb = this->p_od->list[od_list_no].sub_number;
		for (sub_no = 0; sub_no < sub_nb; sub_no++)
		{
			CO_Sub_Object *p_sub = &(this->p_od->list[od_list_no].subs[sub_no]);
			if (p_sub->p_temp_data != NULL)
			{
				CO_memcpy(p_sub->p_data, p_sub->p_temp_data, p_sub->len);
			}
		}
	}
}

/**--- Define public function -----------------------------------------------------------------------------------------*/
/*
 * Attention:
 * - Call after init flash
 * - This function will load all communication, application profile/data in FLASH
 * 	 to operating data/variable of OD
 *
 * */
void CO_OD_STORAGE_init(CO_OD_Storage *p_this,
		uint32_t image_addr,
		uint32_t image_size,
		void *p_temp_comm_para,
		void* p_temp_app_para,
		uint32_t app_profile_size,
		CO_FLASH *p_co_flash,
		CO_OD *p_od,
		uint32_t sw_ver,
		bool is_main_app)
{
	p_this->p_od = p_od;
	/* Init image */
	p_this->image.addr = image_addr;
	p_this->image.size = image_size;

	p_this->image.sign_addr = p_this->image.addr;
	p_this->image.sign_size = sizeof(CO_OD_Sign);

	p_this->image.comm_addr = p_this->image.addr + p_this->image.sign_size;
	p_this->image.comm_size = sizeof(OD_Temp_Comm_Profile_Para_t);

	p_this->image.app_addr = p_this->image.comm_addr + p_this->image.comm_size;
	p_this->image.app_size = app_profile_size;
	p_this->image.boot_setting_size = CO_SIZE_OF_BOOT_SETTING*2;

	if((p_this->image.sign_size + p_this->image.comm_size + app_profile_size) > p_this->image.size)
	{
		CO_detect_error();
		while(1);
	}

	/* Init comm_para, app_para */
	p_this->comm_para.p_data = (void*)p_temp_comm_para;
	p_this->comm_para.size = p_this->image.comm_size;

	p_this->app_para.p_data = (void*)p_temp_app_para;
	p_this->app_para.size = p_this->image.app_size;

	/* Init p_co_flash */
	p_this->p_co_flash = p_co_flash;

	/* Sync data on RAM and FLASH */
	co_load_sign_from_flash(p_this);
	bool sign_is_valid = co_check_sign_is_valid(p_this, sw_ver);
	co_process_when_reset(p_this, sign_is_valid, sw_ver, is_main_app);
}

/*
 * Return:
 * - success (0)
 * - fail (-1)
 * Has "delay"
 * */
int8_t CO_OD_STORAGE_save_sign_to_flash(CO_OD_Storage* this)
{
	int8_t rt = 0;
	do
	{
		rt = CO_FLASH_erase_and_write_to_flash(this->p_co_flash, this->image.sign_addr, (uint32_t)&this->sign, this->image.sign_size);
	}while(rt == 1);
	return rt;
}

/*
 * Return:
 *  - Success (0): Save "buff" to flash successfully
 *  - Fail (-1): Save data to flash fail
 * Attention: This function has "delay" (estimate 20-50ms ....)
 */
int8_t CO_OD_STORAGE_save_comm_para_to_flash(CO_OD_Storage* this)
{
	int8_t rt = 0;
	do
	{
		rt = CO_FLASH_erase_and_write_to_flash(this->p_co_flash, this->image.comm_addr, (uint32_t)this->comm_para.p_data, this->comm_para.size);
	}while(rt == 1);
	return rt;
}
int8_t CO_OD_STORAGE_save_app_para_to_flash(CO_OD_Storage* this)
{
	int8_t rt = 0;
	do
	{
		rt = CO_FLASH_erase_and_write_to_flash(this->p_co_flash,
				this->image.app_addr + this->image.boot_setting_size,
				(uint32_t)this->app_para.p_data + this->image.boot_setting_size,
				this->app_para.size - this->image.boot_setting_size);
	}while(rt == 1);
	return rt;
}
