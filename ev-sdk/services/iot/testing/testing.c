#include <sm_string_util.h>
#include "sm_ec200.h"
#include "sm_logger.h"
#include "pthread.h"
#include "linux/utils/linux_serial.h"
#include "sm_ec200_mqtt.h"
#include "sm_elapsed_timer.h"
#include "sm_mqtt_client.h"
#include "sm_sv_iot.h"
#include "sm_hal.h"
#include "string.h"
#include "sm_bp_data.h"
#include "tiny-json/tiny-json.h"
#include "json-maker/json-maker.h"
#include "sm_ev_data.h"

#define HOST_NAME   "hub.selex.vn"
#define CLIENT_ID   "CM005"
#define TOPIC       "test"
int32_t g_fd;
static const char* TAG = "IOT_TESTING";
static const char* SUB_TAG = "###";

sm_hal_uart_t *g_lte_uart;
sm_hal_io_t *g_lte_reset;
sm_modem_t *g_modem;
sm_ec200_t g_ec200;
sm_mqtt_client_t * mqtt_client;

typedef struct {
    int8_t m_enable;
    int8_t m_value;
    void* m_exval;
} flag_t;
typedef struct {
    flag_t m_connection_flag;
    flag_t m_recv_cmd;
    flag_t m_other_flag;
} test_flag_t;

volatile test_flag_t global_test_flag = {
    .m_connection_flag = {
        .m_value = 0,
        .m_enable = 0
    },
    .m_recv_cmd = {
        .m_value = 0,
        .m_enable = 0
    },
    .m_other_flag = {
        .m_value = 0,
        .m_enable = 0
    },
};

uint8_t only_sub_tag = 1;
void log_puts(char* _str) {
    // if (strstr(_str, "ERROR"))
    //     {
    //     //
    // }else
    {
        if (strstr(_str, "MODEM")) return ;
        if (strstr(_str, "QMTRECV")) return ;
        if (strstr(_str, "MQTT NO ev_data")) return ;
        if (strstr(_str, "MQTT Client connection")) return ;
        if (strstr(_str, "HOST EV")) return ;
        // if (only_sub_tag && strstr(_str, SUB_TAG) == NULL) return;
        if (strstr(_str, "sm_sv_iot")) return ;
    }


    printf("%s\n", _str);
}

void on_connected(int _success, void* _arg);
void on_disconnected(int _success, void* _arg);
void on_subscribed(int _success, void* _arg);
void on_unsubscribed(int _success, void* _arg);
void on_published(int _success, void* _arg);

void iot_on_bp_new_fw(void *arg) {
    LOG_DBG(TAG, "NEW BP FW IS AVAILABLE");
}
int32_t iot_on_bp_req_upgrade(void*arg) {
    LOG_DBG(TAG, "BP REQ UPGARDE");
  return 0;
}
char* iot_on_ev_new_fw(const char* module , const char* new_vs,
    int32_t size, uint16_t crc, const char* link, int32_t* bypass, void* arg) {
    if (strstr(module, "mc")) {
        LOG_INF(TAG, "NEW EV FW MC IS AVAILABLE \n - size: %d\n - crc: %d\n - link: %s\n - bypass: %d"
                                                            , size, crc, link, *bypass);
         return "mc_current_version";
    }
     if (strstr(module, "pmu")){
        LOG_INF(TAG, "NEW EV FW PMU IS AVAILABLE \n - size: %d\n - crc: %d\n - link: %s\n - bypass: %d"
                                                    , size, crc, link, *bypass);
        return "pmu_current_version";
    }
    if (strstr(module, "hmi")){
        LOG_INF(TAG, "NEW EV FW HMI IS AVAILABLE \n - size: %d\n - crc: %d\n - link: %s\n - bypass: %d"
                                                    , size, crc, link, *bypass);
        return "hmi_current_version";
    }
}
int32_t iot_on_ev_req_upgrade(void*arg) {
    LOG_DBG(TAG, "EV REQ UPGARDE :\n %s ", (char *)arg);

  return 0;
}
void iot_on_bp_event(const char* bp_sn, int32_t accepted, void* arg) {
    if (accepted) {
        LOG_DBG(TAG, "BP EV ACCEPTED : %s ", (char *)bp_sn);
    }else {
        LOG_DBG(TAG, "BP EV REJECTED:\n %s ",(char *)bp_sn);
    }
}

