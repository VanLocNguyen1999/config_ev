/*
 * sm_math.h
 *
 *  Created on: Oct 25, 2024
 *      Author: vuonglk
 */

#ifndef EV_SDK_UTILS_SM_MATH_H_
#define EV_SDK_UTILS_SM_MATH_H_
#include "stdint.h"

int8_t sm_math_check_float_equal(float a, float b, float epsilon);

uint32_t sm_math_get_two_complement_value(uint32_t _number);

int32_t sm_math_compare_char_arr(char* _first, char* _second, uint32_t _len);

#define CLAMP(x, low, high)  (((x) < (low)) ? (low) : ((x) > (high)) ? (high) : (x))

#endif /* EV_SDK_UTILS_SM_MATH_H_ */
