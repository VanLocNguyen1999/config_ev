/*
 * sm_counter_utils.h
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */

#ifndef TIMING_SM_COUNTER_UTILS_H_
#define TIMING_SM_COUNTER_UTILS_H_
#include "sm_types.h"
#include "hal_data.h"
#ifdef __cplusplus
extern "C" {
#endif

void     sm_counter_init(void){

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}
uint32_t sm_counter_get(void){                   /* Lấy counter hiện tại      */

	return DWT->CYCCNT;
}
uint32_t sm_counter_elapsed_us(uint32_t start) {  /* Thời gian từ start đến nay (µs) */

	return (DWT->CYCCNT - start) / (SystemCoreClock / 1000000UL);
}
uint32_t sm_counter_elapsed_ms(uint32_t start){   /* Thời gian từ start đến nay (ms) */

	return (DWT->CYCCNT - start) / (SystemCoreClock / 1000UL);
}

#ifdef __cplusplus
}
#endif

#endif /* TIMING_SM_COUNTER_UTILS_H_ */
