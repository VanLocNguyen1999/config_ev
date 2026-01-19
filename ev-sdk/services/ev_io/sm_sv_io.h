//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_IO_H
#define EV_SDK_SM_SV_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_types.h"

#define SM_SV_IO_ENABLE_DEFAULT             (true)
#define SM_SV_IO_DEBOUNCE_TIME_DEFAULT      (100) // ms

#define SM_SV_IO_BLINK_DURATION_DEFAULT     (500)
#define SM_SV_IO_BLINK_REPEAT_TIME_FOREVER  (-1)

enum {
    SM_SV_IO_DEACTIVATE = 0,
    SM_SV_IO_ACTIVATE  = 1,
};

enum {
    IN_12V_DET = 0,
    IN_NUMBER
};
enum {
    OUT_12V_ACT= 0,
    OUT_NUMBER
};
#define IO_SIGNAL   IO_EMERGENCY

enum {
    SM_SV_IO_EVENT_12V_DET_CHANGED = 0,
    SM_SV_IO_EVENT_ACTIVE_12V_CHANGED = 1,
    SM_SV_IO_EVENT_NUMBER = 3
};

enum {
    OP_STATE_NORMAL= 0 ,
    OP_STATE_DEFAULT,
    OP_STATE_FORCE,
    OP_STATE_FORCE_BLINK,
};
/**
 * @brief Function callback IO event
 * @param _event
 * @param _value
 * @param _arg
 * @return NONE
 */
typedef void (*sm_sv_io_event_cb_fn_t)(uint8_t, uint8_t, void*);

typedef struct sm_sv_io_if {
    int32_t (*in_12v_det_fn_t)();
    int32_t (*out_actice_12V_fn_t)(uint8_t);
}sm_sv_io_if_t;

typedef void sm_sv_io_t;

/**
 * @brief sm_sv_io_create
 * @param _bt
 * @return
 */
sm_sv_io_t* sm_sv_io_create(void *_if);

/**
 * @brief sm_sv_io_destroy
 * @param _this
 * @return
 */
int32_t sm_sv_io_destroy(sm_sv_io_t *_this);

/**
 * @brief sm_sv_io_reg_event
 * @param _this
 * @param _fn_cb
 * @param _arg
 * @return
 */
int32_t sm_sv_io_reg_event(sm_sv_io_t *_this, uint8_t _event, sm_sv_io_event_cb_fn_t _fn_cb, void* _arg);

/**
 * @brief sm_sv_io_set_if
 * @param _this
 * @param _if
 * @return
 */
int32_t sm_sv_io_set_if(sm_sv_io_t *_this, sm_sv_io_if_t *_io_if);

/**
 * @brief sm_sv_io_get_key_state
 * @param _this
 * @return
 */
int32_t sm_sv_io_get_key_state(sm_sv_io_t *_this);

/**
 * @brief sm_sv_io_get_parking_state
 * @param _this
 * @return
 */

int32_t sm_sv_io_enable_parking(sm_sv_io_t* _this, uint8_t _enable);

/**
 * @brief sm_sv_io_get_drive_mode_state
 * @param _this
 * @return
 */

int32_t sm_sv_io_enable_drive_mode(sm_sv_io_t* _this, uint8_t _enable);

/**
 * @brief sm_sv_io_get_io_sw_state
 * @param _this
 * @return
 */
int32_t sm_sv_io_get_io_sw_state(sm_sv_io_t *_this, uint8_t _io);

int32_t sm_sv_io_get_state(sm_sv_io_t* _this, uint8_t _io);

int32_t sm_sv_io_enable(sm_sv_io_t* _this, uint8_t _io, uint8_t _enable);

int32_t sm_sv_io_force(sm_sv_io_t* _this, uint8_t _io, uint8_t _value);

int32_t sm_sv_io_release(sm_sv_io_t* _this, uint8_t _io);

int32_t sm_sv_io_blink(sm_sv_io_t* _this, uint8_t _io, int32_t _repeat, int32_t _duration);

int32_t sm_sv_io_cancel_blink(sm_sv_io_t* _this, uint8_t _io);

/**
 *
 * @param _this
 */
void sm_sv_io_process(sm_sv_io_t *_this);


static inline const char* sm_sv_io_event_to_string(uint8_t _event){
    switch (_event) {
        case SM_SV_IO_EVENT_12V_DET_CHANGED:
            return "SM_SV_IO_EVENT_12V_DET_CHANGED";
        case SM_SV_IO_EVENT_ACTIVE_12V_CHANGED:
            return "SM_SV_IO_EVENT_ACTIVE_12V_CHANGED";

        default:
            return "EVENT_UNKNOWN";
    }
}

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_SV_IO_H
