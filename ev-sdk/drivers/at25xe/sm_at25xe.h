/*
 * sm_at25xe.h
 *
 *  Created on: Aug 29, 2024
 *      Author: Ng Thuy Quynh
 */

#ifndef DRIVER_AT25XE_SM_AT25XE_H_
#define DRIVER_AT25XE_SM_AT25XE_H_

#include <sm_hal_io.h>
#include <sm_hal_delay.h>
#include <sm_hal_spi.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    AT25XE01 = 0,   // 1Mbit
    AT25XE02,       // 2Mbit
    AT25XE04,
    AT25XE08,
    AT25XE16,
    AT25XE32,
    AT25XE512,      // 512Kbit
} AT25XE_ID;

typedef struct {
    AT25XE_ID   ID;
    uint8_t     UniqID[8];
    uint32_t    PageSize;
    uint32_t    PageCount;
    uint32_t    SectorSize;
    uint32_t    SectorCount;
    uint32_t    BlockSize;
    uint32_t    BlockCount;
    uint32_t    CapacityInKiloByte;
    uint8_t     StatusRegister1;
    uint8_t     StatusRegister2;
    uint8_t     StatusRegister3;
    uint8_t     Lock;
    sm_hal_spi_t    *driver;
    sm_hal_io_t     *nss_pin;
} at25xe_t;

/**
 *
 * @param flash
 * @param driver
 * @param nss_pin
 */
void at25xe_Begin(at25xe_t *flash, sm_hal_spi_t *driver, sm_hal_io_t *nss_pin);
/**
 *
 * @param flash
 * @return
 */
bool at25xe_Init(at25xe_t *flash);
/**
 *
 * @param flash
 */
void at25xe_EraseChip(at25xe_t *flash);
/**
 *
 * @param flash
 * @param SectorAddr
 */
void at25xe_EraseSector(at25xe_t *flash, uint32_t SectorAddr);
/**
 *
 * @param flash
 * @param BlockAddr
 */
void at25xe_EraseBlock(at25xe_t *flash, uint32_t BlockAddr);
/**
 *
 * @param flash
 * @param PageAddr
 */
void at25xe_ErasePage(at25xe_t *flash, uint32_t PageAddr);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t at25xe_PageToSector(at25xe_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t at25xe_PageToBlock(at25xe_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t at25xe_SectorToBlock(at25xe_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t at25xe_SectorToPage(at25xe_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param BlockAddress
 * @return
 */
uint32_t at25xe_BlockToPage(at25xe_t *flash, uint32_t BlockAddress);
/**
 *
 * @param flash
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_PageSize
 * @return
 */
bool at25xe_IsEmptyPage(at25xe_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
/**
 *
 * @param flash
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_SectorSize
 * @return
 */
bool at25xe_IsEmptySector(at25xe_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
/**
 *
 * @param flash
 * @param Block_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_BlockSize
 * @return
 */
bool at25xe_IsEmptyBlock(at25xe_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param WriteAddr_inBytes
 */
void at25xe_WriteByte(at25xe_t *flash, uint8_t pBuffer, uint32_t WriteAddr_inBytes);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_PageSize
 */
void at25xe_WritePage(at25xe_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_SectorSize
 */
void at25xe_WriteSector(at25xe_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_SectorSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Block_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_BlockSize
 */
void at25xe_WriteBlock(at25xe_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Bytes_Address
 */
void at25xe_ReadByte(at25xe_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address);
/**
 *
 * @param flash
 * @param pBuffer
 * @param ReadAddr
 * @param NumByteToRead
 */
void at25xe_ReadBytes(at25xe_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_PageSize
 */
void at25xe_ReadPage(at25xe_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_SectorSize
 */
void at25xe_ReadSector(at25xe_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToRead_up_to_SectorSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Block_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_BlockSize
 */
void at25xe_ReadBlock(at25xe_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
/**
 *
 * @param flash
 * @param addr
 * @param buff
 * @param len
 */
void at25xe_read(at25xe_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);
/**
 *
 * @param flash
 * @param addr
 * @param buff
 * @param len
 */
void at25xe_write(at25xe_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);

#endif /* DRIVER_AT25XE_SM_AT25XE_H_ */
