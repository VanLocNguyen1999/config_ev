/*
 * sm_spi_flash.h
 *
 *  Created on: Jul 10, 2023
 *      Author: Admin
 */

#ifndef COMMON_SPIFLASH_SM_SPI_FLASH_H_
#define COMMON_SPIFLASH_SM_SPI_FLASH_H_


#include <sm_hal_io.h>
#include <sm_hal_spi.h>
#include "sm_config.h"

#include <stdbool.h>
#include <stdio.h>

#if USING_FS

typedef enum {
    W25Q10 = 0,
    W25Q20,
    W25Q40,
    W25Q80,
    W25Q16,
    W25Q32,
    W25Q64,
    W25Q128,
    W25Q256,
    W25Q512,
} W25QXX_ID_t;

typedef struct {
    W25QXX_ID_t ID;
    uint8_t UniqID[8];
    uint16_t PageSize;
    uint32_t PageCount;
    uint32_t SectorSize;
    uint32_t SectorCount;
    uint32_t BlockSize;
    uint32_t BlockCount;
    uint32_t CapacityInKiloByte;
    uint8_t StatusRegister1;
    uint8_t StatusRegister2;
    uint8_t StatusRegister3;
    uint8_t Lock;
    sm_hal_spi_t *driver;
    sm_hal_io_t *nss_pin;
} w25qxx_t;
/**
 *
 * @param flash
 * @param driver
 * @param nss_pin
 */
void W25qxx_Begin(w25qxx_t *flash, sm_hal_spi_t *driver, sm_hal_io_t *nss_pin);
/**
 *
 * @param flash
 * @return
 */
bool W25qxx_Init(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void W25qxx_EraseChip(w25qxx_t *flash);
/**
 *
 * @param flash
 * @param SectorAddr
 */
void W25qxx_EraseSector(w25qxx_t *flash, uint32_t SectorAddr);
/**
 *
 * @param flash
 * @param BlockAddr
 */
void W25qxx_EraseBlock(w25qxx_t *flash, uint32_t BlockAddr);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t W25qxx_PageToSector(w25qxx_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t W25qxx_PageToBlock(w25qxx_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t W25qxx_SectorToBlock(w25qxx_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t W25qxx_SectorToPage(w25qxx_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param BlockAddress
 * @return
 */
uint32_t W25qxx_BlockToPage(w25qxx_t *flash, uint32_t BlockAddress);
/**
 *
 * @param flash
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_PageSize
 * @return
 */
bool W25qxx_IsEmptyPage(w25qxx_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
/**
 *
 * @param flash
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_SectorSize
 * @return
 */
bool W25qxx_IsEmptySector(w25qxx_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
/**
 *
 * @param flash
 * @param Block_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_BlockSize
 * @return
 */
bool W25qxx_IsEmptyBlock(w25qxx_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param WriteAddr_inBytes
 */
void W25qxx_WriteByte(w25qxx_t *flash, uint8_t pBuffer, uint32_t WriteAddr_inBytes);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_PageSize
 */
void W25qxx_WritePage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_SectorSize
 */
void W25qxx_WriteSector(w25qxx_t *flash, uint8_t *pBuffer,
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
void W25qxx_WriteBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Bytes_Address
 */
void W25qxx_ReadByte(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address);
/**
 *
 * @param flash
 * @param pBuffer
 * @param ReadAddr
 * @param NumByteToRead
 */
void W25qxx_ReadBytes(w25qxx_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_PageSize
 */
void W25qxx_ReadPage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_SectorSize
 */
void W25qxx_ReadSector(w25qxx_t *flash, uint8_t *pBuffer,
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
void W25qxx_ReadBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
void read_flash(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);
void write_flash(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);
#endif
#endif /* COMMON_SPIFLASH_SM_SPI_FLASH_H_ */
