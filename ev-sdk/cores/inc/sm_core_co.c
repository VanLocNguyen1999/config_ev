//
// Created by vnbk on 02/08/2024.
//
/*
 * smcanopen.c
 *
 *  Created on: Jan 19, 2024
 *      Author: Admin
 */

#include <stdbool.h>
#include <comm_od_default_value.h>
#include "CO.h"
#include "CO_SDO.h"
#include "CO_Object.h"
#include "sm_fifo.h"

#include "sm_core_co.h"

#include "sm_logger.h"
#include "sm_co_od_common.h"

typedef struct sm_co_sub_oj sm_co_sub_oj_t;
typedef struct sm_co_sdo    sm_co_sdo_t;
typedef struct sm_canopen   sm_co_impl_t;

#define _impl(x) ((struct sm_co*)(x))

/**
 * @brief
 *
 */
typedef enum {
    SDO_SEND_REQUEST = 0,/**< SDO_SEND_REQUEST */
    SDO_WAIT_RESULT, /**< SDO_WAIT_RESULT */
    SDO_END_REQUEST /**< SDO_END_REQUEST */
} SDO_PROCESS_STATE;

/**
 * @brief
 *
 */
typedef enum {
    SM_SDO_R = 0,/**< SM_SDO_R */
    SM_SDO_W /**< SM_SDO_W */
} SM_SDO_READ_WRITE;

/**
 * @struct sm_co_sub_oj
 * @brief
 *
 */
struct sm_co_sub_oj {
    SM_SDO_READ_WRITE m_rw;
    uint32_t m_node_id;
    uint32_t m_index;
    uint8_t m_sub_index;
    int32_t m_timeout;
    CO_Sub_Object m_parent;
    sm_co_sdo_cb_fn_t m_cb;
    void *m_cb_arg;
};

/**
 * @struct sm_co_sdo
 * @brief
 *
 */
struct sm_co_sdo {
    sm_co_sub_oj_t m_buff[CO_SDO_MAX_LEN];
    sm_co_sub_oj_t *m_now;
    uint32_t m_len_sdo;
    volatile int32_t m_tail;
    volatile int32_t m_head;
    volatile uint32_t m_count;
};
/**
 * @struct sm_canopen
 * @brief
 *
 */
struct sm_co {
    CO *parent;
    sm_co_if_t *p_if;
    sm_fifo_handle_t sdo;
    sm_co_sub_oj_t sub_obj;
    sm_co_sub_oj_t sdo_now;
    bool is_sdo_now;
    bool sdo_process;
    SDO_PROCESS_STATE sdo_state;
};

CO CO_DEVICE;
static struct sm_co g_co_device;

void *tpdo_build_data_impl[TPDO_NUMBER] = { tpdo1_build_data_impl,
                                            tpdo2_build_data_impl,
                                            tpdo3_build_data_impl,
                                            tpdo4_build_data_impl };

//void tpdo1_build_data_impl(uint8_t *buffer) {
//    (void) buffer;
//    printf("Hello w.............");
//}
//
//void tpdo2_build_data_impl(uint8_t *buffer) {
//    (void) buffer;
//}
//
//void tpdo3_build_data_impl(uint8_t *buffer) {
//    (void) buffer;
//}
//
//void tpdo4_build_data_impl(uint8_t *buffer) {
//    (void) buffer;
//}

static bool sm_sdo_push(sm_co_t* _this, sm_co_sub_oj_t *sub_oj) {
    return sm_fifo_push(&_impl(_this)->sdo, sub_oj);
}
static bool sm_sdo_pop(sm_co_t* _this, sm_co_sub_oj_t *sub_oj) {
    return sm_fifo_pop(&_impl(_this)->sdo, sub_oj);
}

static void sm_co_sdo_queue_init(sm_co_t* _this){
    sm_fifo_init(&_impl(_this)->sdo, 10, sizeof(sm_co_sub_oj_t));
}

static void sm_co_SDO_process(sm_co_t* _this);

static void sm_co_can_if_send_impl(CO_CAN_Msg* p_msg){
    if(g_co_device.p_if){
        sm_co_if_send(g_co_device.p_if, p_msg->id.cob_id, p_msg->data, p_msg->data_len, 100);
    }
}

static void sm_co_can_if_recv_impl(const uint32_t can_id, uint8_t* data, void* arg){
    struct sm_co* co_device = (struct sm_co*)(arg);
    if(!co_device){
        return;
    }
    if(CO_can_receive_basic_handle(co_device->parent, can_id, data)) {
        return;
    }
}

