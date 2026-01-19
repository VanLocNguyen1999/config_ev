//
// Created by vuonglk on 05/09/2024.
//

#include "sm_sv_file_receiver.h"
#include "sm_elapsed_timer.h"
#include "sm_co_od_common.h"
#include "sm_sv_file_transmit_common.h"

#include "sm_sv_file_receiver_output_if.h"
#include "sm_crc.h"

#include "sm_logger.h"

#define TAG "sv_file_receiver"

typedef enum {
    RECEIVER_STATE_IDLE,
    RECEIVER_STATE_LOADING_FILE,
    RECEIVER_STATE_SUCCESS,
    RECEIVER_STATE_FAILED
}SV_FILE_RECEIVER_PROCESS_STATE;

#define LOADING_FILE_TIMEOUT 15000

typedef struct{

    sm_memory_info_t m_memory_info;
    sm_file_info_t m_file_info;
    sm_file_data_frame_t m_frame_storage;

    SV_FILE_RECEIVER_PROCESS_STATE m_process_stt;

    sm_co_t* m_co_core;
    file_receiver_event m_cb;
    elapsed_timer_t m_timeout;

    sm_file_output_if_t* m_output_if;

    void* m_cb_arg;
}sm_sv_file_receiver_impl_t;

sm_sv_file_receiver_impl_t g_file_receiver;

#define _impl(x) (sm_sv_file_receiver_impl_t*)(x)

static int32_t set_co_od_sub_obj_data(CO_Sub_Object* _obj, void* _p_data, CO_Sub_Object_Ext_Confirm_Func_t(*confirm_func)(void)){
    if(!_obj)
        return -1;

    _obj->p_data = _p_data;
    _obj->p_ext->p_shadow_data = _p_data;
    _obj->p_ext->confirm_func = confirm_func;
    return 1;
}

static void sm_sv_file_receiver_emit_event(sm_sv_file_receiver_t* _this, SM_SV_FILE_RECEIVER_EVENTS _event){
    sm_sv_file_receiver_impl_t* this = _impl(_this);
    if(this && this->m_cb){
        this->m_cb(_event, this->m_cb_arg);
    }
}

/// Handle for receive memory infos

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_mem_ext_confirm_func(void) {
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load mem info ext is %d", this->m_memory_info.m_memory_ext);
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_mem_start_addr_confirm_func(void) {
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load mem info start addr is 0x%x", this->m_memory_info.m_start_addr);
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_mem_size_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    this->m_file_info.m_file_size = this->m_memory_info.m_mem_size;

    LOG_INF(TAG, "Load file info size is %d", this->m_memory_info.m_mem_size);
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_full_memory_info_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load mem info type is %d", this->m_memory_info.m_memory_type);

    if(this->m_process_stt != RECEIVER_STATE_IDLE){
        LOG_ERR(TAG, "Receiver sv is busy, abort");
        this->m_process_stt = RECEIVER_STATE_FAILED;
        return CO_EXT_CONFIRM_abort;
    }

    int ret = this->m_output_if->init(&this->m_memory_info);
    if(ret < 0){
        LOG_ERR(TAG, "Cannot open memory to storage, abort");
        this->m_process_stt = RECEIVER_STATE_FAILED;
        return CO_EXT_CONFIRM_abort;
    }
    sm_sv_file_receiver_emit_event(this, SM_SV_FILE_RECEIVER_START_RECV);

    this->m_process_stt = RECEIVER_STATE_LOADING_FILE;
    elapsed_timer_resetz(&g_file_receiver.m_timeout, LOADING_FILE_TIMEOUT);
    return CO_EXT_CONFIRM_success;
}

/// Handle for receive file data frames

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_frame_id_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load data frame id is %d", this->m_frame_storage.m_id);
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_frame_size_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load data frame size is %d", this->m_frame_storage.m_size);
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_frame_data_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load data frame data segment");
    return CO_EXT_CONFIRM_success;
}


static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_full_file_data_frame_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load data frame crc is %d \n\n", this->m_frame_storage.m_crc);

    sm_file_data_frame_t* loaded_frame = &this->m_frame_storage;

    uint16_t frame_crc = sm_CRC_CalculateCRC16(loaded_frame->m_data,
                                               loaded_frame->m_size);

    if(frame_crc != loaded_frame->m_crc){
        LOG_ERR(TAG, "Frame CRC invalid, abort");
        this->m_process_stt = RECEIVER_STATE_FAILED;
        return CO_EXT_CONFIRM_abort;
    }

    if(this->m_output_if->save_frame(loaded_frame) < 0){
        LOG_ERR(TAG, "Cannot save frame id %d, abort", loaded_frame->m_id);
        this->m_process_stt = RECEIVER_STATE_FAILED;
        return CO_EXT_CONFIRM_abort;
    }

    elapsed_timer_resetz(&g_file_receiver.m_timeout, LOADING_FILE_TIMEOUT);
    return CO_EXT_CONFIRM_success;
}


/// Handle for receive file info

static CO_Sub_Object_Ext_Confirm_Func_t sm_sv_file_receiver_full_file_info_confirm_func(void){
    sm_sv_file_receiver_impl_t* this = &g_file_receiver;

    LOG_INF(TAG, "Load file info crc is %d", this->m_file_info.m_file_crc);

    if(this->m_output_if->check_valid_file(&this->m_file_info) < 0){
        LOG_ERR(TAG, "File info not valid, abort");
        this->m_process_stt = RECEIVER_STATE_FAILED;
        return CO_EXT_CONFIRM_abort;
    }
    this->m_process_stt = RECEIVER_STATE_SUCCESS;
    return CO_EXT_CONFIRM_success;
}


