//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_STORAGE_H
#define EV_SDK_SM_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_types.h"

typedef struct sm_storage sm_storage_t;

typedef struct {
    int32_t (*init)(sm_storage_t*);
    int32_t (*free)(sm_storage_t*);
    int32_t (*load)(sm_storage_t*, void*);
    int32_t (*store)(sm_storage_t*, const void*);
    int32_t (*clear)(sm_storage_t*);
}sm_storage_proc_t;

struct sm_storage{
    const sm_storage_proc_t* m_proc;
};

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_STORAGE_H