sm_sv_iot_config_t iot_cfg = {
    .m_client_id = CLIENT_ID, .m_host = HOST_NAME, .m_port = 1883, .m_password = "", .m_username = "",
    .m_sync_time = 10,

};

sm_sv_iot_event_t iot_event = {
    .on_bp_new_fw = iot_on_bp_new_fw,
    .on_bp_request_upgrade = iot_on_bp_req_upgrade,
    .on_ev_new_fw = iot_on_ev_new_fw,
    .on_ev_request_upgrade = iot_on_ev_req_upgrade,
    .on_bp_event = iot_on_bp_event,
};
sm_sv_iot_t *sv_iot;

char ev_info[1024];
char* host_get_ev_info(int32_t arg, void* _arg) {
    LOG_DBG(TAG, "HOST GET EV INFO");
    return "EV INFO";
}

sm_mc_data_t g_ev_mc_data = {
    .m_status = 0,
    .m_speed_kmh_mul_10 = 1,
    .m_trip_m = 2,
    .m_motor_temp = 3,
    .m_err_code = 4,
    .m_reverse_state = 5,
    .m_thr_cmd = 6,
    .m_est_tor = 7,
    .m_est_dc_cur = 8,
    .m_allow_dc_cur = 9,
    .m_cur_mode = 0,
    .m_board_temp = 1,
    .m_revol_cnt = 2,
    .m_aver_effic = 3,
    .m_enr_con_effic = 4,
    .m_enr_dur_trip = 5,
    .m_anti_theft_st = 6,
};

sm_pmu_data_t g_ev_pmu_data = {
    .m_key = 0,
    .m_horn = 0,
    .m_brake = 0,
    .m_drive_mode_signal = 0,
    .m_backward_mode_signal = 0,
    .m_uphill_mode = 0,
    .m_lock_status = 0,
    .m_block_status = 0,
    .m_anti_theft_status = 0,
    .m_inactive_mode = 0,
    .m_left_signal = 0,
    .m_right_signal = 0,
    .m_parking = 0,
    .m_low_beam_state = 0,
    .m_high_beam_state = 0,
    .m_range = 1000,
    .m_err_code = 0,
    .m_power_in = 1,
    .m_power_out = 1,
    .m_power_per_km = 35,
    .m_abp_voltage = 12,
    .m_bp_checking_state = {0,0,0},
    .m_port_lock_status = {0,0,0},
    .m_charge_cur_lim = 10,
    .m_discharge_cur_lim = 50,
};

void* host_get_ev_data(int32_t arg, void* _arg) {
    LOG_DBG(TAG, "HOST GET EV DATA");
    if (arg == SM_EV_MODULE_MC) {
        return (void*)&g_ev_mc_data;
    }else if (arg == SM_EV_MODULE_PMU) {
        return (void*) &g_ev_pmu_data;
    }
}
sm_bp_data_t g_bp[3] = {
    {
        .m_sn = "101BE1R001000",
        .m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
        .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
        .m_version = "111",
        .m_assignedSn = CLIENT_ID
    },{
        // .m_sn = "101BE1R001001",
        .m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
    .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
    .m_version = "111",
    .m_assignedSn = CLIENT_ID
    },{
        // .m_sn = "101BE1R001002",
        .m_vol = 64800, .m_cur = 20,    .m_state = 4,    .m_status = 0,    .m_soc = 75,    .m_soh = 90,    .m_cycle = 500,    .m_temps = {25, 26, 27, 28,  27, 28},
        .m_cellVols = {3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620, 3630, 3640,3600, 3605, 3610, 3620},
        .m_version = "111",
        .m_assignedSn = CLIENT_ID
    }
};
sm_bp_data_t* host_get_ev_bp_data(int32_t id, void* _arg) {
    LOG_DBG(TAG, "HOST GET EV BP DATA");
    return &g_bp[id];
}
sm_ev_opt_t* host_get_ev_opt_data(void* _arg) {
    LOG_DBG(TAG, "HOST GET EV OPT DATA");
    static sm_ev_opt_t ev_opt;
    ev_opt.m_driver_mode = 0;
    ev_opt.m_max_speed = 100;
    ev_opt.m_err = 0;
    return &ev_opt;
}
char* host_get_ev_cfg(void* _arg) {
    LOG_DBG(TAG, "HOST GET EV CFG DATA");
    return "CFG_DATA";
}
sm_host_sync_data_if_t host_api_if = {
    .get_ev_bp_data = host_get_ev_bp_data,
    .get_ev_data = host_get_ev_data,
    .get_ev_info = host_get_ev_info,
    .get_ev_opt_data = host_get_ev_opt_data,
    .get_ev_config = host_get_ev_cfg,
};
uint8_t mqtt_connected = 0;
void host_on_conn(int32_t _success, void* _arg) {
    if (!_success) {
        LOG_DBG(TAG, "Connected to host");
        mqtt_connected = 1;
        // if (global_test_flag.m_connection_flag.m_enable)
        //     global_test_flag.m_connection_flag.m_value = 1;
    }
    else {
        LOG_ERR(TAG, "Connection to host failed");
    }
}
void host_on_disconnect(int32_t _success, void* _arg) {
    if (!_success)
        LOG_DBG(TAG, "Disconnected from host");
    else
        LOG_ERR(TAG, "Disconnection to host failed");
}

