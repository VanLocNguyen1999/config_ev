#ifndef SM_BP_H
#define SM_BP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "sm_sv_bp_event.h"
#include "sm_elapsed_timer.h"

#define BP_CMD_QUEUE_SIZE   		16
#define BP_CONNECTION_TIMEOUT		3000

#define BP_DATA_FIELD_NUMBER		6

enum {
	BP_NO_SUPPORT_RETAIN = 0,
	BP_SUPPORT_RETAIN,
	BP_ENABLE_RETAIN,
	BP_DISABLE_RETAIN,
	BP_ACTIVE_RETAIN,
	BP_DEACTIVE_RETAIN
};

typedef struct sm_bp{
    int32_t m_id;
    sm_bp_data_t m_data;

    uint8_t m_is_connected;
    elapsed_timer_t m_timeout;

    uint8_t m_data_count;
    uint8_t m_retain_st;
}sm_bp_t;

typedef struct sm_bp_cmd{
    int32_t m_id;
    SM_BP_CMD m_cmd;
    void* m_data;
    sm_bp_on_cmd_fn_t m_cb;
    void* m_arg;
}sm_bp_cmd_t;

static inline void sm_bp_reset(sm_bp_t* _this){
    sm_bp_reset_data(&_this->m_data);
    _this->m_is_connected = 0;
    elapsed_timer_resetz(&_this->m_timeout, BP_CONNECTION_TIMEOUT);
    _this->m_data_count = 0;
}

static inline void sm_bp_cmd_reset(sm_bp_cmd_t* _this){
    _this->m_id = -1;
    _this->m_cmd = BP_CMD_NUMBER;
    _this->m_data = NULL;
    _this->m_cb = NULL;
    _this->m_arg = NULL;
}

static inline uint8_t sm_bp_verify_sn(const char* _src, char* _dest){
    int16_t index = 0;
    int16_t dest_index = 0;
    char value = _src[index];

    while(value != '\0'){
        value = _src[index];

        if(value >= '0' && value <= '9') {
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value >= 'A' && value <= 'Z'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value >= 'a' && value <= 'z'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value == '.'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value == '_'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        return 0;
    }
    return 1;
}

static inline int32_t sm_bp_reverse_sn(char* _src, char* _dest){
    int32_t len = (int32_t)strlen(_src);
    if(len <= 0){
        return -1;
    }
    for(int index = 0; index < len; index++){
        _dest[index] = _src[len - index -1];
    }
    _dest[len] = '\0';
    return len;
}
static inline int32_t sm_bp_move_bp_to_list(void *_list, void *_bp) {

    sm_bp_t **list = (sm_bp_t**) _list;
    sm_bp_t *bp = (sm_bp_t*) _bp;
    if (!list || !bp)
        return -1;
    for (int i = SM_BP_NUMBER_DEFAULT - 1; i > 0; i--) {
    	if(list[i - 1] == NULL) continue ;
        list[i] = list[i - 1];
    }
    list[0] = bp;

    return 0;
}
static inline int32_t sm_bp_remove_bp_from_list(void *_list, void *_bp) {
    sm_bp_t **list = (sm_bp_t**) _list;
    sm_bp_t *bp = (sm_bp_t*) _bp;

    if (!list || !bp)
        return -1;

    for (int i = 0; i < SM_BP_NUMBER_DEFAULT; i++) {
        if (list[i] == bp) {
            for (int j = i; j < SM_BP_NUMBER_DEFAULT - 1; j++) {
                list[j] = list[j + 1];
            }
            list[SM_BP_NUMBER_DEFAULT - 1] = NULL;
            return 0;
        }
    }
    return -2;
}
#ifdef __cplusplus
}
#endif

#endif /// SM_BP_H
