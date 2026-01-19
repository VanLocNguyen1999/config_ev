//
// Created by vuonglk on 06/09/2024.
//
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_sv_file_receiver_output_if.h"

sm_memory_info_t g_mem_info;
sm_file_info_t  g_file_info;
const char* g_file_path;

#define BIN_FILE_PATH "./output_file.bin"
#define HEX_FILE_PATH "./output_file.hex"
#define TXT_FILE_PATH "./output_file.txt"

#define TAG "file_output"

int32_t sm_file_output_init(sm_memory_info_t* mem_info) {
    g_mem_info = *mem_info;
    if(mem_info->m_memory_ext == SM_MEMORY_EXT_BIN_FILE){
        g_file_path = BIN_FILE_PATH;
    }else if(mem_info->m_memory_ext == SM_MEMORY_EXT_HEX_FILE){
        g_file_path = HEX_FILE_PATH;
    } else{
        g_file_path = TXT_FILE_PATH;
    }

    LOG_INF(TAG, "Loading file to %s", g_file_path);
    FILE* fw_file = fopen(g_file_path, "w");
    fclose(fw_file);
    return 1;
}

int32_t sm_file_output_free(sm_memory_info_t* mem_info) {
    g_file_info.m_file_crc = CRC_16_INIT_VALUE;
    g_file_info.m_file_size = 0;
    g_file_path = NULL;
    return 0;
}

int32_t sm_file_output_save_frame(sm_file_data_frame_t* info) {
    FILE* fw_file = fopen(g_file_path, "a");
    if(fw_file < 0){
        return -1;
    }
    size_t len = fwrite(info->m_data, sizeof(char), info->m_size, fw_file);
    fclose(fw_file);

    return (int32_t)len;
}

int32_t sm_file_output_check_valid_file(sm_file_info_t* file_info) {

    FILE* bin_file = fopen(g_file_path, "rb");

    fseek(bin_file, 0L, SEEK_END);
    int file_size = (int)ftell(bin_file);
    fseek(bin_file, 0L, SEEK_SET);

    uint8_t* g_file_buffer = malloc(file_size * sizeof(char));

    fread(g_file_buffer, sizeof(char), file_size, bin_file);

    uint32_t crc = sm_CRC_CalculateCRC16(g_file_buffer, file_size);

    free(g_file_buffer);

    if(crc != file_info->m_file_crc || file_size != file_info->m_file_size)
        return -1;

    return 1;
}

int32_t sm_file_output_process() {
    return 0;
}

sm_file_output_if_t g_file_recv_output = {
        .init = sm_file_output_init,
        .free = sm_file_output_free,
        .save_frame = sm_file_output_save_frame,
        .check_valid_file = sm_file_output_check_valid_file,
        .process = sm_file_output_process
};


sm_file_output_if_t* sm_file_receiver_get_file_output(){
    g_file_info.m_file_crc = CRC_16_INIT_VALUE;
    g_file_info.m_file_size = 0;

    return &g_file_recv_output;
}
