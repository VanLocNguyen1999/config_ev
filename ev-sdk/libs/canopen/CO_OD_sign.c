/*
 * CO_OD_sign.c
 *
 *  Created on: Oct 5, 2022
 *      Author: Dakaka
 */

#include "CO_OD_sign.h"

/**
 * command [input]: use "Function code command type"
 *
 */
void CO_OD_SIGN_set_func(CO_OD_Sign* this, CO_Func_Code_Group_t group, uint8_t command)
{
	if(CO_FCG__COMM_PARA == group)
	{
		this->func = (this->func & 0b11111100 ) | command;
	}
	else // CO_FCG__APP_PARA == group
	{
		this->func = (this->func & 0b11110011 ) | (uint8_t)(command<<2);
	}
}

CO_Func_Code_Command_t CO_OD_SIGN_get_func(CO_OD_Sign* this, CO_Func_Code_Group_t group)
{
	if(CO_FCG__COMM_PARA == group)
	{
		return (this->func & 0b00000011 );
	}
	else // CO_FCG__APP_PARA == group
	{
		return (uint8_t)((this->func & 0b00001100 ) >> 2);
	}
}
