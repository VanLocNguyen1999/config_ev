#include "CO_OD.h"
#include "stdlib.h"
#include "CO_Object.h"


uint16_t CO_OD_get_no(CO_OD* p_od,const uint16_t index){

    /* Fast search in ordered Object Dictionary. If indexes are mixed, this won't work. */
    /* If Object Dictionary has up to 2^N entries, then N is max number of loop passes. */
    uint16_t cur = 0;
    uint16_t min = 0;
    uint16_t max =p_od->number;
    CO_Object* p_obj;

    while(min < max)
    {
        cur = (min + max) >>1;
        p_obj=&p_od->list[cur];
        /* Is object matched */
        if(index == p_obj->index)
        {
            return cur;
        }

        if(index < p_obj->index)
        {
            max = cur;
            if(max) max--;
        }
        else
        {
            min = cur + 1U;
        }
    }

    if(min == max)
    {
        p_obj = &p_od->list[min];
        /* Is object matched */
        if(index == p_obj->index)
        {
            return min;
        }
    }

    return OD_UNSUPPORT_ENTRY;  /* object does not exist in OD */
}
