#ifndef SM_EC200N_H
#define SM_EC200N_H

#include "sm_types.h"
#include "modules/at-modem/sm_modem.h"
#include "sm_datetime.h"

#define EC200_OK		"OK\r\n"
#define EC200_ERR		"ERROR\r\n"
#define EC200_CONNECT	"CONNECT"
#define EC200_DISCONN	"DISCONNECT"

typedef struct sm_ec200 sm_ec200_t;

typedef struct {
    const char* m_apn;
    const char* m_user;
    const char* m_pass;
}sm_ec200_config_t;

struct sm_ec200{
    sm_modem_t base;
    int rssi;   // 0 - 31
    int ber;    // 99: invalid,
    int ip[4];
    char IMSI[20];
    char simNb[8];
    char name[20];

    sm_ec200_config_t m_config;
};

sm_ec200_t* sm_ec200_create(sm_hal_io_t *reset_pin, sm_hal_uart_t *driver, const sm_ec200_config_t* _config);

/**
 * @brief
 * @param modem
 * @param reset_pin
 * @param driver
 */
int32_t sm_ec200_init(sm_ec200_t *modem);

/**
 * @brief
 * @param modem
 * @param reset_pin
 * @param driver
 */
int32_t sm_ec200_set_config(sm_ec200_t *modem, const sm_ec200_config_t* _config);

/**
 * @brief
 * @param modem
 * @param reset_pin
 * @param driver
 */
int32_t sm_ec200_reinit(sm_ec200_t *modem);

/**
 * @brief
 * @param modem
 */
void sm_ec200_deinit(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_sync(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @param mode
 * @return
 */
int32_t sm_ec200_echo_mode(sm_ec200_t *modem, uint8_t mode); // mode = 0 : disable echo mode, mode = 1 : enable echo mode
/**
 * @brief
 * @param modem
 * @param pin
 * @return
 */
int32_t sm_ec200_set_pin(sm_ec200_t *modem, char *pin);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_get_pin(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_get_signal(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @param name
 * @return
 */
int32_t sm_ec200_get_name(sm_ec200_t *modem, char *name);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_set_function(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @param apn
 * @param user
 * @param pass
 * @return
 */
int32_t sm_ec200_set_apn(sm_ec200_t *modem, const char *apn, const char *user, const char *pass);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_get_network_reg(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_get_simNb(sm_ec200_t *modem);
/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_get_ip(sm_ec200_t *modem);

/**
 * @brief
 * @param modem
 * @param p_time
 * @return
 */
int32_t sm_ec200_get_time_network(sm_ec200_t* modem, sys_datetime_t *p_time);

/**
 * @brief
 * @param modem
 * @return
 */
int32_t sm_ec200_hard_reset(sm_ec200_t *modem) ;
#endif //SM_EC200N_H
