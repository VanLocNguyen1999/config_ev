/*
 * sm_math.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 */

#ifndef UTILS_INCLUDE_SM_MATH_H_
#define UTILS_INCLUDE_SM_MATH_H_

#include <stdint.h>
#include <stdlib.h>
#include "CO_utils.h"


/**
 * @brief
 * @param max
 * @param min
 * @return
 */
static inline int sm_rand(int max, int min){
    return (rand() % (max - min + 1) + min);
}

static inline int32_t sm_is_in_bound(int32_t _value, int32_t _min, int32_t _max, int32_t _default){
    return (_value >= _min) && (_value <= _max) ? _value : _default;
}

#define getUint16(byte)                 CO_getUint16(byte)
#define getUint32(byte)                 CO_getUint32(byte)
#define setUint16(byte,value)           CO_setUint16(byte, value)
#define setUint32(byte,value)           CO_setUint32(byte, value)
#define setBit(reg,value,bit_index)     CO_WRITE_BIT(reg, value, bit_index)
#define getBit(reg,bit_index)           CO_READ_BIT(reg, bit_index)

#endif /* UTILS_INCLUDE_SM_MATH_H_ */
