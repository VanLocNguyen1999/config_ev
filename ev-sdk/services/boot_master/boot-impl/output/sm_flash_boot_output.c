//
// Created by vuonglk on 18/09/2024.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sm_boot_impl.h"
#include "sm_crc.h"

#include "sm_logger.h"

#define _imp(p) ((sm_boot_output_impl_t*)(p))

#define TAG "sm_flash_boot_output"

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct{
    sm_boot_output_if_t m_base;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    sm_fw_signature_t m_fw_signature;
    int32_t m_current_seg_index;
    sm_ex_flash_storage_partition_t* m_flash_out;

    fw_signature_storage_fn_t m_fw_sig_storage_fn;
    void* m_fw_sig_storage_arg;
}sm_boot_output_impl_t;


static int32_t sm_output_flash_init(sm_boot_output_if_t* _self){
    sm_boot_output_impl_t* this = _imp(_self);
    sm_ex_flash_storage_erase_full_partition(this->m_flash_out);
    return 0;
}

static int32_t sm_output_flash_free(sm_boot_output_if_t* _self){
    sm_boot_output_impl_t* this = _imp(_self);

    this->m_current_seg_index = -1;
    this->m_flash_out = NULL;
    this->m_fw_signature.m_size = 0;
    this->m_fw_signature.m_crc = CRC_16_INIT_VALUE;
    this->m_fw_sig_storage_fn = NULL;
    this->m_fw_sig_storage_arg = NULL;

    return 0;
}

static int32_t sm_output_flash_reg_event_callback(sm_boot_output_if_t* _self, SM_BOOT_SLAVE_EVENT _event, sm_boot_slave_event_callback_fn_t _fn, void* _arg){
    if(!_fn){
        return -1;
    }
    _imp(_self)->m_events[_event].m_callback = _fn;
    _imp(_self)->m_events[_event].m_arg = _arg;
    return 0;
}

static int32_t sm_output_flash_request_upgrade(sm_boot_output_if_t* _self, int32_t _src_id){
    sm_boot_output_impl_t* this = _imp(_self);

    (void)_src_id;
    uint8_t ready = 1;
    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&ready, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
    return 0;
}

static int32_t sm_output_flash_set_fw_info(sm_boot_output_if_t* _self, const sm_fw_signature_t* _fw_signature){
    sm_boot_output_impl_t* this = _imp(_self);

    uint8_t success = 0;

    if(_fw_signature->m_crc == this->m_fw_signature.m_crc
        && _fw_signature->m_size == this->m_fw_signature.m_size){
        success = 1;

        if(this->m_fw_sig_storage_fn){
            if(this->m_fw_sig_storage_fn(_fw_signature, this->m_fw_sig_storage_arg) < 0){
                success = 0;
            }
        }
       //LOG_INF(TAG, "Confirm fw info success, storage %s", success?"Success":"Failed");
    }else{
        LOG_ERR(TAG, "Confirm fw info failed");
    }

    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
    return 0;
}

uint8_t test = 0;

static int32_t sm_output_flash_set_seg_fw(sm_boot_output_if_t* _self, const sm_segment_t* _seg){
    sm_boot_output_impl_t* this = _imp(_self);

    uint8_t success = 1;

    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
   //LOG_INF(TAG, "Load %d byte of seg index %d to addr 0x%x, fw addr: 0x%x", _seg->m_size, _seg->m_index, (_seg->m_index - 1)*SM_SEGMENT_FW_SIZE, _seg->m_addr);

    if(this->m_current_seg_index < _seg->m_index){
        sm_ex_flash_storage_partition_write(this->m_flash_out ,((_seg->m_index - 1)*SM_SEGMENT_FW_SIZE),
                    (uint8_t*)_seg->m_data,
                    _seg->m_size);

        this->m_current_seg_index = _seg->m_index;
        this->m_fw_signature.m_crc = sm_CRC_CalculateCRC16_init_value(this->m_fw_signature.m_crc, _seg->m_data, _seg->m_size);
        this->m_fw_signature.m_size += _seg->m_size;
    }

    return (int32_t)_seg->m_length;
}

static const sm_boot_output_if_proc_t g_boot_output_proc_default = {
        .init = sm_output_flash_init,
        .free = sm_output_flash_free,
        .reg_event_callback = sm_output_flash_reg_event_callback,
        .request_upgrade = sm_output_flash_request_upgrade,
        .set_fw_info = sm_output_flash_set_fw_info,
        .set_seg_fw = sm_output_flash_set_seg_fw,
        .process = NULL
};

static sm_boot_output_impl_t g_boot_output_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
};


sm_boot_output_if_t* sm_get_flash_boot_output(sm_ex_flash_storage_partition_t* _flash_out,
                                              fw_signature_storage_fn_t _fn,
                                              void* _arg){
    if(!_flash_out){
        LOG_ERR(TAG, "Flash output invalid");
        return NULL;
    }
   //LOG_INF(TAG, "Open flash output with partition at address 0x%x", sm_ex_flash_storage_get_partition_start_addr(_flash_out));
    sm_ex_flash_storage_erase_full_partition(_flash_out);
    g_boot_output_default.m_current_seg_index = -1;
    g_boot_output_default.m_flash_out = _flash_out;
    g_boot_output_default.m_fw_signature.m_size = 0;
    g_boot_output_default.m_fw_signature.m_crc = CRC_16_INIT_VALUE;
    g_boot_output_default.m_fw_sig_storage_fn = _fn;
    g_boot_output_default.m_fw_sig_storage_arg = _arg;
    return &g_boot_output_default.m_base;
}
