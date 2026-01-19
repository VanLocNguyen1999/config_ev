//
// Created by vuonglk on 04/06/2024.
//

#include <sm_logger.h>
#include <memory>

#include "upgrading_app.h"
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <atomic>
#include <vector>
#include <fstream>

#define TAG "main"
CO_Sub_Object_Ext_t m_ext_obj;

std::vector<int> reportUpgrade;

static std::shared_ptr<upgradingApp> g_upgradingApp;

void devRebootCallback(uint8_t _devId, uint8_t _issSuccess, void *_arg) {
    auto self = g_upgradingApp;
    printf("Rebooted Device %d %s\n", _devId, _issSuccess ? "SUCCESS" : "FAILED");
}

void devOnUpgradingCallback(uint8_t _devId, const char *_ver, uint8_t _err, void *_arg) {
    auto self = g_upgradingApp;

    LOG_INF(TAG, "Upgrade Device %s", (_err == SM_SV_BOOT_MASTER_ERR_NONE) ? "SUCCESS" : "FAIL");

    reportUpgrade.emplace_back(_err);

    if(self->m_devType == DEV_TYPE_MC){
        struct CO_SDO_t *sdoClient = &CO_DEVICE.sdo_client;

        static uint8_t m_reboot_state_1 = 0;
        static struct CO_Sub_Object_t subObject_1 = {
                .p_data = &m_reboot_state_1,
                .attr = ODA_SDO_RW,
                .len = 1,
                .p_ext = &m_ext_obj,
                .p_temp_data = nullptr
        };
        CO_SDOclient_start_download(sdoClient,
                                    1,
                                    SM_DEVICE_MC_BOOT_INDEX,
                                    SM_DEVICE_MC_REBOOT_SUB_INDEX,
                                    &subObject_1,
                                    SM_DEVICE_REBOOT_TIMEOUT);

        elapsed_timer_resetz(&self->m_timeout, 5000);


        while(elapsed_timer_get_remain(&self->m_timeout)){
            CO_SDO_return_t status = CO_SDO_get_status(&CO_DEVICE.sdo_client);
            if(status == CO_SDO_RT_abort){
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
                printf("Write SDO FAILED, err tx: 0x%x - rx: 0x%x!!! \n", CO_DEVICE.sdo_client.tx_abort_code, CO_DEVICE.sdo_client.rx_abort_code);
                g_upgradingApp->m_retry++;
                if (g_upgradingApp->m_retry > 5) {
                    self->setStatusTo(STT_FINISH_UPGRADING);
                    return;
                } else {
                    CO_SDOclient_start_download(sdoClient,
                                                1,
                                                SM_DEVICE_MC_BOOT_INDEX,
                                                SM_DEVICE_MC_REBOOT_SUB_INDEX,
                                                &subObject_1,
                                                SM_DEVICE_REBOOT_TIMEOUT);
                }
            }else if(status == CO_SDO_RT_success) {
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
                self->setStatusTo(STT_FINISH_UPGRADING);
                printf("Write SDO SUCCEED!!! \n");
                return;
            }
        }

    }else{
        self->setStatusTo(STT_FINISH_UPGRADING);
    }
}

void devFinishUpgradingCallback(void* _arg) {
//    LOG_INF(TAG, "All device have been upgraded");
    g_upgradingApp->setStatusTo(STT_FINISH_UPGRADING);
}



int devRebootIf(uint8_t _id, uint32_t _timeout, void *_arg) {
    auto self = g_upgradingApp;
    if (self == nullptr) {
        return -1;
    }

    struct CO_SDO_t *sdoClient = &CO_DEVICE.sdo_client;

    static uint8_t m_reboot_state = 1;
    static struct CO_Sub_Object_t subObject = {
            .p_data = &m_reboot_state,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = &m_ext_obj,
            .p_temp_data = nullptr
    };
    CO_SDO_reset_status(&CO_DEVICE.sdo_client);

    if (CO_SDO_get_status(sdoClient) == CO_SDO_RT_busy) {
        LOG_ERR(TAG, "SDO Status BUSY");
        return -1;
    }

    self->m_isRebooted = false;

    LOG_INF(TAG, "Reboot dev id: %d", self->m_devId);

    switch (self->m_devType) {
        case DEV_TYPE_BP:
            CO_SDOclient_start_download(sdoClient,
                                        0,
                                        SM_DEVICE_BP_BOOT_INDEX,
                                        SN_DEVICE_BP_REBOOT_SUB_INDEX,
                                        &subObject,
                                        SM_DEVICE_REBOOT_TIMEOUT);

            break;

        case DEV_TYPE_HMI:
            CO_SDOclient_start_download(sdoClient,
                                        self->m_devId,
                                        SM_DEVICE_HMI_BOOT_INDEX,
                                        SM_DEVICE_HMI_REBOOT_SUB_INDEX,
                                        &subObject,
                                        SM_DEVICE_REBOOT_TIMEOUT);
            break;
        case DEV_TYPE_PMU:
            CO_SDOclient_start_download(sdoClient,
                                        self->m_devId,
                                        SM_DEVICE_PMU_BOOT_INDEX,
                                        SM_DEVICE_PMU_REBOOT_SUB_INDEX,
                                        &subObject,
                                        SM_DEVICE_REBOOT_TIMEOUT);
            break;

        case DEV_TYPE_MC:
        {
            static uint8_t m_reboot_state_1 = 2;
            static struct CO_Sub_Object_t subObject_1 = {
                    .p_data = &m_reboot_state_1,
                    .attr = ODA_SDO_RW,
                    .len = 1,
                    .p_ext = &m_ext_obj,
                    .p_temp_data = nullptr
            };
            CO_SDOclient_start_download(sdoClient,
                                        1,
                                        SM_DEVICE_MC_BOOT_INDEX,
                                        SM_DEVICE_MC_REBOOT_SUB_INDEX,
                                        &subObject_1,
                                        SM_DEVICE_REBOOT_TIMEOUT);
            break;
        }
        default:
            break;
    }

    return 1;
}

