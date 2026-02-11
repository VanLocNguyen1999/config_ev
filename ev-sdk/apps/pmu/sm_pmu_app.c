//
// Created by vnbk on 24/05/2024.
//

#include "sm_pmu_app.h"
#include "hal_data.h"

#define TAG "SM_PMU_APP"

#define _impl(x) ((sm_pmu_app_t*)x)

#define	ERR_NONE					0
#define	OVER_VOLTAGE_CHARGER		04
#define	ERR_PMU_POWER_LIMIT_1		30
#define	ERR_PMU_POWER_LIMIT_2		31

#define	TIME_5_MIU		3*60*1000



sm_pmu_app_t* g_pmu_app = NULL;

static sm_pmu_app_t g_pmu_app_default;
void sm_led_init(){
	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	g_pmu_app = &g_pmu_app_default;
	pmu_app->m_led.m_state = RUNNING_LIGHT;
    elapsed_timer_resetz(&pmu_app->m_led.m_state_timeout,TIME_5_MIU);
    sm_bsp_led_green_set(0);
    sm_bsp_led_red_set(0);
	sm_bsp_led_blue_set(0);
}

static void led_reset(void) {
    // 1. Đảm bảo hàm rand() có giá trị ngẫu nhiên thực sự (nên gọi srand ở hàm init)

    // 2. Chọn trạng thái ngẫu nhiên
    // Giả sử RUNNING_LIGHT là state đầu tiên và MAX_NUMBER là tổng số state
    g_pmu_app->m_led.m_state = (rand() % (MAX_NUMBER - RUNNING_LIGHT)) + RUNNING_LIGHT;

    // 3. Chọn thời gian timeout ngẫu nhiên từ 1 đến 5 phút (miu)
    // Công thức chuẩn: (rand() % (max - min + 1)) + min
    uint32_t random_minutes = (rand() % 5) + 1;
    uint32_t random_time_ms = random_minutes * 60 * 1000;

    elapsed_timer_resetz(&g_pmu_app->m_led.m_state_timeout, random_time_ms);

    // 4. Reset phần cứng: Tắt tất cả LED để tránh xung đột màu khi chuyển state
    sm_bsp_led_green_set(0);
    sm_bsp_led_red_set(0);
    sm_bsp_led_blue_set(0);
}
static void led_running_light (void){ //G sáng -> B sáng -> Y sáng -> Lặp lại.

    sm_bsp_led_green_set(1);
	sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(0);
	R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    sm_bsp_led_green_set(0);
	sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
	R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    sm_bsp_led_green_set(0);
	sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(1);
	R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
}

static void led_ping_pong_light (void){// G -> B -> R -> B -> (Về G).

    sm_bsp_led_green_set(1);
	sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(0);
	R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    sm_bsp_led_green_set(0);
	sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
	R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    sm_bsp_led_green_set(0);
	sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(1);
	R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    sm_bsp_led_green_set(0);
	sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
	R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
}

static void led_binary_counter_2026(void) {
    // --- BƯỚC 1: SỐ 2 (Nhị phân 010 -> Chỉ Blue sáng) ---
    sm_bsp_led_green_set(0); // Bit 2 (4)
    sm_bsp_led_blue_set(1);  // Bit 1 (2)
    sm_bsp_led_red_set(0);   // Bit 0 (1)
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);

    // --- BƯỚC 2: SỐ 0 (Nhị phân 000 -> Tắt hết) ---
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);

    // --- BƯỚC 3: SỐ 2 (Nhị phân 010 -> Chỉ Blue sáng) ---
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);

    // --- BƯỚC 4: SỐ 6 (Nhị phân 110 -> Green + Blue sáng) ---
    sm_bsp_led_green_set(1); // Bit 2 (4)
    sm_bsp_led_blue_set(1);  // Bit 1 (2)
    sm_bsp_led_red_set(0);   // Bit 0 (1)
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
}

