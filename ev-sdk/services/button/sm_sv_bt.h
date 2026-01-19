//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_BT_H
#define EV_SDK_SM_SV_BT_H

#include "sm_types.h"

#define SM_SV_BTN_SCAN_TIME_DEFAULT               10UL //ms
#define SM_SV_BT_HOLDING_TIME				     (SM_SV_BTN_SCAN_TIME_DEFAULT*100)
#define SM_SV_BT_DEBOUNCE_TIME				     (SM_SV_BTN_SCAN_TIME_DEFAULT*10)

#define SM_SV_BT_MAX_BT_NUM			2

typedef enum SM_SV_BT_EVENT {
    SM_SV_BT_EVENT_PUSH,
	SM_SV_BT_EVENT_RELEASE,
	SM_SV_BT_EVENT_HOLD,
    SM_SV_BT_EVENT_NUMBER
} SM_SV_BT_EVENT;

typedef void sm_sv_bt_t;

/**
 * Ex1: m_holding_time = 1000
 * if (button held in 1000 ms)
 * '-> event callback
 *
 * Ex2: m_holding_time = 0
 * block BT_EVENT_HOLD
 *
 * Note:
 * - The m_margin_time should be 30 (ms)
 */
typedef struct sm_sv_bt_config {
	uint16_t m_holding_time;
	uint16_t m_debounce_time;
    uint16_t m_scan_time;
} sm_sv_bt_config_t;

/**
 *
 */
typedef void (*sm_sv_bt_event_cb_fn_t)(uint8_t, uint8_t, void*);

typedef uint8_t (*sm_bt_if)();

/**
 * @brief sm_sv_bt_create
 * @return
 */
sm_sv_bt_t* sm_sv_bt_create(const sm_sv_bt_config_t* _config);

/**
 * @brief sm_sv_bt_destroy
 * @param _this
 * @return
 */
int32_t sm_sv_bt_destroy(sm_sv_bt_t *_this);

/**
 * @brief
 * @param _this
 * @param _if
 * @return BUTTON_ID: 0->MAX_BUTTON
 */
int32_t sm_sv_bt_add_bt_if(sm_sv_bt_t* _this, sm_bt_if _if);

/**
 * @brief sm_sv_bt_destroy
 * @param _this
 * @param _fn_callback
 * @param _arg
 * @return
 */
int32_t sm_sv_bt_reg_event(sm_sv_bt_t *_this, uint8_t _button_id, sm_sv_bt_event_cb_fn_t _fn_callback, void *_arg);
/**
 * @brief sm_sv_bt_set_config
 * @param _this
 * @param _bt_config
 */
int32_t sm_sv_bt_set_config(sm_sv_bt_t *_this, sm_sv_bt_config_t *_bt_config);

/**
 *
 * @param _this
 */
int32_t sm_sv_bt_process(sm_sv_bt_t *_this);



#endif //EV_SDK_SM_SV_BT_H
