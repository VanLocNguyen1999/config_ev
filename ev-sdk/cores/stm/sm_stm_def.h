//
// Created by vnbk on 25/05/2024.
//

#ifndef EV_SDK_SM_STM_DEF_H
#define EV_SDK_SM_STM_DEF_H

#include <stdint.h>

typedef enum {
    SM_STM_EVENT_NUMBER = 1
}SM_STM_EVENT;

typedef enum {
    SM_STM_STATE_NUMBER = 1
}SM_STM_STATE;

/**
 * @brief: Event handler
 * @param: argument of event.
 * @return: Next state.
 */
typedef int32_t (*sm_stm_event_handler_fn_t)(void*);

#endif //EV_SDK_SM_STM_DEF_H
