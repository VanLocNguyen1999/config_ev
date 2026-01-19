/*
 * sm_at25xe.c
 *
 *  Created on: Aug 29, 2024
 *      Author: N
 */

#include "sm_at25xe.h"


#define AT25XE_DUMMY_BYTE 0xAA

#define at25xe_Delay(x) sm_hal_delay_ms(x)

/*private api*/

/**
 *
 * @param flash
 * @param Data
 * @return
 */
uint8_t at25xe_Spi(at25xe_t *flash, uint8_t Data);
/**
 *
 * @param flash
 * @return
 */
uint32_t at25xe_ReadID(at25xe_t *flash);
/**
 *
 * @param flash
 */
void at25xe_ReadUniqID(at25xe_t *flash);
/**
 *
 * @param flash
 */
void at25xe_WriteEnable(at25xe_t *flash);
/**
 *
 * @param flash
 */
void at25xe_WriteDisable(at25xe_t *flash);
/**
 *
 * @param flash
 */
void at25xe_WaitForWriteEnd(at25xe_t *flash);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @return
 */
uint8_t at25xe_ReadStatusRegister(at25xe_t *flash,
        uint8_t SelectStatusRegister_1_2_3);
/**
 *
 * @param flash
 * @param SelectStatusRegister_1_2_3
 * @param Data
 */
void at25xe_WriteStatusRegister(at25xe_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data);
void at25xe_read_bytes(at25xe_t *flash,uint8_t *buff,uint32_t length);
void at25xe_write_bytes(at25xe_t *flash,uint8_t *buff,uint32_t length);
void at25xe_writeread(at25xe_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length);

void at25xe_cs_assert(at25xe_t *flash);
void at25xe_cs_unassert(at25xe_t *flash);

/*end private api*/

uint8_t at25xe_Spi(at25xe_t *flash, uint8_t _opcode) {
    uint8_t ret;
    at25xe_writeread(flash, &_opcode, &ret, 1);
    return ret;
}

uint32_t at25xe_ReadID(at25xe_t *flash) {
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    at25xe_cs_assert(flash);
    at25xe_Spi(flash,0x9F);
    Temp0 = at25xe_Spi(flash, AT25XE_DUMMY_BYTE); // Manufacture ID: 1Fh for Renesas or Adesto
    Temp1 = at25xe_Spi(flash, AT25XE_DUMMY_BYTE); // Family ID:
    Temp2 = at25xe_Spi(flash, AT25XE_DUMMY_BYTE); // Product version:
    at25xe_cs_unassert(flash);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}

//void at25xe_ReadUniqID(at25xe_t *flash) {
//    at25xe_cs_assert(flash);
//    at25xe_Spi(flash, 0x4B);
//    for (uint8_t i = 0; i < 4; i++){
//        at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
//
//    }
//    for (uint8_t i = 0; i < 8; i++){
//        flash->UniqID[i] = at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
//
//    }
//    at25xe_cs_unassert(flash);
//}

void at25xe_WriteEnable(at25xe_t *flash) {
    at25xe_cs_assert(flash);
    at25xe_Spi(flash, 0x06);
    at25xe_cs_unassert(flash);
    at25xe_Delay(1);
}
void at25xe_WriteDisable(at25xe_t *flash) {
    at25xe_cs_assert(flash);
    at25xe_Spi(flash, 0x04);
    at25xe_cs_unassert(flash);
    at25xe_Delay(1);
}

