/*
 * sm_spi_flash.c
 *
 *  Created on: Jul 10, 2023
 *      Author: Admin
 */


#include <sm_hal_delay.h>
#include "sm_spi_flash.h"

#if USING_FS

/*
 * exflash.c
 *
 *  Created on: Jul 28, 2022
 *      Author: Admin
 */

#define W25QXX_DUMMY_BYTE 0xA5

#define W25qxx_Delay(x) sm_hal_delay_ms(x)

/*private api*/

/**
 *
 * @param flash
 * @param Data
 * @return
 */
uint8_t W25qxx_Spi(w25qxx_t *flash, uint8_t Data);
/**
 *
 * @param flash
 * @return
 */
uint32_t W25qxx_ReadID(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void W25qxx_ReadUniqID(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void W25qxx_WriteEnable(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void W25qxx_WriteDisable(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void W25qxx_WaitForWriteEnd(w25qxx_t *flash);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @return
 */
uint8_t W25qxx_ReadStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @param Data
 */
void W25qxx_WriteStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data);
void W25qxx_read_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length);
void W25qxx_write_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length);
void W25qxx_writeread(w25qxx_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length);

void W25qxx_cs_assert(w25qxx_t *flash);
void W25qxx_cs_unassert(w25qxx_t *flash);

/*end private api*/

uint8_t W25qxx_Spi(w25qxx_t *flash, uint8_t Data) {
    uint8_t ret;
/*  Wire *driver = flash->driver->wire;
    driver->write_read(driver,&Data,&ret, 1,
            SPI_BIT_WIDTH_8_BITS);*/
    W25qxx_writeread(flash, &Data, &ret, 1);
    return ret;
}
uint8_t result[2];
uint32_t W25qxx_ReadID(w25qxx_t *flash) {
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash,0x9F);
    Temp0 = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    Temp1 = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    Temp2 = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    W25qxx_cs_unassert(flash);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}

void W25qxx_ReadUniqID(w25qxx_t *flash) {
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash, 0x4B);
    for (uint8_t i = 0; i < 4; i++){
        W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);

    }
    for (uint8_t i = 0; i < 8; i++){
        flash->UniqID[i] = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);

    }
    W25qxx_cs_unassert(flash);
}

void W25qxx_WriteEnable(w25qxx_t *flash) {
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash, 0x06);
    W25qxx_cs_unassert(flash);
    W25qxx_Delay(1);
}
void W25qxx_WriteDisable(w25qxx_t *flash) {
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash, 0x04);
    W25qxx_cs_unassert(flash);
    W25qxx_Delay(1);
}

uint8_t W25qxx_ReadStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3) {
    uint8_t status = 0;
    W25qxx_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        W25qxx_Spi(flash, 0x05);
        status = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister1 = status;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        W25qxx_Spi(flash, 0x35);
        status = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister2 = status;
    } else {
        W25qxx_Spi(flash, 0x15);
        status = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister3 = status;
    }
    W25qxx_cs_unassert(flash);
    return status;
}
void W25qxx_WriteStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data) {
    W25qxx_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        W25qxx_Spi(flash, 0x01);
        flash->StatusRegister1 = Data;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        W25qxx_Spi(flash, 0x31);
        flash->StatusRegister2 = Data;
    } else {
        W25qxx_Spi(flash, 0x11);
        flash->StatusRegister3 = Data;
    }
    W25qxx_Spi(flash, Data);
    W25qxx_cs_unassert(flash);
}
void W25qxx_WaitForWriteEnd(w25qxx_t *flash) {
    W25qxx_Delay(1);
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash, 0x05);
    do {
        flash->StatusRegister1 = W25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        W25qxx_Delay(1);
    } while ((flash->StatusRegister1 & 0x01) == 0x01);
    W25qxx_cs_unassert(flash);
}

