#include "sm_hal_can.h"
#include "hal_data.h"

#define CAN_CALLBACK_NUM_MAX   8

typedef struct {
    sm_hal_can_rx_cb_fn_t m_rx_cb;
    void                  *m_rx_arg;
    sm_hal_can_tx_cb_fn_t m_tx_cb;
    void                  *m_tx_arg;
} can_irq_t;

typedef struct {
    can_instance_t *m_channel;
    uint32_t m_bitrate;
    can_irq_t irq[CAN_CALLBACK_NUM_MAX];
    uint8_t m_rx_cb_num;
    uint8_t m_tx_cb_num;
}ra_can_t;
static ra_can_t g_can;
#define impl(x)         ((ra_can_t*)(x))

sm_hal_can_t* sm_hal_can_init(const void *_channel, uint32_t _bitrate){
    ra_can_t *can = &g_can;
    can->m_channel = (can_instance_t*)_channel;
    can->m_bitrate = _bitrate;
    for (int i=0; i<CAN_CALLBACK_NUM_MAX; i++){
        can->irq[i].m_rx_cb = NULL;
        can->irq[i].m_rx_arg = NULL;
        can->irq[i].m_tx_cb = NULL;
        can->irq[i].m_tx_arg = NULL;
    }
    can->m_rx_cb_num = 0;
    return (sm_hal_can_t*) can;
}

void sm_hal_can_deinit(sm_hal_can_t *_this){
    ra_can_t *can = impl(_this);
    can->m_channel = NULL;
    can->m_bitrate = 0;
    for (int i=0; i<CAN_CALLBACK_NUM_MAX; i++){
        can->irq[i].m_rx_cb = NULL;
        can->irq[i].m_rx_arg = NULL;
        can->irq[i].m_tx_cb = NULL;
        can->irq[i].m_tx_arg = NULL;
    }
    can->m_rx_cb_num = 0;
}


void sm_hal_can_set_rx_cb(sm_hal_can_t *_this, sm_hal_can_rx_cb_fn_t _cb, void* _arg){
    if (!_this){
        return;
    }
    if (impl(_this)->m_rx_cb_num < CAN_CALLBACK_NUM_MAX){
        impl(_this)->irq[impl(_this)->m_rx_cb_num].m_rx_cb = _cb;
        impl(_this)->irq[impl(_this)->m_rx_cb_num].m_rx_arg = _arg;
        impl(_this)->m_rx_cb_num++;
    }
}


void sm_hal_can_set_tx_cb(sm_hal_can_t *_this, sm_hal_can_tx_cb_fn_t _cb, void* _arg){
    if (!_this){
        return;
    }
    if (impl(_this)->m_tx_cb_num < CAN_CALLBACK_NUM_MAX){
        impl(_this)->irq[impl(_this)->m_tx_cb_num].m_tx_cb = _cb;
        impl(_this)->irq[impl(_this)->m_tx_cb_num].m_rx_arg = _arg;
        impl(_this)->m_tx_cb_num++;
    }
}


int32_t sm_hal_can_set_bit_rate(sm_hal_can_t *_this, uint32_t _bit_rate){
    if (!_this){
        return -1;
    }
    impl(_this)->m_bitrate = _bit_rate;
    return 0;
}


int32_t sm_hal_can_write(sm_hal_can_t *_this, sm_hal_can_msg_t *_msg){
    if (!_this){
        return -1;
    }
    can_frame_t frame;
    frame.id = _msg->id;
    frame.data_length_code = (uint8_t)_msg->length;
    frame.id_mode = CAN_ID_MODE_STANDARD;
    frame.type = CAN_FRAME_TYPE_DATA;
    memcpy(frame.data, _msg->data, 8);
    int32_t err = R_CAN_Write(impl(_this)->m_channel->p_ctrl, 0, &frame);

    return err ? -1 : 0;
}

int32_t sm_hal_can_read(sm_hal_can_t *_this, sm_hal_can_msg_t *_msg){

    (void) _msg;
    if (!_this){
        return -1;
    }
    return 0;
}

int32_t sm_hal_can_open(sm_hal_can_t *_this){
    if (!_this){
        return -1;
    }
    int32_t err = R_CAN_Open(impl(_this)->m_channel->p_ctrl, impl(_this)->m_channel->p_cfg);
    return err ? -1 : 0;
}

int32_t sm_hal_can_close(sm_hal_can_t *_this){
    if (!_this){
        return -1;
    }
    int32_t err = R_CAN_Close(impl(_this)->m_channel->p_ctrl);
    return err ? -1 : 0;
}

int32_t sm_hal_can_rx_irq(sm_hal_can_t* _this, sm_hal_can_msg_t* _msg){
    if (!_this){
        return -1;
    }
    for (int i = 0; i< impl(_this)->m_rx_cb_num; i++){
        if (impl(_this)->irq[i].m_rx_cb){
            impl(_this)->irq[i].m_rx_cb(_this, _msg, impl(_this)->irq[i].m_rx_arg);
        }
    }
    return 0;
}

int32_t sm_hal_can_tx_irq(sm_hal_can_t* _this, sm_hal_can_msg_t* _msg){
    if (!_this){
        return -1;
    }
    for (int i = 0; i< impl(_this)->m_tx_cb_num; i++){
        if (impl(_this)->irq[i].m_tx_cb){
            impl(_this)->irq[i].m_tx_cb(_this, _msg, impl(_this)->irq[i].m_tx_arg);
        }
    }
    return 0;
}
