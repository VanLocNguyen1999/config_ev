//
// Created by vuonglk on 04/06/2024.
//
/*

#include <sm_logger.h>
#include <memory>

#include "debug_app.h"
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <thread>

#define TAG "main"

#define HEX_PATH "./mc.hex"

std::shared_ptr<upgradingApp> g_upgradingApp;



std::vector<int32_t> reportVector;

void devRebootCallback(uint8_t _devId, uint8_t _issSuccess, void *_arg) {
    LOG_INF(TAG, "Reboot Device %d %s", _devId, _issSuccess ? "SUCCESS" : "FAILED");
    delayMs(200);
}

void devOnUpgradingCallback(uint8_t _devId, const char *_ver, uint8_t _err, void *_arg) {
    if(_err == SM_SV_BOOT_MASTER_ERR_NONE){
        LOG_INF(TAG, "Upgrade Device %d with version %s %s", _devId, _ver, "SUCCESS");
    }else{
        LOG_ERR(TAG, "Upgrade Device %d with version %s %s", _devId, _ver, "FAILED");
    }

    reportVector.emplace_back(_err);

    delayMs(1000);

    sm_sv_boot_master_request_upgrade_by_file(g_upgradingApp->m_svBootMaster, g_upgradingApp->m_devId, "2.0.1",
                                              g_upgradingApp->m_fileHexPath.c_str(), &g_upgradingApp->m_devControlIf);

    g_upgradingApp->setStepTo(STEP_1_PREPARE_UPGRADE);
}

void devFinishUpgradingCallback(void* _arg) {
    LOG_INF(TAG, "All device have been upgraded");
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_ext_boot_handle() {
    return CO_EXT_CONFIRM_success;
}

CO_Sub_Object_Ext_t m_ext_obj;

int devRebootIf(uint8_t _id, uint32_t _timeout, void *_arg) {
    auto self = g_upgradingApp;
    if (self == nullptr) {
        return -1;
    }

    struct CO_SDO_t *sdoClient = &CO_DEVICE.sdo_client;

    static uint8_t m_reboot_state = 1;
    struct CO_Sub_Object_t subObject = {
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
            struct CO_Sub_Object_t subObject_1 = {
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

    self->m_isRebooted = false;
    g_upgradingApp->setStepTo(STEP_3_WAIT_DEV_REBOOT);
    return 1;
}

bool devCheckRebootStatusIf(uint8_t _id, void *_arg) {
    auto self = g_upgradingApp;
    if (self == nullptr) {
        return false;
    }
    return self->m_isRebooted;
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

void canopen_process() {
    while (!g_upgradingApp->m_kill){
        app_co_process();
    }
}

void boot_process() {
    while (!g_upgradingApp->m_kill){
        sm_sv_boot_master_process(g_upgradingApp->m_svBootMaster);
    }
}


int main() {
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);
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
        LOG_ERR(TAG, "cannot create boot master");
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

    g_upgradingApp->m_devType = DEV_TYPE_MC;
    g_upgradingApp->m_devId = 2;
    g_upgradingApp->m_fileHexPath = HEX_PATH;

    if(!isFileExist(g_upgradingApp->m_fileHexPath)){
        LOG_ERR(TAG, "Invalid hex path!!!");
        g_upgradingApp->resetUpgrade();
        return -1;
    }

    std::thread canopen_thread = std::thread(&canopen_process);
    std::thread boot_thread = std::thread(&boot_process);

    sm_sv_boot_master_request_upgrade_by_file(g_upgradingApp->m_svBootMaster, g_upgradingApp->m_devId, "2.0.1",
                                              g_upgradingApp->m_fileHexPath.c_str(), &g_upgradingApp->m_devControlIf);

    g_upgradingApp->setStepTo(STEP_1_PREPARE_UPGRADE);

    while(true){
        if(reportVector.size() >= 10){
            g_upgradingApp->m_kill = true;
            LOG_INF(TAG, "############# TEST REPORT ###############");
            for(int i = 0; i < reportVector.size(); i++){
                LOG_INF(TAG, "#Time test: %d --- Error code: %d", i, reportVector[i]);
            }
            LOG_INF(TAG, "##########################################");
            return 0;
        }
        if(g_upgradingApp->m_processStep == STEP_3_WAIT_DEV_REBOOT && !g_upgradingApp->m_isRebooted){
            CO_SDO_return_t status = CO_SDO_get_status(&CO_DEVICE.sdo_client);
            if(status == CO_SDO_RT_abort){
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
                LOG_ERR(TAG, "Write SDO FAILED, err tx: 0x%x - rx: 0x%x!!!", CO_DEVICE.sdo_client.tx_abort_code,
                        CO_DEVICE.sdo_client.rx_abort_code);

            }else if(status == CO_SDO_RT_success) {
                g_upgradingApp->m_isRebooted = true;
                LOG_INF(TAG, "Write SDO SUCCEED!!!");
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
            }
        }

    }

*/