void W25qxx_Begin(w25qxx_t *flash,sm_hal_spi_t *driver,sm_hal_io_t *nss_pin){
    flash->driver = driver;
    flash->nss_pin = nss_pin;
    sm_hal_spi_open(driver);
    sm_hal_io_open (nss_pin, SM_hal_IO_OUTPUT);
}

bool W25qxx_Init(w25qxx_t *flash) {
    flash->Lock = 1;
    W25qxx_Delay(1);
    uint32_t id;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx Init Begin...\r\n");
#endif
    //W25qxx_ReadUniqID(flash);
    id = W25qxx_ReadID(flash);

#if (_W25QXX_DEBUG == 1)
    printf("w25qxx ID:0x%X%X\r\n",(unsigned int)(id>>16),(unsigned int)id);
#endif

    uint32_t myid = id & 0x000000FF;

    switch ((uint8_t)myid) {
    case 0x20: //   w25q512
        flash->ID = W25Q512;
        flash->BlockCount = 1024;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q512\r\n");
#endif
        break;
    case 0x19: //   w25q256
        flash->ID = W25Q256;
        flash->BlockCount = 512;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q256\r\n");
#endif
        break;
    case 0x18: //   w25q128
        flash->ID = W25Q128;
        flash->BlockCount = 256;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q128\r\n");
#endif
        break;
    case 0x17: //   w25q64
        flash->ID = W25Q64;
        flash->BlockCount = 128;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q64\r\n");
#endif
        break;
    case 0x16: //   w25q32
        flash->ID = W25Q32;
        flash->BlockCount = 64;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q32\r\n");
#endif
        break;
    case 0x15: //   w25q16
        flash->ID = W25Q16;
        flash->BlockCount = 32;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q16\r\n");
#endif
        break;
    case 0x14: //   w25q80
        flash->ID = W25Q80;
        flash->BlockCount = 16;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q80\r\n");
#endif
        break;
    case 0x13: //   w25q40
        flash->ID = W25Q40;
        flash->BlockCount = 8;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q40\r\n");
#endif
        break;
    case 0x12: //   w25q20
        flash->ID = W25Q20;
        flash->BlockCount = 4;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q20\r\n");
#endif
        break;
    case 0x11: //   w25q10
        flash->ID = W25Q10;
        flash->BlockCount = 2;
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Chip: w25q10\r\n");
#endif
        break;
    default:
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx Unknown ID\r\n");
#endif
        flash->Lock = 0;
        return false;
    }
    flash->PageSize = 256;
    flash->SectorSize = 0x1000;
    flash->SectorCount = flash->BlockCount * 16;
    flash->PageCount = (flash->SectorCount * flash->SectorSize)
            / flash->PageSize;
    flash->BlockSize = flash->SectorSize * 16;
    flash->CapacityInKiloByte = (flash->SectorCount * flash->SectorSize) / 1024;
    W25qxx_ReadUniqID(flash);
    W25qxx_ReadStatusRegister(flash, 1);
    W25qxx_ReadStatusRegister(flash, 2);
    W25qxx_ReadStatusRegister(flash, 3);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx Page Size: %d Bytes\r\n", flash->PageSize);
    printf("w25qxx Page Count: %d\r\n",flash->PageCount);
    printf("w25qxx Sector Size: %d Bytes\r\n", flash->SectorSize);
    printf("w25qxx Sector Count: %d\r\n", flash->SectorCount);
    printf("w25qxx Block Size: %d Bytes\r\n", flash->BlockSize);
    printf("w25qxx Block Count: %d\r\n", flash->BlockCount);
    printf("w25qxx Capacity: %d KiloBytes\r\n", flash->CapacityInKiloByte);
    printf("w25qxx Init Done\r\n");
#endif
    flash->Lock = 0;
    return true;
}

void W25qxx_EraseChip(w25qxx_t *flash) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = HAL_GetTick();
    printf("w25qxx EraseChip Begin...\r\n");