sm_co_t* sm_co_create(uint8_t _node_id, uint8_t _is_sync, sm_co_if_t *_p_if) {
    sm_co_t* _this = (sm_co_t*)&g_co_device;

    (void)_is_sync;
    if (_this == NULL || !_p_if){
        return NULL;
    }

    sm_co_if_reg_recv_callback(_p_if, sm_co_can_if_recv_impl, _this);

    _impl(_this)->sdo_process = false;
    _impl(_this)->parent = &CO_DEVICE;

    sm_co_sdo_queue_init(_impl(_this));
    CO *p_co = _impl(_this)->parent;

    _impl(_this)->p_if = _p_if;

    CO_CAN_set_can_send_interface(sm_co_can_if_send_impl);

#if CO_FLASH_USE
    CO_FLASH_init(&p_co->flash, sm_co_erase_and_write_data_to_flash_impl,
			sm_co_flash_error_handle_impl);
#endif
#if APP_OD_STORAGE_USE
    #if CO_FLASH__USE & (SW_VERSION != 0)
	CO_OD_STORAGE_init(&p_co->storage,
			APP_OD_STORAGE__STORAGE_REGION__ADDR,
			APP_OD_STORAGE__STORAGE_REGION__SIZE,
			&od_temp_comm_profile_para,
			&od_temp_manu_profile_para,
			sizeof(OD_Temp_Manufacturer_Profile_Para_t),
			&p_co->flash,
			p_co_od,
			SW_VERSION,
			IS_MAIN_APP);
#else
	while(1); /* Code fail */
#endif
#endif

    CO_init_basic(p_co, od_temp_comm_profile_para.x1000_device_type,
                  &od_temp_comm_profile_para.x1018_identity, p_co_od);

    CO_SYNC_init(&p_co->sync, &od_temp_comm_profile_para);

    for (uint8_t i = 0; i < TPDO_NUMBER; i++) {
        CO_TPDO_init(&p_co->tpdos[i], &p_co->sync,
                     &od_temp_comm_profile_para.x1A0x_tpdo_map_para[i], p_co->p_od,
                     &od_temp_comm_profile_para.x180x_tpdo_comm_para[i],
                     tpdo_build_data_impl[i]);
    }
    CO_SDOserver_init(&p_co->sdo_server, &od_temp_comm_profile_para,
                      &p_co->sync, p_co->p_od);
    CO_SDOclient_init(&p_co->sdo_client, &od_temp_comm_profile_para,
                      &p_co->sync);

    CO_set_node_id(p_co, od_temp_manu_profile_para.x500300_node_ID);

//    if (!_is_sync){
//        CO_disable_sync_mode(p_co);
//    }

    sm_co_set_node_id(_this, _node_id);

    return _this;
}

void sm_co_set_self_version(sm_co_t* _this, const uint8_t* _version){

      memcpy(&od_temp_comm_profile_para.x100A_software_version, _version, 4);

    for(int index = 0; index < _impl(_this)->parent->p_od->number; index++){
        if(_impl(_this)->parent->p_od->list[index].index == SDO_VERSION_INDEX){
            _impl(_this)->parent->p_od->list[index].subs[SDO_VERSION_SUB_INDEX].p_data = (void*)_version;
        }
    }
}

int32_t sm_co_sdo_server_set_handle(sm_co_t* _this,
                                    uint32_t _index,
                                    uint8_t _sub_index,
                                    sm_co_sdo_server_confirm_fn_t _confirm_fn,
                                    void* _p_data){

    struct CO_Object_t* obj = NULL;

    for(int index = 0; index < _impl(_this)->parent->p_od->number; index++){
        if(_impl(_this)->parent->p_od->list[index].index == _index){
            obj = &_impl(_this)->parent->p_od->list[index];
        }
    }

    if(!obj || _sub_index >= obj->sub_number){
        return -1;
    }

    if(obj->subs == NULL){
        obj->subs = malloc(obj->sub_number * sizeof (CO_Sub_Object));
    }

    obj->subs[_sub_index].p_data = _p_data;
    obj->subs[_sub_index].p_ext->p_shadow_data = _p_data;
    obj->subs[_sub_index].p_ext->confirm_func = _confirm_fn;
    return 1;
}

sm_co_if_t * sm_co_get_if(sm_co_t* _this){
	return _impl(_this)->p_if;
}
void sm_co_set_node_id(sm_co_t* _this, uint8_t _node_id) {
    CO_set_node_id(_impl(_this)->parent, _node_id);
}

