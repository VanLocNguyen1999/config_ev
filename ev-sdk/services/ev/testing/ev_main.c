//
// Created by vnbk on 13/09/2024.
//
#include "sm_ev_cmd.h"
#include "sm_ev_service.h"
#include "sm_sv_bp.h"
#include "sm_utils.h"

#include "../../../apps/hmi/sm_hmi_storage.h"

#include "threading/thpool.h"

#include "sm_logger.h"
#include "bsp/sm_bsp_sys_linux.h"

#define MAIN_TAG "EV_SERVICE_TESTING"

sm_co_if_t* g_co_if = NULL;
sm_co_t* g_co_core = NULL;
sm_sv_bp_t* g_bp_manager = NULL;
sm_sv_ev_t* g_ev_service = NULL;

const char* convert_ev_event_to_string(int32_t _event){
    switch (_event) {
        case SM_EV_EVENT_KEY_CHANGED:
            return "SM_EV_EVENT_KEY_CHANGED";
        case SM_EV_EVENT_LEFT_SIGNAL_CHANGED:
            return "SM_EV_EVENT_LEFT_SIGNAL_CHANGED";
        case SM_EV_EVENT_RIGHT_SIGNAL_CHANGED:
            return "SM_EV_EVENT_RIGHT_SIGNAL_CHANGED";
        case SM_EV_EVENT_PARKING_CHANGED:
            return "SM_EV_EVENT_PARKING_CHANGED";
        case SM_EV_EVENT_DRIVE_MODE_SIGNAL_CHANGED:
            return "SM_EV_EVENT_DRIVE_MODE_SIGNAL_CHANGED";
        case SM_EV_EVENT_DRIVE_MODE_CHANGED:
            return "SM_EV_EVENT_DRIVE_MODE_CHANGED";
        case SM_EV_EVENT_PHASE_LIGHT_CHANGED:
            return "SM_EV_EVENT_PHASE_LIGHT_CHANGED";
        case SM_EV_EVENT_COS_LIGHT_CHANGED:
            return "SM_EV_EVENT_COS_LIGHT_CHANGED";
        case SM_EV_EVENT_RANGE_CHANGED:
            return "SM_EV_EVENT_RANGE_CHANGED";
        case SM_EV_EVENT_BACKWARD_MODE_SIGNAL_CHANGED:
            return "SM_EV_EVENT_BACKWARD_MODE_SIGNAL_CHANGED";
        case SM_EV_EVENT_BACKWARD_MODE_CHANGED:
            return "SM_EV_EVENT_BACKWARD_MODE_CHANGED";
        case SM_EV_EVENT_LOCK_MODE_CHANGED:
            return "SM_EV_EVENT_LOCK_MODE_CHANGED";
        case SM_EV_EVENT_BLOCK_MODE_CHANGED:
            return "SM_EV_EVENT_BLOCK_MODE_CHANGED";
        case SM_EV_EVENT_ANTI_THEFT_MODE_CHANGED:
            return "SM_EV_EVENT_ANTI_THEFT_MODE_CHANGED";
        case SM_EV_EVENT_INACTIVE_MODE_CHANGED:
            return "SM_EV_EVENT_INACTIVE_MODE_CHANGED";
//        case SM_EV_EVENT_ODO_CHANGED:
//            return "SM_EV_EVENT_ODO_CHANGED";
        case SM_EV_EVENT_SPEED_CHANGED:
            return "SM_EV_EVENT_SPEED_CHANGED";
        case SM_EV_EVENT_MC_STATUS_CHANGED:
            return "SM_EV_EVENT_MC_STATUS_CHANGED";
        case SM_EV_EVENT_TRIP_CHANGED:
            return "SM_EV_EVENT_TRIP_CHANGED";
        case SM_EV_EVENT_ERROR_CHANGED:
            return "SM_EV_EVENT_ERROR_CHANGED";
        default:
            break;
//            return "EV_EVENT_UNKNOWN";
    }
};

void sm_sv_ev_on_event(int32_t _event, int32_t _value, void* _arg){
    LOG_INF(MAIN_TAG, "Ev event: %s: %d", convert_ev_event_to_string(_event), _value);
}

void sm_sv_ev_on_bp_event(int32_t _port, int32_t _event, int32_t _value, void* _arg){
    LOG_INF(MAIN_TAG, "BP %d event : %d: %d", _port, _event, _value);
}

void sm_sv_ev_on_cmd(int32_t _success, uint8_t _cmd, void* _data, void* _arg){

}

