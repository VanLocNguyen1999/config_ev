//
// Created by vnbk on 24/05/2024.
//

#ifndef EV_SDK_SM_PMU_APP_H
#define EV_SDK_SM_PMU_APP_H

#ifdef __cplusplus
extern "C"{
#endif
#include "sm_bsp_pmu.h"
#include "sm_types.h"
#include "sm_elapsed_timer.h"

enum {

	RUNNING_LIGHT, //G sáng -> B sáng -> Y sáng -> Lặp lại.
	PING_PONG, // G -> B -> Y -> B -> (Về G).
	BINARY_COUNTER, // 2 => 0 => 2 => 6
	POLICE_STRO, // (G nháy nhanh 3 lần) -> (Y nháy nhanh 3 lần) -> B luôn tắt (hoặc B nháy cùng G).
	THE_WAVE, // 1. G sáng. 2. G + B cùng sáng. 3. B + Y cùng sáng (G tắt). 4. Y sáng (B tắt). 5. Tắt hết (Reset).
	ACCORDION, // (G + Y) sáng => (B) sáng, (G + Y) tắt => (G + Y) sáng, (B) tắt
	SEVERE_ALERT, //(G + B) ON, (Y) OFF (50ms). => (G + B) OFF, (Y) ON (50ms).
	SEQUENCE_BUILD_UP, //G-B-Y (Delay 500ms)=> G-B-Y (Delay 200ms) => G-B-Y (Delay 50ms) => G-B-Y (on 1s)
	RAIOW_FLOW,
	RANDOM_GLITCH,
	MAX_NUMBER
};

typedef struct {

	elapsed_timer_t m_state_timeout;
	elapsed_timer_t m_led_timeout;
    uint8_t m_state;
} led_t;

typedef struct {
    led_t m_led;
}sm_pmu_app_t;

void sm_led_init     (void);
int32_t sm_pmu_app_process          (void);
extern sm_pmu_app_t* g_pmu_app;

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_PMU_APP_H