bool devCheckRebootStatusIf(uint8_t _id, void *_arg) {
    auto self = g_upgradingApp;
    if (self == nullptr) {
        return false;
    }
    CO_SDO_return_t status = CO_SDO_get_status(&CO_DEVICE.sdo_client);
    if(status == CO_SDO_RT_abort){
        CO_SDO_reset_status(&CO_DEVICE.sdo_client);
        printf("Write SDO FAILED, err tx: 0x%x - rx: 0x%x!!! \n", CO_DEVICE.sdo_client.tx_abort_code, CO_DEVICE.sdo_client.rx_abort_code);
        g_upgradingApp->m_retry++;
        if (g_upgradingApp->m_retry > 5) {
            return true;
        } else {
            devRebootIf(g_upgradingApp->m_devId, 0, &g_upgradingApp);
        }
    }else if(status == CO_SDO_RT_success) {
        CO_SDO_reset_status(&CO_DEVICE.sdo_client);
        printf("Write SDO SUCCEED!!! \n");
        return true;
    }
    return false;
}

bool devCheckUpgradingConditionIf(uint8_t _id, void *_arg) {
    auto self = g_upgradingApp;
    if (self == nullptr) {
        return false;
    }
    return true;
}

void canHandleRecv(uint32_t _can_id, uint8_t* _data, void* _arg){

}


inline bool isFileExist (const std::string& name) {
    struct stat buffer{};
    return (stat (name.c_str(), &buffer) == 0);
}

std::string logPath = "/home/vuonglk/Desktop/selex/log_upgrade_mc2.txt";

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(local_time, "%H:%M:%S");
    return oss.str();
}

void saveLog(const std::string &_log) {
    std::ofstream logFile(logPath, std::ios::app);
    if(logFile.is_open()){
        logFile << getCurrentTime() << _log << std::endl;
        logFile.close();
    }
}

void log_put(const char* _log) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::cout << std::put_time(&tm, "%H-%M-%S - ");
    printf("%s\n", _log);
    saveLog(_log);
}

int32_t app_init(){
    sm_logger_init(log_put, LOG_LEVEL_INFO);
    g_upgradingApp = std::make_shared<upgradingApp>();

    g_upgradingApp->m_bootEvents.on_rebooted_dev = devRebootCallback;
    g_upgradingApp->m_bootEvents.on_upgraded_dev = devOnUpgradingCallback;
    g_upgradingApp->m_bootEvents.on_finish_upgrading = devFinishUpgradingCallback;
    g_upgradingApp->m_bootEvents.arg = &g_upgradingApp;

    g_upgradingApp->m_devControlIf.reboot_dev = devRebootIf;
    g_upgradingApp->m_devControlIf.check_dev_upgrading_condition = devCheckUpgradingConditionIf;
    g_upgradingApp->m_devControlIf.check_is_dev_rebooted = devCheckRebootStatusIf;
    g_upgradingApp->m_devControlIf.arg = &g_upgradingApp;

    g_upgradingApp->m_svBootMaster = sm_sv_boot_master_create(&g_upgradingApp->m_bootEvents);

    if (g_upgradingApp->m_svBootMaster == nullptr) {
        printf("Cannot create boot master\n");
        return -1;
    }

    static sm_co_if_t *coInterface = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF,
                                                             CO_TCP_SERVER_IP,
                                                             CO_TCP_SERVER_PORT,
                                                             nullptr);

    app_co_init(coInterface, canHandleRecv, coInterface);

    sm_sv_boot_master_set_output_if_fn(g_upgradingApp->m_svBootMaster, SM_SV_BOOT_MASTER_OUTPUT_CANOPEN, nullptr, nullptr, nullptr);
    sm_sv_boot_master_set_input_if_fn(g_upgradingApp->m_svBootMaster, SM_SV_BOOT_MASTER_INPUT_FILE, nullptr, nullptr, nullptr);

    CO_disable_sync_mode(&CO_DEVICE);
    return 0;
}

