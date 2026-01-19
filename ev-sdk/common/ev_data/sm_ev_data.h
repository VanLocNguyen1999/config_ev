//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_CORE_EV_DATA_H
#define EV_SDK_SM_CORE_EV_DATA_H

#include "sm_types.h"
#include "sm_bp_data.h"
#include "sm_ev_config.h"
#include "sm_math.h"

#define EV_DEVICE_NAME_SIZE     10
#define EV_SERIAL_MODULE_SIZE   16
#define EV_SERIAL_NUMBER_SIZE   32
#define EV_PCBA_MODULE_SIZE     16
#define EV_HW_VERSION_SIZE      4
#define EV_SW_VERSION_SIZE      4
#define EV_BOOT1_VERSION_SIZE   4
#define EV_BOOT2_VERSION_SIZE   4
#define EV_DATE_MANU_SIZE       10
#define EV_LOT_NUMBER_SIZE      10
#define EV_INFO_SIZE            128

#define EV_VERSION_STRING_LENGTH    16

#define SM_MODULE_ID_DEFAULT            0

#define SM_MODULE_INFO_REGISTRATION     1
#define SM_MODULE_INFO_PRODUCTION       0

typedef struct sm_module_info sm_module_info_t;
typedef sm_module_info_t sm_ev_manu_t;

typedef enum {
    MODULE_STATE_DISCONNECTED = 0,
    MODULE_STATE_CONNECTED = 1,
}SM_MODULE_STATE;
typedef enum {
    PURPOSE_FOR_VEHICLE_MARKET,       // Cho xe thị trường
    PURPOSE_FOR_VEHICLE_INSPECTION,   // Dùng cho hậu kiểm nhà máy
} ev_purpose;

enum {
    EV_MC_MARKET_MODE       = 0,
    EV_MC_INSPECTION_MODE   = 10
};

typedef struct pmu_config_t{

    uint8_t    block_st;
    uint8_t    lock_st;
    uint8_t    anti_st;
    ev_purpose  m_pmu_purpose;
}pmu_config;

typedef struct pmu_sdo_ctrl_t {

    uint8_t fin_vehicle;
    uint8_t m_horn_ctrl;
    uint8_t m_ev_block;
    uint8_t m_ev_lock;
    uint8_t m_ev_anti_mode;
}pmu_sdo_ctrl;

typedef struct sys_energy_t{

    uint32_t   elec_energyDischarge;//Wh
    uint32_t   elec_energyCharge;//Wh
}sys_energy;

struct sm_module_info{
    uint16_t m_crc;
    char m_device_name[EV_DEVICE_NAME_SIZE];
    char m_sn[EV_SERIAL_MODULE_SIZE];
    char m_ev_sn[EV_SERIAL_NUMBER_SIZE];
    char m_pcba_id[EV_PCBA_MODULE_SIZE];
    char m_hw_ver[EV_HW_VERSION_SIZE];
    char m_sw_ver[EV_SW_VERSION_SIZE];
    char m_boot1_ver[EV_BOOT1_VERSION_SIZE];
    char m_boot2_ver[EV_BOOT2_VERSION_SIZE];
    char m_date_manufacture[EV_DATE_MANU_SIZE];
    char m_lotNum[EV_LOT_NUMBER_SIZE];
    char m_registration;
    float m_wheel_radius;
};

static inline void sm_module_reset_data(sm_module_info_t* _this){
    _this->m_crc = 0;
    memset(_this->m_device_name, 0, EV_DEVICE_NAME_SIZE);
    memset(_this->m_sn, 0, EV_SERIAL_MODULE_SIZE);
    memset(_this->m_ev_sn, 0, EV_SERIAL_NUMBER_SIZE);
    memset(_this->m_pcba_id, 0, EV_PCBA_MODULE_SIZE);
    memset(_this->m_hw_ver, 0, EV_HW_VERSION_SIZE);
    memset(_this->m_sw_ver, 0, EV_SW_VERSION_SIZE);
    memset(_this->m_boot1_ver, 0, EV_BOOT1_VERSION_SIZE);
    memset(_this->m_boot2_ver, 0, EV_BOOT2_VERSION_SIZE);
    memset(_this->m_date_manufacture, 0, EV_DATE_MANU_SIZE);
    memset(_this->m_lotNum, 0, EV_LOT_NUMBER_SIZE);
    _this->m_registration = SM_MODULE_INFO_PRODUCTION;
    _this->m_wheel_radius = SM_EV_S2_CONFIG_WHEEL_RADIUS_DEFAULT;
}