uint8_t at25xe_ReadStatusRegister(at25xe_t *flash,
        uint8_t SelectStatusRegister_1_2_3) {
    uint8_t status = 0;
    at25xe_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        at25xe_Spi(flash, 0x05);
        status = at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
        flash->StatusRegister1 = status;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        at25xe_Spi(flash, 0x35);
        status = at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
        flash->StatusRegister2 = status;
    } else {
        at25xe_Spi(flash, 0x15);
        status = at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
        flash->StatusRegister3 = status;
    }
    at25xe_cs_unassert(flash);
    return status;
}
void at25xe_WriteStatusRegister(at25xe_t *flash,
        uint8_t SelectStatusRegister_1_2_3, uint8_t Data) {
    at25xe_cs_assert(flash);
    if (SelectStatusRegister_1_2_3 == 1) {
        at25xe_Spi(flash, 0x01);
        flash->StatusRegister1 = Data;
    } else if (SelectStatusRegister_1_2_3 == 2) {
        at25xe_Spi(flash, 0x31);
        flash->StatusRegister2 = Data;
    } else {
        at25xe_Spi(flash, 0x11);
        flash->StatusRegister3 = Data;
    }
    at25xe_Spi(flash, Data);
    at25xe_cs_unassert(flash);
}
void at25xe_WaitForWriteEnd(at25xe_t *flash) {
    at25xe_Delay(1);
    at25xe_cs_assert(flash);
    at25xe_Spi(flash, 0x05);
    do {
        flash->StatusRegister1 = at25xe_Spi(flash, AT25XE_DUMMY_BYTE);
        at25xe_Delay(1);
    } while ((flash->StatusRegister1 & 0x01) == 0x01);
    at25xe_cs_unassert(flash);
}

void at25xe_Begin(at25xe_t *flash,sm_hal_spi_t *driver,sm_hal_io_t *nss_pin){
    flash->driver = driver;
    flash->nss_pin = nss_pin;
    sm_hal_spi_open(driver);
    sm_hal_io_open (nss_pin, SM_HAL_IO_OUTPUT);
}

bool at25xe_Init(at25xe_t *flash) {
    flash->Lock = 1;
    at25xe_Delay(1);
    uint32_t id;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe Init Begin...\r\n");
#endif
    //at25xe_ReadUniqID(flash);
    id = at25xe_ReadID(flash);

#if (_at25xe_DEBUG == 1)
   // printf("at25xe ID:0x%X%X\r\n",(unsigned int)(id>>16),(unsigned int)id);
#endif

    uint32_t myid = (id >> 8) & 0x000000FF;

    switch ((uint8_t)myid) {
    case 0x42:
        flash->ID = AT25XE01;
        flash->BlockCount = 2;
        break;
    case 0x43:
        flash->ID = AT25XE02;
        flash->BlockCount = 4;
        break;
    case 0x44:
        flash->ID = AT25XE04;
        flash->BlockCount = 8;
        break;
    case 0x45:
        flash->ID = AT25XE08;
        flash->BlockCount = 16;
        break;
    case 0x46:
        flash->ID = AT25XE16;
        flash->BlockCount = 32;
        break;
    case 0x47:
        flash->ID = AT25XE32;
        flash->BlockCount = 64;
        break;
    case 0x65:
        flash->ID = AT25XE512;
        flash->BlockCount = 1;
        break;
    default:
#if (_at25xe_DEBUG == 1)
       // printf("at25xe Unknown ID\r\n");
#endif
        flash->Lock = 0;
        return false;
    }
    flash->PageSize = 256;
    flash->SectorSize = 0x1000; // 4k
    flash->SectorCount = flash->BlockCount * 16;
    flash->PageCount = (flash->SectorCount * flash->SectorSize)
            / flash->PageSize;
    flash->BlockSize = flash->SectorSize * 16; //64k
    flash->CapacityInKiloByte = (flash->SectorCount * flash->SectorSize) / 1024;
//    at25xe_ReadUniqID(flash);
    at25xe_ReadStatusRegister(flash, 1);
    at25xe_ReadStatusRegister(flash, 2);
    at25xe_ReadStatusRegister(flash, 3);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe Page Size: %d Bytes\r\n", flash->PageSize);
   // printf("at25xe Page Count: %d\r\n",flash->PageCount);
   // printf("at25xe Sector Size: %d Bytes\r\n", flash->SectorSize);
   // printf("at25xe Sector Count: %d\r\n", flash->SectorCount);
   // printf("at25xe Block Size: %d Bytes\r\n", flash->BlockSize);
   // printf("at25xe Block Count: %d\r\n", flash->BlockCount);
   // printf("at25xe Capacity: %d KiloBytes\r\n", flash->CapacityInKiloByte);
   // printf("at25xe Init Done\r\n");
#endif
    flash->Lock = 0;
    return true;
}

