//
// Created by admin on 9/24/2024.
//
#include "sm_mdbt42q_ble.h"
#include "sm_logger.h"

#define MDBT42_BUFFER_SIZE                      512

#define BT_RETRIES_INIT                         10
#define MODEM_BAUD_RATE_DF                      9600
#define MODEM_READ_BYTES_DELAY_MS(x)            (x)

#define BLE_RES_SUCCESS	"success"
#define BLE_RES_FAIL	"fail"

static const char* TAG = "BLUETOOTH";

static sm_mdbt42q_t g_mdbt42q_default = {
        .m_base = {
                .driver = NULL,
                .buff = NULL,
                .buff_size = 0,
                .reset_pin = NULL,
                .lock = false
        },
        .m_indicator_logic_on_connecting = 0,
        .m_indicator_pin = NULL,
};

sm_mdbt42q_t* sm_mdbt42q_create(sm_hal_uart_t* _driver,
                                sm_hal_io_t* _rst,
                                sm_hal_io_t* _ind,
                                sm_hal_io_t* _wk){
    if(!_driver || !_rst || !_ind || !_wk) {
        LOG_ERR(TAG, "Invalid arguments");
        return NULL;
    }
    sm_mdbt42q_t* mdbt42q = &g_mdbt42q_default;

    sm_modem_init(&mdbt42q->m_base, _rst, _driver, MDBT42_BUFFER_SIZE);
    mdbt42q->m_indicator_pin = _ind;

    return mdbt42q;
}

int32_t sm_mdbt42q_init(sm_mdbt42q_t* _this){
     if(!_this) {
         LOG_ERR(TAG, "Invalid arguments");
         return -1;
     }

    int32_t err = 0;
    sm_modem_hw_reset(&_this->m_base, 500, 1);
    LOG_INF(TAG, "MDBT42Q reset modem");
    sm_hal_delay_ms(1500);
    err = 0;
    err += sm_mdbt42q_set_default(_this);
    err += sm_mdbt42q_get_indicator_logic(_this);
    if (err) {
        return -1;
    }
    sm_mdbt42q_start_advertising(_this);
    return 0;
}

void sm_mdbt42q_deinit(sm_mdbt42q_t* _this) {
    if(!_this) {
        return;
    }
}

int32_t sm_mdbt42q_set_name(sm_mdbt42q_t* _this, char* _name) {
    if(!_this || !_name) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    char buff[64];
    memset(buff, 0, 64);
    sprintf(buff, "AT+NAME%s", _name);
    int32_t ret = sm_modem_send_cmd(modem,
									buff,
									_name,
									_name,
									MDBT42Q_WAIT_TIMEOUT_MS);
    sm_mdbt42q_soft_reset(_this);
    return (ret == -1) ? -1 : 0;
}

int32_t sm_mdbt42q_start_advertising(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    sm_modem_send_cmd(modem,
                      "AT+ADVSTOP",
                      BLE_RES_SUCCESS,
                      BLE_RES_FAIL,
                      MDBT42Q_WAIT_TIMEOUT_MS);
    return sm_modem_send_cmd(modem,
                             "AT+ADVSTART",
                             BLE_RES_SUCCESS,
                             BLE_RES_FAIL,
                             MDBT42Q_WAIT_TIMEOUT_MS);
}

int32_t sm_mdbt42q_stop_advertising(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    sm_modem_send_cmd(modem, "AT+ADVSTART",
                      BLE_RES_SUCCESS,
                      BLE_RES_FAIL,
                      MDBT42Q_WAIT_TIMEOUT_MS);
    return sm_modem_send_cmd(modem,
                             "AT+ADVSTOP",
                             BLE_RES_SUCCESS,
                             BLE_RES_FAIL,
                             MDBT42Q_WAIT_TIMEOUT_MS);
}

int32_t sm_mdbt42q_disconnect(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    return sm_modem_send_cmd(modem,
                             "AT+DISCONNECT",
                             BLE_RES_SUCCESS,
                             BLE_RES_FAIL,
                             MDBT42Q_WAIT_TIMEOUT_MS);
}

int32_t sm_mdbt42q_set_default(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    int32_t ret = sm_modem_send_cmd(modem,
                                    "AT+DEFAULT",
                                    BLE_RES_SUCCESS,
                                    BLE_RES_FAIL,
                                    MDBT42Q_WAIT_TIMEOUT_MS);
    sm_mdbt42q_soft_reset(_this);
    return ret;
}

int32_t sm_mdbt42q_soft_reset(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    sm_modem_send_cmd(modem,
                      "AT+RESET",
                      BLE_RES_SUCCESS,
                      BLE_RES_FAIL,
                      MDBT42Q_WAIT_TIMEOUT_MS);
    return 0;
}

int32_t sm_mdbt42q_hard_reset(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    sm_modem_hw_reset(&_this->m_base, 2000, 1);
    return 0;
}

int32_t sm_mdbt42q_read_data(sm_mdbt42q_t* _this, char* _data, uint32_t _len) {
    if(!_this) {
        return -1;
    }
    int32_t len = 0;
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    len = sm_modem_read(modem , _data, _len);
    return len;
}

int32_t sm_mdbt42q_write_data(sm_mdbt42q_t* _this, char* _data, uint32_t _len) {
    if(!_this) {
        return -1;
    }
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    return sm_modem_write(modem , _data, _len);
}

int32_t sm_mdbt42q_get_indicator_logic(sm_mdbt42q_t* _this) {
    if(!_this)
        return -1;
    sm_modem_t* modem = (sm_modem_t*)&_this->m_base;
    int32_t ret = sm_modem_send_cmd(modem,
    							    "AT?CONNECTINDICATOR",
                                    "low",
                                    "high",
                                    MDBT42Q_WAIT_TIMEOUT_MS);
    if (ret == 0) {
        // "low"
        _this->m_indicator_logic_on_connecting = 0;
    }else if(ret == -2) {
        // "high"
        _this->m_indicator_logic_on_connecting = 1;
    }else{
        return -1;
    }
    return 0;
}

int32_t sm_mdbt42q_is_connected(sm_mdbt42q_t* _this) {
    if(!_this) {
        return -1;
    }
    if (sm_hal_io_get_value(_this->m_indicator_pin) == _this->m_indicator_logic_on_connecting) {
        return 1;
    }else {
        return 0;
    }
}