const char* host_cmd_str[] = {
    "SM_HOST_CMD_FIND_EV",
    "SM_HOST_CMD_BLOCK_EV",
    "SM_HOST_CMD_LOCK_EV",
    "SM_HOST_CMD_SET_DRIVE_MODE",
    "SM_HOST_CMD_SET_MAX_SPEED",
    "SM_HOST_CMD_SET_ACTIVE",
    "SM_HOST_CMD_SET_LOCK_PORT",
    "SM_HOST_CMD_SET_UNLOCK_PORT",
    "SM_HOST_CMD_CONFIG_VERIFY_BP_OFFLINE",
    "SM_HOST_CMD_NUMBER",
};
void host_on_cmd(int32_t _cmd, void* _ret, void* _arg) {
    LOG_INF(TAG, "HOST ON CMD: %s - %d", host_cmd_str[_cmd], *(int8_t*)_ret);
    global_test_flag.m_recv_cmd.m_value = _cmd;
}
sm_host_event_t g_host_event = {
    .on_connected = host_on_conn,
    .on_disconnected = host_on_disconnect,
    .on_cmd_from_host = host_on_cmd,
};
sm_host_api_t *host_api;
volatile uint8_t host_api_created =0;

void init() {
    g_lte_uart = sm_hal_uart_init(&g_fd, 115200, 0, 0);
    g_lte_reset = sm_hal_io_init(0);
    sm_ec200_init(&g_ec200, g_lte_reset, g_lte_uart);
    g_modem = &g_ec200.base;

    CLIENT: mqtt_client = sm_mqtt_init(&g_ec200);
    if (!mqtt_client) {
        LOG_ERR(TAG, "MQTT_CLIENT INIT FAIL");
        goto CLIENT;
    }
    while (sv_iot == NULL) {
        sv_iot = sm_sv_iot_create(CLIENT_ID, mqtt_client, &iot_cfg, &iot_event, sv_iot);
        sm_hal_delay_ms(10);
    }
    host_api = (sm_host_api_t*)sv_iot;
    host_api->m_proc->init(host_api, &host_api_if, 5);
    sm_host_api_reg_event_handle(host_api, &g_host_event, host_api);
    host_api_created = 1;
    LOG_DBG(TAG, "READY");
}

void uart_process(sm_hal_uart_t *_this, void* _arg) {
    if (!_this) return;
    if (serial_is_open(g_fd)) {
        static uint8_t data[129] = {0,};
        memset(data, '\0',129);
        int32_t len = serial_recv_bytes(g_fd, data, 128);
        if (len >0) {
            for(int i=0; i<len; i++) {
                sm_hal_uart_rx_irq(_this, data[i]);
            }
            // printf("%s", ev_data);
        }
    }
}

