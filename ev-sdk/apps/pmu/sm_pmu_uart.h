/*
 * sm_pmu_uart.h
 *
 *  Created on: 25 Sept 2025
 *      Author: My PC
 */

#ifndef SM_PMU_UART_H_
#define SM_PMU_UART_H_
#ifdef __cplusplus
extern "C"{
#endif
#include "uart_module.h"
#include "ascii_msg_builder.h"
#include "stdio.h"

typedef void sm_pmu_uart_t;
enum {
    CMD_POLL = 0,
    CMD_INPUT,
    CMD_OUTPUT,
    CMD_VOLT_CHARGER,

    CMD_LEFT_LIGHT,
    CMD_RIGH_LIGHT,
    CMD_HORN,
    CMD_EMERGENCY,
    CMD_HIGH_BEAM_LIGHT,
    CMD_POS_BEAM_LIGHT,
    CMD_LOW_BEAM_LIGHT,
    CMD_CHARGER,
    CMD_MC_POWER,

    CMD_RESPONSE,
    CMD_NUMBER
};

extern const char *UART_CMD[CMD_NUMBER];
sm_pmu_uart_t* sm_pmu_uart_create_default(void);
//int32_t sm_pmu_uart_handle_rec_data(sm_pmu_uart_t* _this, void* _arg);
int32_t sm_pmu_uart_polling_msg(sm_pmu_uart_t* _this, void* _arg);
int32_t sm_pmu_uart_cmd_msg(sm_pmu_uart_t* _this, void* _arg);

bool sm_cmd_push(sm_pmu_uart_t* _this, char* data);
bool sm_cmd_pop(sm_pmu_uart_t* _this, char* data);

static inline void ioc_data_to_string(uint8_t *arr, size_t arr_len, char *out_buf, uint32_t buf_size) {
    uint32_t len = 0;
    for (size_t i = 0; i < arr_len && len < buf_size - 1; i++) {
        int written = snprintf(out_buf + len, buf_size - len, "%d", arr[i]);
        if (written < 0) break; // lỗi snprintf
        if (len + (uint32_t)written >= buf_size) break; // tránh tràn buffer
        len += (uint32_t)written;
    }
    out_buf[len] = '\0';
}


#ifdef __cplusplus
};
#endif
#endif /* SM_PMU_UART_H_ */
