/*
 * CO_EMCY.c
 *
 *  Created on: Jul 5, 2022
 *      Author: Dakaka
 */

#include "CO_EMCY.h"

void CO_EMCY_init(CO_EMCY *p_em)
{
	while (p_em == NULL);
	for(int i=0; i<8; i++)
	{
		p_em->error_data.data[i] = 0;
	}
	p_em->error_code.w = 0;
	p_em->error_reg.w = 0;
//	p_em->error_handle = ? -> Must set if used it!
}
