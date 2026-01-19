/*
 * sm_ssl_file_storage.h
 *
 *  Created on: Nov 25, 2024
 *      Author: vuonglk
 */

#ifndef SSL_FILE_SM_SSL_FILE_STORAGE_IMPL_H_
#define SSL_FILE_SM_SSL_FILE_STORAGE_IMPL_H_
#include "sm_ssl_file_storage.h"
#include "sm_ex_flash_storage.h"


int32_t sm_ssl_storage_init_ext_flash_instance(sm_ex_flash_storage_t* _ext_flash);

sm_ssl_storage_proc_t* sm_ssl_storage_get_ext_flash_instance();

#endif /* SSL_FILE_SM_SSL_FILE_STORAGE_IMPL_H_ */
