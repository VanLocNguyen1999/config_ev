//
// Created by vuonglk on 05/09/2024.
//

#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include "sm_logger.h"
#include "sm_sv_file_transfer_input_if.h"
#include "sm_crc.h"

#define TAG "file_input"

#define SEGMENT_SIZE SM_FILE_FRAME_SIZE

static char* g_file_buff = NULL;
static sm_file_info_t g_file_info = {
        .m_file_crc = 0,
        .m_file_size = 0
};

int32_t sm_file_input_init(){
    return 1;
}

int32_t sm_file_input_free(){
    if(g_file_buff){
        free(g_file_buff);
        g_file_buff = NULL;
    }
    g_file_info.m_file_crc = 0;
    g_file_info.m_file_size = 0;
    return 1;
}

int32_t sm_file_get_file_info(sm_file_info_t * _info){
    _info->m_file_size = g_file_info.m_file_size;
    _info->m_file_crc = g_file_info.m_file_crc;
    return 1;
}


int32_t sm_file_get_frame_fw(sm_file_data_frame_t * _frame, uint32_t _index){
    if(!g_file_buff)
        return -1;

    uint32_t file_size = g_file_info.m_file_size;

    uint32_t total_seg = file_size / SEGMENT_SIZE;
    if(file_size % SEGMENT_SIZE != 0){
        total_seg++;
    }

    uint32_t seg_len = (file_size - SEGMENT_SIZE*_index);
    if(seg_len > SEGMENT_SIZE){
        seg_len = SEGMENT_SIZE;
        _frame->m_is_last = false;
    }else{
        _frame->m_is_last = true;
    }

    _frame->m_id = _index;
    _frame->m_crc = sm_CRC_CalculateCRC16((uint8_t*)g_file_buff + _index*SEGMENT_SIZE, seg_len);
    _frame->m_size = seg_len;
    memcpy(_frame->m_data, g_file_buff + _index*SEGMENT_SIZE, seg_len);

    return 1;
}

sm_file_input_if_t g_file_transfer_input = {
        .init = sm_file_input_init,
        .free = sm_file_input_free,
        .get_file_info = sm_file_get_file_info,
        .get_file_data_frame = sm_file_get_frame_fw,
        .process = NULL
};

sm_file_input_if_t* sm_sv_file_transfer_get_file_input(const char* _path){

    FILE* hex_file = fopen(_path, "rb");
    if(!hex_file){
        printf("Cannot open hex file\n");
        return NULL;
    }

    fseek(hex_file, 0L, SEEK_END);
    int file_size = (int)ftell(hex_file);
    fseek(hex_file, 0L, SEEK_SET);

    g_file_buff = malloc(file_size * sizeof(char));
    fread(g_file_buff, sizeof(char), file_size, hex_file);

    g_file_info.m_file_crc = sm_CRC_CalculateCRC16((uint8_t*)g_file_buff, file_size);
    g_file_info.m_file_size = file_size;

    LOG_INF(TAG, "Start transfer file %s", _path);

    return &g_file_transfer_input;
}

