/*
 * sm_w25qxx.c
 *
 *  Created on: Aug 28, 2024
 *      Author: Ng Thuy Quynh
 */
#include "sm_w25qxx.h"
#include "sm_hal.h"
#include "sm_logger.h"
#define W25QXX_DUMMY_BYTE 0xA5

#define w25qxx_Delay(x) sm_hal_delay_us(x)

#define _W25QXX_DEBUG 0

/*private api*/

/**
 *
 * @param flash
 * @param Data
 * @return
 */
uint8_t w25qxx_Spi(w25qxx_t *flash, uint8_t Data);
/**
 *
 * @param flash
 * @return
 */
uint32_t w25qxx_ReadID(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void w25qxx_ReadUniqID(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void w25qxx_WriteEnable(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void w25qxx_WriteDisable(w25qxx_t *flash);
/**
 *
 * @param flash
 */
void w25qxx_WaitForWriteEnd(w25qxx_t *flash);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @return
 */
uint8_t w25qxx_ReadStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @param Data
 */
void w25qxx_WriteStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data);
void w25qxx_read_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length);
void w25qxx_write_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length);
void w25qxx_writeread(w25qxx_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length);

void w25qxx_cs_assert(w25qxx_t *flash);
void w25qxx_cs_unassert(w25qxx_t *flash);

/*end private api*/

uint8_t w25qxx_Spi(w25qxx_t *flash, uint8_t Data) {
    uint8_t ret;
/*  Wire *driver = flash->driver->wire;
    driver->write_read(driver,&Data,&ret, 1,
            SPI_BIT_WIDTH_8_BITS);*/
    w25qxx_writeread(flash, &Data, &ret, 1);
    return ret;
}

uint32_t w25qxx_ReadID(w25qxx_t *flash) {
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash,0x9F);
    Temp0 = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    Temp1 = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    Temp2 = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
    w25qxx_cs_unassert(flash);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}

void w25qxx_ReadUniqID(w25qxx_t *flash) {
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash, 0x4B);
    for (uint8_t i = 0; i < 4; i++){
        w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);

    }
    for (uint8_t i = 0; i < 8; i++){
        flash->UniqID[i] = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);

    }
    w25qxx_cs_unassert(flash);
}

void w25qxx_WriteEnable(w25qxx_t *flash) {
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash, 0x06);
    w25qxx_cs_unassert(flash);
    w25qxx_Delay(1);
}
void w25qxx_WriteDisable(w25qxx_t *flash) {
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash, 0x04);
    w25qxx_cs_unassert(flash);
    w25qxx_Delay(1);
}

uint8_t w25qxx_ReadStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3) {
    uint8_t status = 0;
    w25qxx_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        w25qxx_Spi(flash, 0x05);
        status = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister1 = status;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        w25qxx_Spi(flash, 0x35);
        status = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister2 = status;
    } else {
        w25qxx_Spi(flash, 0x15);
        status = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        flash->StatusRegister3 = status;
    }
    w25qxx_cs_unassert(flash);
    return status;
}
void w25qxx_WriteStatusRegister(w25qxx_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data) {
    w25qxx_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        w25qxx_Spi(flash, 0x01);
        flash->StatusRegister1 = Data;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        w25qxx_Spi(flash, 0x31);
        flash->StatusRegister2 = Data;
    } else {
        w25qxx_Spi(flash, 0x11);
        flash->StatusRegister3 = Data;
    }
    w25qxx_Spi(flash, Data);
    w25qxx_cs_unassert(flash);
}
void w25qxx_WaitForWriteEnd(w25qxx_t *flash) {
    w25qxx_Delay(1);
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash, 0x05);
    do {
        flash->StatusRegister1 = w25qxx_Spi(flash, W25QXX_DUMMY_BYTE);
        w25qxx_Delay(1);
    } while ((flash->StatusRegister1 & 0x01) == 0x01);
    w25qxx_cs_unassert(flash);
}

void w25qxx_Begin(w25qxx_t *flash,sm_hal_spi_t *driver,sm_hal_io_t *nss_pin){
    flash->driver = driver;
    flash->nss_pin = nss_pin;
    sm_hal_spi_open(driver);
//    sm_hal_io_open (nss_pin, SM_HAL_IO_OUTPUT);
}

