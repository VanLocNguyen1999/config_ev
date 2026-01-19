/*
 * sm_bp_retain.h
 *
 *  Created on: 20 Nov 2025
 *      Author: My PC
 */

#ifndef SM_BP_RETAIN_H_
#define SM_BP_RETAIN_H_
#include "sm_bp.h"
#include "sm_sv_bp.h"

typedef void sm_bp_retain_t;

typedef struct{
    void (*enable_retain_cb)(int32_t _id, void* _arg);
    void (*disable_retain_cb)(int32_t _id, void* _arg);
}sm_sv_bp_retain_cb_t;


sm_bp_retain_t* sm_bp_retain_create				(sm_bp_t* bp_list, void* _arg);
sm_bp_retain_t* sm_bp_retain_get                 (void);
int32_t 		sm_bp_retain_process				(sm_bp_retain_t* _this);
int32_t 		sm_bp_active_retain				(sm_bp_retain_t *_this, uint8_t id);
int32_t 		sm_bp_deactive_retain			(sm_bp_retain_t *_this, uint8_t id);
int32_t 		sm_bp_retain_pause				(sm_bp_retain_t *_this);
int32_t 		sm_bp_retain_resume				(sm_bp_retain_t *_this);
int32_t 		sm_bp_get_active_retain_numbs	(sm_bp_retain_t* _this);
float 			sm_bp_retain_get_elapsed_ratio	(sm_bp_retain_t *_this);
#endif /* SM_BP_RETAIN_H_ */
