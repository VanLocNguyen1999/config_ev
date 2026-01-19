/*
 * sm_sll_file_storage.c
 *
 *  Created on: Nov 25, 2024
 *      Author: vuonglk
 */

#include "sm_ssl_file_storage_impl.h"
#include "stdlib.h"
#include "sm_hmi_flash_config.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_utils.h"

#define TAG "sm_ssl_storage_impl"

#define SSL_FILE_MAX_SIZE       (2*1024)
#define SSL_VERIFY_INFO_SIZE    (4+4+2)


static uint32_t true_magic_value = 0x55AA5A5A;

/* each file storage follow this frame
 *
 *_______________________________________________________________________
 *     4 byte     |    4 byte    |       n byte       |   2 byte        |
 *   magic value  |  file size   |     file data      |  file data crc  |
 *________________|______________|____________________|_________________|
 *
 *
 * if verify data invalid -> return file size = -1 or file exist = false
 *
 * */


typedef struct{
    uint32_t m_size;
    uint32_t m_addr;
    sm_ex_flash_storage_partition_t* m_if;
}sm_ssl_file_info_t;


typedef struct{
    sm_ssl_storage_proc_t m_proc;
    sm_ssl_file_info_t m_file_list[SM_SSL_FILE_NUMBER];
}sm_ssl_file_storage_impl_t;

static sm_ssl_file_storage_impl_t* g_instance = NULL;


int32_t sm_ssl_open_file(uint32_t addr){
    if(!g_instance){
        LOG_ERR(TAG, "ssl FS not open yet");
        return -1;
    }

    for(int fd = 0; fd < SM_SSL_FILE_NUMBER; fd++){
        if(addr == g_instance->m_file_list[fd].m_addr){
            return fd;
        }
    }

    return -1;
}

int32_t sm_ssl_clear_file(SM_SSL_FILE_FD _file){
    if(!g_instance || _file >= SM_SSL_FILE_NUMBER || !g_instance->m_file_list[_file].m_if){
        LOG_ERR(TAG, "ssl file argument invalid!");
        return -1;
    }
    LOG_INF(TAG, "Now clear file");
    return sm_ex_flash_storage_erase_full_partition(g_instance->m_file_list[_file].m_if);
}

static int32_t sm_ssl_read_file(SM_SSL_FILE_FD _file, uint8_t* _buff, uint32_t _max_size){
    if(!g_instance || _file >= SM_SSL_FILE_NUMBER || !g_instance->m_file_list[_file].m_if){
        LOG_ERR(TAG, "ssl file argument invalid!");
        return -1;
    }

    uint8_t load_buff[SSL_FILE_MAX_SIZE] = {0,};

    if(sm_ex_flash_storage_partition_read(g_instance->m_file_list[_file].m_if, 0, load_buff, SSL_FILE_MAX_SIZE) < 0){
        LOG_ERR(TAG, "Load ssl file index %d FAILED", _file);
        return -1;
    }

    uint32_t magic_val = getUint32(load_buff);
    if(magic_val != true_magic_value){
        LOG_ERR(TAG, "File not exist by check magic value");
        return -1;
    }

    uint32_t file_size = getUint32(load_buff + 4);
    if(file_size > SSL_FILE_MAX_SIZE){
        LOG_ERR(TAG, "File size %d is too large, invalid", file_size);
        return -1;
    }

    uint8_t* p_data = load_buff + 4 + 4;

    uint16_t file_crc = getUint16(p_data + file_size);
    if(file_crc != sm_CRC_CalculateCRC16(p_data, file_size)){
        LOG_ERR(TAG, "File crc invalid");
        return -1;
    }
    if(_buff){
        if(_max_size < file_size){
            LOG_ERR(TAG, "Buffer is not enough too storage file");
            return -1;
        }
        memset(_buff, 0, _max_size);
        memcpy(_buff, p_data, file_size);
    }
    return file_size;
}


static int32_t sm_ssl_get_file_size(SM_SSL_FILE_FD _file){
    return sm_ssl_read_file(_file, NULL, 0);
}