bool w25qxx_Init(w25qxx_t *flash) {
    flash->Lock = 1;
    w25qxx_Delay(1);
    uint32_t id;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx Init Begin...\r\n");
#endif
    //w25qxx_ReadUniqID(flash);
    id = w25qxx_ReadID(flash);

#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx ID:0x%X%X\r\n",(unsigned int)(id>>16),(unsigned int)id);
#endif

    uint32_t myid = id & 0x000000FF;

    switch ((uint8_t)myid) {
    case 0x20: //   w25q512
        flash->ID = W25Q512;
        flash->BlockCount = 1024;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q512\r\n");
#endif
        break;
    case 0x19: //   w25q256
        flash->ID = W25Q256;
        flash->BlockCount = 512;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q256\r\n");
#endif
        break;
    case 0x18: //   w25q128
        flash->ID = W25Q128;
        flash->BlockCount = 256;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q128\r\n");
#endif
        break;
    case 0x17: //   w25q64
        flash->ID = W25Q64;
        flash->BlockCount = 128;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q64\r\n");
#endif
        break;
    case 0x16: //   w25q32
        flash->ID = W25Q32;
        flash->BlockCount = 64;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q32\r\n");
#endif
        break;
    case 0x15: //   w25q16
        flash->ID = W25Q16;
        flash->BlockCount = 32;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q16\r\n");
#endif
        break;
    case 0x14: //   w25q80
        flash->ID = W25Q80;
        flash->BlockCount = 16;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q80\r\n");
#endif
        break;
    case 0x13: //   w25q40
        flash->ID = W25Q40;
        flash->BlockCount = 8;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q40\r\n");
#endif
        break;
    case 0x12: //   w25q20
        flash->ID = W25Q20;
        flash->BlockCount = 4;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q20\r\n");
#endif
        break;
    case 0x11: //   w25q10
        flash->ID = W25Q10;
        flash->BlockCount = 2;
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Chip: w25q10\r\n");
#endif
        break;
    default:
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx Unknown ID\r\n");
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
    w25qxx_ReadUniqID(flash);
    w25qxx_ReadStatusRegister(flash, 1);
    w25qxx_ReadStatusRegister(flash, 2);
    w25qxx_ReadStatusRegister(flash, 3);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx Page Size: %d Bytes\r\n", flash->PageSize);
    LOG_DBG("W25QXX","w25qxx Page Count: %d\r\n",flash->PageCount);
    LOG_DBG("W25QXX","w25qxx Sector Size: %d Bytes\r\n", flash->SectorSize);
    LOG_DBG("W25QXX","w25qxx Sector Count: %d\r\n", flash->SectorCount);
    LOG_DBG("W25QXX","w25qxx Block Size: %d Bytes\r\n", flash->BlockSize);
    LOG_DBG("W25QXX","w25qxx Block Count: %d\r\n", flash->BlockCount);
    LOG_DBG("W25QXX","w25qxx Capacity: %d KiloBytes\r\n", flash->CapacityInKiloByte);
    LOG_DBG("W25QXX","w25qxx Init Done\r\n");
#endif
    flash->Lock = 0;
    return true;
}

void w25qxx_EraseChip(w25qxx_t *flash) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
    LOG_DBG("W25QXX","w25qxx EraseChip Begin...\r\n");
#endif
    w25qxx_WriteEnable(flash);
    w25qxx_cs_assert(flash);
    w25qxx_Spi(flash, 0xC7);
    w25qxx_cs_unassert(flash);
    w25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx EraseBlock done after %d ms!\r\n", get_tick_count() - StartTime);
#endif
    w25qxx_Delay(10);
    flash->Lock = 0;
}
void w25qxx_EraseSector(w25qxx_t *flash, uint32_t SectorAddr) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
    LOG_DBG("W25QXX","w25qxx EraseSector %d Begin...\r\n", SectorAddr);
#endif
    w25qxx_WaitForWriteEnd(flash);
    SectorAddr = SectorAddr * flash->SectorSize;
    w25qxx_WriteEnable(flash);
    w25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256) {
        w25qxx_Spi(flash, 0x21);
        w25qxx_Spi(flash, (uint8_t) ((SectorAddr & 0xFF000000) >> 24));
    } else {
        w25qxx_Spi(flash, 0x20);
    }
    w25qxx_Spi(flash, (SectorAddr & 0xFF0000) >> 16);
    w25qxx_Spi(flash, (SectorAddr & 0xFF00) >> 8);
    w25qxx_Spi(flash, SectorAddr & 0xFF);
    w25qxx_cs_unassert(flash);
    w25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx EraseSector done after %d ms\r\n", get_tick_count() - StartTime);
