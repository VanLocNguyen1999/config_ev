//
// Created by vnbk on 25/03/2025.
//

#ifndef EV_SDK_SM_BSP_PORTING_H
#define EV_SDK_SM_BSP_PORTING_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_sv_bp.h"
#include "sm_sv_charger.h"

sm_bp_node_id_controller_t *sm_can_master_get_node_id_if();

sm_sv_charger_if_t* sm_get_charger_if();

#ifdef __cplusplus
};
#endif


#endif //EV_SDK_SM_BSP_PORTING_H
