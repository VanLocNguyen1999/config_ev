//
// Created by vnbk on 02/08/2024.
//

#ifndef EV_SDK_SM_CO_H
#define EV_SDK_SM_CO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "sm_co_if.h"
#include "od.h"

typedef void sm_co_t;

typedef CO_Sub_Object_Ext_Confirm_Func_t(*sm_co_sdo_server_confirm_fn_t)(void);

/**
 * @def CO_SDO_MAX_LEN
 * @brief define number sdo message in ring buffer
 *
 */
#define CO_SDO_MAX_LEN 10
#define MAX_RECV_CALLBACK_FUNC_NUM 10

/**
 * @brief
 *
 */
typedef enum {
    SM_SDO_ST_IDLE = 0,
    SM_SDO_ST_BUSY = 1,
    SM_SDO_ST_SUCCESS = 2,   /**< SM_SDO_ST_SUCCESS */
    SM_SDO_ST_ABORT = 3    /**< SM_SDO_ST_ABORT */
}SM_SDO_STATUS_t;

/**
 * @brief
 * @param _status
 * @param _arg
 */
typedef void (*sm_co_sdo_cb_fn_t)(SM_SDO_STATUS_t, int32_t, int32_t, void*);

/**
 * @brief
 *
 * @param _node_id node id
 * @param _p_if function interface
 * @return pointer CANOpen : success - NULL : false
 */
sm_co_t* sm_co_create(uint8_t _node_id, uint8_t _is_sync, sm_co_if_t *_if);

/**
 * @brief
 *
 * @param _this pointer CANOpen
 * @param _nodeID
 */
void sm_co_set_node_id(sm_co_t* _this, uint8_t _node_id);

/**
 * @brief
 *
 * @param _this pointer CANOpen
 * @param _version
 */
void sm_co_set_self_version(sm_co_t* _this, const uint8_t* _version);

/**
 * @brief
 *
 * @param _this pointer CANOpen
 * @param _confirm_fn
 * @param _p_data
 */
int32_t sm_co_sdo_server_set_handle(sm_co_t* _this,
                                    uint32_t _index,
                                    uint8_t _sub_index,
                                    sm_co_sdo_server_confirm_fn_t _confirm_fn,
                                    void* _p_data);

/**
 * @param _this
 */
sm_co_if_t * sm_co_get_if(sm_co_t* _this);

/**
 * @brief
 *
 * @param _this pointer CANOpen
 */
void sm_co_enable_sync_mode(sm_co_t* _this);

/**
 * @brief
 *
 * @param _this pointer CANOpen
 */
void sm_co_disable_sync_mode(sm_co_t* _this);

/**
 * @brief
 * @param _this
 * @return
 */
int32_t sm_co_sdo_is_busy(sm_co_t* _this);

/**
 * @brief
 * @param _this
 * @return
 */
void sm_co_sdo_reset(sm_co_t* _this);

/**
 * @brief SDO client write Object function non-block
 *
 * @param _this pointer CANOpen
 * @param _mux SDO sub object and SDO sub number
 * @param _node_id client node id
 * @param _tx_buff global buffer
 * @param _tx_len length buffer
 * @param _timeout m_timeout request SDO
 * @param _callback m_cb when finish request SDO
 * @param _arg parameter
 * @return true : success
 * @return false : false
 */
int32_t sm_co_sdo_client_send(sm_co_t* _this,
                              uint32_t _index,
                              uint8_t _sub_index,
                              int32_t _node_id,
                              uint8_t *_tx_buff,
                              int32_t _tx_len,
                              int32_t _timeout,
                              sm_co_sdo_cb_fn_t _callback,
                              void *_arg);
/**
 * @brief SDO client write Object function non-block m_now
 *
 * @param _this pointer CANOpen
 * @param _mux SDO sub object and SDO sub number
 * @param _node_id client node id
 * @param _tx_buff global buffer
 * @param _len length buffer
 * @param _timeout m_timeout request SDO
 * @param _callback m_cb when finish request SDO
 * @param _arg parameter
 * @return true : success
 * @return false : false
 */
int32_t sm_co_sdo_client_send_now(sm_co_t* _this,
                                  uint32_t _index,
                                  uint8_t _sub_index,
                                  int32_t _node_id,
                                  uint8_t *_tx_buff,
                                  int32_t _len,
                                  int32_t _timeout,
                                  sm_co_sdo_cb_fn_t _callback,
                                  void *_arg);
/**
 * @brief SDO client read Object function non-block
 *
 * @param _this pointer CANOpen
 * @param _mux SDO sub object and SDO sub number
 * @param _node_id client node id
 * @param _rx_buff global buffer
 * @param _len length buffer
 * @param _timeout m_timeout request SDO
 * @param _callback m_cb when finish request SDO
 * @param _arg parameter
 * @return true : success
 * @return false : false
 */
int32_t sm_co_sdo_client_receive(sm_co_t* _this,
                                 uint32_t _index,
                                 uint8_t _sub_index,
                                 int32_t _node_id,
                                 uint8_t *_rx_buff,
                                 int32_t _len,
                                 int32_t _timeout,
                                 sm_co_sdo_cb_fn_t _callback,
                                 void *_arg);
/**
 * @brief SDO client read Object function non-block m_now
 *
 * @param _this pointer CANOpen
 * @param _mux SDO sub object and SDO sub number
 * @param _node_id client node id
 * @param _tx_buff global buffer
 * @param _len length buffer
 * @param _timeout m_timeout request SDO
 * @param _callback m_cb when finish request SDO
 * @param _arg parameter
 * @return true : success
 * @return false : false
 */
int32_t sm_co_sdo_client_receive_now(sm_co_t* _this,
                                     uint32_t _index,
                                     uint8_t _sub_index,
                                     int32_t _node_id,
                                     uint8_t *_rx_buff,
                                     int32_t _len,
                                     int32_t _timeout,
                                     sm_co_sdo_cb_fn_t _callback,
                                     void *_arg);
/**
 * @brief
 *
 * @param _this pointer CANOpen
 * @param _time_stamp
 */
void sm_co_process(sm_co_t* _this, uint32_t _time_stamp);

/**
 * @brief
 *
 * @param _buffer
 */
void __attribute__((weak)) tpdo1_build_data_impl(uint8_t *_buffer);
/**
 * @brief
 *
 * @param _buffer
 */
void __attribute__((weak)) tpdo2_build_data_impl(uint8_t *_buffer);
/**
 * @brief
 *
 * @param _buffer
 */
void __attribute__((weak)) tpdo3_build_data_impl(uint8_t *_buffer);

/**
 * @brief
 *
 * @param _buffer
 */
void __attribute__((weak)) tpdo4_build_data_impl(uint8_t *_buffer);

#ifdef __cplusplus
}
#endif

#endif //EV_SDK_SM_CO_H
