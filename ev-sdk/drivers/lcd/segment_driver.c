/*
 * segment_driver.c
 *
 *  Created on: Mar 15, 2022
 *      Author: khanh
 */

#include "segment_driver.h"

/*       ----A----                        a  b  c  d  e  f  g
 *      |         |                   0_|__|__|__|__|__|__|__|
 *      F         B                   1_|__|__|__|__|__|__|__|
 *      |         |                   2_|__|__|__|__|__|__|__|
 *       ----G----                    3_|__|__|__|__|__|__|__|
 *      |         |                   4_|__|__|__|__|__|__|__|
 *      E         C                   5_|__|__|__|__|__|__|__|
 *      |         |                   6_|__|__|__|__|__|__|__|
 *       ----D----                    .....
 */

#define BLANK           10
#define MID_LINE        11

Digit_bitmap  _digits[] =        {   {.a = 1, .b = 1, .c = 1, .d = 1, .e = 1, .f = 1, .g = 0},   // 0
                                     {.a = 0, .b = 1, .c = 1, .d = 0, .e = 0, .f = 0, .g = 0},   // 1
                                     {.a = 1, .b = 1, .c = 0, .d = 1, .e = 1, .f = 0, .g = 1},   // 2
                                     {.a = 1, .b = 1, .c = 1, .d = 1, .e = 0, .f = 0, .g = 1},   // 3
                                     {.a = 0, .b = 1, .c = 1, .d = 0, .e = 0, .f = 1, .g = 1},   // 4
                                     {.a = 1, .b = 0, .c = 1, .d = 1, .e = 0, .f = 1, .g = 1},   // 5
                                     {.a = 1, .b = 0, .c = 1, .d = 1, .e = 1, .f = 1, .g = 1},   // 6
                                     {.a = 1, .b = 1, .c = 1, .d = 0, .e = 0, .f = 0, .g = 0},   // 7
                                     {.a = 1, .b = 1, .c = 1, .d = 1, .e = 1, .f = 1, .g = 1},   // 8
                                     {.a = 1, .b = 1, .c = 1, .d = 1, .e = 0, .f = 1, .g = 1},   // 9
                                     {.a = 0, .b = 0, .c = 0, .d = 0, .e = 0, .f = 0, .g = 0},   // Blank
                                     {.a = 0, .b = 0, .c = 0, .d = 0, .e = 0, .f = 0, .g = 1}    // Mid_line
};

static uint8_t get_digit_number_of_value(const uint8_t amx_digit_num, const int32_t val);

void segment_clear_digit_value(Digit_bitmap* p_seg, const uint8_t digit_num){

    for(uint8_t i = 0; i < digit_num; i++){
        p_seg[i] = _digits[BLANK];
    }
}

void segment_set_negative_value(Digit_bitmap* p_seg, const uint8_t digit_num, int32_t val){

    int32_t temp_val = 0 - val;
    int32_t digit_val;

    for(uint8_t i = 0; i < digit_num - 1; i++){

        digit_val = temp_val%10;
        p_seg[i] = _digits[digit_val];
        temp_val /= 10;
    }

    p_seg[digit_num -1] = _digits[MID_LINE];
}

void segment_update_digit_value(Digit_bitmap* p_seg, const uint8_t digit_num, const uint8_t min_digit_show, const int32_t val){

    int32_t temp_val = val;
    int32_t digit_val;
    uint8_t real_digit_num = get_digit_number_of_value(digit_num, val);

    for(uint8_t i = 0; i < digit_num; i++){

        digit_val = temp_val%10;
        if((digit_val == 0) && (i >= min_digit_show) && (i >= real_digit_num)){
            p_seg[i] = _digits[BLANK];
        }
        else p_seg[i] = _digits[digit_val];
        temp_val /= 10;
    }
}

static uint8_t get_digit_number_of_value(const uint8_t max_digit_num, const int32_t val){

    if(val != 0){
        int32_t threshold_value = 10;
        for(uint8_t i = 1; i < max_digit_num+1; i++){
            threshold_value *= 10;
            if(val < threshold_value) return i;
        }
    }
    return 1;
}