void at25xe_EraseChip(at25xe_t *flash) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
   // printf("at25xe EraseChip Begin...\r\n");
#endif
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);
    at25xe_Spi(flash, 0xC7);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe EraseBlock done after %d ms!\r\n", get_tick_count() - StartTime);
#endif
    at25xe_Delay(10);
    flash->Lock = 0;
}
void at25xe_EraseSector(at25xe_t *flash, uint32_t SectorAddr) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
   // printf("at25xe EraseSector %d Begin...\r\n", SectorAddr);
#endif
    at25xe_WaitForWriteEnd(flash);
    SectorAddr = SectorAddr * flash->SectorSize;
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);
    if (flash->ID >= AT25XE512) {
        at25xe_Spi(flash, 0x21);
        at25xe_Spi(flash, (uint8_t) ((SectorAddr & 0xFF000000) >> 24));
    } else {
        at25xe_Spi(flash, 0x20);
    }
    at25xe_Spi(flash, (SectorAddr & 0xFF0000) >> 16);
    at25xe_Spi(flash, (SectorAddr & 0xFF00) >> 8);
    at25xe_Spi(flash, SectorAddr & 0xFF);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe EraseSector done after %d ms\r\n", get_tick_count() - StartTime);
#endif
    at25xe_Delay(1);
    flash->Lock = 0;
}
void at25xe_EraseBlock(at25xe_t *flash, uint32_t BlockAddr) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe EraseBlock %d Begin...\r\n", BlockAddr);
    at25xe_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    at25xe_WaitForWriteEnd(flash);
    BlockAddr = BlockAddr * flash->SectorSize * 16;
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);
    if (flash->ID >= AT25XE512) {
        at25xe_Spi(flash, 0xDC);
        at25xe_Spi(flash, (uint8_t) ((BlockAddr & 0xFF000000) >> 24));
    } else {
        at25xe_Spi(flash, 0xD8);
    }
    at25xe_Spi(flash, (BlockAddr & 0xFF0000) >> 16);
    at25xe_Spi(flash, (BlockAddr & 0xFF00) >> 8);
    at25xe_Spi(flash, BlockAddr & 0xFF);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe EraseBlock done after %d ms\r\n", get_tick_count() - StartTime);
    at25xe_Delay(100);
#endif
    at25xe_Delay(1);
    flash->Lock = 0;
}

void at25xe_ErasePage(at25xe_t *flash, uint32_t PageAddr) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    at25xe_WaitForWriteEnd(flash);
    PageAddr = PageAddr * flash->PageSize;
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);
//    if (flash->ID >= AT25XE512) {
//        at25xe_Spi(flash, 0x21);
//        at25xe_Spi(flash, (uint8_t) ((PageAddr & 0xFF000000) >> 24));
//    } else {
//        at25xe_Spi(flash, 0x81);
//    }
    at25xe_Spi(flash, 0x81);

    at25xe_Spi(flash, (PageAddr & 0xFF0000) >> 16);
    at25xe_Spi(flash, (PageAddr & 0xFF00) >> 8);
    at25xe_Spi(flash, PageAddr & 0xFF);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
    at25xe_Delay(1);
    flash->Lock = 0;
}