#endif
    w25qxx_Delay(1);
    flash->Lock = 0;
}
void w25qxx_EraseBlock(w25qxx_t *flash, uint32_t BlockAddr) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx EraseBlock %d Begin...\r\n", BlockAddr);
    w25qxx_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    w25qxx_WaitForWriteEnd(flash);
    BlockAddr = BlockAddr * flash->SectorSize * 16;
    w25qxx_WriteEnable(flash);
    w25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256) {
        w25qxx_Spi(flash, 0xDC);
        w25qxx_Spi(flash, (uint8_t) ((BlockAddr & 0xFF000000) >> 24));
    } else {
        w25qxx_Spi(flash, 0xD8);
    }
    w25qxx_Spi(flash, (BlockAddr & 0xFF0000) >> 16);
    w25qxx_Spi(flash, (BlockAddr & 0xFF00) >> 8);
    w25qxx_Spi(flash, BlockAddr & 0xFF);
    w25qxx_cs_unassert(flash);
    w25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx EraseBlock done after %d ms\r\n", get_tick_count() - StartTime);
    w25qxx_Delay(100);
#endif
    w25qxx_Delay(1);
    flash->Lock = 0;
}

uint32_t w25qxx_PageToSector(w25qxx_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->SectorSize);
}
uint32_t w25qxx_PageToBlock(w25qxx_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->BlockSize);
}
uint32_t w25qxx_SectorToBlock(w25qxx_t *flash, uint32_t SectorAddress) {
    return ((SectorAddress * flash->SectorSize) / flash->BlockSize);
}
uint32_t w25qxx_SectorToPage(w25qxx_t *flash, uint32_t SectorAddress) {
    return (SectorAddress * flash->SectorSize) / flash->PageSize;
}
uint32_t w25qxx_BlockToPage(w25qxx_t *flash, uint32_t BlockAddress) {
    return (BlockAddress * flash->BlockSize) / flash->PageSize;
}

bool w25qxx_IsEmptyPage(w25qxx_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
    if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToCheck_up_to_PageSize == 0))
        NumByteToCheck_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte, NumByteToCheck_up_to_PageSize);
    w25qxx_Delay(100);
    //uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;

    for (i = OffsetInByte; i < flash->PageSize; i += sizeof(pBuffer)) {
        w25qxx_cs_assert(flash);
        WorkAddress = (i + Page_Address * flash->PageSize);
        if (flash->ID >= W25Q256) {
            w25qxx_Spi(flash, 0x0C);
            w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            w25qxx_Spi(flash, 0x0B);
        }
        w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        w25qxx_Spi(flash, WorkAddress & 0xFF);
        w25qxx_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
        w25qxx_read_bytes(flash, (void*)( pBuffer+j), 1);
        w25qxx_cs_assert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->PageSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->PageSize; i++) {
            w25qxx_cs_assert(flash);
            WorkAddress = (i + Page_Address * flash->PageSize);
            w25qxx_Spi(flash, 0x0B);
            if (flash->ID >= W25Q256) {
                w25qxx_Spi(flash, 0x0C);
                w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                w25qxx_Spi(flash, 0x0B);
            }
            w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            w25qxx_Spi(flash, WorkAddress & 0xFF);
            w25qxx_Spi(flash, 0);
            w25qxx_read_bytes(flash, (void*) pBuffer, 1);

            w25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }

    flash->Lock = 0;
    return true;
    NOT_EMPTY: flash->Lock = 0;
    return false;
}
bool w25qxx_IsEmptySector(w25qxx_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_SectorSize > flash->SectorSize)
            || (NumByteToCheck_up_to_SectorSize == 0))
        NumByteToCheck_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
    w25qxx_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->SectorSize; i += sizeof(pBuffer)) {
        w25qxx_cs_assert(flash);
        WorkAddress = (i + Sector_Address * flash->SectorSize);
        if (flash->ID >= W25Q256) {
            w25qxx_Spi(flash, 0x0C);
            w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            w25qxx_Spi(flash, 0x0B);
        }
        w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        w25qxx_Spi(flash, WorkAddress & 0xFF);
        w25qxx_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
            w25qxx_read_bytes(flash, (void*) (pBuffer+j),1);
        w25qxx_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->SectorSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->SectorSize; i++) {
            w25qxx_cs_assert(flash);
            WorkAddress = (i + Sector_Address * flash->SectorSize);
            if (flash->ID >= W25Q256) {
                w25qxx_Spi(flash, 0x0C);
                w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                w25qxx_Spi(flash, 0x0B);
            }
            w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            w25qxx_Spi(flash, WorkAddress & 0xFF);
            w25qxx_Spi(flash, 0);
            w25qxx_read_bytes(flash, (void*) pBuffer, 1);
            w25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckSector is Empty in %d ms\r\n", get_tick_count() - StartTime);
    w25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckSector is Not Empty in %d ms\r\n", get_tick_count() - StartTime);
    w25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}
