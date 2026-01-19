//
// Created by vuonglk on 05/09/2024.
//

#include "sm_sv_file_transfer.h"
#include "sm_sv_file_transmit_common.h"
#include "sm_sv_file_transfer_input_if.h"
#include "sm_co_od_common.h"
#include "sm_logger.h"

#define TAG "sm_sv_file_transfer"

#define RETRY_ALLOW     5
#define MAX_FILE_QUEUE  4

typedef enum {
    TRANSFER_STATE_IDLE,
    TRANSFER_STATE_PREPARE,
    TRANSFER_STATE_SEND_MEMORY_INFO,
    TRANSFER_STATE_SEND_FILE_DATA_FRAME,
    TRANSFER_STATE_SEND_FILE_INFO,
    TRANSFER_STATE_SUCCESS,
    TRANSFER_STATE_FAILED
} SV_FILE_TRANSFER_PROCESS_STATE;

typedef enum {
    TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_EXT = 0,
    TRANSFER_MEMORY_INFO_STEP_SEND_START_ADDR,
    TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_SIZE,
    TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_TYPE,
    TRANSFER_MEMORY_INFO_STEP_NUMBER
} TRANSFER_MEMORY_INFO_STEPS;

typedef enum {
    TRANSFER_FILE_DATA_STEP_LOAD_FILE_FRAME = 0,
    TRANSFER_FILE_DATA_STEP_SEND_FRAME_ID,
    TRANSFER_FILE_DATA_STEP_SEND_FRAME_SIZE,
    TRANSFER_FILE_DATA_STEP_SEND_FRAME_DATA,
    TRANSFER_FILE_DATA_STEP_SEND_FRAME_CRC,
    TRANSFER_FILE_DATA_STEP_NUMBER
} TRANSFER_FILE_DATA_STEPS;

typedef enum {
    TRANSFER_FILE_INFO_LOAD_FILE_INFO = 0,
    TRANSFER_FILE_INFO_STEP_SEND_FILE_CRC,
    TRANSFER_FILE_INFO_STEP_NUMBER
} TRANSFER_FILE_INFO_STEPS;

typedef struct{
    int32_t m_dest_id;
    const char* m_path;
    uint32_t m_start_addr;
    uint8_t m_mem_type;
    uint8_t m_mem_ext;
}sm_file_transfer_info_t;

typedef struct {
    sm_file_info_t m_file_info;
    sm_file_data_frame_t m_current_frame;
    sm_memory_info_t m_memory_info;

    int32_t m_current_frame_id;

    sm_co_t *m_co_core;

    SV_FILE_TRANSFER_PROCESS_STATE m_process_stt;

    int32_t m_step;

    finish_transfer_cb m_cb;
    void *m_cb_arg;

    uint32_t m_retry;

    sm_file_transfer_info_t m_cur_info;
    sm_file_input_if_t* m_file_input;

    sm_file_transfer_info_t m_info_queue[MAX_FILE_QUEUE];
    uint8_t m_queue_head;
    uint8_t m_queue_tail;

    uint8_t m_isWaiting;
} sm_sv_file_transfer_impl_t;

#define _impl(x) (sm_sv_file_transfer_impl_t*)(x)

static int32_t sm_sv_file_trans_get_queue_size(sm_sv_file_transfer_t *_this){
    sm_sv_file_transfer_impl_t *this = _impl(_this);

    return (this->m_queue_head >= this->m_queue_tail) ? (this->m_queue_head - this->m_queue_tail) : MAX_FILE_QUEUE -  this->m_queue_tail + this->m_queue_head;
}

static int32_t sm_sv_file_transfer_reset_transfer(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);

    this->m_process_stt = TRANSFER_STATE_IDLE;
    this->m_step = -1;
    memset(&this->m_file_info, 0, sizeof(this->m_file_info));
    memset(&this->m_current_frame, 0, sizeof(this->m_current_frame));

    if (this->m_file_input) {
        this->m_file_input->free();
        this->m_file_input = NULL;
    }
    return 0;
}

static void sm_sv_file_transfer_change_process_stt(sm_sv_file_transfer_t *_this,
                                                   SV_FILE_TRANSFER_PROCESS_STATE _new_stt) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    this->m_process_stt = _new_stt;
    this->m_step = 0;
    this->m_retry = 0;
}

static void sm_sv_file_transfer_emit_callback(sm_sv_file_transfer_t *_this, uint8_t _success) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (this && this->m_cb) {
        this->m_cb(_success, this->m_cur_info.m_dest_id, this->m_cb_arg);
    }
}

