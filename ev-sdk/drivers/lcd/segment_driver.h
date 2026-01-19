/*
 * segment_driver.h
 *
 *  Created on: Mar 15, 2022
 *      Author: khanh
 */

#ifndef SERVICE_SEGMENT_DRIVER_SEGMENT_DRIVER_H_
#define SERVICE_SEGMENT_DRIVER_SEGMENT_DRIVER_H_

#include "stdbool.h"
#include "stdint.h"

#define SEGMENT_BLANK   10

typedef struct Digit_bitmap_t Digit_bitmap;
struct Digit_bitmap_t{
    uint8_t  a:1;
    uint8_t  b:1;
    uint8_t  c:1;
    uint8_t  d:1;
    uint8_t  e:1;
    uint8_t  f:1;
    uint8_t  g:1;
};

typedef struct bits_t bits;
struct bits_t{
    uint8_t    b0:1;
    uint8_t    b1:1;
    uint8_t    b2:1;
    uint8_t    b3:1;
    uint8_t    b4:1;
    uint8_t    b5:1;
    uint8_t    b6:1;
    uint8_t    b7:1;
};

typedef union Segment_bytes_t Segment_bytes;
union Segment_bytes_t{
    uint8_t byte;
    bits    bits;
};

void segment_clear_digit_value(Digit_bitmap* p_seg, const uint8_t digit_num);
void segment_set_negative_value(Digit_bitmap* p_seg, const uint8_t digit_num, int32_t val);
void segment_update_digit_value(Digit_bitmap* p_seg, const uint8_t digit_num, const uint8_t min_digit_show, const int32_t val);


#endif /* SERVICE_SEGMENT_DRIVER_SEGMENT_DRIVER_H_ */
