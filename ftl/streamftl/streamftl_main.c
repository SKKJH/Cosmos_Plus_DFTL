/*******************************************************
*
* Copyright (C) 2018-2019 
* Embedde Software Laboratory(ESLab), SUNG KYUN KWAN UNIVERSITY
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

#ifdef WIN32
	#include <stdio.h>
	#include <stdarg.h>
#endif

#include "hil.h"

#include "streamftl_internal.h"

GLOBAL	g_stGlobal;			// StreamFTL Global Information

VOID StreamFTL_PrintConfiguration(char* psExecutableBinary, char* psTraceFile)
{
	// DEBUG
	LOG_PRINTF("[DEBUG] Executable binary: %s \r\n", psExecutableBinary ? psExecutableBinary : "StreamFTL");
	LOG_PRINTF("[DEBUG] Trace: %s \r\n", psTraceFile ? psTraceFile : "None");

	LOG_PRINTF("[DEBUG] CH / WAY / BLOCK / PAGE : %d / %d / %d / %d \r\n", USER_CHANNELS, USER_WAYS, TOTAL_BLOCKS_PER_DIE, PAGES_PER_BLOCK);

	LOG_PRINTF("[DEBUG] g_stGlobal.nStreamCount : %d \r\n", g_stGlobal.nStreamCount);
	LOG_PRINTF("[DEBUG] g_stGlobal.nStreamSize : %d KB\r\n", g_stGlobal.nStreamSize/KB);
	LOG_PRINTF("[DEBUG] g_stGlobal.nActiveBlockCount : %d\r\n", g_stGlobal.nActiveBlockCount);
	LOG_PRINTF("[DEBUG] PAGE_SIZE : %d KB\r\n", LOGICAL_PAGE_SIZE /KB);
	LOG_PRINTF("[DEBUG] VBLOCK_SIZE : %d KB\r\n", g_stGlobal.nVBlockSize / KB);
	LOG_PRINTF("[DEBUG] LOGICAL_FLASH_SIZE : %d GB\r\n", (INT32)(g_stGlobal.nLogicalFlashSize / GB));
	LOG_PRINTF("[DEBUG] OVERPROVISION : %d GB\r\n", (INT32)(g_stGlobal.nOverprovisionSize / GB));
	LOG_PRINTF("[DEBUG] FLASH_SIZE : %d GB\r\n", (INT32)(g_stGlobal.nPhysicalFlashSize/GB));
	LOG_PRINTF("[DEBUG] TOTAL_VBLOCK_COUNT : %d\r\n", g_stGlobal.nVBlockCount);
	LOG_PRINTF("[DEBUG] VPAGE_PER_VBLOCK : %d\r\n", g_stGlobal.nLPagesPerVBlock);
	LOG_PRINTF("[DEBUG] g_stGlobal.nPartitionSize : %d KB\r\n", g_stGlobal.nPartitionSize/KB);
	LOG_PRINTF("[DEBUG] g_stGlobal.nPartitionCount : %d\r\n", g_stGlobal.nPartitionCount);
	//	LOG_PRINTF("[DEBUG] g_stGlobal.nStreamPerPartition: %d\r\n", g_stGlobal.nStreamPerPartition);

	LOG_PRINTF("\r\n");

	LOG_PRINTF("\r\n");
	LOG_PRINTF("[DEBUG] LIMIT_STREAM_PER_PARTITION: %s, MAX_STREAM_PER_PARTITION: %d\r\n", 
		SUPPORT_LIMIT_STREAM_PER_PARTITION ? "On" : "Off", g_stGlobal.nMaxStreamPerPartition);
	LOG_PRINTF("[DEBUG] SMERGE POLICY : %d\r\n", g_stGlobal.eSMergePolicy);
	LOG_PRINTF("\r\n");
}

VOID StreamFTL_InitializeCount(VOID)
{
	INT32	nSize = g_stGlobal.nPartitionCount * sizeof(INT32);

	if (g_stGlobal.paPartWrite == NULL)
	{
		g_stGlobal.paPartWrite = (INT32*)UTIL_MemAlloc(nSize, DUMP_TYPE_NO_DUMP);
		ASSERT(g_stGlobal.paPartWrite);
	}

	OSAL_MEMSET(g_stGlobal.paPartWrite, 0x00, nSize);

	STAT_Initialize();
}

VOID IncreasePartitionIOCount(INT32 nLPN)
{
	INT32	nPartition = LPN_TO_PARTITION(nLPN);

	g_stGlobal.paPartWrite[nPartition]++;
}

//////////////////////////////////////////////////////////////////
//
//	Initialization
//
//////////////////////////////////////////////////////////////////

VOID StreamFTL_InitGlobal(VOID)
{
	g_stGlobal.eSMergePolicy = STREAM_FTL_SMERGE_VICTIM_POLICY_DEFAULT;

	g_stGlobal.bPrintAnalysisInfo = FALSE;

#if (SUPPORT_HOTCOLD == 1)
	g_stGlobal.fHotPartitionRatio			= (float)STREAM_FTL_HOT_PARTITION_RATIO_DEFAULT;
	g_stGlobal.fVictimActiveBlockVictimRange = (float)STREAM_FTL_HOTCOLD_VICTIM_ACTIVE_BLOCK_LOOKUP_RANGE_DEFAULT;

	if (g_stGlobal.fHotPartitionRatio > 0)
	{
		g_stGlobal.bEnableHotColdMgmt = TRUE;
	}
#endif

	g_stGlobal.nMaxStreamPerPartition = MAX_STREAM_PER_PARTITION_DEFAULT;

	g_stGlobal.nActiveBlockCount = NUMBER_ACTIVE_BLOCK_DEFAULT;
	g_stGlobal.fOverProvisionRatio = (float)OVERPROVISION_RATIO_DEFAULT;

	g_stGlobal.nStreamMergeTh	= STREAM_GC_THRESHOLD_DEFAULT;
	g_stGlobal.nBlockGCTh		= FREE_BLOCK_GC_THRESHOLD_DEFAULT;

	UINT32	nPPagesPerVBlock = VNAND_GetPPagesPerVBlock();

	// Real Physical Size
	g_stGlobal.nPhysicalFlashSize = (INT64)VNAND_GetVBlockCount() * nPPagesPerVBlock * PHYSICAL_PAGE_SIZE;

	g_stGlobal.nVBlockSize			= nPPagesPerVBlock * PHYSICAL_PAGE_SIZE;
	g_stGlobal.nLPagesPerVBlock		= nPPagesPerVBlock * LPN_PER_PHYSICAL_PAGE;
	g_stGlobal.nLPagesPerVBlockBits	= UTIL_GetBitCount(g_stGlobal.nLPagesPerVBlock);
	g_stGlobal.nLPagesPerVBlockMask	= (1 << g_stGlobal.nLPagesPerVBlockBits) - 1;

	g_stGlobal.nStreamSize = DEFAULT_STREAM_SIZE;
	g_stGlobal.nPartitionSize = DEFAULT_PARTITION_SIZE;
	g_stGlobal.nStreamRatio = DEFAULT_STREAM_RATIO;

	g_stGlobal.nOverprovisionSize = (INT64)(g_stGlobal.nPhysicalFlashSize * g_stGlobal.fOverProvisionRatio);
	g_stGlobal.nLogicalFlashSize = g_stGlobal.nPhysicalFlashSize - g_stGlobal.nOverprovisionSize;

#if (SUPPORT_STATIC_DENSITY != 0)
	INT64	nLogicalFlashSize = (INT64)SUPPORT_STATIC_DENSITY * GB;
	if (g_stGlobal.nLogicalFlashSize < nLogicalFlashSize)
	{
		ASSERT(0);
	}

	g_stGlobal.nLogicalFlashSize = nLogicalFlashSize;
#endif

	g_stGlobal.fL2PCacheRatio	= L2P_CACHE_RATIO;

	g_stGlobal.nLPNCount		= (INT32)(g_stGlobal.nLogicalFlashSize / LOGICAL_PAGE_SIZE);
	g_stGlobal.nPartitionCount	= (INT32)CEIL(g_stGlobal.nLogicalFlashSize, g_stGlobal.nPartitionSize);
	g_stGlobal.nStreamCount		= (INT32)(CEIL(g_stGlobal.nLogicalFlashSize, g_stGlobal.nStreamSize) * g_stGlobal.nStreamRatio);

	g_stGlobal.nVBlockCount		= VNAND_GetVBlockCount();

	g_stGlobal.nLPagePerStream	= g_stGlobal.nStreamSize / LOGICAL_PAGE_SIZE;
	g_stGlobal.nLPagePerStream32	= CEIL(g_stGlobal.nLPagePerStream, 32);
	g_stGlobal.nStreamPerPartition	= g_stGlobal.nPartitionSize / g_stGlobal.nStreamSize;
	g_stGlobal.nLPagePerPartition			= g_stGlobal.nPartitionSize / LOGICAL_PAGE_SIZE;

#if (SUPPORT_META_BLOCK == 1)
	g_stGlobal.bEnableMetaBlock = TRUE;
#else
	g_stGlobal.bEnableMetaBlock = FALSE;
#endif

	HIL_SetStorageBlocks((UINT32)g_stGlobal.nLPNCount);
}
