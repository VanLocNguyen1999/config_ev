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

#define TX_TIME_SYNC_LOW		10000   /* LOW  10000us  */
#define TX_TIME_SYNC_HIGH		1000   /* HIGH  1000us  */

#define TX_TIME_BIT1_LOW		500   /* LOW  500us */
#define TX_TIME_BIT1_HIGH		1500   /* HIGH 1500us */

#define TX_TIME_BIT0_LOW		1500   /* LOW  1500us */
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

static uint8_t sm_bat_check_sum(const uint8_t *buf, uint8_t len){

	uint16_t s = 0;
	for (uint8_t i = 0; i < len; i++) {
		s += buf[i];
	}
	return (uint8_t) (s);
}

static void sm_one_write_rx_complete(const OneWireRx_Frame_t * _frame, void* arg){

	sm_pmu_app_t* app = (sm_pmu_app_t*)arg;
	if(!_frame || !app) return;
	memcpy(&app->m_rx_frame, _frame, sizeof(OneWireRx_Frame_t));
	uint8_t cs =  sm_bat_check_sum(_frame->data, _frame->byte_count - 2);
	if(!cs) return;
 	if(cs == _frame->data[_frame->byte_count - 2]){

		memcpy(&app->m_bp_data,&_frame->data,sizeof(sm_bp_data_t));
	}
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
//	memset(&pmu_app->m_bp_data,0,sizeof(sm_bp_data_t));
	sm_hal_exti_set_callback(rx_if, sm_exti_rx_cb, pmu_app);
    elapsed_timer_resetz(&pmu_app->m_timeout, 5000);
	g_pmu_app = &g_pmu_app_default;
}
uint32_t data = 0;
int32_t sm_pmu_app_process(void){

	sm_pmu_app_t* pmu_app = &g_pmu_app_default;
	data = (uint8_t)(pmu_app->m_bp_data.m_soc * 2);
	return OneWireTx_send(pmu_app->m_one_write,(uint8_t*)&data,1);
}
