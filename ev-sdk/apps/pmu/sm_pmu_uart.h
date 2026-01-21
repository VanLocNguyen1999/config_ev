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
    CMD_IDLE_READ = 0,
	CMD_IDLE_WRITE,
    CMD_IDLE_NUMBER
};

typedef enum {
    UART_CFG_IDLE = 0,
	UART_CFG_READ,
	UART_CFG_WRITE,
	UART_CFG_NUMBER
}UART_CFG_STATE;

sm_pmu_uart_t* sm_pmu_uart_create_default(void);
//int32_t sm_pmu_uart_handle_rec_data(sm_pmu_uart_t* _this, void* _arg);
int32_t sm_pmu_uart_polling_msg(sm_pmu_uart_t* _this, void* _arg);
int32_t sm_pmu_uart_cmd_msg(sm_pmu_uart_t* _this, void* _arg);

int32_t sm_uart_process(sm_pmu_uart_t* _this, void* _arg);

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
