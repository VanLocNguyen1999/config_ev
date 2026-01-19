//
// Created by NgHuyHai on 8/12/2024.
//
#include "sm_sv_bt.h"
#include "sm_logger.h"
#include "sm_elapsed_timer.h"

#define TAG "SM_SV_BUTTON"

#define _impl(x) ((sm_sv_bt_impl_t*)(x))

typedef struct {
    sm_bt_if m_if;
    uint8_t m_last_meas2;
    uint8_t m_last_meas1;
    uint8_t m_current_meas;
    uint32_t m_time_meas;
}sm_bt_info_t;

typedef struct{
    sm_bt_info_t m_bts[SM_SV_BT_MAX_BT_NUM];
    const sm_sv_bt_config_t* m_config;

    struct {
        sm_sv_bt_event_cb_fn_t    m_fn;
        void                            *m_arg;
    }m_event_cb[SM_SV_BT_MAX_BT_NUM];

    elapsed_timer_t m_scan_timer;
}sm_sv_bt_impl_t;

static sm_sv_bt_impl_t g_sv_btn;

sm_sv_bt_config_t g_cfg_default = {
    .m_debounce_time = SM_SV_BT_DEBOUNCE_TIME,
    .m_holding_time = SM_SV_BT_HOLDING_TIME,
    .m_scan_time = SM_SV_BTN_SCAN_TIME_DEFAULT,
};

static void sm_scan_bt(sm_sv_bt_impl_t* _this){
    for(uint8_t index = 0; index < SM_SV_BT_MAX_BT_NUM; index++){
        if(_this->m_bts[index].m_if){
            uint8_t stt = _this->m_bts[index].m_if();
            if(stt != 0){
                _this->m_bts[index].m_current_meas = 1;
            }else{
                _this->m_bts[index].m_current_meas = 0;
            }
        }
    }
}

sm_sv_bt_t* sm_sv_bt_create(const sm_sv_bt_config_t* _config) {
    sm_sv_bt_impl_t* bt = &g_sv_btn;

    if(!_config){
        bt->m_config = &g_cfg_default;
    }else{
        bt->m_config = _config;
    }

    for (int index = 0; index < SM_SV_BT_MAX_BT_NUM; ++index) {
        bt->m_bts[index].m_if = NULL;
        bt->m_bts[index].m_time_meas = 0;
        bt->m_bts[index].m_current_meas = 0;
        bt->m_bts[index].m_last_meas1 = 0;
        bt->m_bts[index].m_last_meas2 = 0;

        bt->m_event_cb[index].m_fn = NULL;
        bt->m_event_cb[index].m_arg = NULL;
    }

    elapsed_timer_resetz(&bt->m_scan_timer, bt->m_config->m_scan_time);

    LOG_INF(TAG, "Created Button service SUCCESS");

    return bt;
}

int32_t sm_sv_bt_destroy(sm_sv_bt_t *_this) {
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_sv_bt_reg_event(sm_sv_bt_t *_this, uint8_t _button_id, sm_sv_bt_event_cb_fn_t _fn_callback, void *_arg) {
    if (!_this || _button_id > SM_SV_BT_MAX_BT_NUM) {
        return -1;
    }
    _impl(_this)->m_event_cb[_button_id].m_fn = _fn_callback;
    _impl(_this)->m_event_cb[_button_id].m_arg = _arg;
    return 0;
}

int32_t sm_sv_bt_add_bt_if(sm_sv_bt_t* _this, sm_bt_if _if){
    if(!_this || !_if){
        return -1;
    }
    sm_sv_bt_impl_t* bt = _impl(_this);
    for (uint8_t i = 0; i < SM_SV_BT_MAX_BT_NUM; i++) {
        if(!bt->m_bts[i].m_if){
            bt->m_bts[i].m_if = _if;
            bt->m_bts[i].m_time_meas = 0;
            bt->m_bts[i].m_current_meas = 0;
            bt->m_bts[i].m_last_meas1 = 0;
            bt->m_bts[i].m_last_meas2 = 0;
            return i;
        }
    }
    return -1;
}

int32_t sm_sv_bt_set_config(sm_sv_bt_t *_this, sm_sv_bt_config_t *_bt_config) {
    if (!_this || !_bt_config) {
        return -1;
    }
    _impl(_this)->m_config = _bt_config;
    return 0;
}

int32_t sm_sv_bt_process(sm_sv_bt_t *_this) {
    if (!_this) {
        return -1;
    }

    sm_sv_bt_impl_t* sv_bt = _impl(_this);
    if(!elapsed_timer_get_remain(&sv_bt->m_scan_timer)){
        sm_bt_info_t* bt_info = NULL;

        for (uint8_t i = 0 ; i < SM_SV_BT_MAX_BT_NUM; i++)
        {
        	bt_info = &sv_bt->m_bts[i];
        	bt_info->m_last_meas2 = bt_info->m_last_meas1;
        	bt_info->m_last_meas1 = bt_info->m_current_meas;
        }

        sm_scan_bt(sv_bt);

        for(uint8_t i = 0; i < SM_SV_BT_MAX_BT_NUM; i++){
            bt_info = &sv_bt->m_bts[i];
            if(!bt_info){
                continue;
            }

            if((bt_info->m_last_meas2 ^ bt_info->m_last_meas1) & bt_info->m_current_meas){ // egle rising
                bt_info->m_time_meas += sv_bt->m_config->m_scan_time;
/*                if(sv_bt->m_event_cb[i].m_fn){
                    sv_bt->m_event_cb[i].m_fn(i,
                                              SM_SV_BT_EVENT_PUSH,
                                              sv_bt->m_event_cb[i].m_arg);
                }*/
            }else if (bt_info->m_last_meas2 && bt_info->m_last_meas1 && bt_info->m_current_meas){ //duration time measure
                bt_info->m_time_meas += sv_bt->m_config->m_scan_time;
                if(bt_info->m_time_meas >= sv_bt->m_config->m_holding_time){
                    if(sv_bt->m_event_cb[i].m_fn){
                        sv_bt->m_event_cb[i].m_fn(i,
                                                  SM_SV_BT_EVENT_HOLD,
                                                  sv_bt->m_event_cb[i].m_arg);
                    }
                }/*else if(bt_info->m_time_meas >= sv_bt->m_config->m_debounce_time){
                    if(sv_bt->m_event_cb[i].m_fn){
                        sv_bt->m_event_cb[i].m_fn(i,
                                                  SM_SV_BT_EVENT_PUSH,
                                                  sv_bt->m_event_cb[i].m_arg);
                    }
                }*/
            }else if(bt_info->m_last_meas1 & (bt_info->m_last_meas2 ^ bt_info->m_current_meas)){ // egle falling
                if(bt_info->m_time_meas < sv_bt->m_config->m_holding_time &&
                    bt_info->m_time_meas > sv_bt->m_config->m_debounce_time){ //set event release touch
                    if(sv_bt->m_event_cb[i].m_fn){
                        sv_bt->m_event_cb[i].m_fn(i,
                                                  SM_SV_BT_EVENT_RELEASE,
                                                  sv_bt->m_event_cb[i].m_arg);
                    }
                }

                bt_info->m_time_meas = 0;
            }
        }

        elapsed_timer_reset(&sv_bt->m_scan_timer);
    }
    return 0;
}