void sm_co_enable_sync_mode(sm_co_t* _this) {
    CO_recover_sync_mode(_impl(_this)->parent, &od_temp_comm_profile_para);
}
void sm_co_disable_sync_mode(sm_co_t* _this) {
    CO_disable_sync_mode(_impl(_this)->parent);
}

int32_t sm_co_sdo_is_busy(sm_co_t* _this){
    return (CO_SDO_get_status(&_impl(_this)->parent->sdo_client) != CO_SDO_RT_idle);
}

void sm_co_sdo_reset(sm_co_t* _this){
    CO_SDO_reset_status(&_impl(_this)->parent->sdo_client);
}

int32_t sm_co_sdo_client_send(sm_co_t* _this,
                              uint32_t _index,
                              uint8_t _sub_index,
                              int32_t _node_id,
                              uint8_t *_tx_buff,
                              int32_t _tx_len,
                              int32_t _timeout,
                              sm_co_sdo_cb_fn_t _callback,
                              void *_arg) {

    sm_co_sub_oj_t tx_oj = {
            .m_rw = SM_SDO_W,
            .m_parent = {
                    .p_data = _tx_buff,     //<< Address variable receiving data
                    .attr = ODA_SDO_W,      //<< [skip] set ODA_SDO_RW
                    .len = (uint16_t)_tx_len,            //<< data size
                    .p_ext = NULL           //<< [skip] set NULL
            },
            .m_cb = _callback,
            .m_cb_arg = _arg,
            .m_index = _index,
            .m_sub_index = _sub_index,
            .m_node_id = (uint32_t)_node_id,
            .m_timeout = _timeout
    };
    return sm_sdo_push(_this, &tx_oj);
}

int32_t sm_co_sdo_client_send_now(sm_co_t* _this,
                                  uint32_t _index,
                                  uint8_t _sub_index,
                                  int32_t _node_id,
                                  uint8_t *_tx_buff,
                                  int32_t _len,
                                  int32_t _timeout,
                                  sm_co_sdo_cb_fn_t _callback,
                                  void *_arg){

    if(_impl(_this)->is_sdo_now == true) {
        return false;
    }

    sm_co_sub_oj_t *tx_oj = &_impl(_this)->sdo_now;
    tx_oj->m_rw = SM_SDO_W;
    tx_oj->m_parent.p_data = _tx_buff;
    //<< Address variable receiving data
    tx_oj->m_parent.attr = ODA_SDO_RW;	//<< [skip] set ODA_SDO_RW
    tx_oj->m_parent.len =(uint16_t)_len;			//<< data size
    tx_oj->m_parent.p_ext = NULL;		//<< [skip] set NULL
    tx_oj->m_cb = _callback;
    tx_oj->m_cb_arg = _arg;
    tx_oj->m_index = _index;
    tx_oj->m_sub_index = _sub_index;
    tx_oj->m_node_id = (uint32_t)_node_id;
    tx_oj->m_timeout = _timeout;
    _impl(_this)->is_sdo_now = true;

    return true;
}

int32_t sm_co_sdo_client_receive(sm_co_t *_this,
                                 uint32_t _index,
                                 uint8_t _sub_index,
                                 int32_t _node_id,
                                 uint8_t *_rx_buff,
                                 int32_t _len,
                                 int32_t _timeout,
                                 sm_co_sdo_cb_fn_t _callback,
                                 void *_arg) {
    sm_co_sub_oj_t rx_oj =
            {
                    .m_rw = SM_SDO_R,
                    .m_parent = {.p_data = _rx_buff,//<< Address variable receiving data
                            .attr = ODA_SDO_R,    //<< [skip] set ODA_SDO_RW
                            .len = (uint16_t)_len,            //<< data size
                            .p_ext = NULL        //<< [skip] set NULL
                    },
                    .m_cb = _callback,
                    .m_cb_arg = _arg,
                    .m_index = _index,
                    .m_sub_index = _sub_index,
                    .m_node_id = (uint32_t)_node_id,
                    .m_timeout = _timeout
            };
    return sm_sdo_push(_this, &rx_oj);
}