bool w25qxx_IsEmptyBlock(w25qxx_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_BlockSize > flash->BlockSize)
            || (NumByteToCheck_up_to_BlockSize == 0))
        NumByteToCheck_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
    w25qxx_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->BlockSize; i += sizeof(pBuffer)) {
        w25qxx_cs_assert(flash);
        WorkAddress = (i + Block_Address * flash->BlockSize);

        if (flash->ID >= W25Q256) {
            w25qxx_Spi(flash, 0x0C);
            w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            w25qxx_Spi(flash, 0x0B);
        }
        w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        w25qxx_Spi(flash, WorkAddress & 0xFF);
        w25qxx_Spi(flash, 0);
        w25qxx_read_bytes(flash,pBuffer,sizeof(pBuffer));
        w25qxx_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->BlockSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->BlockSize; i++) {
            w25qxx_cs_assert(flash);
            WorkAddress = (i + Block_Address * flash->BlockSize);

            if (flash->ID >= W25Q256) {
                w25qxx_Spi(flash, 0x0C);
                w25qxx_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                w25qxx_Spi(flash, 0x0B);
            }
            w25qxx_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            w25qxx_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            w25qxx_Spi(flash, WorkAddress & 0xFF);
            w25qxx_Spi(flash, 0);
            w25qxx_read_bytes(flash,pBuffer, 1);
            w25qxx_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckBlock is Empty in %d ms\r\n", get_tick_count() - StartTime);
    w25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx CheckBlock is Not Empty in %d ms\r\n", get_tick_count() - StartTime);
    w25qxx_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}

void w25qxx_WriteByte(w25qxx_t *flash, uint8_t pBuffer,
        uint32_t WriteAddr_inBytes) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
    LOG_DBG("W25QXX","w25qxx WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
    w25qxx_WaitForWriteEnd(flash);
    w25qxx_WriteEnable(flash);
    w25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256) {
        w25qxx_Spi(flash, 0x12);
        w25qxx_Spi(flash, (uint8_t) ((WriteAddr_inBytes & 0xFF000000) >> 24));
    } else {
        w25qxx_Spi(flash, 0x02);
    }
    w25qxx_Spi(flash, (WriteAddr_inBytes & 0xFF0000) >> 16);
    w25qxx_Spi(flash, (WriteAddr_inBytes & 0xFF00) >> 8);
    w25qxx_Spi(flash, WriteAddr_inBytes & 0xFF);
    w25qxx_Spi(flash, pBuffer);
    w25qxx_cs_unassert(flash);
    w25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx WriteByte done after %d ms\r\n", get_tick_count() - StartTime);