/******************* Handle for step 1: Transfer memory info *************************/

static void
sm_sv_file_transfer_memory_info_co_cb(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void *_arg) {
    sm_sv_file_transfer_impl_t *this = _impl(_arg);
    this->m_isWaiting = false;
    if (_sdoStt == SM_SDO_ST_ABORT) {
        LOG_ERR(TAG, "SDO status: Abort, Tx_abort: 0x%2X, Rx_Abort: 0x%2X \n", _txCode, _rxCode);
        this->m_retry++;
        if (this->m_retry >= RETRY_ALLOW) {
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_FAILED);
        }
    } else if (_sdoStt == SM_SDO_ST_SUCCESS) {
        this->m_step++;
        if (this->m_step >= TRANSFER_MEMORY_INFO_STEP_NUMBER) {
            this->m_current_frame_id = 0;
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_SEND_FILE_DATA_FRAME);
        }
    }
}

static void sm_sv_file_transfer_send_memory_ext(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file memory ext is %d", this->m_memory_info.m_memory_ext);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_INFO_INDEX,
                          SDO_FILE_TRANSFER_INFO_MEMORY_EXT_SUB_INDEX, this->m_cur_info.m_dest_id,
                          &this->m_memory_info.m_memory_ext, 1, 500, sm_sv_file_transfer_memory_info_co_cb, this);
}

static void sm_sv_file_transfer_send_memory_size(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file info size is %d", this->m_memory_info.m_mem_size);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_INFO_INDEX,
                          SDO_FILE_TRANSFER_INFO_FILE_SIZE_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_memory_info.m_mem_size, 4, 500, sm_sv_file_transfer_memory_info_co_cb,
                          this);
}

static void sm_sv_file_transfer_send_memory_type(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file memory type is %d", this->m_memory_info.m_memory_ext);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_INFO_INDEX,
                          SDO_FILE_TRANSFER_INFO_MEMORY_TYPE_SUB_INDEX, this->m_cur_info.m_dest_id,
                          &this->m_memory_info.m_memory_type, 1, 500, sm_sv_file_transfer_memory_info_co_cb, this);
}

static void sm_sv_file_transfer_send_start_addr(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file start addr is 0x%x", this->m_memory_info.m_start_addr);
    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_INFO_INDEX,
                          SDO_FILE_TRANSFER_INFO_START_ADDR_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_memory_info.m_start_addr, 8, 500, sm_sv_file_transfer_memory_info_co_cb,
                          this);
}

/******************* Handle for step 2: Transfer data frame *************************/

static void
sm_sv_file_transfer_send_file_data_co_cb(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void *_arg) {
    sm_sv_file_transfer_impl_t *this = _impl(_arg);
    this->m_isWaiting = false;
    if (_sdoStt == SM_SDO_ST_ABORT) {
        LOG_ERR(TAG, "SDO status: Abort, Tx_abort: 0x%2X, Rx_Abort: 0x%2X \n", _txCode, _rxCode);
        this->m_retry++;
        if (this->m_retry >= RETRY_ALLOW) {
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_FAILED);
        }
    } else if (_sdoStt == SM_SDO_ST_SUCCESS) {
        this->m_step++;
        if (this->m_step >= TRANSFER_FILE_DATA_STEP_NUMBER) {
            if (this->m_current_frame.m_is_last) {
                LOG_INF(TAG, "Last frame is sent, now send file info");
                sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_SEND_FILE_INFO);
            } else {
                this->m_current_frame_id++;
                sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_SEND_FILE_DATA_FRAME);
            }
        }
    }
}

static void sm_sv_file_transfer_load_current_data_frame(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    int ret = this->m_file_input->get_file_data_frame(&this->m_current_frame, this->m_current_frame_id);
    if (ret < 0) {
        LOG_ERR(TAG, "Cannot load data frame id %d", this->m_current_frame_id);
        sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_FAILED);
        return;
    }
    this->m_step++;
}

static void sm_sv_file_transfer_send_frame_id(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file data frame id %d", this->m_current_frame.m_id);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_DATA_INDEX,
                          SDO_FILE_TRANSFER_DATA_FRAME_ID_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_current_frame.m_id, 4, 500, sm_sv_file_transfer_send_file_data_co_cb,
                          this);

}