uint32_t at25xe_PageToSector(at25xe_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->SectorSize);
}
uint32_t at25xe_PageToBlock(at25xe_t *flash, uint32_t PageAddress) {
    return ((PageAddress * flash->PageSize) / flash->BlockSize);
}
uint32_t at25xe_SectorToBlock(at25xe_t *flash, uint32_t SectorAddress) {
    return ((SectorAddress * flash->SectorSize) / flash->BlockSize);
}
uint32_t at25xe_SectorToPage(at25xe_t *flash, uint32_t SectorAddress) {
    return (SectorAddress * flash->SectorSize) / flash->PageSize;
}
uint32_t at25xe_BlockToPage(at25xe_t *flash, uint32_t BlockAddress) {
    return (BlockAddress * flash->BlockSize) / flash->PageSize;
}

bool at25xe_IsEmptyPage(at25xe_t *flash, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToCheck_up_to_PageSize == 0))
        NumByteToCheck_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n", Page_Address, OffsetInByte, NumByteToCheck_up_to_PageSize);
    at25xe_Delay(100);
    //uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;

    for (i = OffsetInByte; i < flash->PageSize; i += sizeof(pBuffer)) {
        at25xe_cs_assert(flash);
        WorkAddress = (i + Page_Address * flash->PageSize);
        if (flash->ID >= AT25XE512) {
            at25xe_Spi(flash, 0x0C);
            at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            at25xe_Spi(flash, 0x0B);
        }
        at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        at25xe_Spi(flash, WorkAddress & 0xFF);
        at25xe_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
        at25xe_read_bytes(flash, (void*)( pBuffer+j), 1);
        at25xe_cs_assert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->PageSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->PageSize; i++) {
            at25xe_cs_assert(flash);
            WorkAddress = (i + Page_Address * flash->PageSize);
            at25xe_Spi(flash, 0x0B);
            if (flash->ID >= AT25XE512) {
                at25xe_Spi(flash, 0x0C);
                at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                at25xe_Spi(flash, 0x0B);
            }
            at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            at25xe_Spi(flash, WorkAddress & 0xFF);
            at25xe_Spi(flash, 0);
            at25xe_read_bytes(flash, (void*) pBuffer, 1);

            at25xe_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }

    flash->Lock = 0;
    return true;
    NOT_EMPTY: flash->Lock = 0;
    return false;
}
bool at25xe_IsEmptySector(at25xe_t *flash, uint32_t Sector_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_SectorSize > flash->SectorSize)
            || (NumByteToCheck_up_to_SectorSize == 0))
        NumByteToCheck_up_to_SectorSize = flash->SectorSize;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
    at25xe_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->SectorSize; i += sizeof(pBuffer)) {
        at25xe_cs_assert(flash);
        WorkAddress = (i + Sector_Address * flash->SectorSize);
        if (flash->ID >= AT25XE512) {
            at25xe_Spi(flash, 0x0C);
            at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            at25xe_Spi(flash, 0x0B);
        }
        at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        at25xe_Spi(flash, WorkAddress & 0xFF);
        at25xe_Spi(flash, 0);
        for(uint32_t j = 0;j<sizeof(pBuffer);j++)
            at25xe_read_bytes(flash, (void*) (pBuffer+j),1);
        at25xe_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->SectorSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->SectorSize; i++) {
            at25xe_cs_assert(flash);
            WorkAddress = (i + Sector_Address * flash->SectorSize);
            if (flash->ID >= AT25XE512) {
                at25xe_Spi(flash, 0x0C);
                at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                at25xe_Spi(flash, 0x0B);
            }
            at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            at25xe_Spi(flash, WorkAddress & 0xFF);
            at25xe_Spi(flash, 0);
            at25xe_read_bytes(flash, (void*) pBuffer, 1);
            at25xe_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckSector is Empty in %d ms\r\n", get_tick_count() - StartTime);
    at25xe_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckSector is Not Empty in %d ms\r\n", get_tick_count() - StartTime);
    at25xe_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}
