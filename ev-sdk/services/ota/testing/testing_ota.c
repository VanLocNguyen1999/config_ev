//
// Created by vnbk on 18/09/2024.
//
#include <unistd.h>
#include "sm_sv_ota.h"
#include "sm_ota_download.h"
#include "sm_ex_flash_storage.h"
#include "sm_http_client.h"
#include "sm_elapsed_timer.h"

#include "sm_logger.h"
#include "bsp/sm_bsp_sys_linux.h"
#include "threading/thpool.h"

#define MAIN_TAG "Main"

sm_co_if_t* g_co_if = NULL;
sm_co_t* g_co_core = NULL;
sm_ex_flash_storage_t* g_file_storage = NULL;
sm_ota_download_t* g_ota_download = NULL;
sm_sv_upgrade_t* g_upgrade_service = NULL;
sm_sv_ota_t* g_ota_service = NULL;

const char* g_url = "http://s3.ap-southeast-1.amazonaws.com/selex-bss-v2-log/ota_bss%2FBMS_V1.0.17.rc05%20.hex";

int32_t g_mc_fw_size = 166930;
uint16_t g_mc_fw_crc16 = 51922;
const char* g_mc_url = "https://smart-battery-p.s3.ap-southeast-1.amazonaws.com/ota/323f8af2-c962-4f92-9bca-a5930d780366/selex_smc.hex";

int32_t g_pmu_fw_size = 191362;
int32_t g_pmu_fw_crc16 = 55002;
const char* g_pmu_url = "https://smart-battery-p.s3.ap-southeast-1.amazonaws.com/ota/58377476-7210-4923-a3f8-544e493e5528/s3_pmu_hw1.1_v1.0.0.rc00.hex";


int32_t g_total_mc_success = 0;
int32_t g_total_mc = 0;
int32_t g_total_pmu_success = 0;
int32_t g_total_pmu = 0;
int32_t g_module_index = 0;

void ota_testing();

/**
    * @brief: Notify to observer about ota progress starting
    * @param: void* arg
    */
void ota_handle_on_start(void* _arg){
    LOG_INF(MAIN_TAG, "Start ota on ev system");
}

/**
 * @brief: Notify to observer about ota progress on this module
 * @param: const char* module name
 * @param: int32_t total frame downloading,
 * @param: void* _arg
 */
void ota_handle_on_start_module(const char* _module_name, int32_t _total, void* _arg){
    LOG_DBG(MAIN_TAG, "On starting ota module %s", _module_name);
}

/**
 *@brief: Notify to observer about downloading status
 * @param: const char* module name
 * @param: int32_t frame index
 * @param: bool _is done
 * @param: int32_t error code
 * @param: const char* error message
 * @param: void* _arg
 */
void ota_handle_on_downloading_status(const char*_module_name, int32_t _frame_index, bool _last_segment, int32_t _err, const char*_err_msg, void* _arg){
    LOG_DBG(MAIN_TAG, "On downloading firmware module %s. Segment %d, Err %s", _module_name, _frame_index, _err_msg);
}

/**
 * @brief: Notify Upgrading status
* const char*: module name (pmu/mc/hmi/bp)
* const char*: new version
* int32_t: error code
* const char* : error message
* void* arg
*/
void ota_handle_on_upgrading_status(const char *_module_name, const char *_version, int32_t _err, const char*_err_msg, void* _arg){
    LOG_INF(MAIN_TAG, "On Upgrading firmware module %s. new version %s, Err %s", _module_name, _version, _err_msg);
    if(!strcmp(_module_name, "mc")){
        if(_err == SM_OTA_ERR_NONE){
            g_total_mc_success++;
        }
        g_total_mc++;
    }else{
        if(_err == SM_OTA_ERR_NONE){
            g_total_pmu_success++;
        }
        g_total_pmu++;
    }

    LOG_INF(MAIN_TAG, "Total OTA MC: %d, Success: %d", g_total_mc, g_total_mc_success);
    LOG_INF(MAIN_TAG, "Total OTA PMU: %d, Success: %d", g_total_pmu, g_total_pmu_success);

    if(g_module_index >= 2){
        g_module_index = 0;

        sleep(10);
        ota_testing();
    }
}

