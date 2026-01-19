//
// Created by vnbk on 10/04/2024.
//
#include "sm_core_param.h"
#include "sm_logger.h"

#define TAG "sm_core_param"

#define ACTIVE_FLAG		0x55AAF50A
#define UNUSED_FLAG		0x00000000
#define LOW_SECTION		0xFE00	// by block ~ 254KB
#define HIGH_SECTION	0xFF00	// by block ~ 255KB
#define SECTION_SIZE	0x0100	// by block ~ 1KB
#define FREE_ITEM 		0xFF
#define ZERO_ITEM		0x00
#define FREE_BLOCK		0xFFFFFFFF
#define ZERO_BLOCK		0xFFFF0000

#define SECTION_ACTIVE(block) (*((PDWORD)(FlashToMemory(block))) == ACTIVE_FLAG)
#define g_pParamItems PARAM_ITEMS

PPARAMITEM PARAM_ITEMS[MAX_PARAM_ITEM] = { 0 };
INTERNAL BYTE g_nParamCount = 0;
INTERNAL WORD g_nActiveSection = 0;
INTERNAL SYSTEMCALLBACK g_pParamEvents[PARAM_EVENT_COUNT] = { NULL };

INTERNAL PARAMITEM g_pPendingParam[MAX_PARAM_ITEM] = { 0 };
INTERNAL BYTE g_nPendingParamCount = 0;

// Internal functions

PPARAMITEM NewItem(PPARAMITEM pSource);
PPARAMITEM SearchItem(WORD nParam);
VOID Format(WORD nAddr);
WORD SwapSections();
VOID AppendParam(WORD nParam, WORD nValue);
VOID UpdateParamProc(PVOID pParameter);

// API

VOID sm_core_param_init(SYSTEMCALLBACK fnDefineProc)
{
    PARAMITEM pParamBuffer[MAX_PARAM_ITEM] = { 0 };
    PPARAMITEM pItem;
    BYTE nIndex;
    BOOL bFormat = FALSE;

    if (SECTION_ACTIVE(LOW_SECTION))
        g_nActiveSection = LOW_SECTION;
    else if (SECTION_ACTIVE(HIGH_SECTION))
        g_nActiveSection = HIGH_SECTION;
    else {
        Format(LOW_SECTION);
        g_nActiveSection = LOW_SECTION;
        bFormat = TRUE;
    }

    // Define Parameters
    for (nIndex = 0; nIndex < MAX_PARAM_ITEM; nIndex++)
        g_pParamItems[nIndex] = &pParamBuffer[nIndex];

    g_pParamEvents[PARAM_DEFINE_EVENT] = fnDefineProc;
    g_pParamEvents[PARAM_DEFINE_EVENT](NULL);

    g_nParamCount = 0;

    // Create or locate parameters in flash
    if (bFormat)
    {
        WORD nAddr = g_nActiveSection + 1;
        for (nIndex = 0; nIndex < MAX_PARAM_ITEM; nIndex++)
        {
            if (g_pParamItems[nIndex]->nParam != 0)
            {
                WriteFlash(g_pParamItems[nIndex], sizeof(PARAMITEM), nAddr);
                g_pParamItems[nIndex] = (PPARAMITEM)FlashToMemory(nAddr);
                g_nParamCount++;
                nAddr++;
            }
        }
    }
    else
    {
        for (nIndex = 0; nIndex < MAX_PARAM_ITEM; nIndex++)
        {
            if (g_pParamItems[nIndex]->nParam != 0)
            {
                pItem = SearchItem(g_pParamItems[nIndex]->nParam);
                if (pItem == NULL)
                    pItem = NewItem(g_pParamItems[nIndex]);
                g_pParamItems[nIndex] = pItem;
                g_nParamCount++;
            }
        }
    }
    TimerStartShort(1000, UpdateParamProc, NULL);
}

BYTE sm_core_param_count()
{
    return g_nParamCount;
}

BOOL sm_core_param_set_value(WORD nParam, WORD nValue)
{
    BYTE nIndex;
    for (nIndex = 0; nIndex < g_nParamCount; nIndex++)
    {
        if (g_pParamItems[nIndex]->nParam == nParam)
        {
            PARAMITEM stItem = *g_pParamItems[nIndex];
            stItem.nValue = nValue;
            g_pParamItems[nIndex] = NewItem(&stItem);
            if (g_pParamEvents[PARAM_CHANGE_EVENT] != NULL)
            {
                PARAMCHANGEEVENT stEvent = {
                        PARAM_CHANGE_EVENT,
                        nIndex,
                        nParam,
                        nValue };
                g_pParamEvents[PARAM_CHANGE_EVENT](&stEvent);
            }
            if ((nParam >= PARAM_SEND_LOW) && (nParam <= PARAM_SEND_HIGH))    // Nam trong khoang Global param thi gui len
                AppendParam(nParam, nValue);
            return TRUE;
        }
    }
    return FALSE;
}

WORD GetParam(WORD nParam)
{
    BYTE nIndex;
    for (nIndex = 0; nIndex < g_nParamCount; nIndex++)
    {
        if (g_pParamItems[nIndex]->nParam == nParam)
            return g_pParamItems[nIndex]->nValue;
    }
    return 0;
}

