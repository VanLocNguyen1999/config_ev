//
// Created by vuonglk on 19/09/2024.
//

#include <stdio.h>
#include <string.h>
#include "sm_logger.h"
#include "sm_ex_flash_storage_if.h"
#include "sm_ssl_file_storage_impl.h"

#define TAG "ex_flash_test"

void log_put(const char* _log) {
    printf("%s\n", _log);
}

uint8_t read_arr[1024];


int main(){
    memset(read_arr, 8, 1024);
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);

    LOG_INF(TAG, "Start test ex flash module");

    sm_ex_flash_proc_t* flash_proc = sm_sv_ex_flash_storage_get_virtual_flash_proc(1024, 4*1024);

    sm_ex_flash_storage_t* ex_flash = sm_flash_storage_create_default(flash_proc);
    sm_ex_flash_storage_init(ex_flash);
    sm_ex_flash_erase_full(ex_flash);

    LOG_INF(TAG, "Create virtual flash size %d", sm_ex_flash_storage_get_total_size(ex_flash));



    char buff1[8] = "vuonglk";
    char buff2[8] = "selex12";
    char buff3[8] = "1234567";
    uint8_t storage_buff[128] = {0,};

    int32_t ret = 0;

    sm_ssl_storage_init_ext_flash_instance(ex_flash);

    sm_ssl_storage_proc_t* file_storage = sm_ssl_storage_get_ext_flash_instance();

    ret = file_storage->read_file(SM_SSL_CA_CERT_FILE, storage_buff, 128);


    ret = file_storage->clean_file(SM_SSL_CA_CERT_FILE);
    ret = file_storage->write_file(SM_SSL_CA_CERT_FILE, buff2, 7);
    ret = file_storage->append_file(SM_SSL_CA_CERT_FILE, buff1, 7);
    ret = file_storage->append_file(SM_SSL_CA_CERT_FILE, buff3, 7);

    ret = file_storage->read_file(SM_SSL_CA_CERT_FILE, storage_buff, 128);

    ret = file_storage->clean_file(SM_SSL_CA_CERT_FILE);

    ret = file_storage->read_file(SM_SSL_CA_CERT_FILE, storage_buff, 128);

    ret = file_storage->append_file(SM_SSL_CA_CERT_FILE, buff3, 7);

    ret = file_storage->append_file(SM_SSL_CA_CERT_FILE, buff3, 7);

    ret = file_storage->read_file(SM_SSL_CA_CERT_FILE, storage_buff, 128);


    while (1);


    return 0;
}