//
// Created by vuonglk on 05/09/2024.
//

#ifndef EV_SDK_SM_SV_FILE_TRANSMIT_COMMON_H
#define EV_SDK_SM_SV_FILE_TRANSMIT_COMMON_H

#include <stdint.h>

#define SM_FILE_FRAME_SIZE  128

#define SM_MEMORY_TYPE_INTERNAL 0
#define SM_MEMORY_TYPE_EXTERNAL 1

#define SM_MEMORY_EXT_BIN_FILE 0
#define SM_MEMORY_EXT_HEX_FILE 1
#define SM_MEMORY_EXT_TXT_FILE 2

typedef struct {
    int32_t m_id;
    int32_t m_size;
    uint8_t m_data[SM_FILE_FRAME_SIZE];
    uint16_t m_crc;

    uint8_t m_is_last;
}sm_file_data_frame_t;

typedef struct {
    uint32_t m_start_addr;
    uint32_t m_mem_size;
    uint8_t m_memory_ext;
    uint8_t m_memory_type;
}sm_memory_info_t;

typedef struct{
    uint32_t m_file_size;
    uint16_t m_file_crc;
}sm_file_info_t;

#endif //EV_SDK_SM_SV_FILE_TRANSMIT_COMMON_H
