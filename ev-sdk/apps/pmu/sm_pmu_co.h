//
// Created by vnbk on 06/02/2025.
//

#ifndef EV_SDK_SM_PMU_CO_H
#define EV_SDK_SM_PMU_CO_H

/// PDO1 - Define
#define SM_PMU_PDO1_
#define SM_PMU_KEY_BIT_IDX					(0)
#define SM_PMU_HORN_BIT_IDX					(1)
#define SM_PMU_RIGHT_SIGNAL_BIT_IDX			(2)
#define SM_PMU_LEFT_SIGNAL_BIT_IDX			(3)
#define SM_PMU_DRIVE_MODE_BIT_IDX			(4)
#define SM_PMU_PARKING_BIT_IDX				(5)
#define SM_PMU_LOCK_PORT0_BIT_IDX		    (6)
#define SM_PMU_BRAKE_BIT_IDX				(7)

#define SM_PMU_HIGH_BEAM_BIT_IDX			(0)
#define SM_PMU_LOW_BEAM_BIT_IDX				(1)
#define SM_PMU_CHARGING_BIT_IDX				(2)
#define SM_PMU_THEFT_PROTECT_BIT_IDX		(3)
#define SM_PMU_REVERSE_BIT0_IDX				(4)
#define SM_PMU_REVERSE_BIT1_IDX				(5)
#define SM_PMU_LOCK_PORT1_BIT_IDX		    (6)
#define SM_PMU_LOCK_PORT2_BIT_IDX		    (7)


#define BIT_WRITE(value, bit, state)      \
    do {                                  \
        (value) &= (uint8_t)~(1U << (bit)); /* clear bit */ \
        if ((state) > 0)                  \
            (value) |= (uint8_t)(1U << (bit)); /* set bit */ \
    } while (0)



#endif //EV_SDK_SM_PMU_CO_H
