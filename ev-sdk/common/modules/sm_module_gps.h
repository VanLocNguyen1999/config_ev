//
// Created by vnbk on 09/06/2023.
//

#ifndef SM_MODULE_GPS_H
#define SM_MODULE_GPS_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sm_types.h"
#include "sm_datetime.h"
#include "sm_ev_data.h"

#define START_YEAR                  2000

typedef sys_datetime_t sm_gps_datetime_t;

typedef struct sm_gps_module sm_gps_t;

typedef struct{
    int32_t (*init)(sm_gps_t*);
    int32_t (*free)(sm_gps_t*);
    int32_t (*reboot)(sm_gps_t*);
    int32_t (*power_reset)(sm_gps_t*);
    float (*get_lat)(sm_gps_t*);
    float (*get_lon)(sm_gps_t*);
    int32_t (*get_time)(sm_gps_t*, sm_gps_datetime_t*, int8_t);
    int32_t (*get_coordinate)(sm_gps_t*, sm_gps_coordinate_t*);
    bool (*data_is_valid)(sm_gps_t*);
    int32_t (*process)(sm_gps_t*);
}sm_gps_module_proc_t;

struct sm_gps_module{
    const sm_gps_module_proc_t* proc;
};

#ifdef __cplusplus
};
#endif

#endif //SM_MODULE_GPS_H
