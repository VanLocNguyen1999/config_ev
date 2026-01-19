#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>

#include "sm_boot_impl.h"

#define HEX_PATH "./bp_mainapp.hex"

static char* g_file_buffer = NULL;

int main() {
    printf("Hello, World!\n");
    sm_file_input_if_t* bootInputIf;

#if 1
    FILE* hex_file = fopen(HEX_PATH, "rb");
    if(!hex_file){
        printf("Cannot open hex file\n");
        return -1;
    }

    fseek(hex_file, 0L, SEEK_END);
    int file_size = (int)ftell(hex_file);
    fseek(hex_file, 0L, SEEK_SET);

    g_file_buffer = malloc(file_size * sizeof(char) * 2);
    memset(g_file_buffer, 0xFF, file_size * sizeof(char) * 2);

    fread(g_file_buffer, sizeof(char), file_size, hex_file);

    bootInputIf = sm_get_flash_boot_input((uint64_t)g_file_buffer);

#else
    bootInputIf = sm_get_file_boot_input_new(HEX_PATH);

#endif

    bootInputIf->init();

    sm_segment_t seg_data;
    sm_fw_signature_t fw_info;
    seg_data.m_index = 0;

    for(seg_data.m_index = 0; true ; seg_data.m_index++){
        bootInputIf->get_seg_fw(&seg_data);
        printf("Seg %d: address 0x%x, len %d, crc %d, is last %s \n",seg_data.m_index, seg_data.m_addr,
               seg_data.m_length, seg_data.m_crc, seg_data.m_is_last?"yes":"no");
        if(seg_data.m_is_last) {
            break;
        }
    }

    bootInputIf->get_fw_info(&fw_info);
    printf("Fw info: size %d, crc %d\n\n\n", fw_info.m_size, fw_info.m_crc);

    return 0;
}
