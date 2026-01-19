//
// Created by vuonglk on 04/06/2024.
//

#ifndef EV_SDK_UPGRADING_APP_H
#define EV_SDK_UPGRADING_APP_H

#include <cstdio>
#include <sm_sv_upgrading.h>
#include <sm_mb_master_impl.h>
#include <ctime>
#include "sm_co_if.h"
#include "app_co_init.h"
#include <sm_time_utils.h>
#include <sm_logger.h>
#include <memory>
#include <iostream>
#include <iomanip>
#include <atomic>

#define SM_DEVICE_BP_BOOT_INDEX                            0x2001
#define SN_DEVICE_BP_REBOOT_SUB_INDEX                      0x07

#define SM_DEVICE_HMI_BOOT_INDEX                            0x2801
#define SM_DEVICE_HMI_REBOOT_SUB_INDEX                      0x00

#define SM_DEVICE_PMU_BOOT_INDEX                            0x2300
#define SM_DEVICE_PMU_REBOOT_SUB_INDEX                      0x01

#define SM_DEVICE_MC_BOOT_INDEX                            0x2200
#define SM_DEVICE_MC_REBOOT_SUB_INDEX                      0x00

#define SM_DEVICE_REBOOT_TIMEOUT                            200

#define CO_TCP_SERVER_IP    "192.168.1.254"
#define CO_TCP_SERVER_PORT  32000

int64_t get_tick_count() {
    struct timespec ts{};
    int64_t tick = 0U;
    clock_gettime(CLOCK_REALTIME, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}





typedef enum APP_STEP{
    STT_PREPARE_UPGRADE = 0,
    STT_FINISH_UPGRADING,
    STEP_NUMBER
}APP_STEP;

typedef enum DEV_TYPE{
    DEV_TYPE_PMU,
    DEV_TYPE_HMI,
    DEV_TYPE_BP,
    DEV_TYPE_MC,
    DEV_TYPE_NUMBER
}DEV_TYPE;


class upgradingApp{
public:
    upgradingApp() = default;
    ~upgradingApp() = default;

    void setStatusTo(int8_t _status){
        this->m_status = _status;
        this->m_retry = 0;
    }

    void resetUpgrade(){
        this->m_fileHexPath.clear();
        this->m_isRebooted = false;
        this->m_status = 0;
        CO_SDO_reset_status(&CO_DEVICE.sdo_client);
    }

    sm_sv_boot_master_t * m_svBootMaster{};
    int8_t m_devType{};
    int8_t m_devId{};
    std::string m_fileHexPath{};
    std::atomic<bool> m_isRebooted{false};
    sm_sv_boot_master_events_callback_t m_bootEvents{};
    sm_sv_boot_dev_if_fn_t m_devControlIf{};
    int8_t m_status{0};
    uint8_t m_retry{0};
    std::atomic<bool> m_kill;
    elapsed_timer_t m_timeout{};
};


#endif //EV_SDK_UPGRADING_APP_H