#define SM_EV_DATA_IS_CHANGED(cur,value)    ((cur)!=(value))

#define EV_KEY_ON  1
#define EV_KEY_OFF (!EV_KEY_ON)

#define EV_ENTER_PARKING            (1)
#define EV_EXIT_PARKING             (!EV_ENTER_PARKING)

#define EV_MAX_SPEED                (100)
#define EV_MIN_SPEED                (5)
#define EV_MAX_SPEED_DEFAULT        (70)

enum {
    EV_MC_HAFT_ECO_MODE_1   = 0x00,
    EV_MC_ECO_MODE_1        = 0x01,
    EV_MC_HAFT_ECO_MODE_2   = 0x10,
    EV_MC_ECO_MODE_2        = 0x11,
    EV_MC_HAFT_ECO_MODE_3   = 0x20,
    EV_MC_ECO_MODE_3        = 0x21,
    EV_MC_HAFT_SPORT_MODE_2 = 0x30,
    EV_MC_SPORT_MODE_2      = 0x31,
    EV_MC_HAFT_SPORT_MODE_1 = 0x40,
    EV_MC_SPORT_MODE_1      = 0x41,
    EV_MC_REVERSE_MODE_FORWARD  = 0x51,
    EV_MC_REVERSE_MODE_BACKWARD = 0x61,
    EV_MC_HAFT_UP_HILL_MODE     = 0x70,
    EV_MC_UP_HILL_MODE          = 0x71
};

enum {
    EV_ECO_MODE_1 = 0x00,
    EV_ECO_MODE_2 = 0x01,
    EV_ECO_MODE_3 = 0x02,
    EV_SPORT_MODE_2 = 0x03,
    EV_SPORT_MODE_1 = 0x04,
    EV_UP_HILL_MODE = 0x05
};

enum{
    EV_REVERSE_STATE_INACTIVE = 0,
    EV_REVERSE_STATE_ACTIVE_FORWARD = 1,
    EV_REVERSE_STATE_ACTIVE_REVERSE = 2
};

#define EV_ECO_MODE_SIGNAL      (0)
#define EV_SPORT_MODE_SIGNAL    (1)
#define EV_UPHILL_MODE_SIGNAL   (2)

#define EV_ENTER_BACKWARD_MODE  (2)
#define EV_EXIT_BACKWARD_MODE   (1)

#define EV_LOCK_PORT            (1)
#define EV_UNLOCK_PORT          (!EV_LOCK_PORT)

#define EV_ACTIVE_STATE          (1)
#define EV_INACTIVE_STATE        (!EV_ACTIVE_STATE)

#define EV_MOTOR_ACTIVE_STATE          (1)
#define EV_MOTOR_INACTIVE_STATE        (!EV_MOTOR_ACTIVE_STATE)

#define EV_HEAD_LIGHT_ON        (1)
#define EV_HEAD_LIGHT_OFF       (!EV_HEAD_LIGHT_ON)

enum {
    SM_EV_STATE_NORMAL,
    SM_EV_STATE_INACTIVE,
    SM_EV_STATE_LOCK,
    SM_EV_STATE_BLOCK,
    SM_EV_STATE_UPGRADING,
    SM_EV_STATE_NUMBER
};

