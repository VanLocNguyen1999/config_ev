//
// Created by vuonglk on 19/09/2024.
//
#include <stdio.h>
#ifdef __linux
#include <unistd.h>
#endif

#include "sm_boot_slave.h"
#include "sm_fw.h"
#include "sm_crc.h"
#include "sm_string_t.h"
#include "sm_intel_hex.h"

static sm_fw_signature_t g_new_fw_signature = {
        .m_size = 0,
        .m_crc = 0,
        .m_addr = 0,
        .m_is_valid = 0,
        .m_version = {0x01, 0x00, 0x02},
        .m_data_address_offset = -1
};

static string_t* g_file_path = NULL;
static int32_t g_total_seg = 0;
static FILE* g_hex_file;

static uint32_t g_extra_bytes_count = 0;
static uint8_t g_extra_bytes_buff[32] = {0,};
static uint8_t g_finish_file = 0;

int32_t sm_file_init_new(){
    FILE* hex_file = fopen(g_file_path->buffer, "rb");
    if(!hex_file){
        printf("Could NOT open the HEX file %s\n", g_file_path->buffer);
        return -1;
    }

    char* start_line = NULL;
    uint8_t buff_hex[50]; /*seg_data[1024];*/
    size_t size_line = 0;
    size_t line_len = 0;
    int line_number = 0;

    uint16_t crcVal = CRC_16_INIT_VALUE;
    int32_t file_size = 0;

    while (line_len >= 0){
        line_len = getline(&start_line, &size_line, hex_file);
//        printf("get a line with len %d\n", line_len);

        line_number++;

        if(!strcmp(&start_line[line_len-2], "\r\n")){
            line_len -= 2;
        }else if(start_line[line_len-1] == '\n'){
            line_len--;
        }

        int hex_length = string_to_hex(start_line, line_len, buff_hex);
        intel_hex *p_record = intel_hex_process(buff_hex, hex_length);

        if(p_record == NULL){
            printf("recode is NOT hex file format\n");
            return -1;
        }

        if(p_record->record_type == INTEL_HEX_Extended_Linear_Address){
            uint32_t addr_16_31 = (uint32_t)(((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1]);
            uint32_t addr_0_15 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];

            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_16_31<<16)  + addr_0_15;
            }

            printf("Recode Type = 4, line_number: %d\n", line_number);
        }else if(p_record->record_type == INTEL_HEX_Extended_Segment_Address){
            uint32_t addr_4_19 = ((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1];
            uint32_t addr_0 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];
            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_4_19<<4) + addr_0;
            }
            printf("Recode Type = 2, line_number: %d, addr 0x%x\n", line_number, (addr_4_19<<4) + addr_0);
        }else if(p_record->record_type == INTEL_HEX_Data){
            if(g_new_fw_signature.m_data_address_offset < 0){
                g_new_fw_signature.m_data_address_offset = (p_record->addr[0]<<8) + p_record->addr[1];
            }
            printf("Recode Type = 0, line_number: %d, addr 0x%x\n", line_number, (p_record->addr[0]<<8) + p_record->addr[1]);
            break;
        }else if(p_record->record_type == INTEL_HEX_EndFile){
            printf("Finished read hex file\n");

            break;
        }
    }

    g_new_fw_signature.m_size = 0;
    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_addr += g_new_fw_signature.m_data_address_offset;
    g_new_fw_signature.m_crc = CRC_16_INIT_VALUE;

    g_total_seg = file_size/1024;
    if(file_size%1024 != 0){
        g_total_seg += 1;
    }

    printf("Firmware total line: %d \n", line_number);
    printf("Firmware address: 0x%x\n", g_new_fw_signature.m_addr);

    return 0;
}

int32_t sm_file_free_new(){
    g_new_fw_signature.m_size = 0;
    g_new_fw_signature.m_addr = 0;
    g_new_fw_signature.m_crc = 0;
    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_data_address_offset = -1;
    if(g_hex_file){
        fclose(g_hex_file);
        g_hex_file = NULL;
    }
    g_finish_file = 0;
    g_extra_bytes_count = 0;
    memset(g_extra_bytes_buff, 0, 32);
    return 0;
}

