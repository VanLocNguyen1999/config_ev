//
// Created by vuonglk on 29/08/2024.
//

#include <stdbool.h>
#include <stdio.h>
#include "sm_boot_impl.h"
#include "sm_intel_hex.h"
#include "sm_crc.h"

#include "sm_logger.h"

#define TAG "flash_input"

#define BIN_FILE_FLASH_ADDR             0x0D0000U
#define BIN_FILE_MAX_CAPACITY           0x32000

#define FLASH_EMPTY_DATA                0xFF

#define FLASH_STORAGE_MAX_BUFFER        50


static bool check_valid_flash_data(const uint8_t* _buff, uint32_t _size){
    for(int i = 0; i < _size; i++){
        if(_buff[i] != FLASH_EMPTY_DATA){
            return true;
        }
    }
    return false;
}
static uint32_t get_one_line_data(const uint8_t* _buff, uint32_t _size){

    if(_buff[0] == '\n')
        return 0;

    uint32_t data_len = 0;

    for(int i = 1; i < _size; i++){
        if(_buff[i] == '\n'){
            data_len = i + 1;
            break;
        }
    }

    if(data_len == 0) data_len = _size;

    return data_len;
}

static sm_fw_signature_t g_new_fw_signature = {
        .m_size = 0,
        .m_crc = 0,
        .m_addr = 0,
        .m_is_valid = 0,
        .m_version = {0x01, 0x00, 0x02},
        .m_data_address_offset = -1
};

static uint32_t g_get_line_current_addr = 0;


static uint32_t g_extra_bytes_count = 0;
static uint8_t g_extra_bytes_buff[32] = {0,};
static uint8_t g_finish_file = 0;
static sm_ex_flash_storage_partition_t* g_flash_in = NULL;


static uint32_t flash_get_line(char* _buff){
    uint8_t buff_line[FLASH_STORAGE_MAX_BUFFER];

    sm_ex_flash_storage_partition_read(g_flash_in, g_get_line_current_addr , buff_line, FLASH_STORAGE_MAX_BUFFER);

    if(!check_valid_flash_data(buff_line, FLASH_STORAGE_MAX_BUFFER))
        return -1;

    uint32_t data_len = get_one_line_data(buff_line, FLASH_STORAGE_MAX_BUFFER);

    g_get_line_current_addr += data_len;
    memset(_buff, 0, FLASH_STORAGE_MAX_BUFFER);
    memcpy(_buff, buff_line, data_len);

    return data_len;
}

int32_t sm_input_flash_init(){

    char data_buff[FLASH_STORAGE_MAX_BUFFER];
    uint8_t hex_buff[FLASH_STORAGE_MAX_BUFFER];

    int line_number = 0;

    while (true){
        uint32_t line_len = flash_get_line(data_buff);
//       // printf("get a line with len %d\n", line_len);
        if(line_len <= 0 || line_len >= FLASH_STORAGE_MAX_BUFFER)
            break;

        line_number++;

        if(!strcmp((char*)&data_buff[line_len-2], "\r\n")){
            line_len -= 2;
        }else if(data_buff[line_len-1] == '\n'){
            line_len--;
        }

        int hex_length = string_to_hex(data_buff, line_len, hex_buff);
        intel_hex *p_record = intel_hex_process(hex_buff, hex_length);

        if(p_record == NULL){
           //LOG_INF(TAG, "recode is NOT hex file format\n");
            return -1;
        }

        if(p_record->record_type == INTEL_HEX_Extended_Linear_Address){
            uint32_t addr_16_31 = (uint32_t)(((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1]);
            uint32_t addr_0_15 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];

            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_16_31<<16)  + addr_0_15;
            }

           //LOG_INF(TAG, "Recode Type = 4, line_number: %d\n", line_number);
        }else if(p_record->record_type == INTEL_HEX_Extended_Segment_Address){
            uint32_t addr_4_19 = ((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1];
            uint32_t addr_0 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];
            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_4_19<<4) + addr_0;
            }
           //LOG_INF(TAG, "Recode Type = 2, line_number: %d, addr 0x%x\n", line_number, (addr_4_19<<4) + addr_0);
        }else if(p_record->record_type == INTEL_HEX_Data){
            if(g_new_fw_signature.m_data_address_offset < 0){
                g_new_fw_signature.m_data_address_offset = (p_record->addr[0]<<8) + p_record->addr[1];
                break;
            }
        }else if(p_record->record_type == INTEL_HEX_EndFile){
           // printf("Finished read hex file\n");

            break;
        }
    }

    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_addr += g_new_fw_signature.m_data_address_offset;
    g_new_fw_signature.m_crc = CRC_16_INIT_VALUE;

   //LOG_INF(TAG, "Firmware address: 0x%lx", g_new_fw_signature.m_addr);

    g_get_line_current_addr = 0;
    return 0;
}