/**
 *@brief: Notify error on OTA progress.
* const char*: module name (pmu/mc/hmi/bp)
* const char*: new version
* int32_t: error code
* const char* : error message
* void* arg
*/
void ota_handle_on_error(const char *_module_name, const char * _version, int32_t _err, const char * _err_msg, void *_arg){

}

/**
 * @brief: Notify OTA progress module is finished
 *  const char*: module name (pmu/mc/hmi/bp)
 */
void ota_handle_on_finished_module(const char *_module_name, void *_arg){
    LOG_INF(MAIN_TAG, "Finish upgrade module: %s", _module_name);
    g_module_index++;
}

/**
 * @brief: Notify OTA progress finished
 */
void ota_handle_on_finished(void *_arg){
    LOG_INF(MAIN_TAG, "Finish upgrading process");
}

sm_ota_event_t g_ota_handle = {
        .on_start = ota_handle_on_start,
        .on_error = ota_handle_on_error,
        .on_finished = ota_handle_on_finished,
        .on_start_module = ota_handle_on_start_module,
        .on_finished_module = ota_handle_on_finished_module,
        .on_downloading_status = ota_handle_on_downloading_status,
        .on_upgrading_status = ota_handle_on_upgrading_status
};

void ota_testing(){
    if(!g_ota_service){
        LOG_ERR(MAIN_TAG, "OTA service is NOT initialized");
        return;
    }

    sm_sv_ota_reg_event(g_ota_service, &g_ota_handle, g_ota_service);

    sm_sv_ota_request_upgrade(g_ota_service,
                              SM_OTA_LTE_IF,
                              SM_EV_MODULE_MC,
                              "1.0.1",
                              g_mc_fw_size,
                              g_mc_fw_crc16,
                              g_mc_url);

    sm_sv_ota_request_upgrade(g_ota_service,
                              SM_OTA_LTE_IF,
                              SM_EV_MODULE_PMU,
                              "1.10.0",
                              g_pmu_fw_size,
                              g_pmu_fw_crc16,
                              g_pmu_url);

    sm_sv_ota_start(g_ota_service);
}

void ota_upgrading_testing(){
    if(!g_upgrade_service){
        return;
    }
}


void ota_download_cb(int32_t _seg, bool _last_seg,  int32_t _err, const char* _err_msg, void* _arg){
    LOG_INF(MAIN_TAG, "Download segment %d %s", _seg, _err_msg);
    if(_last_seg){
        LOG_INF(MAIN_TAG, "Last Segment, Finish download");
    }
}

void ota_download_testing(){
    g_ota_download->m_proc->reg_cb(g_ota_download, ota_download_cb, g_ota_download);
    g_ota_download->m_proc->init(g_ota_download, g_file_storage, 270163);
    g_ota_download->m_proc->set_param(g_ota_download, (void*)g_url);

    int32_t ret = -1;
    while (1){
        ret = g_ota_download->m_proc->download(g_ota_download, NULL);
        if(ret == SM_OTA_DOWNLOAD_DONE){
            LOG_INF(MAIN_TAG, "Download firmware SUCCESS");
            break;
        }
    }
}

void test_script(char c){
    switch (c) {
        case '0':
            ota_download_testing();
            break;
        case '1':
            ota_upgrading_testing();
            break;
        case '2':
            ota_testing();
            break;
        default:
            LOG_WRN(MAIN_TAG, "Script NOT support");
    }
}

static void sm_co_service_thread(void* _arg);
static void sm_co_interface_thread(void* _arg);
static void sm_upgrading_thread(void* _arg);
static void sm_ota_thread(void* _arg);

