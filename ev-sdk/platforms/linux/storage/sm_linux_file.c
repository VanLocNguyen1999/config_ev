//
// Created by vnbk on 25/09/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sm_linux_file.h"

#define _impl(x)    ((sm_linux_file_impl_t*)(x))

typedef struct {
    FILE* m_file;
    const char* _name;
}sm_linux_file_impl_t;

sm_linux_file_t* sm_linux_file_create(const char* _name){
    sm_linux_file_impl_t* file = malloc(sizeof(sm_linux_file_impl_t));
    if(!file){
        printf("Could NOT create FILE\n");
        return NULL;
    }
    file->_name = _name;
    FILE* hex_file = fopen(_name, "w");
    if(!hex_file){
        printf("Cannot open hex file\n");
        free(file);
        return NULL;
    }
    fclose(hex_file);
    return file;
}
int32_t sm_linux_file_destroy(sm_linux_file_t* _this){
    free(_this);
    return 0;
}
int32_t sm_linux_file_erase(sm_linux_file_t* _this){
    FILE* file = fopen(_impl(_this)->_name, "w");
    if(!file){
        printf("Cannot open hex file\n");
        return -1;
    }
    fclose(file);
    return 0;
}
int32_t sm_linux_file_write(sm_linux_file_t* _this, const char* _content, int32_t _len){
    FILE* file = fopen(_impl(_this)->_name, "w");
    if(!file){
        printf("Cannot open hex file\n");
        return -1;
    }
    fwrite(_content, 1, _len, file);
    fclose(file);
    return 0;
}
int32_t sm_linux_file_read(sm_linux_file_t* _this, char* _content){
    FILE* file = fopen(_impl(_this)->_name, "r");
    if(!file){
        printf("Cannot open hex file\n");
        return -1;
    }
    fseek(file, 0L, SEEK_END);
    int file_size = (int)ftell(file);
    fseek(file, 0L, SEEK_SET);

    fread(_content, 1, file_size, file);
    fclose(file);
    return 0;
}