#endif
    flash->Lock = 0;
}
void w25qxx_WritePage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
    if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToWrite_up_to_PageSize == 0))
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
    if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > flash->PageSize)
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToWrite_up_to_PageSize);
    w25qxx_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    w25qxx_WaitForWriteEnd(flash);
    w25qxx_WriteEnable(flash);
    w25qxx_cs_assert(flash);
    Page_Address = (Page_Address * flash->PageSize) + OffsetInByte;
    if (flash->ID >= W25Q256) {
        w25qxx_Spi(flash, 0x12);
        w25qxx_Spi(flash, (uint8_t) ((Page_Address & 0xFF000000) >> 24));
    } else {
        w25qxx_Spi(flash, 0x02);
    }
    w25qxx_Spi(flash, (Page_Address & 0xFF0000) >> 16);
    w25qxx_Spi(flash, (Page_Address & 0xFF00) >> 8);
    w25qxx_Spi(flash, Page_Address & 0xFF);
    w25qxx_write_bytes(flash,pBuffer,NumByteToWrite_up_to_PageSize);
    w25qxx_cs_unassert(flash);
    w25qxx_WaitForWriteEnd(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToWrite_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            LOG_DBG("W25QXX","\r\n");
            w25qxx_Delay(10);
        }
        LOG_DBG("W25QXX","0x%02X,", pBuffer[i]);
    }
    LOG_DBG("W25QXX","\r\n");
    LOG_DBG("W25QXX","w25qxx WritePage done after %d ms\r\n", StartTime);
    w25qxx_Delay(100);
#endif
    w25qxx_Delay(1);
    flash->Lock = 0;
}
void w25qxx_WriteSector(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_SectorSize) {
    if ((NumByteToWrite_up_to_SectorSize > flash->SectorSize)
            || (NumByteToWrite_up_to_SectorSize == 0))
        NumByteToWrite_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","+++w25qxx WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
    w25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize) {
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","---w25qxx WriteSector Faild!\r\n");
        w25qxx_Delay(100);
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
    StartPage = w25qxx_SectorToPage(flash, Sector_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        w25qxx_WritePage(flash, pBuffer, StartPage, LocalOffset,(uint32_t) BytesToWrite);
        StartPage++;
        BytesToWrite -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","---w25qxx WriteSector Done\r\n");
    w25qxx_Delay(100);
#endif
}
void w25qxx_WriteBlock(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Block_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_BlockSize) {
    if ((NumByteToWrite_up_to_BlockSize > flash->BlockSize)
            || (NumByteToWrite_up_to_BlockSize == 0))
        NumByteToWrite_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","+++w25qxx WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToWrite_up_to_BlockSize);
    w25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize) {
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","---w25qxx WriteBlock Faild!\r\n");
        w25qxx_Delay(100);
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
    StartPage = w25qxx_BlockToPage(flash,Block_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        w25qxx_WritePage(flash,pBuffer, StartPage, LocalOffset, (uint32_t)BytesToWrite);
        StartPage++;
        BytesToWrite -=(int32_t) flash->PageSize -(int32_t) LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","---w25qxx WriteBlock Done\r\n");
    w25qxx_Delay(100);
#endif
}

void w25qxx_ReadByte(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
    LOG_DBG("W25QXX","w25qxx ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
    w25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256)
    {
        w25qxx_Spi(flash,0x0C);
        w25qxx_Spi(flash,(uint8_t) ((Bytes_Address & 0xFF000000) >> 24));
    }
    else
    {
        w25qxx_Spi(flash,0x03);
    }
    w25qxx_Spi(flash,(Bytes_Address & 0xFF0000) >> 16);
    w25qxx_Spi(flash,(Bytes_Address & 0xFF00) >> 8);
    w25qxx_Spi(flash,Bytes_Address & 0xFF);
    //w25qxx_Spi(flash,0);
    *pBuffer = w25qxx_Spi(flash,W25QXX_DUMMY_BYTE);
    w25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx ReadByte 0x%02X done after %d ms\r\n", *pBuffer, get_tick_count() - StartTime);
#endif
    flash->Lock = 0;
}
void w25qxx_ReadBytes(w25qxx_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
#if (_W25QXX_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
    LOG_DBG("W25QXX","w25qxx ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
    w25qxx_cs_assert(flash);

    if (flash->ID >= W25Q256)
    {
        w25qxx_Spi(flash,0x0C);
        w25qxx_Spi(flash,(uint8_t) ((ReadAddr & 0xFF000000) >> 24));
    }
    else
    {
        w25qxx_Spi(flash,0x03);
    }
    w25qxx_Spi(flash,(ReadAddr & 0xFF0000) >> 16);
    w25qxx_Spi(flash,(ReadAddr & 0xFF00) >> 8);
    w25qxx_Spi(flash,ReadAddr & 0xFF);
    //w25qxx_Spi(flash,0);
    w25qxx_read_bytes(flash,pBuffer,NumByteToRead);
    w25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            LOG_DBG("W25QXX","\r\n");
            w25qxx_Delay(10);
        }
        LOG_DBG("W25QXX","0x%02X,", pBuffer[i]);
    }
    LOG_DBG("W25QXX","\r\n");
    LOG_DBG("W25QXX","w25qxx ReadBytes done after %d ms\r\n", StartTime);
    w25qxx_Delay(100);
#endif
    w25qxx_Delay(1);
    flash->Lock = 0;
}
void w25qxx_ReadPage(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize) {
    while (flash->Lock == 1)
        w25qxx_Delay(1);
    flash->Lock = 1;
    if ((NumByteToRead_up_to_PageSize > flash->PageSize) || (NumByteToRead_up_to_PageSize == 0))
        NumByteToRead_up_to_PageSize = flash->PageSize;
    if ((OffsetInByte + NumByteToRead_up_to_PageSize) > flash->PageSize)
        NumByteToRead_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","w25qxx ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToRead_up_to_PageSize);
    w25qxx_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    Page_Address = Page_Address * flash->PageSize + OffsetInByte;
    w25qxx_cs_assert(flash);
    if (flash->ID >= W25Q256)
    {
        w25qxx_Spi(flash,0x0C);
        w25qxx_Spi(flash,(uint8_t) ((Page_Address & 0xFF000000) >> 24));
    }
    else
    {
        w25qxx_Spi(flash,0x0B);
    }
    w25qxx_Spi(flash,(uint8_t)((Page_Address & 0xFF0000) >> 16));
    w25qxx_Spi(flash,(uint8_t)((Page_Address & 0xFF00) >> 8));
    w25qxx_Spi(flash,(uint8_t)(Page_Address & 0xFF));
    w25qxx_Spi(flash,0);
    w25qxx_read_bytes(flash,pBuffer,NumByteToRead_up_to_PageSize);
    w25qxx_cs_unassert(flash);
