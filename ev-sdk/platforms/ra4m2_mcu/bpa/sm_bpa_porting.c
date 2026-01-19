//
// Created by vnbk on 25/03/2025.
//
#include "sm_bsp_porting.h"
#include "sm_bsp_bpa.h"

static int32_t sm_bp_nodeid_select(int32_t _id){
    sm_hal_io_t* node_id = NULL;
    if(_id == 0){
        node_id = sm_bsp_bpa_get_node_id0();
    }else if(_id == 1){
        node_id = sm_bsp_bpa_get_node_id1();
    }else if(_id == 2){
        node_id = sm_bsp_bpa_get_node_id2();
    }else{
        return -1;
    }
    sm_hal_io_set_value(node_id, SM_HAL_IO_ON);
    return 0;
}

static int32_t sm_bp_nodeid_deselect(int32_t _id){
    sm_hal_io_t* node_id = NULL;
    if(_id == 0){
        node_id = sm_bsp_bpa_get_node_id0();
    }else if(_id == 1){
        node_id = sm_bsp_bpa_get_node_id1();
    }else if(_id == 2){
        node_id = sm_bsp_bpa_get_node_id2();
    }else{
        return -1;
    }
    sm_hal_io_set_value(node_id, SM_HAL_IO_OFF);
    return 0;
}

static sm_bp_node_id_controller_t g_node_id_controller = {
        .sm_bp_node_id_select = sm_bp_nodeid_select,
        .sm_bp_node_id_deselect = sm_bp_nodeid_deselect
};

sm_bp_node_id_controller_t* sm_can_master_get_node_id_if(){
    return &g_node_id_controller;
}

static int32_t sm_get_voltage_charger(){
    sm_hal_adc_t* adc = sm_bsp_bpa_get_charger_vol();
    int32_t convert_value = sm_hal_adc_read(adc)*28*3300/4096;
    return convert_value;
}

static int32_t sm_ctl_mosfet_charger(uint8_t _value){
    sm_hal_io_t* io = sm_bsp_bpa_get_charger_sw();
    if(io){
        return sm_hal_io_set_value(io, _value);
    }
    return -1;
}

static sm_sv_charger_if_t g_charger_if = {
        .get_charger_vol = sm_get_voltage_charger,
        .ctl_charger_power = sm_ctl_mosfet_charger
};

sm_sv_charger_if_t* sm_get_charger_if(){
    return &g_charger_if;
}