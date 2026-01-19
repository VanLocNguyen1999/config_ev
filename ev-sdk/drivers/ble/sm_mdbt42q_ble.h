//
// Created by vnbk on 09/06/2023.
//

#ifndef MDBT42Q_H
#define MDBT42Q_H

#include "sm_types.h"
#include "sm_elapsed_timer.h"
#include "at-modem/sm_modem.h"
#include "sm_hal.h"

#define MDBT42Q_WAIT_TIMEOUT_MS         5000

typedef struct {
    sm_modem_t          m_base;
    uint8_t             m_indicator_logic_on_connecting;
    void*               m_indicator_pin;
} sm_mdbt42q_t;

sm_mdbt42q_t* sm_mdbt42q_create(sm_hal_uart_t* _driver,
                                sm_hal_io_t* _rst,
                                sm_hal_io_t* _ind,
                                sm_hal_io_t* _wk);

/**
 * @brief Initializes the MDBT42Q Bluetooth module.
 *
 * This function initializes the MDBT42Q Bluetooth
 * @param _this
 * @param _driver
 * @param _rst
 * @param _ind
 */
int32_t sm_mdbt42q_init(sm_mdbt42q_t* _this);

/**
 * @brief De-initializes the MDBT42Q Bluetooth module.
 * @param _this
 */
void sm_mdbt42q_deinit(sm_mdbt42q_t* _this);

/**
 * @brief Sets the Bluetooth module's device name.
 * @param _this
 * @param _name
 * @return
 */
int32_t sm_mdbt42q_set_name(sm_mdbt42q_t* _this, char* _name);

/**
*
*/
int32_t sm_mdbt42q_start_advertising(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_stop_advertising(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_disconnect(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_set_default(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_soft_reset(sm_mdbt42q_t* _this);

/**
 * 
 * @param _this 
 * @return 
 */
int32_t sm_mdbt42q_hard_reset(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_read_data(sm_mdbt42q_t* _this, char* _data, uint32_t _len);

/**
*
*/
int32_t sm_mdbt42q_write_data(sm_mdbt42q_t* _this, char* _data, uint32_t _len);

/**
*
*/
int32_t sm_mdbt42q_get_indicator_logic(sm_mdbt42q_t* _this);

/**
*
*/
int32_t sm_mdbt42q_is_connected(sm_mdbt42q_t* _this);

/**
*
*/

#endif //MDBT42Q_H
