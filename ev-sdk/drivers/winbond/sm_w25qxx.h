/*
 * sm_w25qxx.h
 *
 *  Created on: Aug 28, 2024
 *      Author: Ng Thuy Quynh
 */

#ifndef DRIVER_WINBOND_SM_W25QXX_H_
#define DRIVER_WINBOND_SM_W25QXX_H_

#include <sm_hal_io.h>
#include <sm_hal_spi.h>
#include <stdbool.h>
#include <stdio.h>

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
} W25QXX_ID;

typedef struct {
    W25QXX_ID   ID;
    uint8_t     UniqID[8];
    uint16_t    PageSize;
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
} w25qxx_t;

/**
 *
 * @param flash
 * @param driver
 * @param nss_pin
 */
void w25qxx_Begin(w25qxx_t *flash, sm_hal_spi_t *driver, sm_hal_io_t *nss_pin);
/**
 *
 * @param flash
 * @return
 */
bool w25qxx_Init(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void w25qxx_EraseChip(w25qxx_t *flash);
/**
 *
 * @param flash
 * @param SectorAddr
 */
void w25qxx_EraseSector(w25qxx_t *flash, uint32_t SectorAddr);
/**
 *
 * @param flash
 * @param BlockAddr
 */
void w25qxx_EraseBlock(w25qxx_t *flash, uint32_t BlockAddr);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t w25qxx_PageToSector(w25qxx_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param PageAddress
 * @return
 */
uint32_t w25qxx_PageToBlock(w25qxx_t *flash, uint32_t PageAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t w25qxx_SectorToBlock(w25qxx_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param SectorAddress
 * @return
 */
uint32_t w25qxx_SectorToPage(w25qxx_t *flash, uint32_t SectorAddress);
/**
 *
 * @param flash
 * @param BlockAddress
 * @return
 */
uint32_t w25qxx_BlockToPage(w25qxx_t *flash, uint32_t BlockAddress);
/**
 *
 * @param flash
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_PageSize
 * @return
 */
bool w25qxx_IsEmptyPage(w25qxx_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
/**
 *
 * @param flash
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_SectorSize
 * @return
 */
bool w25qxx_IsEmptySector(w25qxx_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
/**
 *
 * @param flash
 * @param Block_Address
 * @param OffsetInByte
 * @param NumByteToCheck_up_to_BlockSize
 * @return
 */
bool w25qxx_IsEmptyBlock(w25qxx_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param WriteAddr_inBytes
 */
void w25qxx_WriteByte(w25qxx_t *flash, uint8_t pBuffer, uint32_t WriteAddr_inBytes);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_PageSize
 */
void w25qxx_WritePage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToWrite_up_to_SectorSize
 */
void w25qxx_WriteSector(w25qxx_t *flash, uint8_t *pBuffer,
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
void w25qxx_WriteBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Bytes_Address
 */
void w25qxx_ReadByte(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address);
/**
 *
 * @param flash
 * @param pBuffer
 * @param ReadAddr
 * @param NumByteToRead
 */
void w25qxx_ReadBytes(w25qxx_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Page_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_PageSize
 */
void w25qxx_ReadPage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
/**
 *
 * @param flash
 * @param pBuffer
 * @param Sector_Address
 * @param OffsetInByte
 * @param NumByteToRead_up_to_SectorSize
 */
void w25qxx_ReadSector(w25qxx_t *flash, uint8_t *pBuffer,
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
void w25qxx_ReadBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
/**
 *
 * @param flash
 * @param addr
 * @param buff
 * @param len
 */
void w25qxx_read(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);
/**
 *
 * @param flash
 * @param addr
 * @param buff
 * @param len
 */
void w25qxx_write(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len);


#endif /* DRIVER_WINBOND_SM_W25QXX_H_ */
