//
// Created by vuonglk on 06/09/2024.
//
#include <string.h>
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_sv_file_receiver_output_if.h"
#include "sm_hmi_flash_config.h"

sm_memory_info_t g_mem_info;
sm_file_info_t  g_file_info;
sm_ssl_storage_proc_t* g_file_storage;
int32_t g_fd;

#define TAG "flash_output"

int32_t sm_flash_output_init(sm_memory_info_t* mem_info) {
    g_file_info.m_file_crc = CRC_16_INIT_VALUE;
    g_file_info.m_file_size = 0;
    g_fd = -1;

    g_mem_info = *mem_info;

    if(mem_info->m_memory_type == SM_MEMORY_TYPE_INTERNAL){
        LOG_ERR(TAG, "Not support load internal flash file");
        return -1;
    }
    if(!g_file_storage){
        LOG_ERR(TAG, "file ssl storage invalid");
        return -1;
    }
    g_fd = g_file_storage->get_file_fd(mem_info->m_start_addr);
    if(g_fd < 0){
        return -1;
    }

    return g_file_storage->clean_file(g_fd);
}

int32_t sm_flash_output_free(sm_memory_info_t* mem_info) {
    g_file_info.m_file_crc = CRC_16_INIT_VALUE;
    g_file_info.m_file_size = 0;
    g_fd = -1;
    return 0;
}

int32_t sm_flash_output_save_frame(sm_file_data_frame_t* info) {
    if(!g_file_storage || g_fd < 0){
        LOG_ERR(TAG, "File not create yet");
        return -1;
    }
    if(g_file_storage->append_file(g_fd, info->m_data, info->m_size) < 0){
        LOG_ERR(TAG, "Append file FAILED");
        return -1;
    }
    g_file_info.m_file_crc = sm_CRC_CalculateCRC16_init_value(g_file_info.m_file_crc, info->m_data, info->m_size);
    g_file_info.m_file_size += info->m_size;
    return 1;
}

int32_t sm_flash_output_check_valid_file(sm_file_info_t* file_info) {
    if(g_file_info.m_file_crc != file_info->m_file_crc || g_file_info.m_file_size != file_info->m_file_size){
        return -1;
    }
    return 1;
}

int32_t sm_flash_output_process() {
    return 0;
}

sm_file_output_if_t g_file_recv_output = {
        .init = sm_flash_output_init,
        .free = sm_flash_output_free,
        .save_frame = sm_flash_output_save_frame,
        .check_valid_file = sm_flash_output_check_valid_file,
        .process = sm_flash_output_process
};


sm_file_output_if_t* sm_file_receiver_get_flash_output(sm_ssl_storage_proc_t* _file_storage){
    if(!_file_storage){
        return NULL;
    }
    g_file_info.m_file_crc = CRC_16_INIT_VALUE;
    g_file_info.m_file_size = 0;
    g_file_storage = _file_storage;
    return &g_file_recv_output;
}
