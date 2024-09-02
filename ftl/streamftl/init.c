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

#include "debug.h"

#include "streamftl_internal.h"

static VOID _InitializeStream(VOID);
static VOID _InitializeBlockInfo(VOID);
static VOID _InitializePartition(VOID);
static VOID _InitializeL2PCache(VOID);
static VOID _InitializeHotColdMgmt(VOID);

static VOID _CheckParameter(VOID);

VOID FTL_Initialize(VOID)
{
	StreamFTL_InitGlobal();
	StreamFTL_Initialize();
	StreamFTL_InitializeCount();			// initialize statistics
	VNAND_Initialize();

	StreamFTL_PrintConfiguration(NULL, NULL);

	_CheckParameter();
}

BOOL FTL_Format(VOID)
{
	StreamFTL_Format();
	return TRUE;
}

VOID StreamFTL_Initialize()
{
	ASSERT(g_stGlobal.nPartitionSize >= g_stGlobal.nStreamSize);
	ASSERT(g_stGlobal.nStreamCount >= (g_stGlobal.nLogicalFlashSize / g_stGlobal.nStreamSize));
	ASSERT(FTL_REQUEST_ID_TYPE_COUNT < (1 << FTL_REQUEST_ID_TYPE_BITS));
	ASSERT(GC_MANAGER_MOVE_INFO_COUNT <= (1 << FTL_MOVE_INFO_INDEX_BITS));
	ASSERT(MAX_NUM_OF_NLB <= REQUEST_LPN_COUNT_MAX);

	g_pstStreamFTL = (STREAMFTL_GLOBAL*)MALLOC(sizeof(STREAMFTL_GLOBAL), DUMP_TYPE_STREAMFTL_GLOBAL);
	OSAL_MEMSET(g_pstStreamFTL, 0x00, sizeof(STREAMFTL_GLOBAL));

	ActiveBlock_Initialize();
	_InitializePartition();
	_InitializeBlockInfo();
	_InitializeStream();
	_InitializeL2PCache();
	_InitializeHotColdMgmt();
	BufferPool_Initialize();

	INT32	nCnt;

	nCnt = (g_stGlobal.nStreamPerPartition + 2)*(g_stGlobal.nLPagePerStream);
	if (nCnt < g_stGlobal.nLPagesPerVBlock)
	{
		nCnt = g_stGlobal.nLPagesPerVBlock;
	}

	g_pnLPNsForGC = (INT32 *)MALLOC(sizeof(INT32) * nCnt, DUMP_TYPE_NO_DUMP);
	g_pnVPPNsForGC = (INT32 *)MALLOC(sizeof(INT32) * nCnt, DUMP_TYPE_NO_DUMP);
	g_pnPartitionForGC = (INT32 *)MALLOC(sizeof(INT32) * nCnt, DUMP_TYPE_NO_DUMP);

	g_pstStreamFTL->stGCMgr.eSMergePolicy	= g_stGlobal.eSMergePolicy;
	g_pstStreamFTL->stGCMgr.eCurGCType		= GC_TYPE_NONE;
	g_pstStreamFTL->stGCMgr.nCurMoveIndex	= 0;

	for (int i = 0; i < GC_MANAGER_MOVE_INFO_COUNT; i++)
	{
		GET_GC_MANAGER()->astMoveInfo[i].bFree = TRUE;
	}

	StreamFTL_InitRequestInfo();		// should be get position prior of InitRequestPool
	StreamFTL_InitRequestPool();
}