void ev_cmd_testing(){
    LOG_INF(MAIN_TAG, "/******************** EV COMMAND TESTING ***********************/");
    LOG_INF(MAIN_TAG, "Please choose the character for COMMAND testing!!!!");
    LOG_INF(MAIN_TAG, "Choose '0' for control HORN!!!");
    LOG_INF(MAIN_TAG, "Choose '1' for set DRIVE MODE !!!");
    LOG_INF(MAIN_TAG, "Choose '2' for set MAX SPEED !!!");
    LOG_INF(MAIN_TAG, "Choose '3' for find EV !!!");
    LOG_INF(MAIN_TAG, "Choose '4' for set LOCK PORT !!!");
    LOG_INF(MAIN_TAG, "Choose '5' for set BLOCK PORT !!!");
    LOG_INF(MAIN_TAG, "Choose '6' for config VERIFY PIN offline !!!");
    LOG_INF(MAIN_TAG, "Choose '7' for set LOCK EV !!!");
    LOG_INF(MAIN_TAG, "Choose '8' for set BLOCK EV !!!");
    LOG_INF(MAIN_TAG, "Choose '9' for set ANTI-THEFT EV !!!");
    LOG_INF(MAIN_TAG, "/*******************************************/");

    LOG_INF(MAIN_TAG, "Please Enter command ID first !!!");

    while (getc(stdin) != '\n');
    int c = getchar();

    switch (c) {
        case '0':{
            static uint8_t ctl_horn = 0;
            LOG_INF(MAIN_TAG, "Control Horn command: %d", ctl_horn);
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_CONTROL_HORN, &ctl_horn, sm_sv_ev_on_cmd, g_ev_service);
            ctl_horn = !ctl_horn;
            break;
        }
        case '1':{
            static uint8_t drive_mode = EV_MC_ECO_MODE_1;
            LOG_INF(MAIN_TAG, "Set Drive Mode command: Mode: 0x%x", drive_mode);
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_SET_DRIVE_MODE, &drive_mode, sm_sv_ev_on_cmd, g_ev_service);
            drive_mode ++;
            if(drive_mode >= 5){
                drive_mode = EV_MC_ECO_MODE_1;
            }
            break;
        }
        case '2':{
            LOG_INF(MAIN_TAG, "Set Max Speed command");
            int32_t max_speed = sm_rand(80, 30);
            LOG_INF(MAIN_TAG, "New Max Speed is SET: %d", max_speed);
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_SET_MAX_SPEED, &max_speed, sm_sv_ev_on_cmd, g_ev_service);
            break;
        }
        case '3':{
            LOG_INF(MAIN_TAG, "Find EV command");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_FIND_EV, NULL, sm_sv_ev_on_cmd, g_ev_service);
            break;
        }
        case '4':{
            static uint8_t lock_port = 0;
            LOG_INF(MAIN_TAG, "LOCK PORT command: %s", lock_port ? "LOCK" : "UNLOCK");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_SET_LOCK_PORT, &lock_port, sm_sv_ev_on_cmd, g_ev_service);
            lock_port = !lock_port;
            break;
        }
        case '5':{
            static uint8_t block_bp = 0;
//            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_SET_BLOCK_BP, &block_bp, sm_sv_ev_on_cmd, g_ev_service);
            block_bp = !block_bp;
            break;
        }
        case '6':{
            static uint8_t config_verify = 0;
            LOG_INF(MAIN_TAG, "Config verify bp offline: %s", config_verify ? "ON" : "OFF");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_CONFIG_VERIFY_BP_OFFLINE, &config_verify, sm_sv_ev_on_cmd, g_ev_service);
            config_verify = !config_verify;
            break;
        }
        case '7':{
            static uint8_t lock_ev = 0;
            LOG_INF(MAIN_TAG, "LOCK EV command: %s", lock_ev ? "LOCK" : "UNLOCK");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_LOCK_EV, &lock_ev, sm_sv_ev_on_cmd, g_ev_service);
            lock_ev = !lock_ev;
            break;
        }
        case '8':{
            static uint8_t block_ev = 0;
            LOG_INF(MAIN_TAG, "BLOCK EV command: %s", block_ev ? "BLOCK" : "UNBLOCK");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_BLOCK_EV, &block_ev, sm_sv_ev_on_cmd, g_ev_service);
            block_ev = !block_ev;
            break;
        }
        case '9':{
            static uint8_t anti = 0;
            LOG_INF(MAIN_TAG, "ANTI-THEFT EV command: %s", anti ? "LOCK" : "UNLOCK");
            sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_ANTI_THEFT_EV, &anti, sm_sv_ev_on_cmd, g_ev_service);
            anti = !anti;
            break;
        }
        default:{
            LOG_ERR(MAIN_TAG, "EV Command NOT Support!!!");
            break;
        }
    }
}