void mqtt_process(sm_ec200_t *_this) {
    if (_this) {
        sm_mqtt_client_process(mqtt_client);
    }
}
void *mcu(void *_arg) {
    while (1) {
        uart_process(g_lte_uart, _arg);
        sm_hal_delay_ms(10);
    }
}

void *app(void* _arg) {
    while(1) {
        if (mqtt_connected) {
            mqtt_process(&g_ec200);
            sm_hal_delay_ms(1);
        }
        if (host_api_created) {
            host_api->m_proc->process(host_api);
            sm_hal_delay_ms(50);
        }
    }
}

char buff[128] = {0,};
char *p_buff = buff;
void test_connection(uint32_t _times);
void test_recv_cmd(uint32_t _times);

int main(void){
    sm_logger_init(log_puts, LOG_LEVEL_DEBUG);

    pthread_t mcu_thread_id;
    pthread_t app_process_id;
    pthread_create(&mcu_thread_id, NULL, mcu, NULL);
    pthread_create(&app_process_id, NULL, app, NULL);

    init();
    char ch;
    while (1)
    {
        if (!mqtt_connected) continue;
        printf("---------------TEST SITUATION---------------\n");
        printf("    a : disconnect x times\n");
        printf("    s : find ev \n");
        printf("    d : drive mode\n");
        printf("    f : max speed\n");
        printf("    g : active\n");
        printf("    h : lock_port\n");
        printf("    j : unlock_port\n");
        printf("    k : unlock_port\n");
        printf("    l : unlock_port\n");
        printf("    ; : unlock_port\n");
        printf("    ' : unlock_port\n");
        printf("    \\ : unlock_port\n");

        scanf("%c", &ch);
        while(ch == '\n')
            scanf("%c", &ch);
        switch (ch) {
            case 'q': {
                if (!sm_mqtt_connect(mqtt_client, HOST_NAME, 1883, CLIENT_ID, "", "", on_connected, NULL)) {

                    LOG_DBG(TAG, "sm_mqtt_connect SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_connect FAIL");
                }
                break;
            }
            case 'w': {
                if (!sm_mqtt_disconnect(mqtt_client, on_disconnected, NULL)) {
                    LOG_DBG(TAG, "sm_mqtt_disconnect SUCC");
                }else {
                    LOG_ERR(TAG, "sm_mqtt_disconnect FAIL");
                }
                break;
            }
            case 'e':{
                    char cmd[128] = {'\0',};
                    memset(cmd, '\0', 128);
                    scanf("%s",cmd);
                    if (cmd[strlen(cmd)-1] == '\n') {
                        cmd[strlen(cmd)-1] = '\0';
                    }
                    uint8_t len = strlen(cmd);
                    cmd[len] = '\r';
                    cmd[len+1] = '\n';
                    cmd[len+2] = '\0';
                    printf("%s",cmd);
                    sm_modem_send_cmd(g_modem, cmd, "OK\r\n","OK\r\n", 2000);
                break;
                }
            case 'r': {
                sm_sv_iot_notify_ota_upgrade_status(sv_iot, "ev", "abcdefgh", 0, "NULL");
                break;
            }
            case 't':{
                for (int i = 0; i < 100; i++) {
                    sm_sv_iot_notify_ota_download_status(sv_iot, "ev", i, (i==99), 0, "NULL");
                    sm_hal_delay_ms(2000);
                }
                break;
            }
            case 'y':
                break;
            case 'u':
                break;
            case 'i':
                break;
            case 'o':
                break;
            case 'p':
                break;
            case 'a': {
                printf(" ENTER TIMES: ");
                uint32_t times;
                scanf("%d",&times);
                test_connection(times);
            }
                break;
            case 's': {
                printf(" ENTER TIMES: ");
                uint32_t times;
                scanf("%d",&times);
                test_recv_cmd(times);
            }
                break;
            case 'd':
                break;
            case 'f':
                break;
            case 'g':
                break;
            case 'h':
                break;
            case 'j':
                break;
            case 'k':
                break;
            case 'l':
                break;
            case 'z':
                host_api->m_proc->sync_pause(host_api);
                break;
            case 'x':
                host_api->m_proc->sync_resume(host_api);
                break;
            case 'c':
                if (host_api->m_proc->push_event_to_host(host_api, g_bp[0].m_sn)) {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }else {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }
                break;
            case 'v':
                if (host_api->m_proc->push_event_to_host(host_api, g_bp[1].m_sn)) {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }else {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }
                break;
            case 'b':
                if (host_api->m_proc->push_event_to_host(host_api, g_bp[2].m_sn)) {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }else {
                    LOG_DBG(TAG, "Push BP connection event HOST");
                }
                break;
            case 'n':
                break;
            case 'm':
                if (only_sub_tag)
                    only_sub_tag = 0;
                else
                    only_sub_tag = 1;
                break;
            case '0':
                if (host_api_created)
                    host_api_created = 0;
                else
                    host_api_created = 1;
            default:
                break;
        }
    }
    return 0;
}
void on_connected(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on connected");
    mqtt_connected = 1;
}

