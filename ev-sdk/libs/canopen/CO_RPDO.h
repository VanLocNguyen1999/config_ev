#ifndef CO_RPDO_H_
#define CO_RPDO_H_
#include "CO_PDO.h"

typedef struct CO_RPDO_t CO_RPDO;

struct CO_RPDO_t{
        CO_PDO base;

        /* Communication parameter */
        									//<< cob id
        uint8_t				transmission_type;
		uint16_t			max_inhibit_type_ms;
		uint8_t				compatibility_entry;
		uint16_t			max_event_time_ms;
		uint8_t				sync_start_value;
};

void rpdo_init(CO_RPDO* p_rpdo,const uint16_t cob_id);

#endif