static void sm_sv_file_transfer_send_frame_size(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file data frame size is %d", this->m_current_frame.m_size);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_DATA_INDEX,
                          SDO_FILE_TRANSFER_DATA_FRAME_SIZE_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_current_frame.m_size, 4, 500, sm_sv_file_transfer_send_file_data_co_cb,
                          this);
}

static void sm_sv_file_transfer_send_frame_data(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file data buffer size is %d", this->m_current_frame.m_size);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_DATA_INDEX,
                          SDO_FILE_TRANSFER_DATA_FRAME_DATA_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_current_frame.m_data, this->m_current_frame.m_size, 5000,
                          sm_sv_file_transfer_send_file_data_co_cb, this);
}

static void sm_sv_file_transfer_send_frame_crc(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file data frame crc is %d", this->m_current_frame.m_crc);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_DATA_INDEX,
                          SDO_FILE_TRANSFER_DATA_FRAME_CRC_16_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_current_frame.m_crc, 2, 5000, sm_sv_file_transfer_send_file_data_co_cb,
                          this);
}

/******************* Handle for step 3: Transfer file info *************************/

static void
sm_sv_file_transfer_send_file_info_co_cb(SM_SDO_STATUS_t _sdoStt, int32_t _txCode, int32_t _rxCode, void *_arg) {
    sm_sv_file_transfer_impl_t *this = _impl(_arg);
    this->m_isWaiting = false;
    if (_sdoStt == SM_SDO_ST_ABORT) {
        LOG_ERR(TAG, "SDO status: Abort, Tx_abort: 0x%2X, Rx_Abort: 0x%2X \n", _txCode, _rxCode);
        this->m_retry++;
        if (this->m_retry >= RETRY_ALLOW) {
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_FAILED);
        }
    } else if (_sdoStt == SM_SDO_ST_SUCCESS) {
        this->m_step++;
        if (this->m_step >= TRANSFER_FILE_INFO_STEP_NUMBER) {
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_SUCCESS);
        }
    }
}

static void sm_sv_file_transfer_load_file_info(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    int ret = this->m_file_input->get_file_info(&this->m_file_info);
    if (ret < 0) {
        LOG_ERR(TAG, "Cannot load data file info");
        sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_FAILED);
        return;
    }
    this->m_step++;
}

static void sm_sv_file_transfer_send_file_crc(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this->m_co_core || sm_co_sdo_is_busy(this->m_co_core) || this->m_isWaiting)
        return;

    this->m_isWaiting = true;

    LOG_INF(TAG, "Now send file info crc is %d", this->m_file_info.m_file_crc);

    sm_co_sdo_client_send(this->m_co_core,
                          SDO_FILE_TRANSFER_INFO_INDEX,
                          SDO_FILE_TRANSFER_INFO_FILE_CRC_16_SUB_INDEX, this->m_cur_info.m_dest_id,
                          (uint8_t *) &this->m_file_info.m_file_crc, 2, 500, sm_sv_file_transfer_send_file_info_co_cb,
                          this);
}

