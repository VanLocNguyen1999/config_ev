/*
 * CO_PDO.c
 *
 *  Created on: Jun 25, 2019
 *      Author: quangnd
 */

#include "CO_PDO.h"
#include "stdlib.h"
#include "CO_Object.h"
#include "CO_config.h"
#include "CO_OD.h"

uint8_t mapping_fail_data[8] = {'m','a','p','_','f','a','i','l'};
static uint8_t data_0 = 0x00;
uint8_t *p_data_0	= &data_0;

static inline void CO_PDO_mapping_fail_handle(CO_PDO* p_pdo);

void CO_PDO_init(CO_PDO* p_pdo,
		CO_OD_PDO_Map_Para_Object_t *p_pdo_map_parameter,
		CO_OD* p_od_list)
{
	if(p_pdo == NULL) return;

	//p_pdo->msg; ->not configured yet!
	CO_PDO_set_mapping_parameter(p_pdo, p_pdo_map_parameter);
	CO_PDO_mapping(p_pdo, p_od_list);
}

void CO_PDO_set_mapping_parameter(CO_PDO* p_pdo,
		CO_OD_PDO_Map_Para_Object_t *p_pdo_map_parameter)
{
	if(p_pdo==NULL || p_pdo_map_parameter->object_number >= MAX_MAPPED_OBJECTS)
	{
		CO_detect_error();
		return;
	}

	p_pdo->mapping_parameter.object_number =p_pdo_map_parameter->object_number;
	for(int object_index = 0; object_index < p_pdo->mapping_parameter.object_number; object_index++)
	{
		p_pdo->mapping_parameter.app_object[object_index].w = p_pdo_map_parameter->app_object[object_index].w;
	}
}

void CO_PDO_mapping(CO_PDO* p_pdo, CO_OD* p_od_list)
{
	/* Disable PDO */
	p_pdo->msg.id.valid = CO_PDO_NOT_VALID;

	uint8_t data_index = 0;
	for(int object_index = 0; object_index < p_pdo->mapping_parameter.object_number; object_index++)
	{
		/* Parse mapping parameter object*/
		uint16_t map_index = p_pdo->mapping_parameter.app_object[object_index].index;
		uint8_t map_sub_index = p_pdo->mapping_parameter.app_object[object_index].sub_index;
		uint8_t map_len = p_pdo->mapping_parameter.app_object[object_index].len;
		/* Get data pointer */
		uint16_t od_list_no = CO_OD_get_no(p_od_list, map_index);
		if(OD_UNSUPPORT_ENTRY == od_list_no) //error
		{
			CO_PDO_mapping_fail_handle(p_pdo);
			return;
		}
		CO_Sub_Object* p_sub_obj = CO_OD_get_sub_object_pointer(p_od_list, od_list_no, map_sub_index);
		if(NULL == p_sub_obj) //error
		{
			CO_PDO_mapping_fail_handle(p_pdo);
			return;
		}
		/* Mapping data pointer*/
		for(int i = 0; i < map_len; i++)
		{
			if(i < p_sub_obj->len)
			{
				p_pdo->p_mapping_data[data_index] = (p_sub_obj->p_data + i);
			}
			else
			{
				p_pdo->p_mapping_data[data_index] = p_data_0;
			}
			data_index++;
			if(8 < data_index) //error
			{
				CO_PDO_mapping_fail_handle(p_pdo);
				return;
			}
		}
		/* Enable PDO */
		p_pdo->msg.id.valid = CO_PDO_VALID;
	}
}
static inline void CO_PDO_mapping_fail_handle(CO_PDO* p_pdo)
{
	for(int i = 0; i < 8; i++)
	{
		p_pdo->p_mapping_data[i] = (mapping_fail_data + i);
	}
}
/*

void pdo_map_object(CO_PDO* p_pdo,const CO_Object* p_obj,const uint8_t sub,const uint8_t len){

        if(p_pdo==NULL) return;

        if(p_pdo->mapped_object_no >=MAX_MAPPED_OBJECTS) return;

        PDO_Map_Object* map_obj= &p_pdo->mapped_objects[p_pdo->mapped_object_no];
        
        map_obj->index=p_obj->index;
        map_obj->len=len;
        map_obj->sub=sub;
        map_obj->p_data=p_obj->subs[sub].p_data;
        p_pdo->mapped_object_no++;
}

*/