int32_t sm_input_flash_free(){
    g_new_fw_signature.m_size = 0;
    g_new_fw_signature.m_addr = 0;
    g_new_fw_signature.m_crc = 0;
    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_data_address_offset = -1;


    g_finish_file = 0;
    g_extra_bytes_count = 0;
    memset(g_extra_bytes_buff, 0, 32);
    return 0;
}

int32_t sm_input_flash_get_fw_info(sm_fw_signature_t* _fw){
    if(!g_finish_file){
       // printf("Not read full file hex, cannot read fw info");
        return -1;
    }
    return sm_fw_signature_clone(_fw, &g_new_fw_signature);;
}


int32_t sm_input_flash_get_total_seg(){
    return 0;
}

int32_t sm_input_flash_get_seg_fw(sm_segment_t* _seg){
    int seg_index = _seg->m_index;

    char data_buff[FLASH_STORAGE_MAX_BUFFER];
    uint8_t hex_buff[FLASH_STORAGE_MAX_BUFFER];

    char seg_temp_buff[1024];
    uint32_t seg_temp_size = 0;

    if(g_extra_bytes_count > 0){
        memcpy(seg_temp_buff, g_extra_bytes_buff, g_extra_bytes_count);
        seg_temp_size+=g_extra_bytes_count;
        g_extra_bytes_count = 0;
    }

    while (seg_temp_size < 1024) {
        uint32_t line_len = flash_get_line(data_buff);
//       // printf("get a line with len %d\n", line_len);

        if(line_len <= 0 || line_len >= FLASH_STORAGE_MAX_BUFFER)
            break;

        if(!strcmp(&data_buff[line_len-2], "\r\n")){
            line_len -= 2;
        }else if(data_buff[line_len-1] == '\n'){
            line_len--;
        }

        int hex_length = string_to_hex(data_buff, line_len, hex_buff);
        intel_hex *p_record = intel_hex_process(hex_buff, hex_length);

        if(p_record == NULL){
           // printf("recode is NOT hex file format\n");
            return -1;
        }

        if(p_record->record_type == INTEL_HEX_Data){
            memcpy(seg_temp_buff + seg_temp_size, p_record->data, p_record->byte_count);
            seg_temp_size+= p_record->byte_count;
//           // printf("read %d byte data, seg len now %d\n", p_record->byte_count, seg_temp_size);
        }else if(p_record->record_type == INTEL_HEX_EndFile){
           // printf("Finished read hex file\n");
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

   //LOG_INF(TAG, "Get a segment id %d from flash", _seg->m_index);

    if(g_finish_file && !g_extra_bytes_count){
        sm_seg_set_last_segment(_seg, 1);
       // printf("Last segment is load from storage, size of last segment: %d\n", seg_temp_size);
    }else{
        sm_seg_set_last_segment(_seg, 0);
    }

    return 1;
}


static sm_boot_input_if_t g_boot_input_proc = {
        .init = sm_input_flash_init,
        .free = sm_input_flash_free,
        .get_total_seg = sm_input_flash_get_total_seg,
        .get_fw_info = sm_input_flash_get_fw_info,
        .get_seg_fw = sm_input_flash_get_seg_fw,
        .process = NULL
};

sm_boot_input_if_t* sm_get_flash_boot_input(sm_ex_flash_storage_partition_t* _flash_in){
    if(!_flash_in){
        return NULL;
    }

    g_flash_in = _flash_in;
    g_new_fw_signature.m_size = 0;
    g_new_fw_signature.m_addr = 0;
    g_new_fw_signature.m_crc = 0;
    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_data_address_offset = -1;
    g_get_line_current_addr = 0;
  
    return &g_boot_input_proc;
}