#if (_W25QXX_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
            LOG_DBG("W25QXX","\r\n");
            w25qxx_Delay(10);
        }
        LOG_DBG("W25QXX","0x%02X,", pBuffer[i]);
    }
    LOG_DBG("W25QXX","\r\n");
    LOG_DBG("W25QXX","w25qxx ReadPage done after %d ms\r\n", StartTime);
    w25qxx_Delay(100);
#endif
    w25qxx_Delay(1);
    flash->Lock = 0;
}
void w25qxx_ReadSector(w25qxx_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToRead_up_to_SectorSize) {
    if ((NumByteToRead_up_to_SectorSize > flash->SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
        NumByteToRead_up_to_SectorSize = flash->SectorSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","+++w25qxx ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
    w25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize)
    {
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","---w25qxx ReadSector Faild!\r\n");
        w25qxx_Delay(100);
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
    StartPage = w25qxx_SectorToPage(flash,Sector_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        w25qxx_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","---w25qxx ReadSector Done\r\n");
    w25qxx_Delay(100);
#endif
}
void w25qxx_ReadBlock(w25qxx_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize) {
    if ((NumByteToRead_up_to_BlockSize > flash->BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
        NumByteToRead_up_to_BlockSize = flash->BlockSize;
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","+++w25qxx ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToRead_up_to_BlockSize);
    w25qxx_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize)
    {
#if (_W25QXX_DEBUG == 1)
        LOG_DBG("W25QXX","w25qxx ReadBlock Faild!\r\n");
        w25qxx_Delay(100);
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
    StartPage = w25qxx_BlockToPage(flash,Block_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        w25qxx_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_W25QXX_DEBUG == 1)
    LOG_DBG("W25QXX","---w25qxx ReadBlock Done\r\n");
    w25qxx_Delay(100);
#endif
}
/*Private api*/
void w25qxx_read_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_read(flash->driver, buff,(uint16_t) length);
}
void w25qxx_write_bytes(w25qxx_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_write(flash->driver, buff,(uint16_t) length);
}
void w25qxx_writeread(w25qxx_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length){
    sm_hal_spi_write_read(flash->driver,(uint8_t*) src, dest,(uint16_t) length);
}
void w25qxx_cs_assert(w25qxx_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 0);
}
void w25qxx_cs_unassert(w25qxx_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 1);
}

void w25qxx_read(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        w25qxx_ReadSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
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
void w25qxx_write(w25qxx_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        w25qxx_WriteSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
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