bool at25xe_IsEmptyBlock(at25xe_t *flash, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    if ((NumByteToCheck_up_to_BlockSize > flash->BlockSize)
            || (NumByteToCheck_up_to_BlockSize == 0))
        NumByteToCheck_up_to_BlockSize = flash->BlockSize;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
    at25xe_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    uint8_t pBuffer[32];
    uint32_t WorkAddress;
    uint32_t i;
    for (i = OffsetInByte; i < flash->BlockSize; i += sizeof(pBuffer)) {
        at25xe_cs_assert(flash);
        WorkAddress = (i + Block_Address * flash->BlockSize);

        if (flash->ID >= AT25XE512) {
            at25xe_Spi(flash, 0x0C);
            at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
        } else {
            at25xe_Spi(flash, 0x0B);
        }
        at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
        at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
        at25xe_Spi(flash, WorkAddress & 0xFF);
        at25xe_Spi(flash, 0);
        at25xe_read_bytes(flash,pBuffer,sizeof(pBuffer));
        at25xe_cs_unassert(flash);
        for (uint8_t x = 0; x < sizeof(pBuffer); x++) {
            if (pBuffer[x] != 0xFF)
                goto NOT_EMPTY;
        }
    }
    if ((flash->BlockSize + OffsetInByte) % sizeof(pBuffer) != 0) {
        i -= sizeof(pBuffer);
        for (; i < flash->BlockSize; i++) {
            at25xe_cs_assert(flash);
            WorkAddress = (i + Block_Address * flash->BlockSize);

            if (flash->ID >= AT25XE512) {
                at25xe_Spi(flash, 0x0C);
                at25xe_Spi(flash, (uint8_t) ((WorkAddress & 0xFF000000) >> 24));
            } else {
                at25xe_Spi(flash, 0x0B);
            }
            at25xe_Spi(flash, (WorkAddress & 0xFF0000) >> 16);
            at25xe_Spi(flash, (WorkAddress & 0xFF00) >> 8);
            at25xe_Spi(flash, WorkAddress & 0xFF);
            at25xe_Spi(flash, 0);
            at25xe_read_bytes(flash,pBuffer, 1);
            at25xe_cs_unassert(flash);
            if (pBuffer[0] != 0xFF)
                goto NOT_EMPTY;
        }
    }
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckBlock is Empty in %d ms\r\n", get_tick_count() - StartTime);
    at25xe_Delay(100);
#endif
    flash->Lock = 0;
    return true;
    NOT_EMPTY:
#if (_at25xe_DEBUG == 1)
   // printf("at25xe CheckBlock is Not Empty in %d ms\r\n", get_tick_count() - StartTime);
    at25xe_Delay(100);
#endif
    flash->Lock = 0;
    return false;
}

void at25xe_WriteByte(at25xe_t *flash, uint8_t pBuffer,
        uint32_t WriteAddr_inBytes) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
   // printf("at25xe WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
    at25xe_WaitForWriteEnd(flash);
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);

    if (flash->ID >= AT25XE512) {
        at25xe_Spi(flash, 0x12);
        at25xe_Spi(flash, (uint8_t) ((WriteAddr_inBytes & 0xFF000000) >> 24));
    } else {
        at25xe_Spi(flash, 0x02);
    }
    at25xe_Spi(flash, (WriteAddr_inBytes & 0xFF0000) >> 16);
    at25xe_Spi(flash, (WriteAddr_inBytes & 0xFF00) >> 8);
    at25xe_Spi(flash, WriteAddr_inBytes & 0xFF);
    at25xe_Spi(flash, pBuffer);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe WriteByte done after %d ms\r\n", get_tick_count() - StartTime);
