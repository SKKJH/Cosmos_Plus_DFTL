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

#include "StreamFTL.h"
#include "error.h"
#include "streamftl_map.h"
#include "streamftl_types.h"
#include "dump.h"
#include "streamftl_vnand.h"
#include "util.h"
#include "osal.h"
#include "list.h"

BUFFER_ENTRY* BufferPool_GetEntry(INT32 nEntryIndex)
{
	DEBUG_ASSERT(nEntryIndex < g_pstStreamFTL->stBufferPool.nTotalCount);
	return &g_pstStreamFTL->stBufferPool.pastEntry[nEntryIndex];
}

BOOL BufferPool_IsEmpty(VOID)
{
	return (g_pstStreamFTL->stBufferPool.nFreeCount == 0) ? TRUE : FALSE;
}

INT32 BufferPool_GetFreeCount(VOID)
{
	return g_pstStreamFTL->stBufferPool.nFreeCount;
}

BUFFER_ENTRY* BufferPool_Allocate(VOID)
{
	if (g_pstStreamFTL->stBufferPool.nFreeCount == 0)
	{
		DEBUG_ASSERT(list_empty(&g_pstStreamFTL->stBufferPool.dlFree) == TRUE);
		ASSERT(0);	// never reach here
		return NULL;
	}

	BUFFER_ENTRY*	pstEntry;
	pstEntry = list_first_entry(&g_pstStreamFTL->stBufferPool.dlFree, BUFFER_ENTRY, dlList);
	list_del(&pstEntry->dlList);

	g_pstStreamFTL->stBufferPool.nFreeCount--;

	DEBUG_ASSERT(g_pstStreamFTL->stBufferPool.nFreeCount <= g_pstStreamFTL->stBufferPool.nTotalCount);
	DEBUG_ASSERT(g_pstStreamFTL->stBufferPool.nFreeCount >= 0);

	return pstEntry;
}

VOID BufferPool_Release(BUFFER_ENTRY* pstEntry)
{
	list_add_tail(&pstEntry->dlList, &g_pstStreamFTL->stBufferPool.dlFree);
	g_pstStreamFTL->stBufferPool.nFreeCount++;

	DEBUG_ASSERT(g_pstStreamFTL->stBufferPool.nFreeCount <= g_pstStreamFTL->stBufferPool.nTotalCount);
}

VOID BufferPool_Initialize(VOID)
{
	INT32	nActiveBlockRequirement = NUMBER_ACTIVE_BLOCK_DEFAULT * 2;			// 2 buffers for at each active block
	INT32	nChannelWayInterleaving = USER_CHANNELS * USER_WAYS * 2;			// 1 Programming and 1 buffering at each way

	BUFFER_POOL*		pstBufferPool = &g_pstStreamFTL->stBufferPool;

	pstBufferPool->nTotalCount = MAX(nActiveBlockRequirement, nChannelWayInterleaving);

	// allocate buffer pool
	pstBufferPool->pastEntry = (BUFFER_ENTRY*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, (sizeof(BUFFER_ENTRY) * pstBufferPool->nTotalCount), OSAL_MEMALLOC_FW_ALIGNMENT);

	INIT_LIST_HEAD(&pstBufferPool->dlFree);
	pstBufferPool->nFreeCount = 0;

	BUFFER_ENTRY*		pstEntry;

	// allocate main buffer
	for (int i = 0; i < g_pstStreamFTL->stBufferPool.nTotalCount; i++)
	{
		pstEntry = BufferPool_GetEntry(i);
		pstEntry->pMainBuf = (void*)OSAL_MemAlloc(MEM_TYPE_BUF, BYTES_PER_DATA_REGION_OF_PAGE, BYTES_PER_DATA_REGION_OF_PAGE);
	}

	// allocate spare buffer
	for (int i = 0; i < g_pstStreamFTL->stBufferPool.nTotalCount; i++)
	{
		pstEntry = BufferPool_GetEntry(i);
		pstEntry->pSpareBuf= (void*)OSAL_MemAlloc(MEM_TYPE_BUF, BYTES_PER_SPARE_REGION_OF_PAGE, BYTES_PER_SPARE_REGION_OF_PAGE);
	}

	// add to free list
	for (int i = 0; i < g_pstStreamFTL->stBufferPool.nTotalCount; i++)
	{
		pstEntry = BufferPool_GetEntry(i);
		BufferPool_Release(pstEntry);
	}

	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//	static function
//
///////////////////////////////////////////////////////////////////////////////