void on_disconnected(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on disconnected, Reconnecting to broker");
}

void on_subscribed(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on subscribed %s ", !_success ? "SUCCESS":"FAILURE" );
}

void on_unsubscribed(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on unsubscribed");
}

void on_published(int _success, void* _arg){
    LOG_DBG(TAG, "MQTT Client on published");
}

void test_connection(uint32_t _times) {
    only_sub_tag = 0;
    LOG_INF(SUB_TAG, "TEST CONNECTION START : %d", get_tick_count());
    uint32_t passed = 0;
    bool is_passed  = false;
    for (int i = 0; i < _times; i++){
        //start
        global_test_flag.m_connection_flag.m_enable = 1;
        is_passed  = false;
        sm_mqtt_disconnect(mqtt_client, on_disconnected, NULL);
        global_test_flag.m_connection_flag.m_value = 0;
        //wait
        uint32_t time_cnt = get_tick_count();
        while (get_tick_count() - time_cnt < 30000) {
            if (global_test_flag.m_connection_flag.m_value == 1)
            {
                is_passed = true;
                break;
            }
        }
        //update
        if (is_passed)
            passed++;
        else
            LOG_WRN(TAG, "+1 fail times");
        sm_hal_delay_ms(15*1000);
    }
    global_test_flag.m_connection_flag.m_enable = 0;
    LOG_INF(SUB_TAG, "TEST CONNECTION FINISH : %d", get_tick_count());
    LOG_INF(SUB_TAG, "TEST CONNECTION RESULT: %d / %d %s",
                    passed,
                    _times,
                    _times-passed ? "FAIL" : "PASS");
    only_sub_tag=1;
}

void test_recv_cmd(uint32_t _times) {
    /*only_sub_tag = 0;
    LOG_INF(SUB_TAG, "TEST RECV COMMAND START : %d", get_tick_count());
    uint32_t passed = 0;
    bool is_passed  = false;
    for (int i = 0; i < _times; i++){
        //start
        global_test_flag.m_recv_cmd.m_enable = 1;
        is_passed  = false;

        char buff[128] = {'\0'};
        char* j = buff;
        j = json_objOpen(j,NULL);
        j = json_objOpen(j, "state");
        j = json_int(j, "find_ev", 1);
        j = json_objClose(j);
        j = json_end(j);

        // if (sm_mqtt_publish(mqtt_client, "selex/ev/CM0005/command", j,0,0,on_published,NULL))
        // {
        //     LOG_WRN(TAG, "can't send cmd");
        //     continue;
        // }

        global_test_flag.m_recv_cmd.m_value = -1;
        //wait
        uint32_t time_cnt = get_tick_count();
        while (get_tick_count() - time_cnt < 5000) {
            if (global_test_flag.m_recv_cmd.m_value == SM_HOST_CMD_FIND_EV)
            {
                is_passed = true;
                break;
            }
        }
        //update
        if (is_passed)
            passed++;
        else
            LOG_WRN(TAG, "+1 fail times");
        sm_hal_delay_ms(5*1000);
    }
    global_test_flag.m_recv_cmd.m_enable = 0;
    LOG_INF(SUB_TAG, "TEST RECV COMMAND FINNISH : %d", get_tick_count());
    LOG_INF(SUB_TAG, "TEST RECV COMMAND RESULT: %d / %d %s",
                    passed,
                    _times,
                    _times-passed ? "FAIL" : "PASS");
    only_sub_tag=1;*/
}