#endif
    W25qxx_WriteEnable(flash);
    W25qxx_cs_assert(flash);
    W25qxx_Spi(flash, 0xC7);
    W25qxx_cs_unassert(flash);
    W25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx EraseBlock done after %d ms!\r\n", HAL_GetTick() - StartTime);
#endif
    W25qxx_Delay(10);
    flash->Lock = 0;
}
void W25qxx_EraseSector(w25qxx_t *flash, uint32_t SectorAddr) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = HAL_GetTick();
    printf("w25qxx EraseSector %d Begin...\r\n", SectorAddr);
#endif
    W25qxx_WaitForWriteEnd(flash);
    SectorAddr = SectorAddr * flash->SectorSize;
    W25qxx_WriteEnable(flash);
    W25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256) {
        W25qxx_Spi(flash, 0x21);
        W25qxx_Spi(flash, (uint8_t) ((SectorAddr & 0xFF000000) >> 24));
    } else {
        W25qxx_Spi(flash, 0x20);
    }
    W25qxx_Spi(flash, (SectorAddr & 0xFF0000) >> 16);
    W25qxx_Spi(flash, (SectorAddr & 0xFF00) >> 8);
    W25qxx_Spi(flash, SectorAddr & 0xFF);
    W25qxx_cs_unassert(flash);
    W25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx EraseSector done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
    W25qxx_Delay(1);
    flash->Lock = 0;
}
void W25qxx_EraseBlock(w25qxx_t *flash, uint32_t BlockAddr) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx EraseBlock %d Begin...\r\n", BlockAddr);
    W25qxx_Delay(100);
    uint32_t StartTime = HAL_GetTick();
#endif
    W25qxx_WaitForWriteEnd(flash);
    BlockAddr = BlockAddr * flash->SectorSize * 16;
    W25qxx_WriteEnable(flash);
    W25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256) {
        W25qxx_Spi(flash, 0xDC);
        W25qxx_Spi(flash, (uint8_t) ((BlockAddr & 0xFF000000) >> 24));
    } else {
        W25qxx_Spi(flash, 0xD8);
    }
    W25qxx_Spi(flash, (BlockAddr & 0xFF0000) >> 16);
    W25qxx_Spi(flash, (BlockAddr & 0xFF00) >> 8);
    W25qxx_Spi(flash, BlockAddr & 0xFF);
    W25qxx_cs_unassert(flash);
    W25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx EraseBlock done after %d ms\r\n", HAL_GetTick() - StartTime);
    W25qxx_Delay(100);
#endif
    W25qxx_Delay(1);
    flash->Lock = 0;
}

uint32_t W25qxx_PageToSector(w25qxx_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->SectorSize);
}
uint32_t W25qxx_PageToBlock(w25qxx_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->BlockSize);
}
uint32_t W25qxx_SectorToBlock(w25qxx_t *flash, uint32_t SectorAddress) {
    return ((SectorAddress * flash->SectorSize) / flash->BlockSize);
}
uint32_t W25qxx_SectorToPage(w25qxx_t *flash, uint32_t SectorAddress) {
    return (SectorAddress * flash->SectorSize) / flash->PageSize;
}
uint32_t W25qxx_BlockToPage(w25qxx_t *flash, uint32_t BlockAddress) {
    return (BlockAddress * flash->BlockSize) / flash->PageSize;
}

