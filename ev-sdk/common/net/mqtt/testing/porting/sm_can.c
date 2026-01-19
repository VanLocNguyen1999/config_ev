/*
 * can.c
 *
 *  Created on: Jul 10, 2023
 *      Author: Admin
 */


#include "sm_hal_can.h"
#include "hal_data.h"

#define impl(x)     ((can_instance_t*)(x))

static int32_t can_write(sm_hal_can_t *_this,sm_hal_can_mgs_t *msg);
static int32_t can_set_baud(sm_hal_can_t *_this,uint32_t baud);
static int32_t can_open(sm_hal_can_t *_this);
static int32_t can_close(sm_hal_can_t *_this);

sm_hal_can_proc_t can_func = {.close = can_close,.open = can_open,.set_baudrate = can_set_baud,.write = can_write};
static inline void memcpy_buff(uint8_t dest[], const uint8_t src[], const uint16_t size){
	uint16_t i;
	for(i = 0; i < size; i++){
		dest[i] = src[i];
	}
}
static int32_t can_write(sm_hal_can_t *_this,sm_hal_can_mgs_t *msg){
    /*User code*/
	can_frame_t frame;
	frame.id = msg->id;
	frame.data_length_code = (uint8_t)msg->length;
	frame.id_mode = CAN_ID_MODE_STANDARD;
	frame.type = CAN_FRAME_TYPE_DATA;
	memcpy_buff(frame.data, msg->data, 8);
	R_CAN_Write(impl(_this->handle)->p_ctrl, 0, &frame);
    return 0;
}
static int32_t can_set_baud(sm_hal_can_t *_this,uint32_t baud){
    /*User code*/
    return 0;
}
static int32_t can_open(sm_hal_can_t *_this){
    /*User code*/
    R_CAN_Open(impl(_this->handle)->p_ctrl, impl(_this->handle)->p_cfg);
    return 0;
}
static int32_t can_close(sm_hal_can_t *_this){
    /*User code*/
    R_CAN_Close(impl(_this->handle)->p_ctrl);
    return 0;
}


