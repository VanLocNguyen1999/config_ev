//
// Created by admin on 9/23/2024.
//
#include <stdlib.h>
#include <string.h>
#include "sm_l76x_gps.h"
#include "time.h"
#include "sm_types.h"
#include "sm_logger.h"
#include "sm_string_util.h"
#include "sm_memory.h"
#define GPS_READ_DATA_TIMEOUT 100
const char* TAG = "L76X GPS";
enum {
    GNRMC = 0,
    GNVTG,
    GNGGA,
    GPGSV,
    GNGLL,
};
const char* NMEA[] = {
    "$GNRMC",
    "$GNVTG",
    "$GNGGA",
    "$GPGSV",
    "$GNGLL",
};

static char g_gps_buf[GPS_BUFFER_LENGTH] = {0, };

sm_l76x_t g_l76x = {
    .m_datetime.year = 0, .m_datetime.mon = 1, .m_datetime.day = 1,
    .m_datetime.hour = 0, .m_datetime.min = 0, .m_datetime.sec = 0,
    .m_lat = 0.0, .m_lon = 0.0,
};

static void GNRMC_unpack(sm_l76x_t* _this, const char* data_handle);
static double convert_nmea_to_decimal(double value, char direction);
static uint8_t gps_check_data(sm_l76x_t* _this);

static void gps_recv_data_callback(uint8_t _data, void* _arg){
    if (!_arg) {
        return;
    }
    sm_l76x_t *modem = (sm_l76x_t*) _arg;
	char* buff = (char*)modem->buff;

    if(modem->buf_index < GPS_BUFFER_LENGTH){
        buff[modem->buf_index++] = (char)_data;
    }
}

static uint32_t gps_read_data_from_queue(sm_l76x_t* _this, char* buff){
    memcpy(buff, _this->buff, _this->buf_index);
	return _this->buf_index;
}

sm_l76x_t* sm_l76x_init(sm_hal_uart_t *_if, sm_hal_io_t *_rst) {
    g_l76x.m_driver = _if;
    g_l76x.m_rst_pin = _rst;
    sm_l76x_reboot(&g_l76x);
    g_l76x.buff = g_gps_buf;
    g_l76x.buf_index = 0;
    g_l76x.data_is_valid = 0;
    g_l76x.m_time_valid = 0;
    sm_hal_uart_set_rx_cb(_if, gps_recv_data_callback, &g_l76x);
    return &g_l76x;
}

int32_t sm_l76x_reboot(sm_l76x_t* _this) {
    if (!_this) {
        return -1;
    }
    sm_hal_io_set_value(_this->m_rst_pin, 1);
    sm_hal_delay_ms(200);
    sm_hal_io_set_value(_this->m_rst_pin, 0);
    return 0;
}

int32_t sm_l76x_process(sm_l76x_t* _this) {
    if(_this->buf_index > 0){
//        LOG_DBG("GPS", "data: %s", _this->buff);
        char* sub = strstr(_this->buff, NMEA[GNRMC]);
        while (sub){
            GNRMC_unpack(_this, sub);

            sub = strstr(sub + strlen(NMEA[GNRMC]), NMEA[GNRMC]);
        }

        memset(_this->buff, 0, GPS_BUFFER_LENGTH);
        _this->buf_index = 0;
    }
    return 0;
}

static uint8_t gps_check_data(sm_l76x_t* _this){
    if (_this->m_datetime.year > 99 ||
        _this->m_datetime.mon == 0 || _this->m_datetime.mon > 12 ||
        _this->m_datetime.day == 0 || _this->m_datetime.day > 31 ||
        _this->m_datetime.hour >= 24 ||
        _this->m_datetime.min > 59 ||
        _this->m_datetime.sec > 59 ||
        _this->m_lat > 90 || _this->m_lon > 180){
        return 0;
    }
    return 1;
}

static void GNRMC_unpack(sm_l76x_t* _this, const char* data_handle) {
    // $GNRMC,090327.000,A,2102.217399,N,10547.273680,E,0.00,199.06,230924,,,A,V*05
    char* token;
    char nmea_gnrmc[257];
    memset(nmea_gnrmc, 0, 257);
    if (shortest_substring(data_handle, NMEA[GNRMC], "\r\n", nmea_gnrmc, 256) <= strlen(NMEA[GNRMC])) {
        _this->data_is_valid = 0;
        return;
    }

    token = sm_strtok(nmea_gnrmc, ",");
    if (strcmp(token, NMEA[GNRMC]) != 0) {
        _this->data_is_valid = 0;
        return; // Not a GNRMC sentence
    }

    // Time HHMMSS.000 format
    token = sm_strtok(NULL, ",");
    if (token != NULL) {
        _this->m_datetime.hour = (token[0] - '0') * 10 + (token[1] - '0');
        _this->m_datetime.min = (token[2] - '0') * 10 + (token[3] - '0');
        _this->m_datetime.sec = (token[4] - '0') * 10 + (token[5] - '0');

        if (_this->m_datetime.hour >= 24 ||
            _this->m_datetime.min > 59 ||
            _this->m_datetime.sec > 59) {

            _this->m_datetime.hour = 0;
            _this->m_datetime.min = 0;
            _this->m_datetime.sec = 0;
        }

        if(_this->m_datetime.hour == 23 || _this->m_datetime.hour == 0){
            _this->m_time_valid = 0;
        }else{
            _this->m_time_valid = 1;
        }
    }

    token = sm_strtok(NULL, ",");
    if (token == NULL || *token != 'A') {
        _this->data_is_valid = 0;
        return;
    }

    // Latitude
    token = sm_strtok(NULL, ",");
    if (token != NULL) {
        double latitude = sm_atof(token);
        token = sm_strtok(NULL, ","); // Direction (N/S)
        _this->m_lat = convert_nmea_to_decimal(latitude, *token);
    }

    // Longitude
    token = sm_strtok(NULL, ",");
    if (token != NULL) {
        double longitude  = sm_atof(token);
        token = sm_strtok(NULL, ","); // Direction (E/W)
        _this->m_lon = convert_nmea_to_decimal(longitude, *token);
    }

    // Skip unused fields
    for (int i = 0; i < 2; i++) {
        token = sm_strtok(NULL, ",");
    }

    // DDMMYY
    token = sm_strtok(NULL, ",");
    if (token != NULL) {
        _this->m_datetime.day = (token[0] - '0') * 10 + (token[1] - '0');
        _this->m_datetime.mon = (token[2] - '0') * 10 + (token[3] - '0');
        _this->m_datetime.year = (token[4] - '0') * 10 + (token[5] - '0');

        if (_this->m_datetime.year > 99 ||
        _this->m_datetime.mon == 0 || _this->m_datetime.mon > 12 ||
        _this->m_datetime.day == 0 || _this->m_datetime.day > 31 ){

            _this->m_datetime.year = 0;
            _this->m_datetime.mon = 1;
            _this->m_datetime.day = 1;
        }
        _this->m_time_valid = 2;
    }

    _this->data_is_valid = gps_check_data(_this);
}

static double convert_nmea_to_decimal(double value, char direction) {
    int degrees = (int)(value / 100);
    double minutes = value - (degrees * 100);
    double decimal_degrees = degrees + minutes / 60.0;

    if (direction == 'S' || direction == 'W') {
        decimal_degrees = -decimal_degrees;
    }
    return decimal_degrees;
}