enum {
    EV_ERR_NONE     = 0,
    EV_ERR_UNDER_VOL = 3,
    EV_ERR_OVER_VOL = 6,
    EV_ERR_PHASE_CUR_OFFSET = 17,
    EV_ERR_OVER_CUR = 18,
    EV_ERR_HALL_ANGLE = 20,
    EV_ERR_HALL_ANGLE_VIBRATION = 21,
    EV_ERR_HALL_ANGLE_INIT  = 22,
    EV_ERR_MOTOR_TEMP = 37,
    EV_ERR_MC_FLASH = 53,
    EV_ERR_MC_FLASH_PROTECTION = 54,
    EV_ERR_THROTTLE_TIMEOUT = 65,
    EV_ERR_THROTTLE_OUT_RANGE = 66,
    EV_ERR_THROTTLE_NOT_RELEASE = 67,
    EV_ERR_OVER_LOAD = 68,
    EV_ERR_MC_OVERHEAT_LV1 = 69,
    EV_ERR_MC_OVERHEAT_LV2 = 70,
    EV_ERR_MOTOR_OVERHEAT_LV1 = 71,
    EV_ERR_MOTOR_OVERHEAT_LV2 = 72,
    EV_ERR_HALL_ANGLE_HW_STATE    = 73,
    EV_ERR_HALL_ANGLE_HW_SPEED = 74,
    EV_ERR_BP_UNDER_VOLTAGE = 104,
    EV_ERR_BP_OVER_VOLTAGE = 108,
    EV_ERR_BP_UNDER_OVER_VOLTAGE = 112,
    EV_ERR_BP_CHARGING_OVER_CUR = 116,
    EV_ERR_BP_DISCHARGING_OVER_CUR_1 = 132,
    EV_ERR_BP_DISCHARGING_OVER_CUR_2 = 164,
    EV_ERR_BP_SHORT_CIRCUIT = 228,
    EV_ERR_BP_LIMIT_SOFT_START = 292,
    EV_ERR_READ_SN = 401,
    EV_ERR_DETECTED_12V_POWER_SUPPLY = 402,
	EV_ERR_OVER_VOLTAGE_CHARGER = 404,
	EV_ERR_PMU_POWER_LIMIT_1 = 430,
	EV_ERR_PMU_POWER_LIMIT_2 = 431,
    EV_ERR_SIM_NOT_CONNECTED = 501,
    EV_ERR_LOST_LTE_SIGNAL = 502,
    EV_ERR_BLE             = 503,
    EV_ERR_GPS      = 504,
    EV_ERR_DATETIME         = 505,
    EV_ERR_ANTI_THEFT_WARNING = 506,
    EV_ERR_BLOCK_STATE  = 507,
    EV_ERR_LOST_CAN_NETWORK = 508,
    EV_ERR_LOST_INTERNET    = 509,
};

#define SM_EV_ERR_OFFSET_PMU    400
#define SM_EV_ERR_OFFSET_MC     0
#define SM_EV_ERR_OFFSET_HMI    500
#define SM_EV_ERR_OFFSET_BP     100

#define SM_ERR_BP_LIMIT_SOFT_START  (192)

enum {
    EV_BP_UNCHECKED = 0,
    EV_BP_VALID,
    EV_BP_INVALID,
};

enum{
    EV_CMD_LOCK_PORT = 2,
    EV_CMD_UNLOCK_PORT = 1,
};

enum {
    EV_UNLOCK_STATE = 0x10,
    EV_LOCK_STATE = 0x11,
    EV_PRE_LOCK_STATE = 0x12,
    EV_PRE_UNLOCK_STATE = 0x13
};
enum {
	MC_ANTI_INACTIVATED 		= 1,
	MC_ANTI_ACTIVATED_IDLE 		= 2,
	MC_ANTI_ACTIVATED_RUNNING	= 3
};
enum {
    EV_UNBLOCK_STATE = 0x10,
    EV_BLOCK_STATE = 0x11,
    EV_PRE_BLOCK_STATE = 0x12,
    EV_PRE_UNBLOCK_STATE = 0x13
};

enum {
  EV_BP_MODE_DISCHARGE = 0,
  EV_BP_MODE_CHARGER = 1
};

#define EV_PMU_LOWEST_APB_VOLTAGE       9000
#define EV_BP_LOWEST_CHARGING_CUR       1000


