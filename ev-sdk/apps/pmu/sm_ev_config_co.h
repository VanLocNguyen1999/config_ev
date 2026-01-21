/*
 * sm_ev_config_co.h
 *
 *  Created on: 21 Jan 2026
 *      Author: My PC
 */

#ifndef SM_EV_CONFIG_CO_H_
#define SM_EV_CONFIG_CO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_core_co.h"
#include "sm_co_if.h"

typedef void sm_ev_config_t;

typedef enum {
    EV_CFG_UV_PROTECT,
	EV_CFG_IBAT_LIMIT,
	EV_CFG_ECO_SPEED,
	EV_CFG_SPORT_SPEED,
	EV_CFG_READ,
	EV_CFG_NUMBER
} EV_CFG_CMD;

typedef struct sm_bp_cmd{
	EV_CFG_CMD m_cmd;
    void* m_data;
    void* m_arg;
}sm_ev_config_cmd_t;
int32_t sm_ev_config_co_set_cmd(sm_ev_config_t* _this,  sm_ev_config_cmd_t* _cmd);
#ifdef __cplusplus
}
#endif
#endif /* SM_EV_CONFIG_CO_H_ */
