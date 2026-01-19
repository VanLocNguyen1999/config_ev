
/*
 * CO_OD_storage.h
 *
 *  Created on: Jul 15, 2022
 *      Author: Dakaka
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_H_

#include "CO_config.h"
#include "CO_Object.h"
#include "CO_OD_sign.h"
#include "CO_FLASH.h"
#include "CO_OD.h"
#include "CO_CRC.h"

/************************************************************************
 * Storage Image Map in FLASH:
 * - Region1: Signature (5byte)
 *       - Byte0: Function code
 *       - Byte1234: SW version
 * - Region2: Communication profile
 * - Region3: Application profile
 *
 ************************************************************************/
typedef struct
{
	bool		sign_valid;

	uint32_t	addr;
	uint16_t	size;

	uint32_t	comm_addr;
	uint32_t	comm_size;

	uint32_t	app_addr;
	uint32_t	app_size;
	uint32_t	boot_setting_size;

	uint32_t	sign_addr;
	uint32_t	sign_size;
}CO_OD_Storage_Image_t;

typedef struct
{
	void *p_data;
	uint16_t size;
}CO_OD_Storage_Entry_t;

typedef int8_t (*Earse_And_Write_Data_To_Flash)(uint32_t addr, uint8_t* buff, uint16_t len);

typedef struct CO_OD_Storage_t CO_OD_Storage;
struct CO_OD_Storage_t
{
	CO_OD_Sign sign;
	CO_OD_Storage_Image_t image;

	CO_OD_Storage_Entry_t comm_para;
	CO_OD_Storage_Entry_t app_para;

	CO_FLASH* p_co_flash;
	CO_OD* p_od;
};

/*
 * Attention:
 * - _this function will load all communication, application profile/data in FLASH
 * 	 to operating data/variable of OD
 * */
void CO_OD_STORAGE_init(CO_OD_Storage *p__this,
		uint32_t image_addr,
		uint32_t image_size,
		void *p_temp_comm_para,
		void* p_temp_app_para,
		uint32_t app_profile_size,
		CO_FLASH *p_co_flash,
		CO_OD *p_od,
		uint32_t sw_ver,
		bool is_main_app);


int8_t CO_OD_STORAGE_save_comm_para_to_flash(CO_OD_Storage* _this);
int8_t CO_OD_STORAGE_save_app_para_to_flash(CO_OD_Storage* _this);
int8_t CO_OD_STORAGE_save_sign_to_flash(CO_OD_Storage* _this);

static inline void CO_OD_STORAGE_update_crc16_comm_from_data_in_ram(CO_OD_Storage* _this)
{
    uint16_t comm_para_crc16_val = CO_CRC_CalculateCRC16((uint8_t*)_this->comm_para.p_data, _this->comm_para.size);
    CO_OD_SIGN_set_crc16_comm_para(&_this->sign, comm_para_crc16_val);
}

static inline void CO_OD_STORAGE_update_crc16_app_from_data_in_ram(CO_OD_Storage* _this)
{
	uint16_t app_para_crc16_val = CO_CRC_CalculateCRC16(
			(uint8_t*) ((uint8_t*)_this->app_para.p_data + _this->image.boot_setting_size),
			_this->app_para.size - _this->image.boot_setting_size);
    CO_OD_SIGN_set_crc16_app_para(&_this->sign, app_para_crc16_val);
}



#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_OD_STORAGE_H_ */
