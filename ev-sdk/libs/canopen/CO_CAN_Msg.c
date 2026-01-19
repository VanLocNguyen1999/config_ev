/*
 * CAN_Msg.c
 *
 *  Created on: May 3, 2019
 *      Author: quangnd
 */
#include "CO_CAN_Msg.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "CO_utils.h"

static void co_can_send_impl_default(CO_CAN_Msg* p_msg)
{
	(void) p_msg;
}
CO_CAN_Send_Interface co_can_send = co_can_send_impl_default;
