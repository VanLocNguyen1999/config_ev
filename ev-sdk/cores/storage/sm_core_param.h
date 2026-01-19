//
// Created by vnbk on 10/04/2024.
//

#ifndef EV_SDK_SM_CORE_PARAM_H
#define EV_SDK_SM_CORE_PARAM_H

#include <stdint.h>

void sm_core_param_init(SYSTEMCALLBACK fnDefineProc);
int32_t sm_core_param_count();
int32_t sm_core_param_set_value(uint16_t nParam, uint16_t nValue);
uint16_t GetParam(uint16_t nParam);
void sm_core_param_set_values();
void RegisterParamCallback(uint8_t nEvent, SYSTEMCALLBACK fnCallback);

#define PARAM_DEFINE_EVENT     0
#define PARAM_CHANGE_EVENT     1
#define PARAM_EVENT_COUNT      2

typedef struct{
    uint8_t nEvent;
    uint8_t nIndex;
    uint16_t nParam;
    uint16_t nValue;
} PARAMCHANGEEVENT, *PPARAMCHANGEEVENT;

#pragma pack(1)
typedef struct tagPARAMITEM {
    uint16_t nParam;
    uint16_t nValue;
} PARAMITEM, *PPARAMITEM;

extern PPARAMITEM PARAM_ITEMS[];
#define PARAMETER(index) (PARAM_ITEMS[(index)]->nValue)
#define DEFINE_PARAM(index, param, value) { PARAM_ITEMS[(index)]->nParam = (param); PARAM_ITEMS[(index)]->nValue = (value); }


#define MAX_PARAM_ITEM		50

#define UPDATE_PARAM_INTERVAL  500

#endif //EV_SDK_SM_CORE_PARAM_H