int32_t sm_co_sdo_client_receive_now(sm_co_t *_this,
                                     uint32_t _index,
                                     uint8_t _sub_index,
                                     int32_t _node_id,
                                     uint8_t *_rx_buff,
                                     int32_t _len,
                                     int32_t _timeout,
                                     sm_co_sdo_cb_fn_t _callback,
                                     void *_arg) {

    if(_impl(_this)->is_sdo_now == true){
        return false;
    }

    sm_co_sub_oj_t *rx_oj = &_impl(_this)->sdo_now;
    rx_oj->m_rw = SM_SDO_R;
    rx_oj->m_parent.p_data = _rx_buff;
    //<< Address variable receiving data
    rx_oj->m_parent.attr = ODA_SDO_RW;	//<< [skip] set ODA_SDO_RW
    rx_oj->m_parent.len = (uint16_t)_len;			//<< data size
    rx_oj->m_parent.p_ext = NULL;		//<< [skip] set NULL
    rx_oj->m_cb = _callback;
    rx_oj->m_cb_arg = _arg;
    rx_oj->m_index = _index;
    rx_oj->m_sub_index = _sub_index;
    rx_oj->m_node_id = (uint32_t)_node_id;
    rx_oj->m_timeout = _timeout;

    _impl(_this)->is_sdo_now = true;

    return true;
}

static void sm_co_SDO_process(sm_co_t* _this){
    if (_impl(_this)->sdo_process) {
        sm_co_sub_oj_t *p_sdo = NULL;

        if(_impl(_this)->is_sdo_now == true) {
            p_sdo = &_impl(_this)->sdo_now;
        }else{
            p_sdo = &_impl(_this)->sub_obj;
        }
        switch (_impl(_this)->sdo_state) {
            case SDO_SEND_REQUEST /**< SDO_SEND_REQUEST */:
                CO_SDO_reset_status(&_impl(_this)->parent->sdo_client);

//                LOG_INF("CO", "%s SDO - id %d, index 0x%x, sub 0x%x, len %d\n", p_sdo->m_rw?"Write":"Read",
//                        p_sdo->m_node_id, p_sdo->m_index, p_sdo->m_sub_index, p_sdo->m_parent.len);

                if(p_sdo->m_rw == SM_SDO_R){
                    CO_SDOclient_start_upload(&_impl(_this)->parent->sdo_client,
                                              p_sdo->m_node_id,
                                              (uint16_t)p_sdo->m_index,
                                              p_sdo->m_sub_index,
                                              &p_sdo->m_parent,
                                              (uint16_t)p_sdo->m_timeout);
                }else{
                    CO_SDOclient_start_download(&_impl(_this)->parent->sdo_client,
                                                p_sdo->m_node_id,
                                                (uint16_t)p_sdo->m_index,
                                                p_sdo->m_sub_index,
                                                &p_sdo->m_parent,
                                                (uint16_t)p_sdo->m_timeout);
                }

                _impl(_this)->sdo_state = SDO_WAIT_RESULT;

                break;
            case SDO_WAIT_RESULT/**< SDO_WAIT_RESULT */:
                if((_impl(_this)->parent->sdo_client.status == CO_SDO_RT_success) || (_impl(_this)->parent->sdo_client.status == CO_SDO_RT_abort)) {
                    _impl(_this)->sdo_state = SDO_END_REQUEST;
                }
                break;
            case SDO_END_REQUEST /**< SDO_END_REQUEST */:
                if (p_sdo->m_cb != NULL) {
                    p_sdo->m_cb((SM_SDO_STATUS_t)_impl(_this)->parent->sdo_client.status,
                                (int32_t)_impl(_this)->parent->sdo_client.tx_abort_code,
                                (int32_t)_impl(_this)->parent->sdo_client.rx_abort_code,
                                p_sdo->m_cb_arg);
                }
                CO_SDO_reset_status(&_impl(_this)->parent->sdo_client);
                _impl(_this)->sdo_process = false;
                if(_impl(_this)->is_sdo_now == true){
                    _impl(_this)->is_sdo_now = false;
                }
                break;
            default:
                break;
        }
    } else {
        if(_impl(_this)->is_sdo_now == true){
            _impl(_this)->sdo_process = true;
            _impl(_this)->sdo_state = SDO_SEND_REQUEST;
        }else if(sm_sdo_pop(_this, &_impl(_this)->sub_obj) == true){
            _impl(_this)->sdo_process = true;
            _impl(_this)->sdo_state = SDO_SEND_REQUEST;
        }
    }
}

void sm_co_process(sm_co_t* _this, uint32_t time_stamp) {
    if(!_this){
        return;
    }
    sm_co_if_process(_impl(_this)->p_if);
    CO_process(_impl(_this)->parent, (uint16_t)time_stamp);
    sm_co_SDO_process(_this);
}
