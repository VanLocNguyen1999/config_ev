//
// Created by vnbk on 27/05/2024.
//

#ifndef EV_SDK_SM_PMU_STM_H
#define EV_SDK_SM_PMU_STM_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"

enum {
	PMU_STM_STARTUP = 0,   // Trạng thái khởi tạo ban đầu
	PMU_STM_STARTING,      // Đang khởi động
	PMU_STM_IDLE,          // Rảnh, chưa làm gì
	PMU_STM_RUNNING,       // Đang hoạt động
	PMU_STM_CHARGING,      // Đang sạc
	PMU_STM_POWER_LIMIT,
	PMU_STM_SLEEP,         // Ngủ / tiết kiệm điện
	PMU_STM_UPGRADING,     // Đang nâng cấp firmware/software
	PMU_STM_NUMBER         // Số lượng trạng thái
};

enum {
    PMU_STM_EVENT_INTERNAL = 0,
    PMU_STM_EVENT_TURN_ON_KEY,
    PMU_STM_EVENT_TURN_OFF_KEY,
	PMU_STM_EVENT_FIRST_BP_ASSIGNED,
	PMU_STM_EVENT_ENTER_IDLE,
	PMU_STM_EVENT_ENTER_STARTING,
	PMU_STM_EVENT_ENTER_RUNNING_MODE,
	PMU_STM_EVENT_ENTER_POWER_LIMIT_MODE,
    PMU_STM_EVENT_PLUG_CHARGER,
    PMU_STM_EVENT_UNPLUG_CHARGER,
	PMU_STM_EVENT_CHARGER_FULL,
    PMU_STM_EVENT_TIMEOUT,
    PMU_STM_EVENT_MOTOR_STOP,
    PMU_STM_EVENT_ENTER_UPGRADING_MODE,
    PMU_STM_EVENT_EXIT_UPGRADING_MODE,
    PMU_EVENT_NUMBER
};

int32_t sm_pmu_app_stm_init(void* _app);
void sm_pmu_app_stm_proc(void* _app);
int32_t sm_pmu_app_get_stm(void);
#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_STM_H
