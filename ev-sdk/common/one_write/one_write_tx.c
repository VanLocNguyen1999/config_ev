#include "one_write_tx.h"
#include "hal_data.h"
/* ================================================================
 * Timing (µs)
 * ================================================================ */
#define T_SYNC_LOW    10000   /* LOW  10ms  */
#define T_SYNC_HIGH    1000   /* HIGH  1ms  */

#define T_BIT1_LOW      500   /* LOW  0.5ms */
#define T_BIT1_HIGH    1500   /* HIGH 1.5ms */

#define T_BIT0_LOW     1500   /* LOW  1.5ms */
#define T_BIT0_HIGH     500   /* HIGH 0.5ms */

#define T_STOP_LOW     5000   /* LOW   5ms  */
#define T_STOP_HIGH   50000   /* HIGH 50ms  */

#define DELAY_US(x)			R_BSP_SoftwareDelay(x, BSP_DELAY_UNITS_MICROSECONDS)

static inline void _low(OneWireTx_t *_this) {
	sm_hal_io_set_value(_this->_port, 1);
}
static inline void _high(OneWireTx_t *_this) {
	sm_hal_io_set_value(_this->_port, 0);
}

static void _sync(OneWireTx_t *_this) {
	_low(_this);
	DELAY_US(T_SYNC_LOW);
	_high(_this);
	DELAY_US(T_SYNC_HIGH);
}

static void _bit(OneWireTx_t *this, uint8_t b) {

	if (b) {
		_low(this);
		DELAY_US(T_BIT1_LOW);
		_high(this);
		DELAY_US(T_BIT1_HIGH);
	} else {
	_low(this);
	DELAY_US(T_BIT0_LOW);
	_high(this);
	DELAY_US(T_BIT0_HIGH);
	}
}

static void _byte(OneWireTx_t *this, uint8_t v) {
	for (uint8_t i = 0; i < 8; i++)
		_bit(this, (v >> i) & 1); /* LSB first */
}

static void _stop(OneWireTx_t *this){

    _low(this);
    DELAY_US(T_STOP_LOW);
    _high(this);
    DELAY_US(T_STOP_HIGH);
}

static uint8_t _check_sum(const uint8_t *buf, uint8_t len) {

	uint16_t s = 0;
	for (uint8_t i = 0; i < len; i++) {
		s += buf[i];
	}
	return (uint8_t) (s & 0xFF);
}

/* ================================================================
 * Method: send
 * ================================================================ */

static OWStatus _send(OneWireTx_t *this, const uint8_t *data, uint8_t len) {

	if (!this->_initialized)
		return OW_ERR_NOT_INIT;
	if (!data || len == 0 || len > OW_MAX_DATA)
		return OW_ERR_LEN;
	/* Build frame */
	uint8_t buf[OW_MAX_DATA + 4];
	uint8_t index = 0;

	buf[index++] = OW_MSG_ID;
	buf[index++] = OW_VERSION;
	buf[index++] = len;
	for (uint8_t i = 0; i < len; i++) {
		buf[index++] = data[i];
	}

	uint8_t checksum = _check_sum(buf, index);
	buf[index++] = checksum;
	/* Transmit */
	_sync(this);
	for (uint8_t i = 0; i < index; i++) {
		_byte(this, buf[i]);
	}

	_stop(this);
	this->tx_count++;
	return OW_OK;
}

/* ================================================================
 * Constructor
 * ================================================================ */

OWStatus OneWireTx_Init(OneWireTx_t *this, sm_hal_io_t *port) {
	if (!this || !port)
		return OW_ERR_NULL;

	memset(this, 0, sizeof(OneWireTx_t));
	this->_port = port;
	this->send = _send;
	this->_initialized = 1;
	_high(this);
	return OW_OK;
}