void bp_cmd_testing(){
    LOG_INF(MAIN_TAG, "/******************** BP COMMAND TESTING ***********************/");
    LOG_INF(MAIN_TAG, "Please choose the character for COMMAND testing!!!!");
    LOG_INF(MAIN_TAG, "Choose '0' for write Device SN to BP !!!");
    LOG_INF(MAIN_TAG, "Choose '1' for read Device SN from BP !!!");
    LOG_INF(MAIN_TAG, "Choose '2' for read BP SN !!!");
    LOG_INF(MAIN_TAG, "Choose '3' for read BP version !!!");
    LOG_INF(MAIN_TAG, "Choose '4' for reboot BP !!!");

    LOG_INF(MAIN_TAG, "Please Enter command ID first !!!");

    while (getc(stdin) != '\n');
    int c = getchar();

    switch (c) {
        case '0':{
            LOG_INF(MAIN_TAG, "Write Device SN to BP");
            break;
        }
        case '1':{
            LOG_INF(MAIN_TAG, "Read Device SN from BP");
            break;
        }
        case '2':{
            LOG_INF(MAIN_TAG, "Read BP SN");
            break;
        }
        case '3':{
            LOG_INF(MAIN_TAG, "Read BP Version");
            break;
        }
        case '4':{
            LOG_INF(MAIN_TAG, "Reboot BP");
            break;
        }
    }
}

/*void print_ev_data(const sm_ev_data_t* _ev_data){
    for(int index = 0; index < EV_DATA_NUMBER; index++){
        if(_ev_data[index].m_data) {
            LOG_INF(MAIN_TAG, "EV Data index %d: %d", _ev_data[index].m_index, *((int32_t *) (_ev_data[index].m_data)));
        }
    }
}*/

void print_bp_data(const sm_bp_data_t* _bp_data, int32_t _bp_slot){
    LOG_INF(MAIN_TAG, "BP %d info", _bp_slot);
    LOG_INF(MAIN_TAG, "Serial number: %s", _bp_data->m_sn);
    char version[10];
    sm_ev_version_to_string(_bp_data->m_version, version);
    LOG_INF(MAIN_TAG, "Version: %s", version);
    LOG_INF(MAIN_TAG, "Device SN: %s", _bp_data->m_assignedSn);
    LOG_INF(MAIN_TAG, "SOC: %d", _bp_data->m_soc);
    LOG_INF(MAIN_TAG, "SOH: %d", _bp_data->m_soh);
    LOG_INF(MAIN_TAG, "Voltage: %d", _bp_data->m_vol);
    LOG_INF(MAIN_TAG, "Current: %d", _bp_data->m_cur);
    LOG_INF(MAIN_TAG, "Cycle: %d", _bp_data->m_cycle);
    LOG_INF(MAIN_TAG, "Status: %d", _bp_data->m_status);
    LOG_INF(MAIN_TAG, "State: %d", _bp_data->m_state);
    LOG_INF(MAIN_TAG, "Temperatures: %d, %d, %d, %d, %d, %d",
            _bp_data->m_temps[0],
            _bp_data->m_temps[1],
            _bp_data->m_temps[2],
            _bp_data->m_temps[3],
            _bp_data->m_temps[4],
            _bp_data->m_temps[5]);
    LOG_INF(MAIN_TAG, "Cells Voltage: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
            _bp_data->m_cellVols[0],
            _bp_data->m_cellVols[1],
            _bp_data->m_cellVols[2],
            _bp_data->m_cellVols[3],
            _bp_data->m_cellVols[4],
            _bp_data->m_cellVols[5],
            _bp_data->m_cellVols[6],
            _bp_data->m_cellVols[7],
            _bp_data->m_cellVols[8],
            _bp_data->m_cellVols[9],
            _bp_data->m_cellVols[10],
            _bp_data->m_cellVols[11],
            _bp_data->m_cellVols[12],
            _bp_data->m_cellVols[13],
            _bp_data->m_cellVols[14],
            _bp_data->m_cellVols[15]
            );

}

void print_help(const char _c){
    switch (_c) {
        case '0': {
            /*const sm_ev_data_t* ev_data = sm_sv_ev_get_data(g_ev_service);
            print_ev_data(ev_data);*/
            break;
        }
        case '1': {
            for(int index = 0; index < 3; index++) {
                const sm_bp_data_t *bp_data = sm_sv_ev_get_bp_data(g_ev_service, index);
                if(bp_data){
                    print_bp_data(bp_data, index);
                }
            }
            break;
        }
        case '2': {
            ev_cmd_testing();
            break;
        }
        case '3': {
            bp_cmd_testing();
            break;
        }
        default:
            LOG_WRN(MAIN_TAG, "Please choose right character !!!");
            break;
    }
}