void printf_error(int _error){
    printf("ERROR %d, use help to see instruction\n", _error);
}

void printf_help(){
    printf("********************** HELP MENU *********************** \n");
    printf("*  - Need 2 argument:                                  *\n");
    printf("*  - Argument 1: Device type (HMI, MC, BP, PMU)        *\n");
    printf("*  - Argument 2: Path to file hex                      * \n");
    printf("*  - Example 1: SFUT BP ./bp.hex (on ubuntu)           *\n");
    printf("*  - Example 2: SFUT BP C:\\Users\\bp.hex (on ubuntu)    *\n");
    printf("******************************************************** \n");
}


void canopen_process() {
    while (true){
        sm_sv_boot_master_process(g_upgradingApp->m_svBootMaster);
        app_co_process();
    }
}

void boot_process() {
    while (true){
    }
}


int main(int argc, char **argv) {

    app_init();

    if(argc == 2 && !strcmp(argv[1], "help")){
        printf_help();
        return 0;
    }else if(argc == 3 && strcmp(argv[1], "help") != 0){
        if(!strcmp(argv[1], "BP")){
            g_upgradingApp->m_devType = DEV_TYPE_BP;
            g_upgradingApp->m_devId = 4;
            printf("BP is choose to upgrade\n");
        } else if(!strcmp(argv[1], "HMI")){
            g_upgradingApp->m_devType = DEV_TYPE_HMI;
            g_upgradingApp->m_devId = 8;
            printf("HMI is choose to upgrade\n");
        }else if(!strcmp(argv[1], "PMU")){
            g_upgradingApp->m_devType = DEV_TYPE_PMU;
            g_upgradingApp->m_devId = 1;
            printf("PMU is choose to upgrade\n");
        }else if(!strcmp(argv[1], "MC")){
            g_upgradingApp->m_devType = DEV_TYPE_MC;
            g_upgradingApp->m_devId = 2;
            printf("MC is choose to upgrade\n");
        } else{
            printf("Invalid device...\n");
            printf_error(-1);
            return -1;
        }

        if(!isFileExist(argv[2])){
            printf("Invalid hex path...\n");
            printf_error(-2);
            return -2;
        }

        g_upgradingApp->m_fileHexPath = argv[2];

    }/*else{
        printf_help();
        return -3;
    }*/

    g_upgradingApp->m_fileHexPath = "/home/vuonglk/Downloads/bp_1.hex";
    g_upgradingApp->m_devType = DEV_TYPE_BP;
    g_upgradingApp->m_devId = 4;
    sm_sv_boot_master_request_upgrade_by_file(g_upgradingApp->m_svBootMaster, g_upgradingApp->m_devId, "0.0.0",
                                              g_upgradingApp->m_fileHexPath.c_str(), &g_upgradingApp->m_devControlIf);

    g_upgradingApp->setStatusTo(STT_PREPARE_UPGRADE);

    std::thread canopen_thread = std::thread(&canopen_process);
//    std::thread boot_thread = std::thread(&boot_process);

    CO_disable_sync_mode(&CO_DEVICE);

    while (true){

        if(g_upgradingApp->m_status == STT_FINISH_UPGRADING){

            LOG_ERR(TAG, "Upgrade finish time %ld !!!\n", reportUpgrade.size());

            if(reportUpgrade.size() % 2 == 0){
                LOG_ERR(TAG, "############# TEST REPORT ###############");
                for(int i = 0; i < reportUpgrade.size(); i++){
                    LOG_ERR(TAG, "#Time test: %d --- Error code: %d", i, reportUpgrade[i]);
                }
                LOG_ERR(TAG, "##########################################\n");
            }

            if(reportUpgrade.size() >= 500)
                return -2;


            delayMs(5000);

            sm_sv_boot_master_request_upgrade_by_file(g_upgradingApp->m_svBootMaster, g_upgradingApp->m_devId, "0.0.0",
                                                      g_upgradingApp->m_fileHexPath.c_str(),
                                                      &g_upgradingApp->m_devControlIf);

            g_upgradingApp->setStatusTo(STT_PREPARE_UPGRADE);
        }
    }
}
