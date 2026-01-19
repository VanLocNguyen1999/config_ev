//
// Created by Ng Thuy Quynh on 8/24/2024.
//

#ifndef TEST_API_EVENT_H
#define TEST_API_EVENT_H

#include <stdio.h>
#include <sm_logger.h>
#include "sm_pmu_api.h"
#include "sm_sv_pms.h"
#include "sm_sv_charger.h"
#include "sm_sv_bp.h"

const char* TAG = "PMU_EVENT";

static inline void pmu_on_io_key_changed(int32_t _value, void* _arg ){
    if (_value)
        LOG_INF(TAG,"KEY ON");
    else
        LOG_INF(TAG, "KEY OFF");
}
static inline void pmu_on_io_parking_bt_press(void* _arg) {
    LOG_INF(TAG,"Parking pressed");
}
static inline void pmu_on_io_parking_bt_release(void* _arg) {

}
static inline void pmu_on_io_parking_bt_hold(void* _arg) {
    LOG_INF(TAG,"Parking held");
}
static inline void pmu_on_io_mode_bt_press(void* _arg) {
    LOG_INF(TAG,"Drv mode pressed");
}
static inline void pmu_on_io_mode_bt_release(void* _arg) {

}
static inline void pmu_on_io_mode_bt_hold(void* _arg) {
    LOG_INF(TAG,"Drv mode held");
}

static inline void pmu_on_io_left_signal_status_changed(int32_t _value, void* _arg) {
    switch (_value) {
    case SM_PMU_IO_OFF :
        LOG_INF(TAG,"Left off");
        break;
    case SM_PMU_IO_ON :
        LOG_INF(TAG,"Left on");
        break;
    case SM_PMU_IO_BLINK :
        LOG_INF(TAG,"Left blink");
        break;
    }
}
static inline void pmu_on_io_right_signal_status_changed(int32_t _value, void* _arg){
    switch (_value) {
        case SM_PMU_IO_OFF :
            LOG_INF(TAG,"right off");
        break;
        case SM_PMU_IO_ON :
            LOG_INF(TAG,"right on");
        break;
        case SM_PMU_IO_BLINK :
            LOG_INF(TAG,"right blink");
        break;
    }
}
static inline void pmu_on_io_high_beam_status_changed(int32_t _value, void* _arg){
    switch (_value) {
        case SM_PMU_IO_OFF :
            LOG_INF(TAG,"high_beam off");
        break;
        case SM_PMU_IO_ON :
            LOG_INF(TAG,"high_beam on");
        break;
        case SM_PMU_IO_BLINK :
            LOG_INF(TAG,"high_beam blink");
        break;
    }
}
static inline void pmu_on_io_low_beam_status_changed(int32_t _value, void* _arg){
    switch (_value) {
        case SM_PMU_IO_OFF :
            LOG_INF(TAG,"low_beam off");
        break;
        case SM_PMU_IO_ON :
            LOG_INF(TAG,"low_beam on");
        break;
        case SM_PMU_IO_BLINK :
            LOG_INF(TAG,"low_beam blink");
        break;
    }
}
static inline void pmu_on_io_emergency_status_changed(int32_t _value, void* _arg){
    switch (_value) {
        case SM_PMU_IO_OFF :
            LOG_INF(TAG,"emergency off");
        break;
        case SM_PMU_IO_ON :
            LOG_INF(TAG,"emergency on");
        break;
        case SM_PMU_IO_BLINK :
            LOG_INF(TAG,"emergency blink");
        break;
    }
}

// CHARGER
static inline void pmu_on_charger_plugged(int32_t _value, void* _arg){
    if (_value)
        LOG_INF(TAG,"Charger plugged");
    else
        LOG_INF(TAG, "Charger unplugged");
}
static inline void pmu_on_charged(int32_t _bp_id, void* _arg){
    LOG_INF(TAG, "BP %d is charging", _bp_id);
}
// BP
static inline void pmu_on_bp_connected(int32_t _bp_id, const char* _sn, int32_t _soc, void* _arg){
    LOG_INF(TAG, "BP %d connected", _bp_id);
}
static inline void pmu_on_bp_disconnected(int32_t _bp_id, const char* _sn, void* _arg){
    LOG_INF(TAG, "BP %d disconnected", _bp_id);

}
static inline void pmu_on_bp_update_data(int32_t _bp_id, const sm_bp_data_t*, void* _arg){
    LOG_INF(TAG, "BP %d PDO recived", _bp_id);
}
// PMS
static inline void pmu_on_pms_switch_merge(uint8_t _event, uint8_t _from, uint8_t _into, void* _arg){
    switch (_event) {
        case SM_SV_PMS_EVENT_IDLE :
            LOG_INF(TAG,"PMS event: | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_SWITCH_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_SWITCH_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_SWITCH_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_SWITCH_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_MERGE_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_MERGE_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_MERGE_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_MERGE_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_DISABLE_BP_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_DISABLE_BP_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_DISABLE_BP_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_DISABLE_BP_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_ENABLE_BP_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_ENABLE_BP_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_ENABLE_BP_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_ENABLE_BP_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_FORCE_BP_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_FORCE_BP_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_FORCE_BP_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_FORCE_BP_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_RELEASE_BP_SUCCESS:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_RELEASE_BP_SUCCESS | from_bp: %d to_bp: %d", _from, _into);
        break;
        case SM_SV_PMS_EVENT_RELEASE_BP_FAIL:
            LOG_INF(TAG,"PMS event: SM_SV_PMS_EVENT_RELEASE_BP_FAIL | from_bp: %d to_bp: %d", _from, _into);
        break;
    }
}
static inline void pmu_on_pms_update_est_data(const est_data_t* , void* _arg){

}

#endif //TEST_API_EVENT_H
