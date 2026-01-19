
#include <string.h>
#include "sm_ec200.h"
#include "sm_logger.h"
#include "sm_hal_delay.h"
#include "sm_ev_net_config.h"

#define TAG  "SM_EC200"

#define RESET_MODEM_TIME    (15*1000)

static sm_ec200_t g_ec200_default;

sm_ec200_t* sm_ec200_create(sm_hal_io_t *reset_pin, sm_hal_uart_t *driver, const sm_ec200_config_t* _config){
    sm_ec200_t* ec200 = &g_ec200_default;
    sm_modem_init((sm_modem_t*) ec200, reset_pin, driver, 2048);

    memset(ec200->IMSI, 0, sizeof(ec200->IMSI));
    memset(ec200->ip, 0, sizeof(ec200->ip));
    memset(ec200->simNb, 0, sizeof(ec200->simNb));
    memset(ec200->name, 0, sizeof(ec200->name));

    ec200->ber = 0;
    ec200->rssi = 0;

    if(_config){
        ec200->m_config.m_apn = _config->m_apn;
        ec200->m_config.m_user = _config->m_user;
        ec200->m_config.m_pass = _config->m_pass;
    }else{
        ec200->m_config.m_apn = SM_LTE_APN_DEFAULT;
        ec200->m_config.m_user = SM_LTE_USERNAME_DEFAULT;
        ec200->m_config.m_pass = SM_LTE_PASSWORD_DEFAULT;
    }

    return ec200;
}

int32_t sm_ec200_set_config(sm_ec200_t *modem, const sm_ec200_config_t* _config){
    if(!modem){
        return -1;
    }
    if(_config){
        modem->m_config.m_apn = _config->m_apn;
        modem->m_config.m_user = _config->m_user;
        modem->m_config.m_pass = _config->m_pass;
    }else{
        modem->m_config.m_apn = SM_LTE_APN_DEFAULT;
        modem->m_config.m_user = SM_LTE_USERNAME_DEFAULT;
        modem->m_config.m_pass = SM_LTE_PASSWORD_DEFAULT;
    }

    return 0;
}

int32_t sm_ec200_init(sm_ec200_t *modem) {
    if(!modem){
        return -1;
    }
    sm_modem_hw_reset((sm_modem_t*) modem, 400, 0);
	LOG_DBG(TAG, "Reset modem");

    sm_hal_delay_ms(RESET_MODEM_TIME);

    if (sm_ec200_sync(modem) != 0) {
        LOG_DBG(TAG, "sync fail");
        return -1;
    }
    if (sm_ec200_get_pin(modem) != 0) {
    	LOG_DBG(TAG, "get pin fail");
        return -1;
    }
    if (sm_ec200_set_function(modem) != 0) {
    	LOG_DBG(TAG, "get function fail");
        return -1;
    }
    if (sm_ec200_get_signal(modem) != 0) {
    	LOG_DBG(TAG, "get signal fail");
        return -1;
    }
    if (sm_ec200_get_simNb(modem) != 0) {
    	LOG_DBG(TAG, "get simNb fail");
        return -1;
    }
    if (sm_ec200_get_network_reg(modem) != 0) {
        LOG_DBG(TAG, "get Network fail");
        return -1;
    }

    if(sm_ec200_set_apn(modem, modem->m_config.m_apn, modem->m_config.m_user, modem->m_config.m_pass) != 0){
        LOG_DBG(TAG, "Set LTE config FAILURE");
        return -1;
    }

    sm_ec200_echo_mode(modem, 0);
	LOG_DBG(TAG, "echo mode off");

    return 0;
}

int32_t sm_ec200_reinit(sm_ec200_t *modem){
    return 0;
}

void sm_ec200_deinit(sm_ec200_t *modem) {
	sm_modem_deinit((sm_modem_t*) modem);
}

int32_t sm_ec200_sync(sm_ec200_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             "AT\r\n",
							 EC200_OK,
							 EC200_ERR,
							 1000);
}

int32_t sm_ec200_echo_mode(sm_ec200_t *modem, uint8_t mode) {
	if (mode == 0)
		return sm_modem_send_cmd((sm_modem_t*) modem,
                                 "ATE0\r\n",
								 EC200_OK,
								 EC200_ERR, 1000);
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             "ATE1\r\n",
							 EC200_OK,
							 EC200_ERR,
							 1000);
} // mode = 0 : disable echo mode, mode = 1 : enable echo mode

