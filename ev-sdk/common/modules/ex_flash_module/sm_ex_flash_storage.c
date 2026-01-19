//
// Created by vuonglk on 18/09/2024.
//

#include <stdlib.h>
#include "sm_ex_flash_storage.h"
#include "sm_logger.h"

#define TAG "ex_flash"

typedef struct{
    sm_ex_flash_storage_t m_base;
    uint32_t m_page_size;
    uint32_t m_page_count;
    uint32_t m_total_size;
}sm_ex_flash_storage_impl_t;

sm_ex_flash_storage_impl_t g_ex_flash_storage;

typedef struct{
    sm_ex_flash_storage_t* m_parent;
    uint32_t m_address;
    uint32_t m_size;
}sm_ex_flash_storage_partition_impl_t;

#define _impl_parent(x) (sm_ex_flash_storage_impl_t*)(x)

#define _impl_child(x) (sm_ex_flash_storage_partition_impl_t*)(x)


sm_ex_flash_storage_t* sm_flash_storage_create_default(sm_ex_flash_proc_t* _proc){
    g_ex_flash_storage.m_base.m_proc = _proc;
    return &g_ex_flash_storage.m_base;
}

int32_t sm_ex_flash_storage_init(sm_ex_flash_storage_t* _this){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }

    if(_this->m_proc->open(_this->m_proc) < 0){
        LOG_ERR(TAG, "Open ext flash FAILED");
        return -1;
    }
    parent->m_page_size = _this->m_proc->get_page_size(_this->m_proc);
    parent->m_page_count = _this->m_proc->get_page_count(_this->m_proc);
    parent->m_total_size = parent->m_page_size * parent->m_page_count;
    return 0;
}

int32_t sm_ex_flash_storage_free(sm_ex_flash_storage_t* _this){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }
    _this->m_proc->close(_this->m_proc);
    if(parent != &g_ex_flash_storage){
        free(parent);
        parent = NULL;
    }
    return 0;
}

int32_t sm_ex_flash_erase_full(sm_ex_flash_storage_t* _this){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }
    return _this->m_proc->erase_full(_this->m_proc);
}

int32_t sm_ex_flash_storage_get_total_size(sm_ex_flash_storage_t* _this){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }
    return (int32_t)parent->m_total_size;
}

int32_t sm_ex_flash_storage_read(sm_ex_flash_storage_t* _this, uint32_t _addr, uint8_t* _buf, uint32_t _max_len){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }
    return _this->m_proc->read_bytes(_this->m_proc, _addr, _buf, _max_len);
}

int32_t sm_ex_flash_storage_write(sm_ex_flash_storage_t* _this, uint32_t _addr, const uint8_t* _data, uint32_t _len){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }
    return _this->m_proc->write_bytes(_this->m_proc, _addr, _data, _len);
}

int32_t sm_ex_flash_storage_erase(sm_ex_flash_storage_t* _this, uint32_t _addr, uint32_t _len){
    sm_ex_flash_storage_impl_t* parent = _impl_parent(_this);
    if(!parent){
        return -1;
    }

    uint32_t stop_addr = _addr + _len;

    if(stop_addr > parent->m_total_size){
        return -1;
    }

    for(int id = 0; id < parent->m_page_count; id++){
        uint32_t current_addr = id * parent->m_page_size;
        if(current_addr >= _addr && current_addr < stop_addr){
//            LOG_INF(TAG, "Erase ext flash block %d", id);
            _this->m_proc->erase_pages(_this->m_proc, id);
        }
    }
    return 0;
}


sm_ex_flash_storage_partition_t* sm_ex_flash_storage_create_partition(sm_ex_flash_storage_t* _parent, uint32_t _start_addr, uint32_t _size){
    sm_ex_flash_storage_partition_impl_t* child = malloc(sizeof(sm_ex_flash_storage_partition_impl_t));
    if(!child){
        return NULL;
    }

    child->m_parent = _parent;
    child->m_address = _start_addr;
    child->m_size = _size;

    return child;
}

sm_ex_flash_storage_t* sm_ex_flash_storage_get_partition_parent(sm_ex_flash_storage_partition_t* _partition){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return NULL;
    }
    return child->m_parent;
}


int32_t sm_ex_flash_storage_free_partition(sm_ex_flash_storage_partition_t* _partition){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return -1;
    }
    free(child);
    _partition = NULL;
    return 0;
}

uint32_t sm_ex_flash_storage_get_partition_start_addr(sm_ex_flash_storage_partition_t* _partition){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return 0;
    }
    return child->m_address;
}

uint32_t sm_ex_flash_storage_get_partition_total_size(sm_ex_flash_storage_partition_t* _partition){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return 0;
    }
    return child->m_size;
}

int32_t sm_ex_flash_storage_erase_full_partition(sm_ex_flash_storage_partition_t* _partition){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return -1;
    }

    return sm_ex_flash_storage_erase(child->m_parent, child->m_address, child->m_size);
}


int32_t sm_ex_flash_storage_erase_partition(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, uint32_t _size){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return -1;
    }
    if((_offset + _size) > child->m_size){
        return -1;
    }

    return sm_ex_flash_storage_erase(child->m_parent, child->m_address + _offset, _size);
}

int32_t sm_ex_flash_storage_partition_write(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, const uint8_t* _data, uint32_t _size){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return -1;
    }
    if((_offset + _size) > child->m_size){
        return -1;
    }

    return sm_ex_flash_storage_write(child->m_parent, child->m_address + _offset, _data, _size);
}

int32_t sm_ex_flash_storage_partition_read(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, uint8_t* _buf, int32_t _max_len){
    sm_ex_flash_storage_partition_impl_t* child = _impl_child(_partition);
    if(!child){
        return -1;
    }
    if((_offset + _max_len) > child->m_size){
        return -1;
    }

    return sm_ex_flash_storage_read(child->m_parent, child->m_address + _offset, _buf, _max_len);
}