bool W25qxx_IsEmptyPage(w25qxx_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
    if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToCheck_up_to_PageSize == 0))
        NumByteToCheck_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte, NumByteToCheck_up_to_PageSize);
    W25qxx_Delay(100);
    //uint32_t StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;

    for (i = OffsetInByte; i < flash->PageSize; i += sizeof(pBuffer)) {
        W25qxx_cs_assert(flash);
        WorkAddress = (i + Page_Address * flash->PageSize);
        if (flash->ID >= W25Q256) {
            W25qxx_Spi(flash, 0x0C);
            W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            W25qxx_Spi(flash, 0x0B);
        }
        W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        W25qxx_Spi(flash, WorkAddress & 0xFF);
        W25qxx_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
        W25qxx_read_bytes(flash, (void*)( pBuffer+j), 1);
        W25qxx_cs_assert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->PageSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->PageSize; i++) {
            W25qxx_cs_assert(flash);
            WorkAddress = (i + Page_Address * flash->PageSize);
            W25qxx_Spi(flash, 0x0B);
            if (flash->ID >= W25Q256) {
                W25qxx_Spi(flash, 0x0C);
                W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                W25qxx_Spi(flash, 0x0B);
            }
            W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            W25qxx_Spi(flash, WorkAddress & 0xFF);
            W25qxx_Spi(flash, 0);
            W25qxx_read_bytes(flash, (void*) pBuffer, 1);

            W25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }

    flash->Lock = 0;
    return true;
    NOT_EMPTY: flash->Lock = 0;
    return false;
}
bool W25qxx_IsEmptySector(w25qxx_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_SectorSize > flash->SectorSize)
            || (NumByteToCheck_up_to_SectorSize == 0))
        NumByteToCheck_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
    W25qxx_Delay(100);
    uint32_t StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->SectorSize; i += sizeof(pBuffer)) {
        W25qxx_cs_assert(flash);
        WorkAddress = (i + Sector_Address * flash->SectorSize);
        if (flash->ID >= W25Q256) {
            W25qxx_Spi(flash, 0x0C);
            W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            W25qxx_Spi(flash, 0x0B);
        }
        W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        W25qxx_Spi(flash, WorkAddress & 0xFF);
        W25qxx_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
            W25qxx_read_bytes(flash, (void*) (pBuffer+j),1);
        W25qxx_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->SectorSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->SectorSize; i++) {
            W25qxx_cs_assert(flash);
            WorkAddress = (i + Sector_Address * flash->SectorSize);
            if (flash->ID >= W25Q256) {
                W25qxx_Spi(flash, 0x0C);
                W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                W25qxx_Spi(flash, 0x0B);
            }
            W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            W25qxx_Spi(flash, WorkAddress & 0xFF);
            W25qxx_Spi(flash, 0);
            W25qxx_read_bytes(flash, (void*) pBuffer, 1);
            W25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckSector is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    W25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckSector is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    W25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}
bool W25qxx_IsEmptyBlock(w25qxx_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_BlockSize > flash->BlockSize)
            || (NumByteToCheck_up_to_BlockSize == 0))
        NumByteToCheck_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
    W25qxx_Delay(100);
    uint32_t StartTime = HAL_GetTick();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->BlockSize; i += sizeof(pBuffer)) {
        W25qxx_cs_assert(flash);
        WorkAddress = (i + Block_Address * flash->BlockSize);

        if (flash->ID >= W25Q256) {
            W25qxx_Spi(flash, 0x0C);
            W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            W25qxx_Spi(flash, 0x0B);
        }
        W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        W25qxx_Spi(flash, WorkAddress & 0xFF);
        W25qxx_Spi(flash, 0);
        W25qxx_read_bytes(flash,pBuffer,sizeof(pBuffer));
        W25qxx_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->BlockSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->BlockSize; i++) {
            W25qxx_cs_assert(flash);
            WorkAddress = (i + Block_Address * flash->BlockSize);

            if (flash->ID >= W25Q256) {
                W25qxx_Spi(flash, 0x0C);
                W25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                W25qxx_Spi(flash, 0x0B);
            }
            W25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            W25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            W25qxx_Spi(flash, WorkAddress & 0xFF);
            W25qxx_Spi(flash, 0);
            W25qxx_read_bytes(flash,pBuffer, 1);
            W25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckBlock is Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    W25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx CheckBlock is Not Empty in %d ms\r\n", HAL_GetTick() - StartTime);
    W25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}