sm_sv_file_receiver_t* sm_sv_file_receiver_create_default(sm_co_t* _co_core, sm_file_output_if_t* _output_if,
                                                          file_receiver_event _cb, void* _arg){
    struct CO_Object_t* file_info_obj = NULL;
    struct CO_Object_t* file_data_obj = NULL;

    for(int index = 0; index < p_co_od->number; index++){
        if(p_co_od->list[index].index == SDO_FILE_TRANSFER_INFO_INDEX){
            file_info_obj = &p_co_od->list[index];
        }
        if(p_co_od->list[index].index == SDO_FILE_TRANSFER_DATA_INDEX){
            file_data_obj = &p_co_od->list[index];
        }
    }

    if(!file_data_obj || !file_info_obj){
        LOG_ERR(TAG, "Cannot find od object !!!");
        return NULL;
    }


    // set canopen callback for recv memory info
    set_co_od_sub_obj_data(&file_info_obj->subs[SDO_FILE_TRANSFER_INFO_MEMORY_EXT_SUB_INDEX],
                           &g_file_receiver.m_memory_info.m_memory_ext,
                           sm_sv_file_receiver_mem_ext_confirm_func);

    set_co_od_sub_obj_data(&file_info_obj->subs[SDO_FILE_TRANSFER_INFO_START_ADDR_SUB_INDEX],
                           &g_file_receiver.m_memory_info.m_start_addr,
                           sm_sv_file_receiver_mem_start_addr_confirm_func);

    set_co_od_sub_obj_data(&file_info_obj->subs[SDO_FILE_TRANSFER_INFO_FILE_SIZE_SUB_INDEX],
                           &g_file_receiver.m_memory_info.m_mem_size,
                           sm_sv_file_receiver_mem_size_confirm_func);

    set_co_od_sub_obj_data(&file_info_obj->subs[SDO_FILE_TRANSFER_INFO_MEMORY_TYPE_SUB_INDEX],
                           &g_file_receiver.m_memory_info.m_memory_type,
                           sm_sv_file_receiver_full_memory_info_confirm_func);


    // set canopen callback for recv file info

    set_co_od_sub_obj_data(&file_info_obj->subs[SDO_FILE_TRANSFER_INFO_FILE_CRC_16_SUB_INDEX],
                           &g_file_receiver.m_file_info.m_file_crc,
                           sm_sv_file_receiver_full_file_info_confirm_func);


    // set canopen callback for recv file data frame
    set_co_od_sub_obj_data(&file_data_obj->subs[SDO_FILE_TRANSFER_DATA_FRAME_ID_SUB_INDEX],
                           &g_file_receiver.m_frame_storage.m_id,
                           sm_sv_file_receiver_frame_id_confirm_func);

    set_co_od_sub_obj_data(&file_data_obj->subs[SDO_FILE_TRANSFER_DATA_FRAME_SIZE_SUB_INDEX],
                           &g_file_receiver.m_frame_storage.m_size,
                           sm_sv_file_receiver_frame_size_confirm_func);

    set_co_od_sub_obj_data(&file_data_obj->subs[SDO_FILE_TRANSFER_DATA_FRAME_DATA_SUB_INDEX],
                           g_file_receiver.m_frame_storage.m_data,
                           sm_sv_file_receiver_frame_data_confirm_func);

    set_co_od_sub_obj_data(&file_data_obj->subs[SDO_FILE_TRANSFER_DATA_FRAME_CRC_16_SUB_INDEX],
                           &g_file_receiver.m_frame_storage.m_crc,
                           sm_sv_file_receiver_full_file_data_frame_confirm_func);

    g_file_receiver.m_co_core = _co_core;
    g_file_receiver.m_cb = _cb;
    g_file_receiver.m_cb_arg = _arg;
    g_file_receiver.m_output_if = _output_if;

    elapsed_timer_resetz(&g_file_receiver.m_timeout, LOADING_FILE_TIMEOUT);

    return &g_file_receiver;
}

int32_t sm_sv_file_receiver_process(sm_sv_file_receiver_t* _this){
    sm_sv_file_receiver_impl_t* this = _impl(_this);
    if(!this){
        return -1;
    }

    switch (this->m_process_stt) {
        case RECEIVER_STATE_IDLE:
            break;
        case RECEIVER_STATE_LOADING_FILE:
            if(!elapsed_timer_get_remain(&this->m_timeout)){
                this->m_process_stt = RECEIVER_STATE_FAILED;
                LOG_ERR(TAG, "Timeout when loading file!");
            }
            break;
        case RECEIVER_STATE_SUCCESS:
            LOG_INF(TAG, "Receiver file SUCCESS!!!");
            sm_sv_file_receiver_emit_event(this, SM_SV_FILE_RECEIVER_RECV_SUCCESS);
            this->m_process_stt = RECEIVER_STATE_IDLE;
            break;
        case RECEIVER_STATE_FAILED:
            LOG_ERR(TAG, "Receiver file FAILED!!!");
            sm_sv_file_receiver_emit_event(this, SM_SV_FILE_RECEIVER_RECV_FAILED);
            this->m_process_stt = RECEIVER_STATE_IDLE;
            break;
    }

    return 0;
}
