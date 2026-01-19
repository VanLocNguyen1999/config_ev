/*
 * sm_ssl_file_storage.h
 *
 *  Created on: Nov 25, 2024
 *      Author: vuonglk
 */

#ifndef SM_SSL_FILE_STORAGE_H_
#define SM_SSL_FILE_STORAGE_H_
#include "stdint.h"
#include "stdbool.h"

typedef enum{
    SM_SSL_CA_CERT_FILE,
    SM_SSL_CLIENT_CERT_FILE,
    SM_SSL_CLIENT_KEY_FILE,
    SM_SSL_FILE_NUMBER
}SM_SSL_FILE_FD;

typedef struct {
    int32_t (*get_file_fd)(uint32_t addr);
    int32_t (*get_file_size)(SM_SSL_FILE_FD fd);                                 // return file size, -1 = file not exist
    int32_t (*clean_file)(SM_SSL_FILE_FD fd);                                    // return 0 = success, -1 = fail
    int32_t (*write_file)(SM_SSL_FILE_FD fd, uint8_t* data, uint32_t size);      // trunc file, return total bytes write, -1 = failed
    int32_t (*read_file)(SM_SSL_FILE_FD fd, uint8_t* buff, uint32_t size);       // return total bytes read,  -1 = failed
    int32_t (*append_file)(SM_SSL_FILE_FD fd, uint8_t* data, uint32_t size);     // return current total file byte, -1 = failed
}sm_ssl_storage_proc_t;

#endif /* SM_SSL_FILE_STORAGE_H_ */
