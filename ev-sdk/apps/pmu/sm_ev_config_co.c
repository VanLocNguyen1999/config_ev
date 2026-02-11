/*
 * sm_ev_config_co.c
 *
 *  Created on: 21 Jan 2026
 *      Author: My PC
 */

#include "sm_core_co.h"
#include "sm_co_od_common.h"

#include "sm_co_if.h"
#include "sm_ev_config_co.h"

 /*EV config*/
#define SDO_EV_CONFIG_PARA_INDEX					0x2180

// Write Sub-Index
#define SDO_SUBIDX_PURPOSE_CFG_WRITE    	(0x00)
#define SDO_SUBIDX_ENA_PURPOSE_SW_WRITE 	(0x01)
#define SDO_SUBIDX_SPORT_SPEED_INSP_WRITE 	(0x02)
#define SDO_SUBIDX_ECO_SPEED_INSP_WRITE 	(0x03)
#define SDO_SUBIDX_IBAT_LIMIT_INSP_WRITE 	(0x04)
#define SDO_SUBIDX_AC_UV_THRES_INSP_WRITE 	(0x05)

// Read Sub-Index
#define SDO_SUBIDX_PURPOSE_CFG_READ	    	(0x06)

#define SDO_MC_DRIVER_MODE_INDEX            (0x2130)
#define SDO_MC_DRIVER_MODE_SUB_INDEX        (0x00)

#define CMD_SUCCESS    (0)
#define CMD_FAILURE    (-1)

#define _impl(x)			((sm_ev_config_co_impl_t*)(x))

typedef enum {

    DISABLE_PURPOSE_SWITCH,    // Không cho phép chuyển đổi
    ENABLE_PURPOSE_SWITCH    // Cho phép chuyển đổi
} Purpose_sw_cfg_t;


typedef enum {
    PURPOSE_FOR_VEHICLE_MARKET,       // Cho xe thị trường
    PURPOSE_FOR_VEHICLE_INSPECTION,   // Dùng cho hậu kiểm nhà máy
    PURPOSE_FOR_DEBUG                 // Dùng để Debug
} Vehicle_purpose_t;
typedef struct {
	Vehicle_purpose_t       purpose;
	Purpose_sw_cfg_t   		switch_config;
    InspectionConfig_t      inspection_config;
} sm_purpose_cf_t;

typedef struct{
    sm_co_t* m_co;
    void* m_co_if;
    InspectionConfig_t m_data_write;
    sm_purpose_cf_t m_data_config;

    sm_ev_config_cmd_t m_cmd;
}sm_ev_config_co_impl_t;

static sm_ev_config_co_impl_t g_ev_config;
sm_ev_config_para_t* sm_ev_config_para_create(sm_co_t *m_co){

    if(!m_co){
        return NULL;
    }
    g_ev_config.m_co = m_co;
    g_ev_config.m_co_if = (void*)sm_co_get_if(m_co);

    return (sm_ev_config_para_t*)&g_ev_config;
}

const InspectionConfig_t* sm_ev_config_get_para(sm_ev_config_para_t* _this) {

    return &(_impl(_this)->m_data_config.inspection_config);
}
static void sm_bp_co_wrirte_sdo_cb(SM_SDO_STATUS_t _status, int32_t _tx_err,
		int32_t _rx_err, void *_arg) {

	(void)_rx_err;
	(void)_tx_err;
	sm_ev_config_co_impl_t* ev_co = (sm_ev_config_co_impl_t*)(_arg);
	if (_status == SM_SDO_ST_SUCCESS) {

        if(ev_co->m_cmd.m_cb){
        	ev_co->m_cmd.m_cb(ev_co->m_cmd.m_data,
        						CMD_SUCCESS,
								ev_co->m_cmd.m_arg);
        }
	} else if (_status == SM_SDO_ST_ABORT) {
	       if(ev_co->m_cmd.m_cb){
	    	   ev_co->m_cmd.m_cb(ev_co->m_cmd.m_data,
	    			   	   	   	   CMD_FAILURE,
									ev_co->m_cmd.m_arg);
	        }
	}

}
void sm_ev_set_uv_protect(sm_ev_config_para_t *_this, float uv_protect){

	_impl(_this)->m_data_write.uv_protect = uv_protect;
}

