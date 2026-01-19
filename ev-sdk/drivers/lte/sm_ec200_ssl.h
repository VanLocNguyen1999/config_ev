/*
 * sm_ec200_ssl.h
 *
 *  Created on: Oct 8, 2024
 *      Author: admin
 */

#ifndef SM_EC200_SSL_H
#define SM_EC200_SSL_H

#include <stdint.h>
#include <sm_ec200.h>

#define  CA_CERTIFICATE_FILE_NAME    "UFS:sm_cacert.pem"
#define  CC_CERTIFICATE_FILE_NAME    "UFS:sm_cccert.pem"
#define  CK_CERTIFICATE_FILE_NAME    "UFS:sm_ckkey.pem"



// Config ssl mode
int32_t sm_ec200_config_ssl_auth_mode(sm_ec200_t *modem);
int32_t sm_ec200_config_ssl_version(sm_ec200_t *modem);
int32_t sm_ec200_config_cipher_suite(sm_ec200_t *modem);
int32_t sm_ec200_ignore_local_time(sm_ec200_t *modem);

// Upload cert
int32_t sm_ec200_upload_certificate_authority(sm_ec200_t *modem, const char *ca_file_name);
int32_t sm_ec200_upload_client_certificate(sm_ec200_t *modem, const char *cc_file_name);
int32_t sm_ec200_upload_client_key(sm_ec200_t *modem, const char *ck_file_name);

// Config cert
int32_t sm_ec200_config_certificate_authority(sm_ec200_t *modem, const char *ca_file_name);
int32_t sm_ec200_config_client_certificate(sm_ec200_t *modem, const char *cc_file_name);
int32_t sm_ec200_config_client_key(sm_ec200_t *modem, const char *ck_file_name);

// Control Module Storage
int32_t sm_ec200_get_mem_space(sm_ec200_t *modem);
int32_t sm_ec200_get_list_files(sm_ec200_t *modem);
int32_t sm_ec200_upload_file(sm_ec200_t *modem, const char *file_name, const char *data, uint32_t data_size);
int32_t sm_ec200_remove_file(sm_ec200_t *modem, const char *file_name);



#endif // SM_EC200_SSL_H