void W25qxx_WriteByte(w25qxx_t *flash, uint8_t pBuffer,
        uint32_t WriteAddr_inBytes) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = HAL_GetTick();
    printf("w25qxx WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
    W25qxx_WaitForWriteEnd(flash);
    W25qxx_WriteEnable(flash);
    W25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256) {
        W25qxx_Spi(flash, 0x12);
        W25qxx_Spi(flash, (uint8_t) ((WriteAddr_inBytes & 0xFF000000) >> 24));
    } else {
        W25qxx_Spi(flash, 0x02);
    }
    W25qxx_Spi(flash, (WriteAddr_inBytes & 0xFF0000) >> 16);
    W25qxx_Spi(flash, (WriteAddr_inBytes & 0xFF00) >> 8);
    W25qxx_Spi(flash, WriteAddr_inBytes & 0xFF);
    W25qxx_Spi(flash, pBuffer);
    W25qxx_cs_unassert(flash);
    W25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx WriteByte done after %d ms\r\n", HAL_GetTick() - StartTime);
#endif
    flash->Lock = 0;
}
void W25qxx_WritePage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
    if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToWrite_up_to_PageSize == 0))
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
    if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > flash->PageSize)
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToWrite_up_to_PageSize);
    W25qxx_Delay(100);
    uint32_t StartTime = HAL_GetTick();