void sm_ev_set_ibat_limit(sm_ev_config_para_t *_this, float ibat_limit){

	_impl(_this)->m_data_write.ibat_limit = ibat_limit;
}

void sm_ev_set_eco_speed(sm_ev_config_para_t *_this, uint8_t eco_speed){

	_impl(_this)->m_data_write.eco_speed = eco_speed;
}

void sm_ev_set_sport_speed(sm_ev_config_para_t *_this, uint8_t sport_speed){

	_impl(_this)->m_data_write.sport_speed = sport_speed;
}

static int32_t sm_ev_co_wrirte_uv_protect(sm_ev_config_co_impl_t *_this){

    return sm_co_sdo_client_send(_impl(_this)->m_co,
    							 SDO_EV_CONFIG_PARA_INDEX,
								 SDO_SUBIDX_AC_UV_THRES_INSP_WRITE,
								 MC_NODE_ID_DEFAULT,
                                 (void*)&_this->m_data_write.uv_protect,
                                 sizeof(_impl(_this)->m_data_write.uv_protect),
                                 1000,
								 sm_bp_co_wrirte_sdo_cb,
                                 _this);
}

static int32_t sm_ev_co_wrirte_ibat_limit(sm_ev_config_co_impl_t *_this){

    return sm_co_sdo_client_send(_impl(_this)->m_co,
    							 SDO_EV_CONFIG_PARA_INDEX,
								 SDO_SUBIDX_IBAT_LIMIT_INSP_WRITE,
								 MC_NODE_ID_DEFAULT,
								 (void*)&_this->m_data_write.ibat_limit,
                                 sizeof(_impl(_this)->m_data_write.ibat_limit),
                                 1000,
								 sm_bp_co_wrirte_sdo_cb,
                                 _this);
}

static int32_t sm_ev_co_wrirte_eco_speed(sm_ev_config_co_impl_t *_this){

    return sm_co_sdo_client_send(_impl(_this)->m_co,
    							 SDO_EV_CONFIG_PARA_INDEX,
								 SDO_SUBIDX_ECO_SPEED_INSP_WRITE,
								 MC_NODE_ID_DEFAULT,
								 (void*)&_this->m_data_write.eco_speed,
                                 sizeof(_impl(_this)->m_data_write.eco_speed),
                                 1000,
								 sm_bp_co_wrirte_sdo_cb,
                                 _this);
}

static int32_t sm_ev_co_wrirte_sport_speed(sm_ev_config_co_impl_t *_this){

    return sm_co_sdo_client_send(_impl(_this)->m_co,
    							 SDO_EV_CONFIG_PARA_INDEX,
								 SDO_SUBIDX_SPORT_SPEED_INSP_WRITE,
								 MC_NODE_ID_DEFAULT,
								 (void*)&_this->m_data_write.sport_speed,
                                 sizeof(_impl(_this)->m_data_write.sport_speed),
                                 1000,
								 sm_bp_co_wrirte_sdo_cb,
                                 _this);
}

static int32_t sm_ev_co_read_ev_config(sm_ev_config_co_impl_t* _this){
    return sm_co_sdo_client_receive(_impl(_this)->m_co,
    								SDO_EV_CONFIG_PARA_INDEX,
									SDO_SUBIDX_PURPOSE_CFG_READ,
									MC_NODE_ID_DEFAULT,
									(void*)&_this->m_data_config,
                                    sizeof(sm_purpose_cf_t),
                                    1000,
									sm_bp_co_wrirte_sdo_cb,
                                    _this);
}

int32_t sm_ev_config_co_set_cmd(sm_ev_config_para_t* _this, sm_ev_config_cmd_t _cmd){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd = _cmd;

    switch (_cmd.m_cmd){
        case EV_CFG_UV_PROTECT:
        	sm_ev_co_wrirte_uv_protect(_impl(_this));
            break;
        case EV_CFG_IBAT_LIMIT:
        	sm_ev_co_wrirte_ibat_limit(_impl(_this));
            break;
        case EV_CFG_ECO_SPEED:
        	sm_ev_co_wrirte_eco_speed(_impl(_this));
            break;
        case EV_CFG_SPORT_SPEED:
        	sm_ev_co_wrirte_sport_speed(_impl(_this));
            break;
        case EV_CFG_READ:
        	sm_ev_co_read_ev_config(_impl(_this));
            break;
        default:
        	return -1;
    }
    return 0;
}