int32_t sm_file_get_fw_info_new(sm_fw_signature_t* _fw){
    if(!g_finish_file){
        printf("Not read full file hex, cannot read fw info");
        return -1;
    }

    sm_fw_signature_clone(_fw, &g_new_fw_signature);
    return 1;
}

int32_t sm_file_get_total_seg_new(){
    return g_total_seg;
}

int32_t sm_file_get_seg_fw_new(sm_segment_t* _seg){
    int seg_index = _seg->m_index;

    if(!g_hex_file){
        g_hex_file = fopen(g_file_path->buffer, "rb");
        printf("Open hex file\n");
        if(!g_hex_file){
            printf("Could NOT open the HEX file %s\n", g_file_path->buffer);
            _seg->m_is_last = true;
            return -1;
        }
    }

    char* start_line = NULL;
    uint8_t buff_hex[50];
    int size_line = 0;
    int line_len;

    char* seg_temp_buff = malloc(1024);  // 32 for extra bytes
    uint32_t seg_temp_size = 0;

    if(g_extra_bytes_count > 0){
        memcpy(seg_temp_buff, g_extra_bytes_buff, g_extra_bytes_count);
        seg_temp_size+=g_extra_bytes_count;
        g_extra_bytes_count = 0;
    }

    while (seg_temp_size < 1024){
        line_len = getline(&start_line, &size_line, g_hex_file);
//        printf("get a line with len %d\n", line_len);

        if(line_len <= 0){
            _seg->m_is_last = true;
            break;
        }

        if(!strcmp(&start_line[line_len-2], "\r\n")){
            line_len -= 2;
        }else if(start_line[line_len-1] == '\n'){
            line_len--;
        }

        int hex_length = string_to_hex(start_line, line_len, buff_hex);
        intel_hex *p_record = intel_hex_process(buff_hex, hex_length);

        if(p_record == NULL){
            printf("recode is NOT hex file format\n");
            free(seg_temp_buff);
            return -1;
        }

        if(p_record->record_type == INTEL_HEX_Data){
            memcpy(seg_temp_buff + seg_temp_size, p_record->data, p_record->byte_count);
            seg_temp_size+= p_record->byte_count;
//            printf("read %d byte data, seg len now %d\n", p_record->byte_count, seg_temp_size);
        }else if(p_record->record_type == INTEL_HEX_EndFile){
            printf("Finished read hex file\n");
            g_finish_file = true;
            break;
        }
    }

    if(seg_temp_size > 1024){
        g_extra_bytes_count = seg_temp_size - 1024;
        memcpy(g_extra_bytes_buff, seg_temp_buff+1024, g_extra_bytes_count);
        seg_temp_size = 1024;
    }

    uint16_t crc = sm_CRC_CalculateCRC16((uint8_t*)seg_temp_buff, seg_temp_size);
    g_new_fw_signature.m_crc = sm_CRC_CalculateCRC16_init_value(g_new_fw_signature.m_crc, (uint8_t*)seg_temp_buff, seg_temp_size);
    g_new_fw_signature.m_size += seg_temp_size;

    sm_seg_set_info(_seg, seg_index, g_new_fw_signature.m_addr + seg_index*1024, seg_temp_size, crc);
    sm_seg_fill_data(_seg, (uint8_t*)seg_temp_buff, seg_temp_size);

    if(g_finish_file && !g_extra_bytes_count){
        sm_seg_set_last_segment(_seg, 1);
        printf("Last segment is load from storage, size of last segment: %d\n", seg_temp_size);
    }else{
        sm_seg_set_last_segment(_seg, 0);
    }

    free(seg_temp_buff);
    return 1;
}

static sm_boot_input_if_t g_boot_input_proc = {
        .init = sm_file_init_new,
        .free = sm_file_free_new,
        .get_total_seg = sm_file_get_total_seg_new,
        .get_fw_info = sm_file_get_fw_info_new,
        .get_seg_fw = sm_file_get_seg_fw_new,
        .process = NULL
};

sm_boot_input_if_t* sm_get_file_boot_input_new(const char* file_path){
    if(file_path){
        string_dispose(g_file_path);
        g_file_path = string_createz(file_path);
        return &g_boot_input_proc;
    }
    return NULL;
}