static void led_police_strobe_light(void) {
    // --- CỤM 1: GREEN nháy nhanh 3 lần ---
    for(int i = 0; i < 3; i++) {
        sm_bsp_led_green_set(1);
        sm_bsp_led_blue_set(0);
        sm_bsp_led_red_set(0);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS); // Nháy cực nhanh 50ms

        sm_bsp_led_green_set(0);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
    }

    // --- CỤM 2: BLUE & RED nháy luân phiên hoặc cùng lúc 3 lần ---
    for(int i = 0; i < 3; i++) {
        sm_bsp_led_green_set(0);
        sm_bsp_led_blue_set(1);
        sm_bsp_led_red_set(1);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

        sm_bsp_led_blue_set(0);
        sm_bsp_led_red_set(0);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

static void led_the_wave_light(void) {
    // 1. Sóng bắt đầu từ GREEN
    sm_bsp_led_green_set(1);
    sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    // 2. Sóng lan sang BLUE (Green vẫn đang sáng)
    sm_bsp_led_green_set(1);
    sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    // 3. Sóng lan sang RED (Green tắt, Blue vẫn sáng)
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(1);
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    // 4. Sóng chỉ còn ở RED (Blue tắt)
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(1);
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    // 5. Kết thúc một nhịp sóng (Tắt hết trước khi lặp lại)
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
}
static void led_accordion_light(void) {
    // Bước 1: Dãn ra (Hai đầu sáng)
    sm_bsp_led_green_set(1);
    sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(1);
    R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);

    // Bước 2: Co vào (Giữa sáng)
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(0);
    R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);
}
static void led_severe_alert_light(void) {
    for(int i = 0; i < 5; i++) {
        // Nhóm lạnh ON, nhóm nóng OFF
        sm_bsp_led_green_set(1);
        sm_bsp_led_blue_set(1);
        sm_bsp_led_red_set(0);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

        // Nhóm lạnh OFF, nhóm nóng ON
        sm_bsp_led_green_set(0);
        sm_bsp_led_blue_set(0);
        sm_bsp_led_red_set(1);
        R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

static void led_sequence_build_up_light(void) {
    uint32_t delay_val = 400;

    for(int j = 0; j < 4; j++) {
        // Chạy vòng tròn G -> B -> R
        sm_bsp_led_green_set(1); sm_bsp_led_blue_set(0); sm_bsp_led_red_set(0);
        R_BSP_SoftwareDelay(delay_val, BSP_DELAY_UNITS_MILLISECONDS);

        sm_bsp_led_green_set(0); sm_bsp_led_blue_set(1); sm_bsp_led_red_set(0);
        R_BSP_SoftwareDelay(delay_val, BSP_DELAY_UNITS_MILLISECONDS);

        sm_bsp_led_green_set(0); sm_bsp_led_blue_set(0); sm_bsp_led_red_set(1);
        R_BSP_SoftwareDelay(delay_val, BSP_DELAY_UNITS_MILLISECONDS);

        delay_val = (delay_val * 60) / 100; // Giảm delay đi 40% sau mỗi vòng
    }

    // Kết thúc: Sáng rực tất cả trong 1 giây
    sm_bsp_led_green_set(1);
    sm_bsp_led_blue_set(1);
    sm_bsp_led_red_set(1);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
}
static void led_rainbow_flow_light(void) {
    // 1. ĐỎ
    sm_bsp_led_red_set(1); sm_bsp_led_green_set(0); sm_bsp_led_blue_set(0);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 2. CAM / VÀNG (Red + Green)
    sm_bsp_led_red_set(1); sm_bsp_led_green_set(1); sm_bsp_led_blue_set(0);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 3. LỤC (Xanh lá)
    sm_bsp_led_red_set(0); sm_bsp_led_green_set(1); sm_bsp_led_blue_set(0);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 4. LAM (Xanh dương)
    sm_bsp_led_red_set(0); sm_bsp_led_green_set(0); sm_bsp_led_blue_set(1);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 5. CHÀM (Green + Blue)
    sm_bsp_led_red_set(0); sm_bsp_led_green_set(1); sm_bsp_led_blue_set(1);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 6. TÍM (Red + Blue)
    sm_bsp_led_red_set(1); sm_bsp_led_green_set(0); sm_bsp_led_blue_set(1);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    // 7. TRẮNG (Kết hợp cả 3 màu để kết thúc vòng cầu vồng)
    sm_bsp_led_red_set(1); sm_bsp_led_green_set(1); sm_bsp_led_blue_set(1);
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
}
static void led_random_glitch_light(void) {
    // Chạy 15 nhịp nháy ngẫu nhiên
    for(int k = 0; k < 15; k++) {
        // rand() % 2 trả về 0 hoặc 1
        sm_bsp_led_green_set(rand() % 2);
        sm_bsp_led_blue_set(rand() % 2);
        sm_bsp_led_red_set(rand() % 2);

        // Delay ngẫu nhiên từ 30ms đến 130ms
        uint32_t r_delay = (rand() % 200) + 100;
        R_BSP_SoftwareDelay(r_delay, BSP_DELAY_UNITS_MILLISECONDS);
    }
    // Đảm bảo tắt hết sau khi xong hiệu ứng
    sm_bsp_led_green_set(0);
    sm_bsp_led_blue_set(0);
    sm_bsp_led_red_set(0);
}

int32_t sm_pmu_app_process(void){

    if(!elapsed_timer_get_remain(&g_pmu_app->m_led.m_state_timeout)){
    	led_reset();
    }

	switch (g_pmu_app->m_led.m_state) {
	case RUNNING_LIGHT:
		led_running_light();
		break;
	case PING_PONG:
		led_ping_pong_light();
		break;
	case BINARY_COUNTER:
		led_binary_counter_2026();
		break;
	case POLICE_STRO:
		led_police_strobe_light();
		break;
	case THE_WAVE:
		led_the_wave_light();
		break;
	case ACCORDION:
		led_accordion_light();
		break;
	case SEVERE_ALERT:
		led_severe_alert_light();
		break;
	case SEQUENCE_BUILD_UP:
		led_sequence_build_up_light();
		break;
	case RAIOW_FLOW:
		led_rainbow_flow_light();
		break;
	case RANDOM_GLITCH:
		led_random_glitch_light();
		break;
	default:
		led_reset();
		break;
	}
	return 0;
}
