
#include "sm_math.h"
#include "stdio.h"
#include "math.h"

int8_t sm_math_check_float_equal(float a, float b, float epsilon) {
    return fabs(a - b) < epsilon;
}

uint32_t sm_math_get_two_complement_value(uint32_t _number){
    return ~_number + 1;
}

int32_t sm_math_compare_char_arr(char* _first, char* _second, uint32_t _len){
    if(!_first || !_second){
        return -2;
    }
    for(uint32_t i = 0; i < _len; i++){
        if(_first[i] < _second[i]){
            return -1;
        }
        if(_first[i] > _second[i]){
            return 1;
        }
    }
    return 0;
}

