/*
 * one_write_tx.h
 *
 *  Created on: 29 Jun 2026
 *      Author: My PC
 */
/**
 * @file  onewire_tx.h
 * @brief Gửi dữ liệu qua 1 dây GPIO theo giao thức PWM-encoded
 *
 *  Frame:
 *    [0]    0x3A      — MSG ID   (cố định)
 *    [1]    0x11      — Version  (cố định)
 *    [2]    len       — Số byte data (1~8)
 *    [3..N] data      — Payload bất kỳ
 *    [N+1]  checksum  — Tổng byte 0→N, lấy 8 bit thấp
 *
 *  Dùng:
 *    OneWireTx tx;
 *    OneWireTx_Init(&tx, GPIOA, GPIO_PIN_1, &htim2);
 *
 *    uint8_t buf[] = {0x01, 0x02, 0x03};
 *    tx.send(&tx, buf, 3);
 */
#ifndef ONE_WRITE_TX_H_
#define ONE_WRITE_TX_H_
#include "sm_types.h"
#include "sm_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OW_MSG_ID       0x3A
#define OW_VERSION      0x11
#define OW_MAX_DATA     8

typedef enum {
    OW_OK = 0,
    OW_ERR_NULL,
    OW_ERR_NOT_INIT,
    OW_ERR_LEN,
} OWStatus;

typedef struct OneWireTx OneWireTx_t;

struct OneWireTx {
    /* private */
    sm_hal_io_t        *_port;
    uint8_t              _initialized;

    /* stats */
    uint32_t tx_count;
    uint32_t tx_error;

    /* method */
    OWStatus (*send)(OneWireTx_t *self, const uint8_t *data, uint8_t len);
};

OWStatus OneWireTx_Init(OneWireTx_t *self, sm_hal_io_t *port);

#ifdef __cplusplus
}
#endif

#endif /* ONE_WRITE_TX_H_ */