#define EV_DATA_KEY_DEFAULT             EV_KEY_OFF
#define EV_DATA_HORN_DEFAULT            0
#define EV_DATA_BRAKE_DEFAULT           0
#define EV_DATA_LEFT_SIGNAL_DEFAULT     0
#define EV_DATA_RIGHT_SIGNAL_DEFAULT    0
#define EV_DATA_PACKING_DEFAULT         EV_ENTER_PARKING
#define EV_DATA_LOW_BEAM_DEFAULT        0
#define EV_DATA_HIGH_BEAM_DEFAULT       0
#define EV_DATA_RANGE_DEFAULT           0
#define EV_DATA_DISCHAR_CUR_LIM_DEFAULT 0
#define EV_DATA_CHAR_CUR_LIM_DEFAULT    0
#define EV_DATA_POWER_IN_DEFAULT        0
#define EV_DATA_POWER_OUT_DEFAULT       0
#define EV_DATA_POWER_PER_KM_DEFAULT    0
#define EV_DATA_ABP_VOL_DEFAULT         0
#define EV_DATA_SPEED_DEFAULT           0
#define EV_DATA_DRIVE_MODE_DEFAULT      EV_SPORT_MODE_1
#define EV_DATA_TRIP_DEFAULT            (-1)
#define EV_DATA_MC_STATUS_DEFAULT       0
#define EV_DATA_MOTOR_TEMP_DEFAULT      0
#define EV_DATA_BOARD_TEMP_DEFAULT      0
#define EV_DATA_REVERSER_STATE_DEFAULT  EV_REVERSE_STATE_INACTIVE
#define EV_DATA_THROTTLE_CMD_DEFAULT    0
#define EV_DATA_ESTIMATE_TOR_DEFAULT    0
#define EV_DATA_ESTIMATE_DC_CUR_DEFAULT 0
#define EV_DATA_ALLOW_DC_CUR_DEFAULT    0
#define EV_DATA_AVE_EFFIC_DEFAULT       0
#define EV_DATA_ENR_CON_EFFIC_DEFAULT   0
#define EV_DATA_ENR_DUR_TRIP_DEFAULT    0
#define EV_DATA_MC_REV_CNT_DEFAULT      0
#define EV_DATA_ANTI_STATUS_DEFAULT     0

#define EV_DATA_ODO_DEFAULT             0
#define EV_DATA_ERR_DEFAULT             0

typedef struct {
    uint8_t     m_status;
    uint16_t    m_speed_rpm;
    uint16_t    m_speed_kmh;
    uint32_t    m_trip_m;
    uint16_t    m_motor_temp;
    uint8_t     m_err_code;
    uint8_t     m_reverse_state;
    uint8_t     m_thr_cmd;
    uint8_t     m_est_tor;
    uint8_t     m_est_dc_cur;
    uint8_t     m_allow_dc_cur;
    uint8_t     m_cur_mode;
    uint8_t     m_board_temp;
    uint32_t    m_revol_cnt;
    uint8_t     m_aver_effic;
    uint8_t     m_enr_con_effic;
    uint16_t    m_enr_dur_trip;
    uint16_t    m_anti_theft_st;
    uint32_t    charge_capacitor_time;
    ev_purpose  m_mc_purpose;
}sm_mc_data_t;

typedef struct {
    uint8_t m_key;
    uint8_t m_horn;
    uint8_t m_brake;
    uint8_t m_left_signal;
    uint8_t m_right_signal;
    uint8_t m_drive_mode_signal;
    uint8_t m_emergency;
    uint8_t m_parking;
    uint8_t m_pos_light_sw;
    uint8_t m_low_beam_light_sw;
    uint8_t m_high_beam_light_sw;
    uint8_t m_charger_st;
}sm_ioc_data_input_t;
typedef union {
    sm_ioc_data_input_t st;   // Truy cập theo field
    uint8_t arr[sizeof(sm_ioc_data_input_t)]; // Truy cập như mảng
} ioc_data_input_t;
typedef struct {

    uint8_t m_left_light;
    uint8_t m_right_light;
    uint8_t m_emergency;

    uint8_t m_high_beam_light;
    uint8_t m_pos_light;
    uint8_t m_low_beam_light;

    uint8_t m_horn;

    uint8_t m_mc_power;
    uint8_t m_charger;
}sm_ioc_data_output_t;

