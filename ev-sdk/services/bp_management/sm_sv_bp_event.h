//
// Created by vnbk on 23/09/2024.
//

#ifndef EV_SDK_SM_SV_BP_EVENT_H
#define EV_SDK_SM_SV_BP_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_bp_data.h"

typedef enum {
    BP_CMD_REBOOT,
    BP_CMD_CHARGE,
    BP_CMD_ONLY_DISCHARGE,
    BP_CMD_DISCHARGE,
	BP_CMD_RETAIN_DISCHARGE,
    BP_CMD_STANDBY,
    BP_CMD_READ_SN,
    BP_CMD_READ_ASSIGNED_DEV,
    BP_CMD_WRITE_ASSIGNED_DEV,
    BP_CMD_READ_VERSION,
    BP_CMD_RECONFIG_ID,
    BP_CMD_FORCE_DISCHARGE,
    BP_CMD_SET_BLOCK,
    BP_CMD_SET_ACTIVE,
    BP_CMD_SET_CYCLE,
    BP_CMD_NUMBER
} SM_BP_CMD;

#define SM_BP_CMD_SUCCESS    (0)
#define SM_BP_CMD_FAILURE    (-1)

/**
 * @brief
 * @param id:
 * @param cmd
 * @param is_success
 * @param data
 * @param arg
*/
typedef void (*sm_bp_on_cmd_fn_t)(int32_t, SM_BP_CMD, int32_t, void *, void *);

typedef enum {
    BP_AUTH_FAILURE,
    BP_AUTH_SUCCESS,
    BP_AUTH_EVENT_NUMBER
} SM_BP_AUTH_EVENT;

typedef void (*sm_bp_auth_event_fn_t)(int32_t, SM_BP_AUTH_EVENT, const char *, int32_t, void *);

typedef struct {
    void (*on_bp_connected)(int32_t, const char *, int32_t, void *);

    void (*on_bp_disconnected)(int32_t, const char *, void *);

    void (*on_bp_update_data)(int32_t, const sm_bp_data_t *, void *);

    void (*bp_assgin_fail)(int32_t _id,void *);
} sm_sv_bp_event_cb_t;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_SV_BP_EVENT_H