static void sm_ev_service_thread(void* _arg);
static void sm_co_interface_thread(void* _arg);
static void sm_co_service_thread(void* _arg);

void test_pmu_lock(){
    static uint8_t value = 1;

    LOG_DBG(MAIN_TAG, "Test PMU Lock");
    sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_LOCK_EV, &value, sm_sv_ev_on_cmd, g_ev_service);
    value = !value;
}

void test_pmu_block(){
    static uint8_t value = 1;

    LOG_DBG(MAIN_TAG, "Test PMU BLock");
    sm_sv_ev_set_cmd(g_ev_service, SM_EV_CMD_BLOCK_EV, &value, sm_sv_ev_on_cmd, g_ev_service);
    value = !value;
}
sm_sv_ev_event_t g_ev_event_handle = {
        .on_ev_event = sm_sv_ev_on_event,
        .on_bp_event = sm_sv_ev_on_bp_event
};

int main(int argc, char** argv){
    sm_logger_init(sm_bsp_linux_logger_put, LOG_LEVEL_DEBUG);
    LOG_INF(MAIN_TAG, "EV service testing !!!!!!!!!!");

    g_co_if = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF, CO_HOST_DEFAULT, CO_PORT_DEFAULT, NULL);
    if(!g_co_if){
        LOG_ERR(MAIN_TAG, "Created CanOpen interface FAILURE");
        return EXIT_FAILURE;
    }

    g_co_core = sm_co_create(8, false, g_co_if);
    if(!g_co_core){
        LOG_ERR(MAIN_TAG, "Created CanOpen CORE FAILURE");
        return EXIT_FAILURE;
    }

    g_bp_manager = sm_sv_bp_create(SM_SV_BP_NUMBER_DEFAULT, g_co_core, false, NULL);
    if(!g_bp_manager){
        LOG_ERR(MAIN_TAG, "Created BP Management FAILURE");
        return EXIT_FAILURE;
    }

    g_ev_service = sm_sv_ev_create(g_co_core,
                                   g_bp_manager,
                                   sm_ev_get_opt_storage(),
                                   sm_get_config_ev_storage());
    if(!g_ev_service){
        LOG_ERR(MAIN_TAG, "Created EV Service FAILURE");
        return EXIT_FAILURE;
    }
    sm_sv_ev_reg_event(g_ev_service, &g_ev_event_handle, g_ev_service);

    threadpool thread_pool = thpool_init(4);
    thpool_add_work(thread_pool, sm_ev_service_thread, g_ev_service);
    thpool_add_work(thread_pool, sm_co_service_thread, g_co_core);
    thpool_add_work(thread_pool, sm_co_interface_thread, g_co_if);

    while (1){
        LOG_INF(MAIN_TAG, "/******************** STARTING ******************* 555/");
        LOG_INF(MAIN_TAG, "Please choose the character for testing !!!!");
        LOG_INF(MAIN_TAG, "Choose '0' for read EV ev_data testing !!!");
        LOG_INF(MAIN_TAG, "Choose '1' for read BP ev_data testing !!!");
        LOG_INF(MAIN_TAG, "Choose '2' for execute EV CMD testing !!!");
        LOG_INF(MAIN_TAG, "Choose '3' for execute BP CMD testing !!!");

        LOG_INF(MAIN_TAG, "Please Enter character for testing !!!");
        int c = getchar();

        print_help(c);

        while (getc(stdin) != '\n');

        LOG_INF(MAIN_TAG, "/******************* END *******************/");
        LOG_INF(MAIN_TAG, "\n\n");
    }
}

static void sm_ev_service_thread(void* _arg){
    sm_sv_ev_t* ev_service = (sm_sv_ev_t*)(_arg);
    elapsed_timer_t timer = {
            .m_duration = 1000,
            .m_start_time = (int32_t)get_tick_count()
    };
    while (1){
        sm_sv_ev_process(ev_service);
        if(!elapsed_timer_get_remain(&timer)){
//            test_pmu_block();
            elapsed_timer_reset(&timer);
        }
    }
}

static void sm_co_interface_thread(void* _arg){
    sm_co_if_t* co_if = (sm_co_if_t*)_arg;
    while (1){
        sm_co_if_process(co_if);
    }
}

static void sm_co_service_thread(void* _arg){
    sm_co_t* core_co = (sm_co_t*)_arg;
    elapsed_timer_t co_timer = {
            .m_duration = 1,
            .m_start_time = get_tick_count()
    };
    while (1){
//        sm_co_if_process(g_co_if);
        if(!elapsed_timer_get_remain(&co_timer)) {
            sm_co_process(core_co, 1);
            elapsed_timer_reset(&co_timer);
        }
    }
}