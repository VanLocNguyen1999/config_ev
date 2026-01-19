//
// Created by vnbk on 28/09/2024.
//

#ifndef EV_SDK_SM_HMI_SIGNAL_LIGHT_H
#define EV_SDK_SM_HMI_SIGNAL_LIGHT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_hal_io.h"
#include "sm_hal_adc.h"

#define ADC_12BIT_RESOLUTION            (4095)
#define ADC_REFERENCE_VOLTAGE_mV        (3300)
#define INPUT_VOLTAGE_DIVIDER_x100      (570)

typedef struct {
    sm_hal_io_t* m_left_signal;
    sm_hal_io_t* m_right_signal;
    sm_hal_adc_t* m_input_voltage;
}sm_hmi_io_t;

static inline void sm_hmi_io_set_left_signal(sm_hmi_io_t* _this, uint8_t _value){
    sm_hal_io_set_value(_this->m_left_signal, _value);
}
static inline void sm_hmi_io_set_right_signal(sm_hmi_io_t* _this, uint8_t _value){
    sm_hal_io_set_value(_this->m_right_signal, _value);
}
static inline void sm_hmi_io_set_signal(sm_hmi_io_t* _this, uint8_t _value){
    sm_hal_io_set_value(_this->m_left_signal, _value);
    sm_hal_io_set_value(_this->m_right_signal, _value);
}
static inline void sm_hmi_io_turn_off_signal(sm_hmi_io_t* _this){
    sm_hal_io_set_value(_this->m_left_signal, 0);
    sm_hal_io_set_value(_this->m_right_signal, 0);
}
static inline void sm_hmi_io_blink_signal(sm_hmi_io_t* _this){
    sm_hal_io_set_value(_this->m_left_signal, !sm_hal_io_get_value(_this->m_left_signal));
    sm_hal_io_set_value(_this->m_right_signal, !sm_hal_io_get_value(_this->m_right_signal));
}

static inline int32_t sm_hmi_io_get_input_vol(sm_hmi_io_t* _this){
    int32_t adc_value = sm_hal_adc_read(_this->m_input_voltage, 12);
    return adc_value*ADC_REFERENCE_VOLTAGE_mV/100*INPUT_VOLTAGE_DIVIDER_x100/ADC_12BIT_RESOLUTION;
}

#ifdef __cplusplus
};
#endif

#endif //EV_SDK_SM_HMI_SIGNAL_LIGHT_H