#endif
    W25qxx_WaitForWriteEnd(flash);
    W25qxx_WriteEnable(flash);
    W25qxx_cs_assert(flash);
    Page_Address = (Page_Address * flash->PageSize) + OffsetInByte;
    if (flash->ID >= W25Q256) {
        W25qxx_Spi(flash, 0x12);
        W25qxx_Spi(flash, (uint8_t) ((Page_Address & 0xFF000000) >> 24));
    } else {
        W25qxx_Spi(flash, 0x02);
    }
    W25qxx_Spi(flash, (Page_Address & 0xFF0000) >> 16);
    W25qxx_Spi(flash, (Page_Address & 0xFF00) >> 8);
    W25qxx_Spi(flash, Page_Address & 0xFF);
    W25qxx_write_bytes(flash,pBuffer,NumByteToWrite_up_to_PageSize);
    W25qxx_cs_unassert(flash);
    W25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = HAL_GetTick() - StartTime;
    for (uint32_t i = 0; i < NumByteToWrite_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            W25qxx_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("w25qxx WritePage done after %d ms\r\n", StartTime);
    W25qxx_Delay(100);
#endif
    W25qxx_Delay(1);
    flash->Lock = 0;
}
void W25qxx_WriteSector(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_SectorSize) {
    if ((NumByteToWrite_up_to_SectorSize > flash->SectorSize)
            || (NumByteToWrite_up_to_SectorSize == 0))
        NumByteToWrite_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    printf("+++w25qxx WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
    W25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize) {
#if (_W25QXX_DEBUG == 1)
        printf("---w25qxx WriteSector Faild!\r\n");
        W25qxx_Delay(100);
#endif
        return;
    }
    uint32_t StartPage;
    int32_t BytesToWrite;
    uint32_t LocalOffset;
    if ((OffsetInByte + NumByteToWrite_up_to_SectorSize) > flash->SectorSize)
        BytesToWrite = (int32_t)flash->SectorSize - (int32_t)OffsetInByte;
    else
        BytesToWrite =(int32_t) NumByteToWrite_up_to_SectorSize;
    StartPage = W25qxx_SectorToPage(flash, Sector_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        W25qxx_WritePage(flash, pBuffer, StartPage, LocalOffset,(uint32_t) BytesToWrite);
        StartPage++;
        BytesToWrite -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
    printf("---w25qxx WriteSector Done\r\n");
    W25qxx_Delay(100);
#endif
}
void W25qxx_WriteBlock(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Block_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_BlockSize) {
    if ((NumByteToWrite_up_to_BlockSize > flash->BlockSize)
            || (NumByteToWrite_up_to_BlockSize == 0))
        NumByteToWrite_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    printf("+++w25qxx WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToWrite_up_to_BlockSize);
    W25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize) {
#if (_W25QXX_DEBUG == 1)
        printf("---w25qxx WriteBlock Faild!\r\n");
        W25qxx_Delay(100);
#endif
        return;
    }
    uint32_t StartPage;
    int32_t BytesToWrite;
    uint32_t LocalOffset;
    if ((OffsetInByte + NumByteToWrite_up_to_BlockSize) > flash->BlockSize)
        BytesToWrite = (int32_t)flash->BlockSize -(int32_t) OffsetInByte;
    else
        BytesToWrite = (int32_t)NumByteToWrite_up_to_BlockSize;
    StartPage = W25qxx_BlockToPage(flash,Block_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        W25qxx_WritePage(flash,pBuffer, StartPage, LocalOffset, (uint32_t)BytesToWrite);
        StartPage++;
        BytesToWrite -=(int32_t) flash->PageSize -(int32_t) LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
    printf("---w25qxx WriteBlock Done\r\n");
    W25qxx_Delay(100);
#endif
}

void W25qxx_ReadByte(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = HAL_GetTick();
    printf("w25qxx ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
    W25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256)
    {
        W25qxx_Spi(flash,0x0C);
        W25qxx_Spi(flash,(uint8_t) ((Bytes_Address & 0xFF000000) >> 24));
    }
    else
    {
        W25qxx_Spi(flash,0x03);
    }
    W25qxx_Spi(flash,(Bytes_Address & 0xFF0000) >> 16);
    W25qxx_Spi(flash,(Bytes_Address & 0xFF00) >> 8);
    W25qxx_Spi(flash,Bytes_Address & 0xFF);
    //W25qxx_Spi(flash,0);
    *pBuffer = W25qxx_Spi(flash,W25QXX_DUMMY_BYTE);
    W25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx ReadByte 0x%02X done after %d ms\r\n", *pBuffer, HAL_GetTick() - StartTime);
#endif
    flash->Lock = 0;
}
void W25qxx_ReadBytes(w25qxx_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = HAL_GetTick();
    printf("w25qxx ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
    W25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256)
    {
        W25qxx_Spi(flash,0x0C);
        W25qxx_Spi(flash,(uint8_t) ((ReadAddr & 0xFF000000) >> 24));
    }
    else
    {
        W25qxx_Spi(flash,0x03);
    }
    W25qxx_Spi(flash,(ReadAddr & 0xFF0000) >> 16);
    W25qxx_Spi(flash,(ReadAddr & 0xFF00) >> 8);
    W25qxx_Spi(flash,ReadAddr & 0xFF);
    //W25qxx_Spi(flash,0);
    W25qxx_read_bytes(flash,pBuffer,NumByteToRead);
    W25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = HAL_GetTick() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            W25qxx_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("w25qxx ReadBytes done after %d ms\r\n", StartTime);
    W25qxx_Delay(100);
#endif
    W25qxx_Delay(1);
    flash->Lock = 0;
}
void W25qxx_ReadPage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize) {
    while (flash->Lock == 1)
        W25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToRead_up_to_PageSize > flash->PageSize) || (NumByteToRead_up_to_PageSize == 0))
        NumByteToRead_up_to_PageSize = flash->PageSize;
    if ((OffsetInByte + NumByteToRead_up_to_PageSize) > flash->PageSize)
        NumByteToRead_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    printf("w25qxx ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToRead_up_to_PageSize);
    W25qxx_Delay(100);
    uint32_t StartTime = HAL_GetTick();
#endif
    Page_Address = Page_Address * flash->PageSize + OffsetInByte;
    W25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256)
    {
        W25qxx_Spi(flash,0x0C);
        W25qxx_Spi(flash,(uint8_t) ((Page_Address & 0xFF000000) >> 24));
    }
    else
    {
        W25qxx_Spi(flash,0x0B);
    }
    W25qxx_Spi(flash,(uint8_t)((Page_Address & 0xFF0000) >> 16));
    W25qxx_Spi(flash,(uint8_t)((Page_Address & 0xFF00) >> 8));
    W25qxx_Spi(flash,(uint8_t)(Page_Address & 0xFF));
    W25qxx_Spi(flash,0);
    W25qxx_read_bytes(flash,pBuffer,NumByteToRead_up_to_PageSize);
    W25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = HAL_GetTick() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            printf("\r\n");
            W25qxx_Delay(10);
        }
        printf("0x%02X,", pBuffer[i]);
    }
    printf("\r\n");
    printf("w25qxx ReadPage done after %d ms\r\n", StartTime);
    W25qxx_Delay(100);
