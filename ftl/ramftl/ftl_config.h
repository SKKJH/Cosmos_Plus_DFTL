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
#ifndef __FTL_CONFIG_H__
#define __FTL_CONFIG_H__

#include "nsc_driver.h"
#include "xparameters.h"
#include "nvme.h"

#include "cosmos_types.h"

#define DEBUG_FTL			(0)

#if (DEBUG_FTL == 0)
	#define FTL_DEBUG_PRINTF
#else
	#define FTL_DEBUG_PRINTF	xil_printf
#endif

#define RAMDISK_SIZE_MB		(256)
#define RAMDISK_SIZE_KB		(RAMDISK_SIZE_MB * 1024)
#define RAMDISK_SIZE_BYTE	(RAMDISK_SIZE_KB * 1024)


#define LOGICAL_BLOCK_COUNT		(RAMDISK_SIZE_BYTE / HOST_BLOCK_SIZE)
#define SLICES_PER_SSD			LOGICAL_BLOCK_COUNT
#define MAX_LPN					LOGICAL_BLOCK_COUNT


#define InitFTL()			FTL_Initialize()

// ftl interface
void FTL_Initialize();
void FTL_Run(void);

void FTL_ReadPage(unsigned int nCmdSlotTag, unsigned int nLPN, unsigned int nCount);
void FTL_WritePage(unsigned int nCmdSlotTag, unsigned int nLPN, unsigned int nCount);

#ifndef GREEDY_FTL
	#define SyncAllLowLevelReqDone()				// do nothing, just for GREEDY_FTL_3_0_0
#endif


#endif // #end of __FTL_CONFIG_H__ 
