//
// Created by vnbk on 24/05/2024.
//

#include "sm_pmu_app.h"
#include "hal_data.h"
#include "sm_utils.h"
#define TAG "SM_PMU_APP"

#define _impl(x) ((sm_pmu_app_t*)x)

#define	ERR_NONE					0
#define	OVER_VOLTAGE_CHARGER		04
#define	ERR_PMU_POWER_LIMIT_1		30
#define	ERR_PMU_POWER_LIMIT_2		31

#define	TIME_5_MIU		3*60*1000

#define TX_TIME_SYNC_LOW		48000   /* LOW  10000us  */
#define TX_TIME_SYNC_HIGH		1000   /* HIGH  1000us  */

#define TX_TIME_BIT1_LOW		500   /* LOW  500us */
#define TX_TIME_BIT1_HIGH		1000   /* HIGH 1500us */

#define TX_TIME_BIT0_LOW		1000   /* LOW  1500us */
#define TX_TIME_BIT0_HIGH		500   /* HIGH 500us */

#define TX_TIME_STOP_LOW		5000   /* LOW   5000us  */
#define TX_TIME_STOP_HIGH		10000   /* HIGH 2ms  */

#define TX_OW_MSG_ID       0x3A
#define TX_OW_VERSION      0x11
/**************************************************************/
#define RX_TIME_SYNC_MAX		68200
#define RX_TIME_SYNC_MIN		55800

#define RX_TIME_BIT0_MAX		4400
#define RX_TIME_BIT0_MIN		3600

#define RX_TIME_BIT1_MAX		2200
#define RX_TIME_BIT1_MIN		1800

#define RX_TIME_STOP_MAX		22000
#define RX_TIME_STOP_MIN		18000

//#define RX_TIME_SYNC_MAX		11000
//#define RX_TIME_SYNC_MIN		9000
//
//#define RX_TIME_BIT1_MAX		550
//#define RX_TIME_BIT1_MIN		450
//
//#define RX_TIME_BIT0_MAX		1650
//#define RX_TIME_BIT0_MIN		1350
//
//#define RX_TIME_STOP_MAX		5500
//#define RX_TIME_STOP_MIN		4500

sm_pmu_app_t* g_pmu_app = NULL;

static sm_pmu_app_t g_pmu_app_default;
enum{

	EVENT_RISING = 0,
	EVENT_FALLING
};

static OneWire_Para_t one_wire_para = {
		.m_id = TX_OW_MSG_ID,
		.m_version = TX_OW_VERSION,
		.m_time_syns_low = TX_TIME_SYNC_LOW,
		.m_time_syns_high = TX_TIME_SYNC_HIGH,
		.m_time_bit0_low = TX_TIME_BIT0_LOW,
		.m_time_bit0_high = TX_TIME_BIT0_HIGH,
		.m_time_bit1_low = TX_TIME_BIT1_LOW,
		.m_time_bit1_high = TX_TIME_BIT1_HIGH,
		.m_time_stop_low = TX_TIME_STOP_LOW,
		.m_time_stop_high = TX_TIME_STOP_HIGH,

		.m_time_syns_min = RX_TIME_SYNC_MIN,
		.m_time_syns_max = RX_TIME_SYNC_MAX,
		.m_time_bit0_max = RX_TIME_BIT0_MAX,
		.m_time_bit0_min = RX_TIME_BIT0_MIN,
		.m_time_bit1_max = RX_TIME_BIT1_MAX,
		.m_time_bit1_min = RX_TIME_BIT1_MIN,
		.m_time_stop_min = RX_TIME_STOP_MIN,
		.m_time_stop_max = RX_TIME_STOP_MAX
};