int32_t sm_sv_file_transfer_process(sm_sv_file_transfer_t *_this) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this)
        return -1;

    switch (this->m_process_stt) {
        case TRANSFER_STATE_IDLE:
            if(this->m_queue_head != this->m_queue_tail){
                this->m_queue_tail++;
                if(this->m_queue_tail >= MAX_FILE_QUEUE){
                    this->m_queue_tail = 0;
                }
                this->m_cur_info = this->m_info_queue[this->m_queue_tail];
                sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_PREPARE);
                LOG_INF(TAG, "Get file info from queue, now start transfer");
            }
            break;
        case TRANSFER_STATE_PREPARE:
            this->m_file_input = sm_sv_file_transfer_get_file_input(this->m_cur_info.m_path);
            if (!this->m_file_input) {
                LOG_ERR(TAG, "Input if not valid");
                sm_sv_file_transfer_emit_callback(_this, false);
                sm_sv_file_transfer_reset_transfer(_this);
                return -1;
            }
            this->m_file_input->init();
            this->m_file_input->get_file_info(&this->m_file_info);
            this->m_memory_info.m_mem_size = this->m_file_info.m_file_size;
            this->m_memory_info.m_start_addr = this->m_cur_info.m_start_addr;
            this->m_memory_info.m_memory_type = this->m_cur_info.m_mem_type;
            this->m_memory_info.m_memory_ext = this->m_cur_info.m_mem_ext;
            this->m_current_frame_id = 0;
            LOG_INF(TAG, "Start transfer file");
            sm_sv_file_transfer_change_process_stt(this, TRANSFER_STATE_SEND_MEMORY_INFO);
            break;
        case TRANSFER_STATE_SEND_MEMORY_INFO:
            if (this->m_step == TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_EXT) {
                sm_sv_file_transfer_send_memory_ext(this);
            } else if (this->m_step == TRANSFER_MEMORY_INFO_STEP_SEND_START_ADDR) {
                sm_sv_file_transfer_send_start_addr(this);
            } else if (this->m_step == TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_SIZE) {
                sm_sv_file_transfer_send_memory_size(this);
            } else if (this->m_step == TRANSFER_MEMORY_INFO_STEP_SEND_MEMORY_TYPE) {
                sm_sv_file_transfer_send_memory_type(this);
            }
            break;
        case TRANSFER_STATE_SEND_FILE_DATA_FRAME:
            if (this->m_step == TRANSFER_FILE_DATA_STEP_LOAD_FILE_FRAME) {
                sm_sv_file_transfer_load_current_data_frame(this);
            } else if (this->m_step == TRANSFER_FILE_DATA_STEP_SEND_FRAME_ID) {
                sm_sv_file_transfer_send_frame_id(this);
            } else if (this->m_step == TRANSFER_FILE_DATA_STEP_SEND_FRAME_SIZE) {
                sm_sv_file_transfer_send_frame_size(this);
            } else if (this->m_step == TRANSFER_FILE_DATA_STEP_SEND_FRAME_DATA) {
                sm_sv_file_transfer_send_frame_data(this);
            } else if (this->m_step == TRANSFER_FILE_DATA_STEP_SEND_FRAME_CRC) {
                sm_sv_file_transfer_send_frame_crc(this);
            }
            break;
        case TRANSFER_STATE_SEND_FILE_INFO:
            if (this->m_step == TRANSFER_FILE_INFO_LOAD_FILE_INFO) {
                sm_sv_file_transfer_load_file_info(this);
            } else if (this->m_step == TRANSFER_FILE_INFO_STEP_SEND_FILE_CRC) {
                sm_sv_file_transfer_send_file_crc(this);
            }
            break;
        case TRANSFER_STATE_SUCCESS:
            LOG_ERR(TAG, "Transfer file SUCCESS!!!");
            sm_sv_file_transfer_emit_callback(_this, true);
            sm_sv_file_transfer_reset_transfer(_this);
            break;
        case TRANSFER_STATE_FAILED:
            LOG_ERR(TAG, "Transfer file FAILED!!!");
            sm_sv_file_transfer_emit_callback(_this, false);
            sm_sv_file_transfer_reset_transfer(_this);
            break;
    }
    if (this->m_file_input && this->m_file_input->process) {
        this->m_file_input->process();
    }
    return 0;
}

sm_sv_file_transfer_t *sm_sv_file_transfer_create(sm_co_t *_co_core, finish_transfer_cb _cb, void *_arg) {
    sm_sv_file_transfer_impl_t *this = malloc(sizeof(sm_sv_file_transfer_impl_t));

    if (!this || !_co_core)
        return NULL;

    this->m_co_core = _co_core;
    this->m_cb = _cb;
    this->m_cb_arg = _arg;
    this->m_isWaiting = false;

    this->m_cur_info.m_dest_id = -1;
    this->m_queue_head = this->m_queue_tail = 0;

    sm_sv_file_transfer_reset_transfer(this);
    return this;
}

int32_t sm_sv_file_transfer_add_request(sm_sv_file_transfer_t *_this,
                                        int32_t _id,
                                        uint64_t _start_addr,
                                        uint8_t _memory_type,
                                        uint8_t _memory_extension,
                                        const char*_path) {
    sm_sv_file_transfer_impl_t *this = _impl(_this);
    if (!this)
        return -1;

    this->m_queue_head++;
    if(this->m_queue_head >= MAX_FILE_QUEUE){
        this->m_queue_head = 0;
    }
    this->m_info_queue[this->m_queue_head].m_dest_id = _id;
    this->m_info_queue[this->m_queue_head].m_path = _path;
    this->m_info_queue[this->m_queue_head].m_mem_ext = _memory_extension;
    this->m_info_queue[this->m_queue_head].m_mem_type = _memory_type;
    this->m_info_queue[this->m_queue_head].m_start_addr = _start_addr;
    return 0;
}