VOID InitStream(STREAM* pstStream)
{
	memset(pstStream->paBitmap, 0x00, sizeof(UINT32) * g_stGlobal.nLPagePerStream32);

	pstStream->nStartLPN = INVALID_LPN;
	pstStream->nStartVPPN = INVALID_PPN;
	pstStream->nEndVPPN = INVALID_LPN;
	pstStream->nVPC = 0;
	pstStream->nAllocatedBlocks = 0;
	pstStream->bActive = FALSE;
	pstStream->bFree = TRUE;

	// start/end를 제외한 가운데 block
	for (int i = 0; i < BLOCK_PER_STREAM; i++)
	{
		pstStream->pnVBN[i] = INVALID_PBN;
		pstStream->pnEC[i] = INVALID_EC;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//	static function
//
/////////////////////////////////////////////////////////////////////////////////////////////

static VOID _InitializeStream(VOID)
{
	int i, j;

	INT32	nSize = sizeof(STREAM) * g_stGlobal.nStreamCount;
	g_pstStreamFTL->stStreamMgr.pstStreams = (STREAM *)MALLOC(nSize, DUMP_TYPE_STREAM);
	OSAL_MEMSET(g_pstStreamFTL->stStreamMgr.pstStreams, 0x00, sizeof(STREAM) * g_stGlobal.nStreamCount);

	nSize = sizeof(int) * g_stGlobal.nLPagePerStream32 * g_stGlobal.nStreamCount;
	g_pstStreamFTL->stStreamMgr.pstStreamValidBitmap = (INT32*)MALLOC(nSize, DUMP_TYPE_STREAM_VALID_BITMAP);

	nSize = sizeof(int) * (BLOCK_PER_STREAM) * g_stGlobal.nStreamCount;
	g_pstStreamFTL->stStreamMgr.pstStreamBlock = (INT32*)MALLOC(nSize, DUMP_TYPE_STREAM_BLOCK);

	nSize = sizeof(INT8) * (BLOCK_PER_STREAM) * g_stGlobal.nStreamCount;
	g_pstStreamFTL->stStreamMgr.pstStreamEC = (INT8*)MALLOC(nSize, DUMP_TYPE_STREAM_EC);

	INIT_LIST_HEAD(&g_pstStreamFTL->stStreamMgr.dlFreeStreamPool);

	for (i = 0; i < g_stGlobal.nStreamCount; i++)
	{
		GET_STREAM(i)->paBitmap = &g_pstStreamFTL->stStreamMgr.pstStreamValidBitmap[i * g_stGlobal.nLPagePerStream32];
		OSAL_MEMSET(GET_STREAM(i)->paBitmap, 0x00, sizeof(int) * g_stGlobal.nLPagePerStream32);

		GET_STREAM(i)->nStartLPN = INVALID_LPN;
		GET_STREAM(i)->nStartVPPN = INVALID_PPN;
		GET_STREAM(i)->nEndVPPN = INVALID_PPN;

		GET_STREAM(i)->pnVBN	= &g_pstStreamFTL->stStreamMgr.pstStreamBlock[i * BLOCK_PER_STREAM];
		GET_STREAM(i)->pnEC		= &g_pstStreamFTL->stStreamMgr.pstStreamEC[i * BLOCK_PER_STREAM];

		INIT_LIST_HEAD(&GET_STREAM(i)->dlList);

		GET_STREAM(i)->nStreamIndex = i;
		GET_STREAM(i)->bFree = TRUE;

		// mid block (if the partition can allocate more than 2 blocks
		for (j = 0; j < BLOCK_PER_STREAM; j++)
		{
			GET_STREAM(i)->pnVBN[j]	= INVALID_PBN;
			GET_STREAM(i)->pnEC[j]	= INVALID_EC;
		}

		list_add(&GET_STREAM(i)->dlList, &g_pstStreamFTL->stStreamMgr.dlFreeStreamPool);
	}

	g_pstStreamFTL->stStreamMgr.nFreeStreamCount = g_stGlobal.nStreamCount;
}

static VOID _InitializeBlockInfo(VOID)
{
	INIT_LIST_HEAD(&g_pstStreamFTL->stBlockMgr.dlAllocatedBlocks);
	INIT_LIST_HEAD(&g_pstStreamFTL->stBlockMgr.dlFreeBlocks);

	g_pstStreamFTL->stBlockMgr.pstBIT = (BLOCK_INFO *)MALLOC(sizeof(BLOCK_INFO) * g_stGlobal.nVBlockCount, DUMP_TYPE_BIT);
	memset(g_pstStreamFTL->stBlockMgr.pstBIT, 0x00, sizeof(BLOCK_INFO) * g_stGlobal.nVBlockCount);
	
	g_pstStreamFTL->stBlockMgr.nFreeBlockCount = 0;

	for (int i = 0; i < g_stGlobal.nVBlockCount; i++)
	{
		GET_BLOCK_INFO(i)->nBlockNo = i;
		INIT_LIST_HEAD(&GET_BLOCK_INFO(i)->dlFreeOrAllocated);

		if (VNAND_IsBad(i) == FALSE)
		{
			StreamFTL_ReleaseBlock(i);
		}
		else
		{
			GET_BLOCK_INFO(i)->bBad = TRUE;
			GET_BLOCK_INFO(i)->bFree = TRUE;
		}
	}
}

static VOID _InitializePartition(VOID)
{
	g_pstStreamFTL->stPartitionMgr.pstPartitions = (PARTITION *)MALLOC(sizeof(PARTITION) * g_stGlobal.nPartitionCount, DUMP_TYPE_PARTITION);

	memset(g_pstStreamFTL->stPartitionMgr.pstPartitions, 0x00, sizeof(PARTITION) * g_stGlobal.nPartitionCount);

	for (int i = 0; i < g_stGlobal.nPartitionCount; i++)
	{
		INIT_LIST_HEAD(&GET_PARTITION(i)->dlStream);
		INIT_LIST_HEAD(&GET_PARTITION(i)->dlVictimPartition);

		GET_PARTITION(i)->nPartitionNo = i;

#if (SUPPORT_L2P_CACHE == 1)
		GET_PARTITION(i)->m_nL2PCacheIndex = INVALID_INDEX;
#endif

#if (SUPPORT_HOTCOLD == 1)
		GET_PARTITION(i)->m_nOverwriteRatioIndexPrev = (INT8)INVALID_INDEX;
#endif
	}

	INIT_LIST_HEAD(&g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead);

	ASSERT(g_stGlobal.nMaxStreamPerPartition < pow((sizeof(GET_PARTITION(0)->nNumStream) * 7), 2));		// 7: unsigned INT8

#if (SUPPORT_HOTCOLD == 1)
	ASSERT(STREAM_FTL_MAX_HOT_RATIO_INDEX < pow(sizeof(GET_PARTITION(0)->m_nOverwriteRatioIndexPrev) * 7, 2));		// 7: unsigned INT8
#endif
}

VOID InitializeL2PCacheEntry(INT32 nIndex)
{
	DEBUG_ASSERT(nIndex < g_pstStreamFTL->stL2PCache.m_nEntryCount);

	L2P_CACHE_ENTRY*	pstCacheEntry = GET_L2P_CACHE_ENTRY(nIndex);

	pstCacheEntry->m_nPartition = INVALID_PARTITION;

	// mid block (if the partition can allocate more than 2 blocks
	for (INT32 i = 0; i < g_stGlobal.nLPagePerPartition; i++)
	{
		pstCacheEntry->m_pnL2P[i] = NOT_CACHED_PPN;
	}
}

static VOID _InitializeL2PCache(VOID)
{
#if (SUPPORT_L2P_CACHE == 1)

	g_pstStreamFTL->stL2PCache.m_nEntryCount = (INT32)(g_stGlobal.nPartitionCount * g_stGlobal.fL2PCacheRatio);

	if (g_pstStreamFTL->stL2PCache.m_nEntryCount == 0)
	{
		LOG_PRINTF("[DEBUG] L2P Cache Disable \r\n");
		return;
	}

	LOG_PRINTF("[DEBUG] L2P Cache Enable, Ratio: %.3f, Count :%d\r\n", g_stGlobal.fL2PCacheRatio, g_pstStreamFTL->stL2PCache.m_nEntryCount);

	INT32	i;

	INT32	nSize = sizeof(L2P_CACHE_ENTRY) * g_pstStreamFTL->stL2PCache.m_nEntryCount;
	g_pstStreamFTL->stL2PCache.m_pstL2PCacheEntries = (L2P_CACHE_ENTRY*)MALLOC(nSize, DUMP_TYPE_L2PCACHE);
	OSAL_MEMSET(g_pstStreamFTL->stL2PCache.m_pstL2PCacheEntries, 0x00, nSize);

	nSize = g_pstStreamFTL->stL2PCache.m_nEntryCount * sizeof(INT32) * g_stGlobal.nLPagePerPartition;
	g_pstStreamFTL->stL2PCache.m_pnL2P = (INT32*)MALLOC(nSize, DUMP_TYPE_L2PCACHE_PPN);

	nSize = g_pstStreamFTL->stL2PCache.m_nEntryCount * sizeof(INT32) * g_stGlobal.nLPagePerPartition;
	g_pstStreamFTL->stL2PCache.m_pnStream = (INT32*)MALLOC(nSize, DUMP_TYPE_L2PCACHE_STREAM);

	L2P_CACHE_ENTRY*	pstCacheEntry;

	INIT_LIST_HEAD(&g_pstStreamFTL->stL2PCache.m_dlFreeEntry);
	INIT_LIST_HEAD(&g_pstStreamFTL->stL2PCache.m_dlUsedLRU);

	for (i = 0; i < g_pstStreamFTL->stL2PCache.m_nEntryCount; i++)
	{
		pstCacheEntry = GET_L2P_CACHE_ENTRY(i);
		pstCacheEntry->m_pnL2P		= &g_pstStreamFTL->stL2PCache.m_pnL2P[i * g_stGlobal.nLPagePerPartition];
		pstCacheEntry->m_pnStream	= &g_pstStreamFTL->stL2PCache.m_pnStream[i * g_stGlobal.nLPagePerPartition];

		InitializeL2PCacheEntry(i);

		INIT_LIST_HEAD(&pstCacheEntry->m_dlList);

		// add to free list
		list_add(&pstCacheEntry->m_dlList, &g_pstStreamFTL->stL2PCache.m_dlFreeEntry);

		DEBUG_ASSERT(INDEX_OF_L2P_CACHE_ENTRY(pstCacheEntry) == i);
	}

#endif	// end of #if (SUPPORT_L2P_CACHE == 1)
}

static VOID _InitializeHotColdMgmt(VOID)
{
	if (g_stGlobal.bEnableHotColdMgmt == FALSE)
	{
		g_pstStreamFTL->stHotCold.m_bEnable = FALSE;
		LOG_PRINTF("[DEBUG] HotCold Disable\r\n");
		return;
	}

#if (SUPPORT_HOTCOLD == 1)
	OSAL_MEMSET(&g_pstStreamFTL->stHotCold, 0x00, sizeof(HOTCOLD_MANAGER));

	g_pstStreamFTL->stHotCold.m_bEnable = TRUE;
	g_pstStreamFTL->stHotCold.fHotPartitionRatio = g_stGlobal.fHotPartitionRatio;
	g_pstStreamFTL->stHotCold.fVictimActiveBlockVictimRange = g_stGlobal.fVictimActiveBlockVictimRange;

	g_pstStreamFTL->stHotCold.m_nCurHotIndex = (INT16)INVALID_INDEX;

	LOG_PRINTF("[DEBUG] HotCold Enable, HotOverWriteRatio: %.2f, VictimActiveBlockRatio:%.2f\r\n",
			g_pstStreamFTL->stHotCold.fHotPartitionRatio, g_pstStreamFTL->stHotCold.fVictimActiveBlockVictimRange);
#else
	ASSERT(0);	// Feature not eabled
#endif
	return;
}

static VOID _CheckParameter(VOID)
{
	// Check Parameters
	ASSERT(USER_CHANNELS <= (1 << CHANNEL_BITS));
	ASSERT(USER_WAYS <= (1 << NUM_BIT_WAY));
	ASSERT(TOTAL_BLOCKS_PER_LUN <= (1 << NUM_BIT_BLOCK));
	ASSERT(PAGES_PER_BLOCK <= (1 << NUM_BIT_PPAGE));

	ASSERT(g_stGlobal.nStreamSize > 0);
	ASSERT(g_stGlobal.nPartitionSize > 0);
	ASSERT(g_stGlobal.nPartitionSize >= g_stGlobal.nStreamSize);
	ASSERT(g_stGlobal.nStreamRatio > 0);

	ASSERT(g_stGlobal.nActiveBlockCount > 0);

	ASSERT(LOGICAL_PAGE_SIZE > 0);
	ASSERT(g_stGlobal.nVBlockSize > 0);
	ASSERT(g_stGlobal.nVBlockSize > LOGICAL_PAGE_SIZE);

	ASSERT(g_stGlobal.nLogicalFlashSize > 0);
	ASSERT(g_stGlobal.nOverprovisionSize > 0);

	ASSERT(g_stGlobal.nStreamSize <= g_stGlobal.nVBlockSize);

	return;
}

