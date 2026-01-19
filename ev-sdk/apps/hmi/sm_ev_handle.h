//
// Created by vnbk on 19/09/2024.
//

#ifndef EV_SDK_SM_EV_HANDLE_H
#define EV_SDK_SM_EV_HANDLE_H

#include "sm_types.h"

void sm_ev_on_event(int32_t _event, int32_t _value, void* _arg);
void sm_ev_on_bp_event(int32_t _port, int32_t _event, int32_t _value, void* _arg);

void* sm_ev_get_manu(void*_arg);
void* sm_ev_get_module_info(int32_t _type, void* _arg);
void* sm_ev_get_data(void*_arg);
void* sm_ev_get_ev_config(void*_arg);
void* sm_ev_get_bp_data(int32_t _id, void*_arg);
int32_t sm_ev_get_gps_data(void*, void* _arg);
int32_t sm_ev_get_network_data(int32_t* _rssi, char* _simNb, void* _arg);
int32_t sm_ev_get_optional_data(char* _data, void* _arg);


#endif //EV_SDK_SM_EV_HANDLE_H
