/*
 * sm_co_if.c
 *
 *  Created on: May 17, 2024
 *      Author: DELL 5425
 */
#include <string.h>

#include "sm_co_if.h"
#include "sm_hal.h"

#define SM_CO_FUNCTION_CALLBACK_NUM		4
#define SM_QUEUE_CAN_MSG                32

struct sm_co_if {
	sm_hal_can_t* 					    m_can_port;
    struct {
        sm_co_if_recv_callback_fn_t 	m_fn;
        void*							m_arg;
    }sm_co_recv_cb[SM_CO_FUNCTION_CALLBACK_NUM];

    sm_hal_can_msg_t m_queue[SM_QUEUE_CAN_MSG];
    uint8_t m_head;
    uint8_t m_tail;
};

static sm_co_if_t g_co_if_df;

static void sm_can_rx_cb(sm_hal_can_t *_this, sm_hal_can_msg_t *_msg, void* _arg){

    (void)_arg;
    (void)_this;
    g_co_if_df.m_queue[g_co_if_df.m_head].id = _msg->id;
    g_co_if_df.m_queue[g_co_if_df.m_head].length = _msg->length;
    memcpy(g_co_if_df.m_queue[g_co_if_df.m_head].data, _msg->data, 8);

    g_co_if_df.m_head++;

    if(g_co_if_df.m_head >= SM_QUEUE_CAN_MSG){
        g_co_if_df.m_head = 0;
    }
}

sm_co_if_t *sm_co_if_create(uint8_t _type, const char *_argv, int _argc, void* _arg){

    (void)_type;
    (void)_argv;
    (void)_argc;
    (void)_arg;
	return NULL;
}

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *_argv, int _argc, void* _arg){

    (void)_type;
    (void)_argv;
    (void)_argc;
	g_co_if_df.m_can_port = (sm_hal_can_t*)_arg;
	for (int i=0; i < SM_CO_FUNCTION_CALLBACK_NUM; i++){
        g_co_if_df.sm_co_recv_cb[i].m_fn = NULL;
        g_co_if_df.sm_co_recv_cb[i].m_arg = NULL;
	}

	for(int index = 0; index < SM_QUEUE_CAN_MSG; index++){
	    g_co_if_df.m_queue[index].id = 0;
	    g_co_if_df.m_queue[index].length = 0;
	}
	 g_co_if_df.m_head = 0;
	 g_co_if_df.m_tail = 0;

    sm_hal_can_set_rx_cb(_arg, sm_can_rx_cb, &g_co_if_df);

	return &g_co_if_df;
}

int sm_co_if_free(sm_co_if_t* _this){

    (void) _this;
	return 0;
}

int sm_co_if_reg_recv_callback(sm_co_if_t* _this, sm_co_if_recv_callback_fn_t callback_fn, void* _arg){
    if(!_this){
        return -1;
    }
	for(int index = 0; index < SM_CO_FUNCTION_CALLBACK_NUM; index++){
        if(_this->sm_co_recv_cb[index].m_fn == NULL){
            _this->sm_co_recv_cb[index].m_fn = callback_fn;
            _this->sm_co_recv_cb[index].m_arg = _arg;
            return 0;
        }
    }
    return -1;
}

void* sm_co_if_get_interface(sm_co_if_t* _this){
	return (void*)_this->m_can_port;
}

int sm_co_if_set_config(sm_co_if_t *_this, const char *_argv, int _argc, void* _arg){

    (void)_this;
    (void)_argv;
    (void)_argc;
    (void)_arg;
	return 0;
}

int sm_co_if_connect(sm_co_if_t *_this){

    (void)_this;
	return 0;
}

int sm_co_if_disconnect(sm_co_if_t *_this){

    (void)_this;
	return 0;
}

int sm_co_if_is_connected(sm_co_if_t *_this){

    (void)_this;
	return 0;
}

int sm_co_if_send(sm_co_if_t *_this, uint32_t frame_id, const unsigned char *data, int len, int timeout){

    (void) timeout;
    sm_hal_can_msg_t msg = {
            .id = frame_id,
            .length = (uint16_t)len,
    };
    memcpy(msg.data, data, (size_t)len);
    return sm_hal_can_write(_this->m_can_port, &msg);
}

int sm_co_if_recv(sm_co_if_t *_this, unsigned char *buf, int max_len, int timeout){
	(void) timeout;
	(void)_this;
	(void) buf;
	(void) max_len;
	return 0;
}

int sm_co_if_process(sm_co_if_t *_this)
{
    if (!_this)
        return -1;

    if (_this->m_head != _this->m_tail)
    {
        sm_hal_can_msg_t *msg = &_this->m_queue[_this->m_tail];
        for (int i = 0; i < SM_CO_FUNCTION_CALLBACK_NUM; i++)
        {
            if (_this->sm_co_recv_cb[i].m_fn != NULL)
            {
                _this->sm_co_recv_cb[i].m_fn (msg->id, msg->data, _this->sm_co_recv_cb[i].m_arg);
            }
        }

        _this->m_tail++;
        if(_this->m_tail >= SM_QUEUE_CAN_MSG){
            _this->m_tail = 0;
        }
    }

    return 0;
}