/*    while (true){
        sm_sv_boot_master_process(g_upgradingApp->m_svBootMaster);
        app_co_process();
        if (g_upgradingApp->m_processStep == STEP_1_GET_DEVICE_TYPE) {
            LOG_INF(TAG, "STEP 1: START UPGRADING APPLICATION ");
            LOG_INF(TAG, "Chose device type to upgrade:");
            LOG_INF(TAG, "1: Upgrade BP");
            LOG_INF(TAG, "2: Upgrade HMI");
            LOG_INF(TAG, "3: Upgrade PMU");
            LOG_INF(TAG, "4: Upgrade MC");


            int type = getc(stdin);
            while (getc(stdin) != '\n');
            printf("get char %d\n", type);

            if(type == '1'){
                g_upgradingApp->m_devType = DEV_TYPE_BP;
                g_upgradingApp->m_devId = 4;
                LOG_INF(TAG, "BP is choose to upgrade...");

            }else if(type == '2'){
                g_upgradingApp->m_devType = DEV_TYPE_HMI;
                g_upgradingApp->m_devId = 8;
                LOG_INF(TAG, "HMI is choose to upgrade...");

            }else if(type == '3'){
                g_upgradingApp->m_devType = DEV_TYPE_PMU;
                g_upgradingApp->m_devId = 1;
                LOG_INF(TAG, "PMU is choose to upgrade...");

            }else if(type == '4'){
                g_upgradingApp->m_devType = DEV_TYPE_MC;
                g_upgradingApp->m_devId = 2;
                LOG_INF(TAG, "MC is choose to upgrade...");

            }else{
                LOG_ERR(TAG, "Wrong device, type again!!!");
                g_upgradingApp->resetUpgrade();
                continue;
            }


            g_upgradingApp->setStepTo(STEP_2_GET_FILE_HEX_PATH);
        } else if(g_upgradingApp->m_processStep == STEP_2_GET_FILE_HEX_PATH){
            LOG_INF(TAG, "STEP 2: Enter path to file hex ");

            getline(std::cin, g_upgradingApp->m_fileHexPath);

            LOG_INF(TAG, "hex path is %s", g_upgradingApp->m_fileHexPath.c_str());

            if(!isFileExist(g_upgradingApp->m_fileHexPath)){
                LOG_ERR(TAG, "Invalid hex path!!!");
                g_upgradingApp->resetUpgrade();
                continue;
            }

            sm_sv_boot_master_request_upgrade_by_file(g_upgradingApp->m_svBootMaster, g_upgradingApp->m_devId, "2.0.1",
                                              g_upgradingApp->m_fileHexPath.c_str(), &g_upgradingApp->m_devControlIf, &g_upgradingApp);


            g_upgradingApp->setStepTo(STEP_3_WAIT_DEV_REBOOT);

            LOG_INF(TAG, "Now start upgrading...");

        }else if(g_upgradingApp->m_processStep == STEP_3_WAIT_DEV_REBOOT && !g_upgradingApp->m_isRebooted){
            CO_SDO_return_t status = CO_SDO_get_status(&CO_DEVICE.sdo_client);
            if(status == CO_SDO_RT_abort){
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
                LOG_ERR(TAG, "Write SDO FAILED, err tx: 0x%x - rx: 0x%x!!!", CO_DEVICE.sdo_client.tx_abort_code,
                        CO_DEVICE.sdo_client.rx_abort_code);

            }else if(status == CO_SDO_RT_success) {
                g_upgradingApp->m_isRebooted = true;
                LOG_INF(TAG, "Write SDO SUCCEED!!!");
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
            }
        }else if(g_upgradingApp->m_processStep == STEP_4_UPGRADING_DEVICE){
            continue;
        } else if(g_upgradingApp->m_processStep == STEP_5_FINISH_UPGRADING){
            LOG_INF(TAG, "UPGRADED DONE, PRESS ANY KEY TO CONTINUE...");
            g_upgradingApp->resetUpgrade();
            while (getc(stdin) != '\n');
        }
    }*/
