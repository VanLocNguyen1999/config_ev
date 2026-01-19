//
// Created by admin on 9/16/2024.
//

#ifndef SM_EC200_HTTP_H
#define SM_EC200_HTTP_H

#include "sm_ec200.h"

/**
 * @brief Configures an HTTP parameter on the specified modem.
 * @param modem A pointer to the modem structure.
 * @param par The parameter to be configured.
 * @param val The value to set for the parameter.
 * @return An integer status code indicating success or failure of the operation.
 */
int32_t sm_ec200_http_config(sm_ec200_t* modem, const char* par, int val);

/**
 * 
 * @param modem 
 * @param header 
 * @return 
 */
int32_t sm_ec200_http_add_header(sm_ec200_t* modem, const char* header);
/**
 * @brief Sends HTTP data to the specified modem.
 *
 * @param modem A pointer to the modem structure.
 * @param data A pointer to the data to be sent.
 * @param len The length of the data to be sent.
 * @param status_code The status code of request.
 * @return An integer status code indicating success (0) or failure (non-zero) of the operation.
 */
int32_t sm_ec200_http_write(sm_ec200_t *modem, char *data, uint32_t len, int32_t* status_code);

/**
 * @brief Reads data from the HTTP response on the specified modem.
 * @param modem A pointer to the modem structure.
 * @param data A buffer to store the read data.
 * @return An integer status code indicating success or failure of the operation.
 */
int32_t sm_ec200_http_read(sm_ec200_t *modem, char *data);

/**
 * @brief Sends an HTTP GET request starting from a specified range on the given modem.
 * @param modem A pointer to the modem structure.
 * @param start The starting byte position of the range to be requested.
 * @param range The number of bytes to be requested starting from the start position.
 * @return An integer status code indicating success or failure of the operation.
 */
int32_t sm_ec200_http_get_range(sm_ec200_t* modem,uint32_t start, uint32_t range, int32_t* status_code);
/**
 * @brief
 * @param modem
 * @param url
 * @return
 */
int32_t sm_ec200_set_http_url(sm_ec200_t *modem, char *url);
/**
 * @brief
 * @param modem
 * @param data
 * @return
 */
int32_t sm_ec200_data_incoming_http(sm_ec200_t* modem, char* data);
#endif //SM_EC200_HTTP_H
