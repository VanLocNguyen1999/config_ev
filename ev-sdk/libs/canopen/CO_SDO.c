#include "stdlib.h"
#include "stdio.h"
#include "CO_SDO.h"
#include "CO_OD.h"

void CO_SDO_abort_and_response(CO_SDO *p_sdo, const uint32_t code)
{
	p_sdo->tx_msg.data[0] = (uint8_t)(CCS_ABORT << 5);
	CO_memcpy(&(p_sdo->tx_msg.data[4]), (uint8_t*) &code, 4);
	p_sdo->tx_msg.data_len = 8;

	CO_CAN_send(&p_sdo->tx_msg);

	CO_SDO_abort(p_sdo, code);
}

void CO_SDO_abort(CO_SDO *p_sdo, const uint32_t code)
{
	p_sdo->tx_abort_code = code;

	CO_SDOtimeout_reset_and_inactivate(p_sdo);

	p_sdo->rx_msg.is_new	= 0;
	p_sdo->is_internal_processing = false;
	p_sdo->state = CO_SDO_ST_IDLE;
	p_sdo->status = CO_SDO_RT_abort;
}

void CO_SDO_set_transmission_type(CO_SDO *p_sdo, uint16_t type)
{
	bool valid = p_sdo->valid;
	p_sdo->valid = false;
	p_sdo->trans_type = type;
	p_sdo->valid = valid;
}
void CO_SDO_set_sync_mask_reg(CO_SDO *p_sdo, uint32_t mask)
{
	bool valid = p_sdo->valid;
	p_sdo->valid = false;
	p_sdo->sync_mask_reg = mask;
	p_sdo->valid = valid;
}