//}

#include <stdio.h>
#include <limits.h>

#define V 6  // Number of vertices
#define INF INT_MAX  // Infinite cost

// Function to find the vertex with the minimum distance
int minDistance(int dist[], int visited[]) {
    int min = INF, min_index = -1;
    for (int v = 0; v < V; v++) {
        if (!visited[v] && dist[v] < min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkstra’s Algorithm with Early Stopping
int dijkstra(int graph[V][V], int src, int dest) {
    int dist[V];     // Distance from src to each vertex
    int visited[V];  // Visited nodes

    // Initialize distances and visited set
    for (int i = 0; i < V; i++) {
        dist[i] = INF;
        visited[i] = 0;
    }
    dist[src] = 0;  // Distance to source is 0

    // Find shortest paths
    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, visited);
        if (u == -1) break; // No more reachable nodes
        visited[u] = 1;

        // Stop early if we reached the destination
        if (u == dest) {
            return dist[dest]; // Return shortest distance to destination
        }

        // Update distances of adjacent vertices
        for (int v = 0; v < V; v++) {
            if (!visited[v] && graph[u][v] && dist[u] != INF && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    return dist[dest]; // Return shortest distance to destination
}

// Function to modify edge weights dynamically
void updateEdge(int graph[V][V], int from, int to, int newWeight) {
    graph[from][to] = newWeight;
    graph[to][from] = newWeight; // Since it's an undirected graph
}

void addEdge(int src, int dest, int weight) {
    Node* newNode = createNode(dest, weight);
    newNode->next = adjList[src];
    adjList[src] = newNode;

    newNode = createNode(src, weight); // Add reverse edge
    newNode->next = adjList[dest];
    adjList[dest] = newNode;
}

// Function to add an edge
void addEdge(int src, int dest, int weight) {
    Node* newNode = createNode(dest, weight);
    newNode->next = adjList[src];
    adjList[src] = newNode;

    newNode = createNode(src, weight); // Undirected Graph
    newNode->next = adjList[dest];
    adjList[dest] = newNode;
}

// Example usage
int main() {
    // Example adjacency matrix (Graph)
    int graph[V][V] = {
            {0, 4, 1, 0, 0, 10},
            {4, 0, 5, 3, 0, 0}, // B -> C = 5
            {1, 5, 0, 5, 5, 0},
            {0, 3, 5, 0, 1, 6},
            {0, 0, 5, 1, 0, 2},
            {10, 0, 0, 6, 2, 0}
    };

    int source = 0; // Start node
    int destination = 1; // Target node (B)

    printf("Original Shortest Distance from %d to %d: %d\n", source, destination, dijkstra(graph, source, destination));

    // Modify the edge weight B -> C (1 → 2) dynamically
    updateEdge(graph, 1, 2, 2);

    printf("Updated Shortest Distance from %d to %d: %d\n", source, destination, dijkstra(graph, source, destination));

    return 0;
}
