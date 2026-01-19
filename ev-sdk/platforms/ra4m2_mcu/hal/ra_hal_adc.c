/*
 * ra_hal_adc.c
 *
 *  Created on: Oct 1, 2024
 *      Author: vuonglk
 */

#include <stdint.h>
#include <stdio.h>
#include "sm_hal_adc.h"
#include "hal_data.h"

#define _impl(x)	((ra_hal_adc_t*)x)

typedef struct {
    adc_instance_t* m_channel;
    uint8_t m_channel_num;
    uint32_t m_flag;
}ra_hal_adc_t;

ra_hal_adc_t g_adc;

sm_hal_adc_t* sm_hal_adc_init(void* _handle, int32_t _channel, uint32_t _flag){
    g_adc.m_channel = _handle;
    g_adc.m_channel_num = _channel;
    g_adc.m_flag = _flag;
    return &g_adc;
}


void sm_hal_adc_deinit(sm_hal_adc_t *_this){
    return ;
}

int32_t sm_hal_adc_start(sm_hal_adc_t *_this){
    const adc_channel_cfg_t g_adc0_channel_cfg =
            {
                    .scan_mask          = _impl(_this)->m_flag,
                    .scan_mask_group_b  = 0,
                    .priority_group_a   = (adc_group_a_t) 0,
                    .add_mask           = 0,
                    .sample_hold_mask   = 0,
                    .sample_hold_states = 0,
            };
    R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    R_ADC_ScanStart(&g_adc0_ctrl);
    return 0;
}

uint16_t sm_hal_adc_read(sm_hal_adc_t *_this){
    uint16_t val = 0;
    sm_hal_adc_start(_this);
    R_ADC_Read(&g_adc0_ctrl, _impl(_this)->m_channel_num, &val);
    return val;
}

int32_t sm_hal_adc_stop(sm_hal_adc_t *_this){
    return 0;
}

void sm_hal_adc_set_cb(sm_hal_adc_t *_this, void(*on_converted_fn_t)(uint16_t)){
    return ;
}

int32_t sm_hal_adc_open(sm_hal_adc_t *_this){
    ra_hal_adc_t* this = (ra_hal_adc_t*) _this;
    R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    return 0;
}
int32_t sm_hal_adc_close(sm_hal_adc_t *_this){
    return 0;
}

