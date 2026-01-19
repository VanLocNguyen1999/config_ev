/*
 * CO_error.h
 *
 *  Created on: Jun 15, 2022
 *      Author: dacnq
 */

#ifndef CO_ERROR_H_
#define CO_ERROR_H_

#include "CO_utils.h"



/* Debug mode: Jump to forever loop (while(1)) when detect error
 *  - Enable (1)
 *  - Disable (0)
 */
#define CO__DEBUG_MODE													1

/* SDO option : enable (set 1), disable (set 0) ------------------------------------------------*/
#define CO_SDO_CHECK_C_BIT_OF_CS										1
#define CO_SDO_CHECK_MATCH_DATA_LENGTH									1
#define CO_SDO_ENABLE_PROCESS_EXTENSION_IN_SERVER_DOWNLOAD				1
#define CO_SDO_USE_SYNC_WINDOW_LENGTH									0

/* TPDO option : enable (set 1), disable (set 0) -----------------------------------------------*/
#define CO_TPDO_USE_INHIBIT_TIMER										0
#define CO_TPDO_USE_EVENT_TIMER											0

/* FLASH option : enable (set 1), disable (set 0) ----------------------------------------------*/
#define CO_FLASH__USE													0
/**
 * Describe:
 * - Cac ham luu data vao flash tai lop board ton tai "delay". Dieu nay dan toi ham "CO_process" ton tai "delay" do ham nay co su dung chuc nang tu CO_FLASH,
 *  chuc nang SYNC hoat dong sai. Giai phap:
 * 			+ Lua chon "1", CO_FLASH se cung cap ham kich hoat viec luu tru (start function) va kiem tra tinh trang (get status).
 * 			Viec luu tru thuc su duoc dat trong ham "CO_FLASH_process" .
 * 			-> Van de: Khi su dung phuong an nay, nguoi trien khai phai quan ly tot viec phan luong hoac du an nen su dung RTOS.
 * 			Neu khong, se co van de xay ra
 * 			+ Lua chon "0": Chap nhan "Delay". CO_FLASH chi cung cap ham ghi truc tiep va ton tai "delay" (CO_FLASH_erase_and_write_data_to_flash)
 * 			-> De trien khai nhung ton tai "delay" tai ham "CO_process"
 */
#define CO_FLASH_USE_WITHOUT_DELAY										0 //<< Must set 0, 1 not support yet!

/* Storage option : enable (set 1), disable (set 0) ----------------------------------------------*/
/**
 * Describe:
 * - 1: Delete value in "Parameter setting partition of FLASH, save default parameter value in code into flash
 * - 0: Skip
 */

#define CO_OD_STORAGE__SAVE_DEFAULT_PARA_INTO_FLASH_WHEN_UPDATE_FW		0


static inline void CO_detect_error(void)
{
#if CO__DEBUG_MODE
  while (1){}
#endif
}

#endif /* CO_ERROR_H_ */