#endif
    flash->Lock = 0;
}
void at25xe_WritePage(at25xe_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > flash->PageSize)
            || (NumByteToWrite_up_to_PageSize == 0))
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
    if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > flash->PageSize)
        NumByteToWrite_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToWrite_up_to_PageSize);
    at25xe_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    at25xe_WaitForWriteEnd(flash);
    at25xe_WriteEnable(flash);
    at25xe_cs_assert(flash);
    Page_Address = (Page_Address * flash->PageSize) + OffsetInByte;
    if (flash->ID >= AT25XE512) {
        at25xe_Spi(flash, 0x12);
        at25xe_Spi(flash, (uint8_t) ((Page_Address & 0xFF000000) >> 24));
    } else {
        at25xe_Spi(flash, 0x02);
    }
    at25xe_Spi(flash, (Page_Address & 0xFF0000) >> 16);
    at25xe_Spi(flash, (Page_Address & 0xFF00) >> 8);
    at25xe_Spi(flash, Page_Address & 0xFF);
    at25xe_write_bytes(flash,pBuffer,NumByteToWrite_up_to_PageSize);
    at25xe_cs_unassert(flash);
    at25xe_WaitForWriteEnd(flash);
#if (_at25xe_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToWrite_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
           // printf("\r\n");
            at25xe_Delay(10);
        }
       // printf("0x%02X,", pBuffer[i]);
    }
   // printf("\r\n");
   // printf("at25xe WritePage done after %d ms\r\n", StartTime);
    at25xe_Delay(100);
#endif
    at25xe_Delay(1);
    flash->Lock = 0;
}
void at25xe_WriteSector(at25xe_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_SectorSize) {
    if ((NumByteToWrite_up_to_SectorSize > flash->SectorSize)
            || (NumByteToWrite_up_to_SectorSize == 0))
        NumByteToWrite_up_to_SectorSize = flash->SectorSize;
#if (_at25xe_DEBUG == 1)
   // printf("+++at25xe WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
    at25xe_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize) {
#if (_at25xe_DEBUG == 1)
       // printf("---at25xe WriteSector Faild!\r\n");
        at25xe_Delay(100);
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
    StartPage = at25xe_SectorToPage(flash, Sector_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        at25xe_WritePage(flash, pBuffer, StartPage, LocalOffset,(uint32_t) BytesToWrite);
        StartPage++;
        BytesToWrite -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_at25xe_DEBUG == 1)
   // printf("---at25xe WriteSector Done\r\n");
    at25xe_Delay(100);
#endif
}
void at25xe_WriteBlock(at25xe_t *flash, uint8_t *pBuffer,
        uint32_t Block_Address, uint32_t OffsetInByte,
        uint32_t NumByteToWrite_up_to_BlockSize) {
    if ((NumByteToWrite_up_to_BlockSize > flash->BlockSize)
            || (NumByteToWrite_up_to_BlockSize == 0))
        NumByteToWrite_up_to_BlockSize = flash->BlockSize;
#if (_at25xe_DEBUG == 1)
   // printf("+++at25xe WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToWrite_up_to_BlockSize);
    at25xe_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize) {
#if (_at25xe_DEBUG == 1)
       // printf("---at25xe WriteBlock Faild!\r\n");
        at25xe_Delay(100);
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
    StartPage = at25xe_BlockToPage(flash,Block_Address)
            + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do {
        at25xe_WritePage(flash,pBuffer, StartPage, LocalOffset, (uint32_t)BytesToWrite);
        StartPage++;
        BytesToWrite -=(int32_t) flash->PageSize -(int32_t) LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToWrite > 0);
#if (_at25xe_DEBUG == 1)
   // printf("---at25xe WriteBlock Done\r\n");
    at25xe_Delay(100);
#endif
}

void at25xe_ReadByte(at25xe_t *flash, uint8_t *pBuffer, uint32_t Bytes_Address) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
   // printf("at25xe ReadByte at address %d begin...\r\n", Bytes_Address);
#endif
    at25xe_cs_assert(flash);

    if (flash->ID >= AT25XE512)
    {
        at25xe_Spi(flash,0x0C);
        at25xe_Spi(flash,(uint8_t) ((Bytes_Address & 0xFF000000) >> 24));
    }
    else
    {
        at25xe_Spi(flash,0x03);
    }
    at25xe_Spi(flash,(Bytes_Address & 0xFF0000) >> 16);
    at25xe_Spi(flash,(Bytes_Address & 0xFF00) >> 8);
    at25xe_Spi(flash,Bytes_Address & 0xFF);
    //at25xe_Spi(flash,0);
    *pBuffer = at25xe_Spi(flash,AT25XE_DUMMY_BYTE);
    at25xe_cs_unassert(flash);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe ReadByte 0x%02X done after %d ms\r\n", *pBuffer, get_tick_count() - StartTime);