VOID Resm_core_param_set_values()
{
    PARAMITEM pParamBuffer[MAX_PARAM_ITEM] = { { 0 } };
    BYTE nIndex;
    WORD nAddr;
    Format(LOW_SECTION);
    g_nActiveSection = LOW_SECTION;

    // Define Parameters
    for (nIndex = 0; nIndex < MAX_PARAM_ITEM; nIndex++)
        g_pParamItems[nIndex] = &pParamBuffer[nIndex];

    if (g_pParamEvents[PARAM_DEFINE_EVENT] != NULL)
        g_pParamEvents[PARAM_DEFINE_EVENT](NULL);

    g_nParamCount = 0;
    nAddr = g_nActiveSection + 1;
    for (nIndex = 0; nIndex < MAX_PARAM_ITEM; nIndex++)
    {
        if (g_pParamItems[nIndex]->nParam != 0)
        {
            WriteFlash(g_pParamItems[nIndex], sizeof(PARAMITEM), nAddr);
            g_pParamItems[nIndex] = (PPARAMITEM)FlashToMemory(nAddr);
            g_nParamCount++;
            nAddr++;
        }
    }
}

VOID RegisterParamCallback(BYTE nEvent, SYSTEMCALLBACK fnCallback)
{
    g_pParamEvents[nEvent] = fnCallback;
}

// Internal function

INTERNAL PPARAMITEM NewItem(PPARAMITEM pSource)
{
WORD nAddr = g_nActiveSection + 1;
PPARAMITEM pItem;
while (nAddr < g_nActiveSection + SECTION_SIZE)
{
pItem = (PPARAMITEM)FlashToMemory(nAddr);
if (pItem->nParam == pSource->nParam)
{
DWORD dwZero = ZERO_BLOCK;
WriteFlash(&dwZero, sizeof(DWORD), nAddr);
}
else if ((*((PDWORD)pItem)) == FREE_BLOCK)
{
WriteFlash(pSource, sizeof(PARAMITEM), nAddr);
return pItem;
}
nAddr++;
}
// No free block found, swap the sections
nAddr = SwapSections();
WriteFlash(pSource, sizeof(PARAMITEM), nAddr);
return (PPARAMITEM)FlashToMemory(nAddr);
}

INTERNAL PPARAMITEM SearchItem(WORD nParam)
{
WORD nAddr = g_nActiveSection + 1;
PPARAMITEM pItem;
while (nAddr < g_nActiveSection + SECTION_SIZE)
{
pItem = (PPARAMITEM)FlashToMemory(nAddr);
if (pItem->nParam == nParam)
return pItem;
nAddr++;
}
return NULL;
}

INTERNAL VOID Format(WORD nAddr)
{
DWORD dwHeader = ACTIVE_FLAG;
// Erase section
EraseFlash(nAddr >> 9, 1);
// Write section header
WriteFlash(&dwHeader, sizeof(DWORD), nAddr);
}

INTERNAL WORD SwapSections()
{
    WORD nReserved;
    BYTE nIndex;
    PARAMITEM stItem;
    WORD nAddr;
    DWORD dwFlag = UNUSED_FLAG;

    if (g_nActiveSection == LOW_SECTION)
    {
        Format(HIGH_SECTION);
        nReserved = HIGH_SECTION;
    }
    else
    {
        Format(LOW_SECTION);
        nReserved = LOW_SECTION;
    }

    nAddr = nReserved + 1;

    for (nIndex = 0; nIndex < g_nParamCount; nIndex++)
    {
        if (g_pParamItems[nIndex]->nParam != ZERO_ITEM)
        {
            stItem = *g_pParamItems[nIndex]; // The item must be read to RAM before writing to flash
            WriteFlash(&stItem, sizeof(PARAMITEM), nAddr);
            g_pParamItems[nIndex] = (PPARAMITEM)FlashToMemory(nAddr);
            nAddr++;
        }
    }

    WriteFlash(&dwFlag, sizeof(DWORD), g_nActiveSection);
    g_nActiveSection = nReserved;
    return nAddr;
}

INTERNAL VOID AppendParam(WORD nParam, WORD nValue)
{
g_pPendingParam[g_nPendingParamCount].nParam = nParam;
g_pPendingParam[g_nPendingParamCount].nValue = nValue;
g_nPendingParamCount++;

if (g_nPendingParamCount == MAX_PARAM_ITEM)
UpdateParamProc(NULL);
}

INTERNAL VOID UpdateParamProc(PVOID pParameter)
{
if (g_nPendingParamCount != 0)
{
// Dong goi ban tin gui qua TCP/IP
// Type: PACKAGE_TYPE_DEVICE_PARAM
// Addr: ROOT
// Data: (PBYTE)g_pPendingRegisters
// Size: g_nPendingRegisterCount

PreparePackage(PACKAGE_TYPE_DEVICE_PARAM,g_nPendingParamCount << 1, g_pPendingParam);
g_nPendingParamCount = 0;
}
TimerStartShort(UPDATE_PARAM_INTERVAL, UpdateParamProc, NULL);
}