int main(int argc, char** argv){
    sm_logger_init(sm_bsp_linux_logger_put, LOG_LEVEL_DEBUG);
    LOG_INF(MAIN_TAG, "OTA service testing.....");

    sm_http_client_t* http_client_curl = sm_http_client_create(NULL);

    g_ota_download = sm_ota_http_download_create(http_client_curl);
    if(!g_ota_download){
        LOG_ERR(MAIN_TAG, "Could NOT create http download");
        return EXIT_FAILURE;
    }

    g_file_storage = sm_ex_flash_storage_create(NULL);
    if(!g_file_storage){
        LOG_ERR(MAIN_TAG, "Could NOT create file storage");
        return EXIT_FAILURE;
    }

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

    g_upgrade_service = sm_sv_ev_upgrade_create_default(g_co_core);
    if(!g_upgrade_service){
        LOG_ERR(MAIN_TAG, "Created Upgrading service FAILURE");
        return EXIT_FAILURE;
    }

    g_ota_service = sm_sv_ota_create(http_client_curl, g_upgrade_service, g_file_storage);
    if(!g_ota_service){
        LOG_ERR(MAIN_TAG, "Created OTA service FAILURE");
        return EXIT_FAILURE;
    }

    threadpool thread_pool = thpool_init(4);
//    thpool_add_work(thread_pool, sm_upgrading_thread, g_upgrade_service);
//    thpool_add_work(thread_pool, sm_ota_thread, g_ota_service);
    thpool_add_work(thread_pool, sm_co_service_thread, g_co_core);
//    thpool_add_work(thread_pool, sm_co_interface_thread, g_co_if);

//    elapsed_timer_t co_timer = {
//            .m_duration = 1,
//            .m_start_time = (int32_t)get_tick_count()
//    };
//    ota_testing();
    while (1){
        LOG_INF(MAIN_TAG, "/******************** STARTING ******************* 555/");
        LOG_INF(MAIN_TAG, "Please choose the character for testing !!!!");
        LOG_INF(MAIN_TAG, "Choose '0' for download firmware testing !!!");
        LOG_INF(MAIN_TAG, "Choose '1' for upgrade firmware testing !!!");
        LOG_INF(MAIN_TAG, "Choose '2' for full ota testing !!!");

        LOG_INF(MAIN_TAG, "Please Enter character for testing !!!");
        int c = getchar();

        test_script(c);

        while (getc(stdin) != '\n');

/*
        sm_co_if_process(g_co_if);
        if(!elapsed_timer_get_remain(&co_timer)) {
            sm_co_process(g_co_core, 1);
            elapsed_timer_reset(&co_timer);
        }
        sm_sv_ota_process(g_ota_service);*/
    }

    LOG_INF(MAIN_TAG, "Finish OTA Testing. Exit program");

    return EXIT_SUCCESS;
}

static void sm_co_service_thread(void* _arg){
    sm_co_t* core_co = (sm_co_t*)_arg;
    elapsed_timer_t co_timer = {
            .m_duration = 1,
            .m_start_time = (int32_t)get_tick_count()
    };
    while (1){
        sm_co_if_process(g_co_if);

        if(!elapsed_timer_get_remain(&co_timer)) {
            sm_co_process(core_co, 1);
            elapsed_timer_reset(&co_timer);
        }

        sm_sv_ota_process(g_ota_service);
    }
}

static void sm_co_interface_thread(void* _arg){
    sm_co_if_t* co_if = (sm_co_if_t*)_arg;
    while (1){
        sm_co_if_process(co_if);
    }
}

static void sm_upgrading_thread(void* _arg){
    sm_sv_upgrade_t* upgrade_service = (sm_sv_upgrade_t*)_arg;
    while (1){
        sm_sv_ev_upgrade_process(upgrade_service);
    }
}
static void sm_ota_thread(void* _arg){
    sm_sv_ota_t* ota_service = (sm_sv_ota_t*)_arg;
    while (1){
        sm_sv_ota_process(ota_service);
    }
}