static int32_t sm_ssl_write_file(SM_SSL_FILE_FD _file, uint8_t* _data, uint32_t _size){
    if(!g_instance || _file >= SM_SSL_FILE_NUMBER || !g_instance->m_file_list[_file].m_if){
        LOG_ERR(TAG, "ssl file argument invalid!");
        return -1;
    }

    if(_size > SSL_FILE_MAX_SIZE){
        LOG_ERR(TAG, "File too large to storage");
        return -1;
    }

    if(sm_ssl_clear_file(_file) < 0){
        LOG_ERR(TAG, "Erase file FAILED");
        return -1;
    }

    int32_t ret = 0;
    uint16_t crc = sm_CRC_CalculateCRC16(_data, _size);
    LOG_INF(TAG, "Now write %d byte to file fd %d", _size, _file);


    if(sm_ex_flash_storage_partition_write(g_instance->m_file_list[_file].m_if, 0, (uint8_t*)&true_magic_value, 4) < 0){
        ret = -1;
    }

    if(sm_ex_flash_storage_partition_write(g_instance->m_file_list[_file].m_if, 4, (uint8_t*)&_size, 4) < 0){
        ret = -1;
    }

    if(sm_ex_flash_storage_partition_write(g_instance->m_file_list[_file].m_if, 8, _data, _size) < 0){
        ret = -1;
    }

    if(sm_ex_flash_storage_partition_write(g_instance->m_file_list[_file].m_if, 8 + _size, (uint8_t*)&crc, 2) < 0){
        ret = -1;
    }

    return (ret < 0) ? ret : _size;
}


int32_t sm_ssl_append_file(SM_SSL_FILE_FD _file, uint8_t* _data, uint32_t _size){
    if(!g_instance || _file >= SM_SSL_FILE_NUMBER || !g_instance->m_file_list[_file].m_if){
        LOG_ERR(TAG, "ssl file argument invalid!");
        return -1;
    }

    if(_size > SSL_FILE_MAX_SIZE){
        LOG_ERR(TAG, "File too large to storage");
        return -1;
    }

    int32_t current_size = 0;
    uint8_t load_buff[SSL_FILE_MAX_SIZE] = {0,};

    current_size = sm_ssl_read_file(_file, load_buff, SSL_FILE_MAX_SIZE);
    if(current_size < 0){
        LOG_WRN(TAG, "File not exist, create new one");
        current_size = 0;
    }

    if(current_size + _size > SSL_FILE_MAX_SIZE){
        LOG_ERR("TAG", "File not enough space to storage!");
        return -1;
    }

    memcpy(load_buff + current_size, _data, _size);

    LOG_INF(TAG, "Now append %d byte to file fd %d", _size, _file);

    return sm_ssl_write_file(_file, load_buff, current_size + _size);
}



int32_t sm_ssl_storage_init_ext_flash_instance(sm_ex_flash_storage_t* _ext_flash){
    if(!_ext_flash){
        LOG_ERR(TAG, "ext flash invalid");
        return -1;
    }

    if(!g_instance){
        g_instance = malloc(sizeof(sm_ssl_file_storage_impl_t));

        if(!g_instance){
            LOG_ERR(TAG, "Create instance failed!");
            return -1;
        }


        g_instance->m_proc.append_file = sm_ssl_append_file;
        g_instance->m_proc.get_file_fd = sm_ssl_open_file;
        g_instance->m_proc.clean_file = sm_ssl_clear_file;
        g_instance->m_proc.get_file_size = sm_ssl_get_file_size;
        g_instance->m_proc.read_file = sm_ssl_read_file;
        g_instance->m_proc.write_file = sm_ssl_write_file;


        g_instance->m_file_list[SM_SSL_CA_CERT_FILE].m_addr = SM_HMI_SSL_CA_CERT_STORAGE_ADDR;
        g_instance->m_file_list[SM_SSL_CA_CERT_FILE].m_if = sm_ex_flash_storage_create_partition(_ext_flash,
                                                                                               SM_HMI_SSL_CA_CERT_STORAGE_ADDR,
                                                                                               SM_HMI_SSL_CA_CERT_STORAGE_SIZE);

        g_instance->m_file_list[SM_SSL_CLIENT_CERT_FILE].m_addr = SM_HMI_SSL_CLIENT_CERT_STORAGE_ADDR;
        g_instance->m_file_list[SM_SSL_CLIENT_CERT_FILE].m_if  = sm_ex_flash_storage_create_partition(_ext_flash,
                                                                                                   SM_HMI_SSL_CLIENT_CERT_STORAGE_ADDR,
                                                                                                   SM_HMI_SSL_CLIENT_CERT_STORAGE_SIZE);

        g_instance->m_file_list[SM_SSL_CLIENT_KEY_FILE].m_addr = SM_HMI_SSL_CLIENT_KEY_STORAGE_ADDR;
        g_instance->m_file_list[SM_SSL_CLIENT_KEY_FILE].m_if  = sm_ex_flash_storage_create_partition(_ext_flash,
                                                                                                  SM_HMI_SSL_CLIENT_KEY_STORAGE_ADDR,
                                                                                                  SM_HMI_SSL_CLIENT_KEY_STORAGE_SIZE);

        return 0;
    }else{
        LOG_ERR(TAG, "ssl file instance is already init");
        return -1;
    }
}



sm_ssl_storage_proc_t* sm_ssl_storage_get_ext_flash_instance(){
    if(!g_instance){
        LOG_ERR(TAG, "ssl file instance is not init yet!");
        return NULL;
    }
    return &g_instance->m_proc;
}
