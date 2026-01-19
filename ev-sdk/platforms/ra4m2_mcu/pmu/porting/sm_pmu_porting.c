/*
 * sm_pmu_porting.c
 *
 *  Created on: 26 Aug 2025
 *      Author: My PC
 */
#include "sm_bsp_porting.h"
#include "sm_bsp_pmu.h"

//static int32_t sm_bp_nodeid_select(int32_t _id){
//    sm_hal_io_t* node_id = NULL;
//    if(_id == 0){
//        node_id = sm_bsp_pmu_get_node_id1();
//    }else if(_id == 1){
//        node_id = sm_bsp_pmu_get_node_id2();
//    }else if(_id == 2){
//        node_id = sm_bsp_pmu_get_node_id3();
//    }else{
//        return -1;
//    }
//    sm_hal_io_set_value(node_id, SM_HAL_IO_ON);
//    return 0;
//}
//
//static int32_t sm_bp_nodeid_deselect(int32_t _id){
//    sm_hal_io_t* node_id = NULL;
//    if(_id == 0){
//        node_id = sm_bsp_pmu_get_node_id1();
//    }else if(_id == 1){
//        node_id = sm_bsp_pmu_get_node_id2();
//    }else if(_id == 2){
//        node_id = sm_bsp_pmu_get_node_id3();
//    }else{
//        return -1;
//    }
//    sm_hal_io_set_value(node_id, SM_HAL_IO_OFF);
//    return 0;
//}
//
//static sm_bp_node_id_controller_t g_node_id_controller = {
//        .sm_bp_node_id_select = sm_bp_nodeid_select,
//        .sm_bp_node_id_deselect = sm_bp_nodeid_deselect
//};

//sm_bp_node_id_controller_t* sm_can_master_get_node_id_if(){
//    return &g_node_id_controller;
//}

static int32_t sm_get_voltage_charger(){

    int32_t convert_value = 28*3300/4096;
    return convert_value;
}

static int32_t sm_ctl_mosfet_charger(uint8_t _value){

    (void) _value;
    return -1;
}

static sm_sv_charger_if_t g_charger_if = {
        .get_charger_vol = sm_get_voltage_charger,
        .ctl_charger_power = sm_ctl_mosfet_charger
};

sm_sv_charger_if_t* sm_get_charger_if(){
    return &g_charger_if;
}



