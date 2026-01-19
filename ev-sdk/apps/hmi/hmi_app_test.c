//
// Created by vnbk on 07/08/2024.
//
#include "sm_bsp_hmi.h"
#include "sm_app_hmi.h"

#include "sch/sm_core_sch.h"
//#include "bsp/sm_bsp_sys_linux.h"

#include "sm_logger.h"

#define MAIN_TAG "PMU_APP"

sm_sch_t* g_core_sch = NULL;

int hmi_app_test(){
//    sm_logger_init(sm_bsp_linux_logger_put, LOG_LEVEL_DEBUG);
//    LOG_INF(MAIN_TAG, "This is PMU Application");

//    g_core_sch = sm_sch_create_default();
    sm_hmi_app_init();

    while (1){
        sm_hmi_app_process();
//        sm_sch_process(g_core_sch);
    }
}
