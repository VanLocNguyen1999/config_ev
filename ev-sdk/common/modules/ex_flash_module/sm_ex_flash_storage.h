//
// Created by vnbk on 16/09/2024.
//

#ifndef EV_SDK_SM_EX_FLASH_STORAGE_H
#define EV_SDK_SM_EX_FLASH_STORAGE_H


#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"

typedef struct sm_ex_flash_storage sm_ex_flash_storage_t;
typedef void sm_ex_flash_storage_partition_t;
typedef struct sm_ex_flash_proc sm_ex_flash_proc_t;

struct sm_ex_flash_proc{
    int32_t (*open)(sm_ex_flash_proc_t*);
    int32_t (*write_bytes)(sm_ex_flash_proc_t*, uint32_t addr, const uint8_t* data, uint32_t len);
    int32_t (*read_bytes)(sm_ex_flash_proc_t*, uint32_t addr, uint8_t* buf, uint32_t max_len);
    int32_t (*erase_pages)(sm_ex_flash_proc_t*, uint32_t page_id);
    int32_t (*erase_full)(sm_ex_flash_proc_t*);
    int32_t (*get_page_count)(sm_ex_flash_proc_t*);
    int32_t (*get_page_size)(sm_ex_flash_proc_t*);
    int32_t (*close)(sm_ex_flash_proc_t*);
};

struct sm_ex_flash_storage{
    sm_ex_flash_proc_t* m_proc;
};

sm_ex_flash_storage_t* sm_flash_storage_create_default(sm_ex_flash_proc_t* _proc);

int32_t sm_ex_flash_storage_init(sm_ex_flash_storage_t* _this);

int32_t sm_ex_flash_storage_free(sm_ex_flash_storage_t* _this);

int32_t sm_ex_flash_erase_full(sm_ex_flash_storage_t* _this);

int32_t sm_ex_flash_storage_get_total_size(sm_ex_flash_storage_t* _this);

int32_t sm_ex_flash_storage_read(sm_ex_flash_storage_t* _this, uint32_t _addr, uint8_t* _buf, uint32_t _max_len);

int32_t sm_ex_flash_storage_write(sm_ex_flash_storage_t* _this, uint32_t _addr, const uint8_t* _data, uint32_t _len);

int32_t sm_ex_flash_storage_erase(sm_ex_flash_storage_t* _this, uint32_t _addr, uint32_t _len);

/****
 *
 * @param _this
 * @param _start_addr
 * @param _size
 * @return
 */
sm_ex_flash_storage_partition_t* sm_ex_flash_storage_create_partition(sm_ex_flash_storage_t* _this, uint32_t _start_addr, uint32_t _size);

sm_ex_flash_storage_t* sm_ex_flash_storage_get_partition_parent(sm_ex_flash_storage_partition_t* _partition);

int32_t sm_ex_flash_storage_free_partition(sm_ex_flash_storage_partition_t* _partition);

uint32_t sm_ex_flash_storage_get_partition_start_addr(sm_ex_flash_storage_partition_t* _partition);

uint32_t sm_ex_flash_storage_get_partition_total_size(sm_ex_flash_storage_partition_t* _partition);

int32_t sm_ex_flash_storage_erase_full_partition(sm_ex_flash_storage_partition_t* _partition);

int32_t sm_ex_flash_storage_erase_partition(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, uint32_t _size);

int32_t sm_ex_flash_storage_partition_write(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, const uint8_t* _data, uint32_t _size);

int32_t sm_ex_flash_storage_partition_read(sm_ex_flash_storage_partition_t* _partition, uint32_t _offset, uint8_t* _buf, int32_t _max_len);

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EX_FLASH_STORAGE_H