#endif
    flash->Lock = 0;
}
void at25xe_ReadBytes(at25xe_t *flash, uint8_t *pBuffer, uint32_t ReadAddr,
        uint32_t NumByteToRead) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
#if (_at25xe_DEBUG == 1)
    uint32_t StartTime = get_tick_count();
   // printf("at25xe ReadBytes at Address:%d, %d Bytes  begin...\r\n", ReadAddr, NumByteToRead);
#endif
    at25xe_cs_assert(flash);

    if (flash->ID >= AT25XE512)
    {
        at25xe_Spi(flash,0x0C);
        at25xe_Spi(flash,(uint8_t) ((ReadAddr & 0xFF000000) >> 24));
    }
    else
    {
        at25xe_Spi(flash,0x03);
    }
    at25xe_Spi(flash,(ReadAddr & 0xFF0000) >> 16);
    at25xe_Spi(flash,(ReadAddr & 0xFF00) >> 8);
    at25xe_Spi(flash,ReadAddr & 0xFF);
    //at25xe_Spi(flash,0);
    at25xe_read_bytes(flash,pBuffer,NumByteToRead);
    at25xe_cs_unassert(flash);
#if (_at25xe_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
           // printf("\r\n");
            at25xe_Delay(10);
        }
       // printf("0x%02X,", pBuffer[i]);
    }
   // printf("\r\n");
   // printf("at25xe ReadBytes done after %d ms\r\n", StartTime);
    at25xe_Delay(100);
