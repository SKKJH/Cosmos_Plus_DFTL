/*******************************************************
*
* Copyright (C) 2018-2019 
* Embedded Software Laboratory(ESLab), SUNG KYUN KWAN UNIVERSITY
* 
* This file is part of ESLab's Flash memory firmware
* 
* This source can not be copied and/or distributed without the express
* permission of ESLab
*
* Author: DongYoung Seo (dongyoung.seo@gmail.com)
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#include "ftl_config.h"
#include "osal.h"
#include "debug.h"

#include "list.h"
#include "cosmos_types.h"

#include "host_lld.h"

#include "ramftl.h"
#include "hil.h"
#include "hil_config.h"


char*	g_pRAMDisk;

static void* _GetRamDiskAddrForLPN(unsigned int nLPN);

void FTL_Initialize(void)
{
	g_pRAMDisk = (char*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, RAMDISK_SIZE_BYTE, 0);
	ASSERT(g_pRAMDisk);

	memset(g_pRAMDisk, 0xFF, RAMDISK_SIZE_BYTE);

	HIL_SetStorageBlocks(LOGICAL_BLOCK_COUNT);

	PRINTF("RAM FTL Initialization Done! \r\n");
	PRINTF("Storage Capacity: %d MB \r\n", HIL_GetStorageBlocks() * HOST_BLOCK_SIZE / 1024 / 1024);
	PRINTF("Host block size: %d KB \r\n", HOST_BLOCK_SIZE / 1024);	
}


void FTL_Run(void)
{
	// nothing to do

	return;
}

void FTL_ReadPage(unsigned int nCmdSlotTag, unsigned int nLPN, unsigned int nCount)
{
	unsigned int nBufAddr;

	for (unsigned int i = 0; i < nCount; i++)
	{
		nBufAddr = (unsigned int)_GetRamDiskAddrForLPN(nLPN + i);
		set_auto_tx_dma(nCmdSlotTag, i, nBufAddr, NVME_COMMAND_AUTO_COMPLETION_ON);
	}
}

void FTL_WritePage(unsigned int nCmdSlotTag, unsigned int nLPN, unsigned int nCount)
{
	unsigned int nBufAddr;

	for (unsigned int i = 0; i < nCount; i++)
	{
		nBufAddr = (unsigned int)_GetRamDiskAddrForLPN(nLPN + i);
		set_auto_rx_dma(nCmdSlotTag, i, nBufAddr, NVME_COMMAND_AUTO_COMPLETION_ON);
	}
}

BOOL FTL_Format(void)
{
	// nothing to do

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	STATIC FUNCTIONS
//

static void* _GetRamDiskAddrForLPN(unsigned int nLPN)
{
	return (void*)(g_pRAMDisk + (nLPN * HOST_BLOCK_SIZE));
}