int32_t sm_ec200_set_pin(sm_ec200_t *modem, char *pin) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[128];
	memset(buff, 0, p_modem->buff_size);

	sprintf(buff, "AT+CPIN=%s\r\n", pin);

	return sm_modem_send_cmd((sm_modem_t*) modem,
                             buff,
							 EC200_OK,
							 EC200_ERR,
                             1000);
}

int32_t sm_ec200_get_pin(sm_ec200_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             "AT+CPIN?\r\n",
							 EC200_OK,
							 EC200_ERR,
                             1000);
}

int32_t sm_ec200_get_signal(sm_ec200_t *modem) {
	char buffer[40];
	memset(buffer, 0, 40);
	int32_t rc = sm_modem_send_recv(modem,
									"AT+CSQ\r\n", EC200_OK, EC200_ERR,
									buffer, 40,
									1000);

	if (rc == 0) {
		sscanf(buffer,
               "AT+CSQ\r\r\n+CSQ: %d,%d\r\n\r\nOK\r\n",
               &modem->rssi,
               &modem->ber);
	}
	return rc;
}

int32_t sm_ec200_get_name(sm_ec200_t *modem, char *name) {
	char buffer[32];
	memset(buffer, 0, 32);
	int32_t err = sm_modem_send_recv((sm_modem_t*)modem,
									"AT+GMM\r\n",
									EC200_OK, EC200_ERR,
									buffer, 32,
									1000);
	if (!err) {
		memcpy(name, buffer + 2, 4);
	}
	return err;
}

int32_t sm_ec200_set_function(sm_ec200_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             "AT+CFUN=1\r\n",
							 EC200_OK,
							 EC200_ERR,
							 1000);
}

int32_t sm_ec200_set_apn(sm_ec200_t *modem, const char *apn, const char *user, const char *pass) {
	char buff[256] = { 0 };
	sprintf(buff, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1\r\n", apn, user, pass);
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             buff,
							 EC200_OK,
							 EC200_ERR,
                             1000);
}

int32_t sm_ec200_get_network_reg(sm_ec200_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem,
                             "AT+CREG?\r\n",
							 EC200_OK,
							 EC200_ERR, 1000);
}

int32_t sm_ec200_get_simNb(sm_ec200_t *modem) {
	char buffer[64];
	memset(buffer, 0, 64);
	int32_t err = sm_modem_send_recv((sm_modem_t*) modem,
                                   "AT+CIMI\r\n",
								   EC200_OK,
								   EC200_ERR,
								   buffer, 64,
								   1000);
	if (!err) {
		sscanf(buffer,
				"AT+CIMI\r\n\r\n%s\r\n\r\nOK\r\n",
				(char*) modem->IMSI);
		strncpy(modem->simNb, modem->IMSI + 7, 8);
	}
	return err;
}

int32_t sm_ec200_get_ip(sm_ec200_t *modem) {
	if (sm_modem_send_cmd(&modem->base,
							"AT+QIACT=1\r\n",
							EC200_OK,
							EC200_ERR,
							10000) == 0) {
		char buffer[64];
		memset(buffer, 0, 64);
		if (sm_modem_send_recv(&modem->base,
                              "AT+QIACT?\r\n",
							  EC200_OK,
							  EC200_ERR,
							  buffer, 64,
                              10000) == 0) {
			char *str = strstr(buffer, "+QIACT");
			sscanf(str,
                   "+QIACT: 1,1,1,\"%d.%d.%d.%d\"",
                   &modem->ip[0],
					&modem->ip[1],
                    &modem->ip[2],
                    &modem->ip[3]);
			return 0;
		}
	}
	return -1;
}

int32_t sm_ec200_get_time_network(sm_ec200_t* modem, sys_datetime_t *p_time) {
	char buffer[64];
	memset (buffer, 0, 64);

	if (sm_modem_send_recv ((sm_modem_t*) modem,
							"AT+QLTS=2\r\n",
							EC200_OK,
							EC200_ERR,
							buffer, 64,
							1000) < 0){
		return -1;
	}

	char *str_time = strstr (buffer, "\"");
	if (str_time == NULL){
		return -1;
	}
	int year = 0;
	uint8_t num_scan = 0 ;
	num_scan = sscanf (str_time, "\"%d/%d/%d,%d:%d:%d+",
                       &year,
                       &p_time->mon,
                       &p_time->day,
                       &p_time->hour,
                       &p_time->min,
                       &p_time->sec);
	p_time->year = year - 2000;
	if (num_scan < 6) {
		return -1;
	}
	return 0;
}

int32_t sm_ec200_hard_reset(sm_ec200_t *modem) {
	sm_modem_hw_reset((sm_modem_t*) modem, 200, 1);
	return 0;
}
