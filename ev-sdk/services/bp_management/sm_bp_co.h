/*
 * sm_bp_co.h
 *
 *  Created on: Mar 12, 2024
 *      Author: vnbk
 */

#ifndef SERVICES_BP_SERVICE_SM_BP_CO_H_
#define SERVICES_BP_SERVICE_SM_BP_CO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_bp.h"
#include "sm_core_co.h"
#include "sm_co_if.h"

typedef void sm_bp_co_t;

typedef struct{
    void (*update_data_cb)(int32_t id, const sm_bp_data_t* data, void* arg);
    void (*finish_cmd_cb)(const sm_bp_cmd_t* cmd, int32_t status, void* arg);
    void* arg;
}sm_bp_co_cb_t;

sm_bp_co_t* sm_bp_co_create             (sm_co_t* _co,sm_bp_t* _bp_list
                                            ,uint8_t _bp_num,sm_bp_co_cb_t _cb);

int32_t sm_bp_co_destroy                (sm_bp_co_t* _this);

int32_t sm_bp_co_reset_cmd              (sm_bp_co_t* _this);

int32_t sm_bp_co_is_busy                (sm_bp_co_t* _this);

int32_t sm_bp_co_set_cmd                (sm_bp_co_t* _this, const sm_bp_cmd_t* _cmd);

int32_t sm_bp_co_set_cmd_force          (sm_bp_co_t* _this, const sm_bp_cmd_t* _cmd);

int32_t sm_bp_co_process                (sm_bp_co_t* _this);
int32_t sm_bp_co_charge                 (sm_bp_co_t* _this);
int32_t sm_bp_co_discharge              (sm_bp_co_t* _this);
int32_t sm_bp_co_retain_discharge		(sm_bp_co_t* _this);
int32_t sm_bp_co_only_discharge         (sm_bp_co_t* _this);
int32_t sm_bp_co_cycle                  (sm_bp_co_t* _this);
int32_t sm_bp_co_active                 (sm_bp_co_t* _this);
int32_t sm_bp_co_block                  (sm_bp_co_t* _this);
int32_t sm_bp_co_force_discharge        (sm_bp_co_t* _this);
int32_t sm_bp_co_write_node_id          (sm_bp_co_t* _this);
int32_t sm_bp_co_write_dev              (sm_bp_co_t* _this);
int32_t sm_bp_co_read_dev               (sm_bp_co_t* _this);
int32_t sm_bp_co_read_version           (sm_bp_co_t* _this);
int32_t sm_bp_co_read_sn                (sm_bp_co_t* _this);
int32_t sm_bp_co_standby                (sm_bp_co_t *_this);
#ifdef __cplusplus
}
#endif


#endif /* SERVICES_BP_SERVICE_SM_BP_CO_H_ */