#endif
    at25xe_Delay(1);
    flash->Lock = 0;
}
void at25xe_ReadPage(at25xe_t *flash, uint8_t *pBuffer, uint32_t Page_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize) {
    while (flash->Lock == 1)
        at25xe_Delay(1);
    flash->Lock = 1;
    if ((NumByteToRead_up_to_PageSize > flash->PageSize) || (NumByteToRead_up_to_PageSize == 0))
        NumByteToRead_up_to_PageSize = flash->PageSize;
    if ((OffsetInByte + NumByteToRead_up_to_PageSize) > flash->PageSize)
        NumByteToRead_up_to_PageSize = flash->PageSize - OffsetInByte;
#if (_at25xe_DEBUG == 1)
   // printf("at25xe ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Page_Address, OffsetInByte, NumByteToRead_up_to_PageSize);
    at25xe_Delay(100);
    uint32_t StartTime = get_tick_count();
#endif
    Page_Address = Page_Address * flash->PageSize + OffsetInByte;
    at25xe_cs_assert(flash);
    if (flash->ID >= AT25XE512)
    {
        at25xe_Spi(flash,0x0C);
        at25xe_Spi(flash,(uint8_t) ((Page_Address & 0xFF000000) >> 24));
    }
    else
    {
        at25xe_Spi(flash,0x0B);
    }
    at25xe_Spi(flash,(uint8_t)((Page_Address & 0xFF0000) >> 16));
    at25xe_Spi(flash,(uint8_t)((Page_Address & 0xFF00) >> 8));
    at25xe_Spi(flash,(uint8_t)(Page_Address & 0xFF));
    at25xe_Spi(flash,0);
    at25xe_read_bytes(flash,pBuffer,NumByteToRead_up_to_PageSize);
    at25xe_cs_unassert(flash);
#if (_at25xe_DEBUG == 1)
    StartTime = get_tick_count() - StartTime;
    for (uint32_t i = 0; i < NumByteToRead_up_to_PageSize; i++)
    {
        if ((i % 8 == 0) && (i > 2))
        {
           // printf("\r\n");
            at25xe_Delay(10);
        }
       // printf("0x%02X,", pBuffer[i]);
    }
   // printf("\r\n");
   // printf("at25xe ReadPage done after %d ms\r\n", StartTime);
    at25xe_Delay(100);
#endif
    at25xe_Delay(1);
    flash->Lock = 0;
}
void at25xe_ReadSector(at25xe_t *flash, uint8_t *pBuffer,
        uint32_t Sector_Address, uint32_t OffsetInByte,
        uint32_t NumByteToRead_up_to_SectorSize) {
    if ((NumByteToRead_up_to_SectorSize > flash->SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
        NumByteToRead_up_to_SectorSize = flash->SectorSize;
#if (_at25xe_DEBUG == 1)
   // printf("+++at25xe ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
    at25xe_Delay(100);
#endif
    if (OffsetInByte >= flash->SectorSize)
    {
#if (_at25xe_DEBUG == 1)
       // printf("---at25xe ReadSector Faild!\r\n");
        at25xe_Delay(100);
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
    StartPage = at25xe_SectorToPage(flash,Sector_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        at25xe_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_at25xe_DEBUG == 1)
   // printf("---at25xe ReadSector Done\r\n");
    at25xe_Delay(100);
#endif
}
void at25xe_ReadBlock(at25xe_t *flash, uint8_t *pBuffer, uint32_t Block_Address,
        uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize) {
    if ((NumByteToRead_up_to_BlockSize > flash->BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
        NumByteToRead_up_to_BlockSize = flash->BlockSize;
#if (_at25xe_DEBUG == 1)
   // printf("+++at25xe ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Block_Address, OffsetInByte, NumByteToRead_up_to_BlockSize);
    at25xe_Delay(100);
#endif
    if (OffsetInByte >= flash->BlockSize)
    {
#if (_at25xe_DEBUG == 1)
       // printf("at25xe ReadBlock Faild!\r\n");
        at25xe_Delay(100);
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
    StartPage = at25xe_BlockToPage(flash,Block_Address) + (OffsetInByte / flash->PageSize);
    LocalOffset = OffsetInByte % flash->PageSize;
    do
    {
        at25xe_ReadPage(flash,pBuffer, StartPage, LocalOffset,(uint32_t) BytesToRead);
        StartPage++;
        BytesToRead -= (int32_t)flash->PageSize - (int32_t)LocalOffset;
        pBuffer += flash->PageSize - LocalOffset;
        LocalOffset = 0;
    } while (BytesToRead > 0);
#if (_at25xe_DEBUG == 1)
   // printf("at25xe ReadBlock Done\r\n");
    at25xe_Delay(100);
#endif
}
/*Private api*/
void at25xe_read_bytes(at25xe_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_read(flash->driver, buff,(uint16_t) length);
}
void at25xe_write_bytes(at25xe_t *flash,uint8_t *buff,uint32_t length){
    sm_hal_spi_write(flash->driver, buff,(uint16_t) length);
}
void at25xe_writeread(at25xe_t *flash,const uint8_t*src, uint8_t *dest,uint32_t length){
    sm_hal_spi_write_read(flash->driver,(uint8_t*) src, dest,(uint16_t) length);
}
void at25xe_cs_assert(at25xe_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 0);
}
void at25xe_cs_unassert(at25xe_t *flash){
    sm_hal_io_set_value(flash->nss_pin, 1);
}

void at25xe_read(at25xe_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        at25xe_ReadSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
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
void at25xe_write(at25xe_t *flash,uint32_t addr, uint8_t *buff, uint32_t len){
    uint32_t addr_begin = addr;
    uint32_t pageremain;
    pageremain = 4096 - addr_begin % 4096;
    if (len <= pageremain)
        pageremain = len;
    while (1)
    {
        at25xe_WriteSector (flash, buff, addr_begin / 4096, addr_begin % 4096, pageremain);
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
