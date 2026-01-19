//
// Created by Ng Thuy Quynh on 8/21/2024.
//
#include <sm_logger.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "sm_elapsed_timer.h"
#include "sm_pmu_api.h"
#include "porting/sm_bsp_pmu.h"
#include "pthread.h"
#include "test_api_event.h"

void log_print(char* _str) {
    printf("%s\n", _str);
}
sm_pmu_event_t g_pmu_event = {
    .on_io_key_changed = pmu_on_io_key_changed,
    .on_io_parking_bt_press = pmu_on_io_parking_bt_press,
    .on_io_parking_bt_release = pmu_on_io_parking_bt_release,
    .on_io_parking_bt_hold = pmu_on_io_parking_bt_hold,
    .on_io_mode_bt_press = pmu_on_io_mode_bt_press,
    .on_io_mode_bt_release = pmu_on_io_mode_bt_release,
    .on_io_mode_bt_hold = pmu_on_io_mode_bt_hold,

    .on_io_left_signal_status_changed = pmu_on_io_left_signal_status_changed,
    .on_io_right_signal_status_changed = pmu_on_io_right_signal_status_changed,
    .on_io_high_beam_status_changed = pmu_on_io_high_beam_status_changed,
    .on_io_low_beam_status_changed = pmu_on_io_low_beam_status_changed,
    .on_io_emergency_status_changed = pmu_on_io_emergency_status_changed,

    // CHARGER
    .on_charger_plugged = pmu_on_charger_plugged,
    .on_charged = pmu_on_charged,
    // BP
    .on_bp_connected = pmu_on_bp_connected,
    .on_bp_disconnected = pmu_on_bp_disconnected,
    .on_bp_update_data = pmu_on_bp_update_data,
    // PMS
    .on_pms_switch_merge = pmu_on_pms_switch_merge,
    .on_pms_update_est_data = pmu_on_pms_update_est_data,

};

int64_t get_tick_count(){
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int64_t )tick;
}

void* pmu_thread() {
    sm_pmu_api_process();
}
int main() {
    sm_logger_init(log_print, LOG_LEVEL_DEBUG);

    sm_bsp_pmu_init();
    sm_pmu_api_init(&g_pmu_event);

    pthread_t pid;
    pthread_create(&pid, NULL, pmu_thread, NULL);


    char ch;
    while (1) {
        ch = getchar();
        switch (ch) {
            case 'q':
                LOG_DBG(TAG, "Key st: %d",sm_pmu_api_io_get_key_status());
                break;
            case 'w':
                LOG_DBG(TAG, "Left st: %d",sm_pmu_api_io_get_left_signal_status());
                break;
            case 'e':
                LOG_DBG(TAG, "Right st: %d",sm_pmu_api_io_get_right_signal_status());
                break;
            case 'r':
                LOG_DBG(TAG, "High beam st: %d",sm_pmu_api_io_get_high_beam_status());
                break;
            case 't':
                LOG_DBG(TAG, "Low beam st: %d",sm_pmu_api_io_get_low_beam_status());
                break;
            case 'y':
                LOG_DBG(TAG, "Emergency st: %d", sm_pmu_api_io_get_emergency_status());
                break;
            case 'u':
                LOG_DBG(TAG, "Parking st: %d", sm_pmu_api_io_get_parking_button_status());
                break;
            case 'i':
                LOG_DBG(TAG, "DRV mode st: %d", sm_pmu_api_io_get_mode_button_status());
                break;
            case 'o':
                LOG_DBG(TAG,"Horn st: %d", sm_pmu_api_io_get_horn_status());
                break;
            case 'p':
                LOG_DBG(TAG, "Set left on");
                sm_pmu_api_io_set_left_signal(1);
                break;
            case 'a':
                LOG_DBG(TAG, "Set left off");
                sm_pmu_api_io_set_left_signal(0);
                break;
            case 's':
                LOG_DBG(TAG, "Set left blink");
                sm_pmu_api_io_blink_left_signal(10, 1000);
                break;
            case 'd':
                LOG_DBG(TAG, "Release left %d", sm_pmu_api_io_reset_left_signal());
                break;
            case 'f':
                LOG_DBG(TAG, "Set right on");
                sm_pmu_api_io_set_right_signal(1);
                break;
            case 'g':
                LOG_DBG(TAG, "Set right off");
                sm_pmu_api_io_set_right_signal(0);
                break;
            case 'h':
                LOG_DBG(TAG, "Set right blink");
                sm_pmu_api_io_blink_right_signal(10, 1000);
                break;
            case 'j':
                LOG_DBG(TAG, "Release right %d", sm_pmu_api_io_reset_right_signal());
                break;
            case 'k':
            LOG_DBG(TAG, "Set hight beam on");
            sm_pmu_api_io_set_high_beam(1);
                break;
            case 'l':
                LOG_DBG(TAG, "Set high beam off");
            sm_pmu_api_io_set_high_beam(0);
                break;
            case 'z':
                LOG_DBG(TAG, "Set high beam blink");
            sm_pmu_api_io_blink_high_beam(10, 1000);
                break;
            case 'x':
                LOG_DBG(TAG, "Release high beam %d", sm_pmu_api_io_reset_high_beam());
                break;
            case 'c':
                LOG_DBG(TAG, "Set low beam on");
            sm_pmu_api_io_set_low_beam(1);
                break;
            case 'v':
                LOG_DBG(TAG, "Set low beam off");
            sm_pmu_api_io_set_low_beam(0);
                break;
            case 'b':
                LOG_DBG(TAG, "Set low beam blink");
            sm_pmu_api_io_blink_low_beam(10, 1000);
                break;
            case 'n':
                LOG_DBG(TAG, "Release low beam %d", sm_pmu_api_io_reset_low_beam());
                break;
            case 'm':
                LOG_DBG(TAG, "Set horn on");
            sm_pmu_api_io_set_horn(1);
                break;
            case '1':
                LOG_DBG(TAG, "Set low beam off");
            sm_pmu_api_io_set_horn(0);
                break;
            case '2':
                LOG_DBG(TAG, "Set horn blink");
                sm_pmu_api_io_blink_horn(10, 1000);
                break;
            case '3':
                LOG_DBG(TAG, "Assign bp 0");
            sm_pmu_api_bp_auth(0);
                break;
            case '4':
                LOG_DBG(TAG, "Find first bp");
            sm_pmu_api_first_bp_auth(0);
                break;
            case '5':
                LOG_DBG(TAG, "Get BP num: %d", sm_pmu_api_bp_get_number());
                break;
            case '6':
                break;
            case '7':
                LOG_DBG(TAG, "Disable bp 0");
                sm_pmu_api_disable_bp(0);
                break;
            case '8':
                LOG_DBG(TAG, "Enable bp 0");
                sm_pmu_api_enable_bp(0);
                break;
            case '9':
                LOG_DBG(TAG, "Force to bp 0");
                sm_pmu_api_force_bp(0);
                break;
            case '0':
                LOG_DBG(TAG, "Release all bp");
                sm_pmu_api_release_all_bp(0);
                break;
            case '-':
                break;
            case '=':
                break;
            case '[':
                break;
            case ']':
                break;
            case ';':
                break;
            case '/':
                break;
            case '.':
                break;
            case ',':
                break;
        }
    }
}