#endif
    W25qxx_Delay(1);
    flash->Lock = 0;
}
void W25qxx_ReadSector(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToRead_up_to_SectorSize) {
    if ((NumByteToRead_up_to_SectorSize > flash->SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
        NumByteToRead_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    printf("+++w25qxx ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
    W25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize)
    {
#if (_W25QXX_DEBUG == 1)
        printf("---w25qxx ReadSector Faild!\r\n");
        W25qxx_Delay(100);
#endif
        return;
    }
    uint32_t StartPage;
    int32_t BytesToRead;
    uint32_t LocalOffset;
    if ((OffsetInByte + NumByteToRead_up_to_SectorSize) > flash->SectorSize)
        BytesToRead = (int32_t)flash->SectorSize -(int32_t) OffsetInByte;
    else
        BytesToRead = (int32_t)NumByteToRead_up_to_SectorSize;
    StartPage = W25qxx_SectorToPage(flash,Sector_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        W25qxx_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
    printf("---w25qxx ReadSector Done\r\n");
    W25qxx_Delay(100);
#endif
}
void W25qxx_ReadBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize) {
    if ((NumByteToRead_up_to_BlockSize > flash->BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
        NumByteToRead_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    printf("+++w25qxx ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToRead_up_to_BlockSize);
    W25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize)
    {
#if (_W25QXX_DEBUG == 1)
        printf("w25qxx ReadBlock Faild!\r\n");
        W25qxx_Delay(100);
#endif
        return;
    }
    uint32_t StartPage;
    int32_t BytesToRead;
    uint32_t LocalOffset;
    if ((OffsetInByte + NumByteToRead_up_to_BlockSize) > flash->BlockSize)
        BytesToRead = (int32_t)flash->BlockSize -(int32_t) OffsetInByte;
    else
        BytesToRead = (int32_t)NumByteToRead_up_to_BlockSize;
    StartPage = W25qxx_BlockToPage(flash,Block_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        W25qxx_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
    printf("---w25qxx ReadBlock Done\r\n");
    W25qxx_Delay(100);
#endif
}
/*Private api*/
void W25qxx_read_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_read(flash->driver, buff,(uint16_t) length);
}
void W25qxx_write_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_write(flash->driver, buff,(uint16_t) length);
}
void W25qxx_writeread(w25qxx_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length){
    sm_hal_spi_write_read(flash->driver,(uint8_t*) src, dest,(uint16_t) length);
}
void W25qxx_cs_assert(w25qxx_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 0);
}
void W25qxx_cs_unassert(w25qxx_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 1);
}

void read_flash(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        W25qxx_ReadSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
        if (len == pageremain)
            break;
        else
        {
            buff += pageremain;
            addr_begin += pageremain;
            len -= pageremain;
            if (len > 4096)
                pageremain = 4096;
            else
            {
                pageremain = len;
            }
        }
    }
}
void write_flash(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        W25qxx_WriteSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
        if (len == pageremain)
            break;
        else
        {
            buff += pageremain;
            addr_begin += pageremain;
            len -= pageremain;
            if (len > 4096)
                pageremain = 4096;
            else
            {
                pageremain = len;
            }
        }
    }
}
w25qxx_t bms_exflash;

#endif
