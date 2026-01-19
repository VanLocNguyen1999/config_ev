// Created by vuonglk on 18/09/2024.
//

#include "sm_ex_flash_storage_if.h"
#include "stdlib.h"

#include "sm_w25qxx.h"


typedef struct{
    sm_ex_flash_proc_t m_proc;
    w25qxx_t* m_w25q;
}sm_ex_flash_storage_w25q_impl_t;

#define _impl(x) (sm_ex_flash_storage_w25q_impl_t*)(x)


int32_t sm_ex_flash_storage_w25q_open(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return w25qxx_Init(this->m_w25q) ? 0 : -1;
}

int32_t sm_ex_flash_storage_w25q_write_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, const uint8_t* _data, uint32_t _len) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    if((_addr + _len) > (this->m_w25q->SectorCount * this->m_w25q->SectorSize)){
        return -1;
    }
    w25qxx_write(this->m_w25q, _addr, _data, _len);
    return 0;
}

int32_t sm_ex_flash_storage_w25q_read_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, uint8_t* _buf, uint32_t _max_len) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if((_addr + _max_len) > (this->m_w25q->SectorCount * this->m_w25q->SectorSize)){
        return -1;
    }
    w25qxx_read(this->m_w25q, _addr, _buf, _max_len);
    return 0;
}

int32_t sm_ex_flash_storage_w25q_erase_pages(sm_ex_flash_proc_t* _this, uint32_t _page_id) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    w25qxx_EraseSector(this->m_w25q, _page_id);
    return 0;
}

int32_t sm_ex_flash_storage_w25q_erase_full(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    w25qxx_EraseChip(this->m_w25q);
    return 0;
}

int32_t sm_ex_flash_storage_w25q_get_page_count(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return (int32_t)this->m_w25q->SectorCount;
}

int32_t sm_ex_flash_storage_w25q_get_page_size(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return (int32_t)this->m_w25q->SectorSize;
}

int32_t sm_ex_flash_storage_w25q_close(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_w25q_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return 0;
}


sm_ex_flash_storage_w25q_impl_t g_ex_flash_w25q_if = {
        .m_proc.open = sm_ex_flash_storage_w25q_open,
        .m_proc.close = sm_ex_flash_storage_w25q_close,
        .m_proc.erase_full = sm_ex_flash_storage_w25q_erase_full,
        .m_proc.erase_pages = sm_ex_flash_storage_w25q_erase_pages,
        .m_proc.get_page_count = sm_ex_flash_storage_w25q_get_page_count,
        .m_proc.get_page_size = sm_ex_flash_storage_w25q_get_page_size,
        .m_proc.write_bytes = sm_ex_flash_storage_w25q_write_bytes,
        .m_proc.read_bytes = sm_ex_flash_storage_w25q_read_bytes
};

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_w25q_proc(w25qxx_t* _w25q){
    g_ex_flash_w25q_if.m_w25q = _w25q;
    return &g_ex_flash_w25q_if.m_proc;
}