static sm_icm_init_data_t g_icm_init = {
		.m_flag = 1,
		.m_motor_pole_pairs = 30,
		.m_motor_power = 2000,
		.m_motor_speed = 800,
		.m_motor_vol = 60,
		.m_set_max_speed = 70,
		.m_wheel_radius = 0.2618f, //0.2618
		.m_ev_purpose = 1
};
static sm_icm_cycle_data_t g_icm_cycle = {

		.m_set_anti_thef = 0,
		.m_set_break_mode = 0,
		.m_set_forward_mode = 0,
		.m_set_max_speed = 70,
		.m_set_reverse_mode = 0,
		.m_set_side_stand_mode = 0,
		.m_set_time_enter_parking = 5,
		.m_side_stand_single = 0
};
static void sm_icm_init_data_build(uint8_t *_buffer){

	_buffer[0] = 35;//0x23;
	_buffer[1] = 14;

	uint16_t tmp = 0;
	if (g_icm_init.m_motor_vol >= 24U) {
		tmp = (uint16_t)((g_icm_init.m_motor_vol - 24U) / 12U);
	}
	uint8_t vol_data = (uint8_t) tmp;

	uint8_t byte2 = 0;
	byte2 |= (g_icm_init.m_flag & 0x01U) << 7;
	byte2 |= (vol_data & 0x07U) << 4;             // 3 bit -> bit 6..4
	byte2 |= (g_icm_init.m_ev_purpose & 0x03U) << 2; // 2 bit -> bit 3..2
	_buffer[2] = byte2;

	_buffer[3] = (uint8_t)	g_icm_init.m_motor_pole_pairs;
	_buffer[4] = (uint8_t)	(g_icm_init.m_motor_power*0.01);

	_buffer[5] = (uint8_t) ((g_icm_init.m_motor_speed >> 8) & 0xFF);
	_buffer[6] = (uint8_t) (g_icm_init.m_motor_speed & 0xFF);

	uint16_t wheel_radius = (uint16_t) (g_icm_init.m_wheel_radius*1000);
	_buffer[7] = (uint8_t) ((wheel_radius >> 8) & 0xFF);
	_buffer[8] = (uint8_t) (wheel_radius & 0xFF);

	_buffer[9] = (uint8_t) (g_icm_init.m_set_max_speed);

	_buffer[10] = 0;
	_buffer[11] = 0;
	_buffer[12] = 0;
}

static void sm_icm_cycle_data_build(uint8_t *_buffer){

	sm_pmu_app_t* app = &g_pmu_app_default;
	_buffer[0] = 36;//0x23;
	_buffer[1] = 14;

	uint8_t byte2 = 0;
	byte2 |= (app->m_icm_cycle.m_set_reverse_mode & 0x03U) << 4;
	byte2 |= (app->m_icm_cycle.m_set_forward_mode & 0x03U) << 6;
	_buffer[2] = byte2;

	uint8_t byte3 = 0;
	byte3 |= (app->m_icm_cycle.m_set_side_stand_mode & 0x03U) << 2;
	_buffer[3] = byte3;

	_buffer[4] = 0;

	uint8_t byte5 = 0;
	byte5 |= (app->m_icm_cycle.m_set_anti_thef & 0x03U) << 2;
	_buffer[5] = byte5;

	uint8_t byte6 = 0;
	byte6 |= (app->m_icm_cycle.m_side_stand_single & 0x03U) << 3;
	_buffer[6] = byte6;

	_buffer[7] = 0;

	uint8_t byte8 = 0;
	byte8 |= (app->m_icm_cycle.m_set_break_mode & 0x03U) << 5;
	_buffer[8] = byte8;

	_buffer[9] = 0;
	_buffer[10] = 0;

	_buffer[11] = app->m_icm_cycle.m_set_max_speed;
	_buffer[12] = 0;
}
static void sm_one_write_rx_complete(const OneWireRx_Frame_t * _frame, void* arg){

	(void)_frame;
	(void)arg;
}
static OneWireRx_callback_t one_write_cb = {
		.on_frame_complete = sm_one_write_rx_complete,
		.on_error = NULL
};
static void sm_exti_rx_cb(uint32_t event, void* arg){

	sm_pmu_app_t* app = (sm_pmu_app_t*)arg;
	if(event == EVENT_RISING){

		OneWireRx_OnFallingEdge(app->m_one_write);
		return;
	}
	OneWireRx_OnRisingEdge(app->m_one_write);
}
void sm_one_write_init(){
	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	sm_hal_io_t* tx_if = sm_bsp_get_one_write_tx();
	sm_hal_exti_t* rx_if = bsp_get_exti_one_write_rx();
	if(!tx_if || !rx_if)
		return;
	sm_one_wire_t* one_wire = sm_one_wire_create(tx_if,&one_wire_para,one_write_cb,pmu_app);
	if(!one_wire) return;
	pmu_app->m_one_write = one_wire;
	pmu_app->m_icm_init_counter = 0;
	sm_hal_exti_set_callback(rx_if, sm_exti_rx_cb, pmu_app);
    elapsed_timer_resetz(&pmu_app->m_timeout, 5000);
    memcpy(&pmu_app->m_icm_cycle, &g_icm_cycle,sizeof(sm_icm_cycle_data_t));
	g_pmu_app = &g_pmu_app_default;
}

int32_t sm_pmu_app_process(void){

	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	uint8_t init_data[14] = {0};
	if(pmu_app->m_icm_init_counter < 5){
		sm_icm_init_data_build(init_data);
		pmu_app->m_icm_init_counter ++;
	}else {
		sm_icm_cycle_data_build(init_data);
	}
	OneWireTx_send(pmu_app->m_one_write, init_data, 14);
	return 0;
}
