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

typedef void sm_ev_config_para_t;

typedef enum {
    EV_CFG_UV_PROTECT,
	EV_CFG_IBAT_LIMIT,
	EV_CFG_ECO_SPEED,
	EV_CFG_SPORT_SPEED,
	EV_CFG_READ,
	EV_CFG_NUMBER
} EV_CFG_CMD;

typedef void (*sm_ev_config_cmd_fn_t)(void* , int32_t , void *);
typedef struct sm_ev_config_cmd{
	EV_CFG_CMD m_cmd;
	sm_ev_config_cmd_fn_t m_cb;
    void* m_arg;
    void* m_data;
}sm_ev_config_cmd_t;
int32_t sm_ev_config_co_set_cmd(sm_ev_config_para_t* _this,  sm_ev_config_cmd_t _cmd);
sm_ev_config_para_t* sm_ev_config_para_create(sm_co_t *m_co);
void sm_ev_set_uv_protect(sm_ev_config_para_t *_this, float uv_protect);
void sm_ev_set_ibat_limit(sm_ev_config_para_t *_this, float ibat_limit);
void sm_ev_set_eco_speed(sm_ev_config_para_t *_this, uint8_t eco_speed);
void sm_ev_set_sport_speed(sm_ev_config_para_t *_this, uint8_t sport_speed);
#ifdef __cplusplus
}
#endif
#endif /* SM_EV_CONFIG_CO_H_ */
