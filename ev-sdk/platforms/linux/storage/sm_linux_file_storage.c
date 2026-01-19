//
// Created by vnbk on 18/09/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sm_ex_flash_storage.h"

#define _impl(x)    ((sm_file_storage_t*)(x))

typedef struct {
    FILE* m_file;
    int32_t m_length;
    int32_t m_offset;
}sm_file_storage_t;

static sm_file_storage_t g_file_storage = {
        .m_file = NULL
};

typedef sm_file_storage_t sm_file_partition_t;

sm_ex_flash_storage_t* sm_ex_flash_storage_create(void* _driver){
    (void)_driver;
    return &g_file_storage;
}

int32_t sm_ex_flash_storage_init(){

}

int32_t sm_ex_flash_erase_full(sm_ex_flash_storage_t* _this){

}

int32_t sm_ex_flash_storage_free(sm_ex_flash_storage_t* _this){

}

int32_t sm_ex_flash_storage_read(sm_ex_flash_storage_t* _this, uint32_t _addr, uint8_t* _buf, int32_t _max_len){

}

int32_t sm_ex_flash_storage_write(sm_ex_flash_storage_t* _this, uint32_t _addr, const uint8_t* _data, int32_t _len){

}

int32_t sm_ex_flash_storage_erase(sm_ex_flash_storage_t* _this, uint32_t _addr, int32_t _len){

}
/****
 *
 * @param _this
 * @param _start_addr
 * @param _size
 * @return
 */
static const char* g_file_name = "fw.hex";
static const char* g_buffer = NULL;
sm_ex_flash_storage_partition_t* sm_ex_flash_storage_create_partition(sm_ex_flash_storage_t* _this, uint32_t _start_addr, uint32_t _size){
    if(!_this){
        return NULL;
    }
    return _this;
}

int32_t sm_ex_flash_storage_free_partition(sm_ex_flash_storage_t* _this, sm_ex_flash_storage_partition_t* _partition){

}

uint64_t sm_ex_flash_storage_get_start_addr(sm_ex_flash_storage_partition_t* _this){
    FILE* hex_file = fopen(g_file_name, "rb");
    if(!hex_file){
        printf("Cannot open hex file\n");
        return -1;
    }

    fseek(hex_file, 0L, SEEK_END);
    int file_size = (int)ftell(hex_file);
    fseek(hex_file, 0L, SEEK_SET);

    g_buffer = malloc(file_size * sizeof(char) * 2);
    memset(g_buffer, 0xFF, file_size * sizeof(char) * 2);

    fread(g_buffer, sizeof(char), file_size, hex_file);

    return (uint64_t)g_buffer;
}

int32_t sm_ex_flash_storage_erase_full_partition(sm_ex_flash_storage_partition_t* _this){
    FILE* file = fopen(g_file_name, "w");
    if(!file){
        return -1;
    }

    fclose(file);
    return 0;
}

int32_t sm_ex_flash_storage_erase_partition(sm_ex_flash_storage_partition_t* _this, uint32_t _offset){

}


int32_t sm_ex_flash_storage_partition_write(sm_ex_flash_storage_partition_t* _this, uint32_t _offset, const uint8_t* _data, uint32_t _size){
    if(!_this){
        return -1;
    }
    FILE* file = fopen(g_file_name, "a");
    if(!file){
        return -1;
    }
    fwrite(_data, 1, _size, file);

    fclose(file);

    return 0;
}

int32_t sm_ex_flash_storage_partition_read(sm_ex_flash_storage_t* _this, uint32_t _offset, uint8_t* _buf, int32_t _len){
    if(!_this){
        return -1;
    }
    FILE* file = fopen(g_file_name, "r");
    if(!file){
        return -1;
    }
    fseek(file, _impl(_this)->m_offset, _impl(_this)->m_offset + _len);
    fread(_buf, 1, _len, file);

    fclose(file);

    return 0;
}