typedef union {
    sm_ioc_data_output_t st;   // Truy cập theo field
    uint8_t arr[sizeof(sm_ioc_data_output_t)]; // Truy cập như mảng
} ioc_data_output_t;
typedef struct {
    uint8_t m_key;
    uint8_t m_horn;
    uint8_t m_brake;
    uint8_t m_drive_mode_signal;
    uint8_t m_backward_mode_signal;
    uint8_t m_uphill_mode;
    uint8_t m_lock_status;
    uint8_t m_block_status;
    uint8_t m_anti_theft_status;
    uint8_t m_inactive_mode;
    uint8_t m_left_signal;
    uint8_t m_right_signal;
    uint8_t m_parking;
    uint8_t m_low_beam_state;
    uint8_t m_pos_light_state;
    uint8_t m_high_beam_state;
    uint16_t m_range;
    uint16_t m_err_code;
    uint32_t m_energy_in;
    uint32_t m_energy_out;
    uint8_t m_power_per_km;
    uint16_t m_abp_voltage;
    uint8_t m_bp_mode;
    uint8_t m_bp_checking_state[SM_BP_NUMBER_DEFAULT];
    uint8_t m_port_lock_status[SM_BP_NUMBER_DEFAULT];
    uint32_t m_charge_cur_lim;
    uint32_t m_discharge_cur_lim;
    uint8_t m_purpose_state;
}sm_pmu_data_t;

static inline void sm_pmu_data_reset(sm_pmu_data_t* _this){
    _this->m_energy_in = EV_DATA_POWER_IN_DEFAULT;
    _this->m_energy_out = EV_DATA_POWER_OUT_DEFAULT;
    _this->m_power_per_km = EV_DATA_POWER_PER_KM_DEFAULT;
    _this->m_range = EV_DATA_RANGE_DEFAULT;
    _this->m_discharge_cur_lim = EV_DATA_DISCHAR_CUR_LIM_DEFAULT;
    _this->m_charge_cur_lim = EV_DATA_CHAR_CUR_LIM_DEFAULT;
    _this->m_lock_status = EV_UNLOCK_STATE;
    _this->m_block_status = EV_UNBLOCK_STATE;
    _this->m_anti_theft_status = EV_UNBLOCK_STATE;
    _this->m_inactive_mode = EV_ACTIVE_STATE;
    _this->m_parking = EV_INACTIVE_STATE;
}

typedef struct {
    sm_pmu_data_t* m_pmu_data;
    sm_mc_data_t* m_mc_data;

    uint8_t m_max_speed;
    uint8_t m_driver_mode;

    int32_t m_err;
    uint32_t m_odo;
    int32_t m_speed;
    int32_t m_trip;
    uint8_t m_active;
    uint8_t m_motor_active_state;
}sm_ev_data_t;

typedef struct {
    uint8_t m_max_speed;
    uint8_t m_driver_mode;
}sm_ev_opt_t;

typedef struct{
    int32_t m_odo;
}sm_ev_sec_opt_t;

typedef struct gps_coordinate{
    float lat;
    float lon;
}sm_gps_coordinate_t;

enum {
    SM_EV_MODULE_MC = 0,
    SM_EV_MODULE_PMU,
    SM_EV_MODULE_HMI,
    SM_EV_MODULE_BP,
    SM_EV_MODULE_NUMBER
};

static inline const char* sm_ev_convert_module_to_string(uint8_t _type){
    switch (_type) {
        case SM_EV_MODULE_MC:
            return "mc";
        case SM_EV_MODULE_PMU:
            return "pmu";
        case SM_EV_MODULE_HMI:
            return "hmi";
        case SM_EV_MODULE_BP:
            return "bp";
        default:
            return "module_unknown";
    }
}

static inline void sm_ev_version_to_string(char byte[], char* ver_string){
    if(!ver_string){
        return;
    }
    int len = sprintf(ver_string, "%d.%d.%d.%d",
            byte[2],
            byte[1],
            byte[0],
            byte[3]);
    ver_string[len] = '\0';
}
static inline int32_t sm_ev_version_compare(const char* _src, char* _other){
    char src_ver[3];

    src_ver[0] = _src[2];
    src_ver[1] = _src[1];
    src_ver[2] = _src[0];
    return sm_math_compare_char_arr(src_ver, _other, 3);
}

#endif //EV_SDK_SM_CORE_EV_DATA_H
