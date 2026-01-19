/*
 * CO_CRC.h
 *
 *  Created on: Oct 13, 2022
 *      Author: Dakaka
 *
 *  Simply from bootloader/CRC
 */

#ifndef LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_CRC_H_
#define LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_CRC_H_

#include <stdint.h>
#include <stdlib.h>
/**
 * This function makes a CRC16 calculation on Length data bytes
 *
 * RETURN VALUE: 16 bit result of CRC calculation
 */
uint16_t CO_CRC_CalculateCRC16(const uint8_t *Buffer, uint32_t Length);

#endif /* LIBS_SELEX_LIBC_CANOPEN_CLIB_CO_CRC_H_ */
