//
// Created by vnbk on 24/09/2024.
//

#ifndef EV_SDK_SM_EV_CONFIG_H
#define EV_SDK_SM_EV_CONFIG_H

#ifdef __cplusplus
extern  "C"{
#endif

#include "sm_types.h"

#define SM_EV_CONFIG_LOCK_PORT_ENABLE    (1)
#define SM_EV_CONFIG_LOCK_PORT_DISABLE    (!SM_EV_CONFIG_LOCK_PORT_ENABLE)

#define SM_EV_CONFIG_UPHILL_ENABLE      (1)
#define SM_EV_CONFIG_UPHILL_DISABLE     (!SM_EV_CONFIG_UPHILL_ENABLE)
#define SM_EV_CONFIG_UPHILL_DEFAULT     (SM_EV_CONFIG_UPHILL_ENABLE)

#define SM_EV_CONFIG_AUTH_BP_DEFAULT                    (0x11)
#define SM_EV_CONFIG_AUTH_BP_DISABLE                    (0x00)
#define SM_EV_CONFIG_AUTH_BP_LEVEL_MIN                  (SM_EV_CONFIG_AUTH_BP_DISABLE)
#define SM_EV_CONFIG_AUTH_BP_LEVEL_MAX                  (0x02)
#define SM_EV_CONFIG_AUTH_BP_LEVEL_DEFAULT              (0x01)

#define SM_EV_CONF_AUTH_BP_OFFLINE_MASK                 (0x0F)
#define SM_EV_CONF_AUTH_BP_ONLINE_MASK                  (0xF0)

#define SM_EV_CONFIG_WHEEL_RADIUS_MIN               (0.2f)
#define SM_EV_CONFIG_WHEEL_RADIUS_MAX               (0.3f)
#define SM_EV_S2_CONFIG_WHEEL_RADIUS_DEFAULT        (0.2364f)
#define SM_EV_S3_CONFIG_WHEEL_RADIUS_DEFAULT        (0.2618f)
#define SM_EV_WHEEL_RADIUS_EPSILON                  ((float)(1e-5)) //0.00001

#define SM_EV_CONFIG_INACTIVE_MODE_ENABLE                (1)
#define SM_EV_CONFIG_INACTIVE_MODE_DISABLE               (!SM_EV_CONFIG_INACTIVE_MODE_ENABLE)
#define SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_MIN        (2) ///km
#define SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_MAX        (50) ///km
#define SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_DEFAULT    (20) ///km

#define SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_MIN     (3)   ///km
#define SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_MAX     (100) ///km
#define SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_DEFAULT (30)  ///km
#define SM_EV_CONFIG_INACTIVE_MODE_SAVE_TIME_DEFAULT     (300) ///second

#define SM_EV_CONFIG_KM_STORAGE_ODO_MAX             (10) // km
#define SM_EV_CONFIG_KM_STORAGE_ODO_MIN             (1)  // km
#define SM_EV_CONFIG_KM_STORAGE_ODO_DEFAULT         (5) /// km

#define SM_EV_CONFIG_ODO_PASS_LENGTH                (10)
#define SM_EV_CONFIG_ODO_PASS_DEFAULT               "selex123@"

#define SM_EV_CONFIG_SYNC_TIME_MAX                  (120)
#define SM_EV_CONFIG_SYNC_TIME_MIN                  (5)
#define SM_EV_CONFIG_SYNC_TIME_DEFAULT              (35)

#define SM_EV_CONFIG_AUTH_MODULE_DISABLE            (0)
#define SM_EV_CONFIG_AUTH_MODULE_LEVEL_0            SM_EV_CONFIG_AUTH_MODULE_DISABLE
#define SM_EV_CONFIG_AUTH_MODULE_LEVEL_1            (1) /// WARNING
#define SM_EV_CONFIG_AUTH_MODULE_LEVEL_2            (2) /// FORCE ACTION
#define SM_EV_CONFIG_AUTH_MODULE_LEVEL_DEFAULT      SM_EV_CONFIG_AUTH_MODULE_LEVEL_2

#define SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_DEFAULT      (30)    ///seconds
#define SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_MIN          (10)    ///seconds
#define SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_MAX          (30*60)    ///seconds

#define SM_EV_CONFIG_SIZE_OF                        (sizeof(sm_ev_config_t))

enum {
    SM_EV_CONF_AUTH_BP = 0,
    SM_EV_CONF_UPHILL_MODE,
    SM_EV_CONF_LOCK_PORT,
    SM_EV_CONF_INACTIVE_MODE,
    SM_EV_CONF_WHEEL_RADIUS,
    SM_EV_CONF_KM_ODO_STORED,
    SM_EV_CONF_ODO_PASS,
    SM_EV_CONF_SYNC_TIME,
    SM_EV_CONF_AUTH_MODULE,
    SM_EV_CONF_NUMBER
};

typedef struct{
    uint8_t m_enable;
    uint16_t m_save_time;
    uint16_t m_km_warning;
    uint16_t m_km_force_stop;
}sm_ev_inactive_mode_config_t;

typedef struct{
    uint8_t m_level;
    uint16_t m_detected_time;
    uint8_t m_optional[8];
}sm_auth_module_config_t;

typedef struct {
    uint8_t m_auth_bp;
    uint8_t m_uphill_mode;
    uint8_t m_lock_port;

    sm_ev_inactive_mode_config_t m_inactive_mode_config;

    uint8_t m_km_store_odo;
    uint8_t m_odo_pass[SM_EV_CONFIG_ODO_PASS_LENGTH];

    uint8_t m_sync_time;

    sm_auth_module_config_t m_auth_module;
}sm_ev_config_t;

static inline void sm_ev_config_reset_default(sm_ev_config_t* _config){
    _config->m_lock_port = SM_EV_CONFIG_LOCK_PORT_ENABLE;
    _config->m_auth_bp = SM_EV_CONFIG_AUTH_BP_DEFAULT;
    _config->m_uphill_mode = SM_EV_CONFIG_UPHILL_DEFAULT;
    _config->m_km_store_odo = SM_EV_CONFIG_KM_STORAGE_ODO_DEFAULT;
    memcpy(_config->m_odo_pass, SM_EV_CONFIG_ODO_PASS_DEFAULT, strlen(SM_EV_CONFIG_ODO_PASS_DEFAULT));
    _config->m_inactive_mode_config.m_enable = SM_EV_CONFIG_INACTIVE_MODE_DISABLE;
    _config->m_inactive_mode_config.m_km_warning = SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_DEFAULT;
    _config->m_inactive_mode_config.m_km_force_stop = SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_DEFAULT;
    _config->m_inactive_mode_config.m_save_time = SM_EV_CONFIG_INACTIVE_MODE_SAVE_TIME_DEFAULT;
    _config->m_sync_time = SM_EV_CONFIG_SYNC_TIME_DEFAULT;
    _config->m_auth_module.m_level = SM_EV_CONFIG_AUTH_MODULE_LEVEL_DEFAULT;
    _config->m_auth_module.m_detected_time = SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_DEFAULT;
    memset(_config->m_auth_module.m_optional, 0, sizeof(_config->m_auth_module.m_optional));
}

static inline void sm_ev_config_clone(const sm_ev_config_t* _src, sm_ev_config_t* _dest){
    _dest->m_lock_port =  _src->m_lock_port;
    _dest->m_auth_bp = _src->m_auth_bp;
    _dest->m_uphill_mode = _src->m_uphill_mode ;
    _dest->m_km_store_odo =  _src->m_km_store_odo;
    memcpy(_dest->m_odo_pass,  _src->m_odo_pass, SM_EV_CONFIG_ODO_PASS_LENGTH);
    _dest->m_inactive_mode_config.m_enable =  _src->m_inactive_mode_config.m_enable ;
    _dest->m_inactive_mode_config.m_km_warning = _src->m_inactive_mode_config.m_km_warning;
    _dest->m_inactive_mode_config.m_km_force_stop = _src->m_inactive_mode_config.m_km_force_stop;
    _dest->m_inactive_mode_config.m_save_time = _src->m_inactive_mode_config.m_save_time ;
    _dest->m_sync_time = _src->m_sync_time;
    _dest->m_auth_module.m_level = _src->m_auth_module.m_level;
    _dest->m_auth_module.m_detected_time = _src->m_auth_module.m_detected_time;
    memcpy(_dest->m_auth_module.m_optional, _src->m_auth_module.m_optional, sizeof(_dest->m_auth_module.m_optional));
}

static inline bool sm_ev_config_inactive_mode_validate(sm_ev_inactive_mode_config_t* _config){
    bool validate = true;
    if(_config->m_enable != SM_EV_CONFIG_INACTIVE_MODE_ENABLE && _config->m_enable != SM_EV_CONFIG_INACTIVE_MODE_DISABLE){
        _config->m_enable = SM_EV_CONFIG_INACTIVE_MODE_ENABLE;
        validate = false;
    }

    if(_config->m_km_warning < SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_MIN || _config->m_km_warning > SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_MAX){
        _config->m_km_warning = SM_EV_CONFIG_INACTIVE_MODE_KM_WARNING_DEFAULT;
        validate = false;
    }

    if(_config->m_km_force_stop < _config->m_km_warning ||
        _config->m_km_force_stop < SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_MIN ||
        _config->m_km_force_stop > SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_MAX)
    {
        _config->m_km_force_stop = SM_EV_CONFIG_INACTIVE_MODE_KM_FORCE_STOP_DEFAULT;
        validate = false;
    }

    return validate;
}

static inline bool sm_ev_config_auth_module_validate(sm_ev_config_t* _this){
    bool validate = true;
    if(_this->m_auth_module.m_level > SM_EV_CONFIG_AUTH_MODULE_LEVEL_2){
        _this->m_auth_module.m_level = SM_EV_CONFIG_AUTH_MODULE_LEVEL_DEFAULT;
        validate = false;
    }

    if(_this->m_auth_module.m_detected_time < SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_MIN ||
        _this->m_auth_module.m_detected_time > SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_MAX){
        _this->m_auth_module.m_detected_time = SM_EV_CONFIG_AUTH_MODULE_DETECTED_TIME_DEFAULT;
        validate = false;
    }

    return validate;
}

static inline bool sm_ev_config_validate(sm_ev_config_t* _config){
    bool validate = true;

    uint8_t offline = _config->m_auth_bp & 0x0F;
    uint8_t online = (_config->m_auth_bp >> 4) & 0x0F;

    if(offline > SM_EV_CONFIG_AUTH_BP_LEVEL_MAX){
        offline = SM_EV_CONFIG_AUTH_BP_LEVEL_DEFAULT;
        validate = false;
    }

    if(online > SM_EV_CONFIG_AUTH_BP_LEVEL_MAX){
        online = SM_EV_CONFIG_AUTH_BP_LEVEL_DEFAULT;
        validate = false;
    }

    if(validate == false){
        _config->m_auth_bp = (online << 4) | offline;
    }

    if(_config->m_uphill_mode != SM_EV_CONFIG_UPHILL_ENABLE && _config->m_uphill_mode != SM_EV_CONFIG_UPHILL_DISABLE){
        _config->m_uphill_mode = SM_EV_CONFIG_UPHILL_DEFAULT;
        validate = false;
    }

    validate = sm_ev_config_inactive_mode_validate(&_config->m_inactive_mode_config);

    if(_config->m_km_store_odo < SM_EV_CONFIG_KM_STORAGE_ODO_MIN || _config->m_km_store_odo > SM_EV_CONFIG_KM_STORAGE_ODO_MAX){
        _config->m_km_store_odo = SM_EV_CONFIG_KM_STORAGE_ODO_DEFAULT;
        validate = false;
    }

    if(_config->m_odo_pass[0] == '\0'){
        memcpy(_config->m_odo_pass,  SM_EV_CONFIG_ODO_PASS_DEFAULT, SM_EV_CONFIG_ODO_PASS_LENGTH);
        validate = false;
    }

    if(_config->m_sync_time < SM_EV_CONFIG_SYNC_TIME_MIN || _config->m_sync_time > SM_EV_CONFIG_SYNC_TIME_MAX){
        _config->m_sync_time = SM_EV_CONFIG_SYNC_TIME_DEFAULT;
        validate = false;
    }

    validate = sm_ev_config_auth_module_validate(_config);

    return validate;
}

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_EV_CONFIG_H
