//
// Created by vuonglk on 18/09/2024.
//

#include <string.h>
#include "sm_ex_flash_storage_if.h"
#include "stdlib.h"

#define PAGE_SIZE   (4*1024)
#define PAGE_COUNT  (512)
#define EMPTY_VALUE 0x00

typedef struct{
    sm_ex_flash_proc_t m_proc;
    uint32_t m_page_size;
    uint32_t m_page_count;
    uint8_t m_virtual_buff[PAGE_COUNT*PAGE_SIZE];
}sm_ex_flash_storage_virtual_flash_impl_t;

#define _impl(x) (sm_ex_flash_storage_virtual_flash_impl_t*)(x)

int32_t sm_ex_flash_storage_file_open(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return 0;
}

int32_t sm_ex_flash_storage_file_write_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, const uint8_t* _data, uint32_t _len) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    if((_addr + _len) > (this->m_page_size*this->m_page_count)){
        return -1;
    }
    memcpy(this->m_virtual_buff + _addr, _data, _len);
    return 0;
}

int32_t sm_ex_flash_storage_file_read_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, uint8_t* _buf, uint32_t _max_len) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    if((_addr + _max_len) > (this->m_page_size*this->m_page_count)){
        return -1;
    }
    memcpy(_buf, this->m_virtual_buff + _addr, _max_len);
    return 0;
}

int32_t sm_ex_flash_storage_file_erase_pages(sm_ex_flash_proc_t* _this, uint32_t _page_id) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    memset(this->m_virtual_buff + _page_id*this->m_page_size, EMPTY_VALUE, this->m_page_size);
    return 0;
}

int32_t sm_ex_flash_storage_file_erase_full(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    memset(this->m_virtual_buff, EMPTY_VALUE, this->m_page_size * this->m_page_count);
    return 0;
}

int32_t sm_ex_flash_storage_file_get_page_count(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return (int32_t)this->m_page_count;
}

int32_t sm_ex_flash_storage_file_get_page_size(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return (int32_t)this->m_page_size;
}

int32_t sm_ex_flash_storage_file_close(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_virtual_flash_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return 0;
}


sm_ex_flash_storage_virtual_flash_impl_t g_file_if = {
        .m_proc.open = sm_ex_flash_storage_file_open,
        .m_proc.close = sm_ex_flash_storage_file_close,
        .m_proc.erase_full = sm_ex_flash_storage_file_erase_full,
        .m_proc.erase_pages = sm_ex_flash_storage_file_erase_pages,
        .m_proc.get_page_count = sm_ex_flash_storage_file_get_page_count,
        .m_proc.get_page_size = sm_ex_flash_storage_file_get_page_size,
        .m_proc.write_bytes = sm_ex_flash_storage_file_write_bytes,
        .m_proc.read_bytes = sm_ex_flash_storage_file_read_bytes
};

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_virtual_flash_proc(){
    g_file_if.m_page_size = PAGE_SIZE;
    g_file_if.m_page_count = PAGE_COUNT;
    return &g_file_if.m_proc;
}
