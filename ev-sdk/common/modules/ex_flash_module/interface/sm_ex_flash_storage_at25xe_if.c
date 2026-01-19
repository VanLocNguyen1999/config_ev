//
// Created by vnbk on 14/03/2025.
//
#include "sm_ex_flash_storage_if.h"

typedef struct{
    sm_ex_flash_proc_t m_proc;
    at25xe_t* m_at25xe;
}sm_ex_flash_storage_at25xe_impl_t;

#define _impl(x) (sm_ex_flash_storage_at25xe_impl_t*)(x)

int32_t sm_ex_flash_storage_at25xe_open(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return at25xe_Init(this->m_at25xe) ? 0 : -1;
}

int32_t sm_ex_flash_storage_at25xe_write_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, const uint8_t* _data, uint32_t _len) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    if((_addr + _len) > (this->m_at25xe->SectorCount * this->m_at25xe->SectorSize)){
        return -1;
    }
    at25xe_write(this->m_at25xe, _addr, _data, _len);
    return 0;
}

int32_t sm_ex_flash_storage_at25xe_read_bytes(sm_ex_flash_proc_t* _this, uint32_t _addr, uint8_t* _buf, uint32_t _max_len) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if((_addr + _max_len) > (this->m_at25xe->SectorCount * this->m_at25xe->SectorSize)){
        return -1;
    }
    at25xe_read(this->m_at25xe, _addr, _buf, _max_len);
    return 0;
}

int32_t sm_ex_flash_storage_at25xe_erase_pages(sm_ex_flash_proc_t* _this, uint32_t _page_id) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    at25xe_EraseSector(this->m_at25xe, _page_id);
    return 0;
}

int32_t sm_ex_flash_storage_at25xe_erase_full(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    at25xe_EraseChip(this->m_at25xe);
    return 0;
}

int32_t sm_ex_flash_storage_at25xe_get_page_count(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return (int32_t)this->m_at25xe->SectorCount;
}

int32_t sm_ex_flash_storage_at25xe_get_page_size(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    return (int32_t)this->m_at25xe->SectorSize;
}

int32_t sm_ex_flash_storage_at25xe_close(sm_ex_flash_proc_t* _this) {
    sm_ex_flash_storage_at25xe_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }
    return 0;
}

sm_ex_flash_storage_at25xe_impl_t g_ex_flash_at25xe_if = {
        .m_proc.open = sm_ex_flash_storage_at25xe_open,
        .m_proc.close = sm_ex_flash_storage_at25xe_close,
        .m_proc.erase_full = sm_ex_flash_storage_at25xe_erase_full,
        .m_proc.erase_pages = sm_ex_flash_storage_at25xe_erase_pages,
        .m_proc.get_page_count = sm_ex_flash_storage_at25xe_get_page_count,
        .m_proc.get_page_size = sm_ex_flash_storage_at25xe_get_page_size,
        .m_proc.write_bytes = sm_ex_flash_storage_at25xe_write_bytes,
        .m_proc.read_bytes = sm_ex_flash_storage_at25xe_read_bytes
};

sm_ex_flash_proc_t* sm_sv_ex_flash_storage_get_at25xe_proc(at25xe_t* _at25xe){
    g_ex_flash_at25xe_if.m_at25xe = _at25xe;
    return &g_ex_flash_at25xe_if.m_proc;
}