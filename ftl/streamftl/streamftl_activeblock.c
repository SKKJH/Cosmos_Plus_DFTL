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

#include "cosmos_types.h"

#include "host_lld.h"

#include "ftl_config.h"

#include "error.h"
#include "StreamFTL.h"
#include "streamftl_main.h"
#include "streamftl_map.h"
#include "init.h"
#include "dump.h"
#include "util.h"
#include "debug.h"
#include "osal.h"

#include "streamftl_internal.h"

static BOOL _IsBufferingLPNSuspect(INT32 nLPN);
static VOID _AddBufferingLPNHash(INT32 nLPN);
static ACTIVE_BLOCK* _HotCold_GetLRUActiveBlock(INT32 nPartition);
static BOOL _HotCold_IsHotPartition(INT32 nPartition);
static INT32 _HotCold_GetActiveBlockVictimRange(VOID);
static INT32 _HotCold_GetHotIndex(VOID);

VOID ActiveBlock_Initialize(VOID)
{
	ASSERT(ACTIVE_BLOCK_BUFFERING_COUNT <= (1 << ACTIVE_BLOCK_BUFFERING_INDEX_BITS));

	INIT_LIST_HEAD(&g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks);
	INIT_LIST_HEAD(&g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks);
	g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks = 0;

	INT32 nSize = sizeof(ACTIVE_BLOCK) * g_stGlobal.nActiveBlockCount;

	g_pstStreamFTL->stActiveBlockMgr.pstActiveBlock = (ACTIVE_BLOCK*)MALLOC(nSize, DUMP_TYPE_ACTIVE_BLOCK);
	OSAL_MEMSET(g_pstStreamFTL->stActiveBlockMgr.pstActiveBlock, 0x00, nSize);

	for (int i = 0; i < g_stGlobal.nActiveBlockCount; i++)
	{
		GET_ACTIVE_BLOCK(i)->nStreamNo		= INVALID_STREAM;
		GET_ACTIVE_BLOCK(i)->nRecentLPN		= INVALID_LPN;
		GET_ACTIVE_BLOCK(i)->nRecentVPPN	= INVALID_PPN;

		GET_ACTIVE_BLOCK(i)->nIndex = i;

		for (int nPGMBufferingIndex = 0; nPGMBufferingIndex < ACTIVE_BLOCK_BUFFERING_COUNT; nPGMBufferingIndex++)
		{
			GET_ACTIVE_BLOCK(i)->astBuffering[nPGMBufferingIndex].bFree = TRUE;
		}

		INIT_LIST_HEAD(&GET_ACTIVE_BLOCK(i)->dlUsedOrFree);
		list_add(&GET_ACTIVE_BLOCK(i)->dlUsedOrFree, &g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks);
	}

	// allocate NAND I/O buffer for main & spare area
	for (int nActiveBlockInex = 0; nActiveBlockInex < g_stGlobal.nActiveBlockCount; nActiveBlockInex++)
	{
		for (int nBuffering = 0; nBuffering < ACTIVE_BLOCK_BUFFERING_COUNT; nBuffering++)
		{
			GET_ACTIVE_BLOCK(nActiveBlockInex)->astBuffering[nBuffering].pstBufferEntry = NULL;
		}
	}

	// Initialize GC Active Block, get Head entry
	g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC = list_first_entry(&g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks, ACTIVE_BLOCK, dlUsedOrFree);
	list_del(&g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC->dlUsedOrFree);	// remove from list
	g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks++;

	g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge = list_first_entry(&g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks, ACTIVE_BLOCK, dlUsedOrFree);
	list_del(&g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge->dlUsedOrFree);	// remove from list
	g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks++;
}

VOID ActiveBlock_AddBufferingLPN(ACTIVE_BLOCK* pstActiveBlock, FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, UINT32 nHostCmdSlotTag, INT16 nDMAIndex)
{
	PROGRAM_UNIT*	pstProgram = &pstActiveBlock->astBuffering[pstActiveBlock->nCurProgramBuffering];

	if (pstProgram->nLPNCount == 0)
	{
		DEBUG_ASSERT(pstProgram->bFree == TRUE);
		pstProgram->bFree = FALSE;
		pstProgram->nVPPN = pstActiveBlock->nRecentVPPN;

		// AllocateBuffer
		pstProgram->pstBufferEntry = BufferPool_Allocate();
	}

	pstProgram->astRequestID[pstProgram->nLPNCount] = stReqID;
	pstProgram->anLPN[pstProgram->nLPNCount] = nLPN;

	// temporal copy operation, this will be optimived with DMA memcpy
	void* pBufferingAddr = pstProgram->pstBufferEntry->pMainBuf;
	pBufferingAddr = (void*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * pstProgram->nLPNCount));

	if (pBuf != NULL)
	{
		// bGC | sMerge,
		DEBUG_ASSERT((pstActiveBlock == g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC) || (pstActiveBlock == g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge));

		OSAL_MEMCPY(pBufferingAddr, pBuf, LOGICAL_PAGE_SIZE);
	}
	else
	{
		// host write request, receive data from host 
		set_auto_rx_dma(nHostCmdSlotTag, nDMAIndex, (unsigned int)pBufferingAddr, NVME_COMMAND_AUTO_COMPLETION_ON);
		pstProgram->nDMAReqTail			= g_hostDmaStatus.fifoTail.autoDmaRx;
		pstProgram->nDMAOverFlowCount	= g_hostDmaAssistStatus.autoDmaRxOverFlowCnt;
	}

#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
	UINT32 nSpareAddr = (UINT32)pstProgram->pstBufferEntry->pSpareBuf;
	nSpareAddr = nSpareAddr + (pstProgram->nLPNCount * sizeof(UINT32));
	*((UINT32*)nSpareAddr) = nLPN;
#endif

	pstProgram->nLPNCount++;
	_AddBufferingLPNHash(nLPN);
}

VOID ActiveBlock_RemoveBufferingLPNHash(INT32 nLPN)
{
	INT32	nBucketIndex = GET_BUCKET_INDEX(nLPN);
	DEBUG_ASSERT(g_pstStreamFTL->stActiveBlockMgr.stBufferingLPN.anBufferingLPNCount[nBucketIndex] > 0);
	g_pstStreamFTL->stActiveBlockMgr.stBufferingLPN.anBufferingLPNCount[nBucketIndex]--;
}

BOOL ActiveBlock_ReadFromBuffering(INT32 nLPN, INT32 nVPPN, INT32* pMainBuf, void* pSpareBuf)
{
	if (_IsBufferingLPNSuspect(nLPN) == FALSE)
	{
		return FALSE;
	}

	ACTIVE_BLOCK*	pstActiveBlock;
	PROGRAM_UNIT*	pstProgram;

	list_for_each_entry(ACTIVE_BLOCK, pstActiveBlock, &g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks, dlUsedOrFree)
	{
		// because this is active pool, ActiveBlock should have its LPN
		DEBUG_ASSERT(pstActiveBlock->nRecentLPN != INVALID_LPN);
		for (INT32 i = 0; i < ACTIVE_BLOCK_BUFFERING_COUNT; i++)
		{
			pstProgram = &pstActiveBlock->astBuffering[i];
			if (pstProgram->bFree == TRUE)
			{
				continue;
			}

			for (UINT32 nLPNIndex = 0; nLPNIndex < pstProgram->nLPNCount; nLPNIndex++)
			{
				if (pstProgram->anLPN[nLPNIndex] == nLPN)
				{
					// HIT
					//PRINTF("Read from buffering LPN: %d\n", nLPN);

					INT32 nLPNOffset = LPN_OFFSET_FROM_VPPN(nVPPN);
					VOID* pDataAddr = pMainBuf;
					pDataAddr = (VOID*)((UINT32)pDataAddr + (LOGICAL_PAGE_SIZE * nLPNOffset));

					VOID* pBufferingAddr = pstProgram->pstBufferEntry->pMainBuf;
					pBufferingAddr = (VOID*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * nLPNIndex));

					OSAL_MEMCPY(pDataAddr, pBufferingAddr, LOGICAL_PAGE_SIZE);

					((UINT32*)pSpareBuf)[nLPNOffset] = ((UINT32*)pstProgram->pstBufferEntry->pSpareBuf)[nLPNIndex];
#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
					DEBUG_ASSERT(((UINT32*)pSpareBuf)[nLPNOffset] == nLPN);
#endif
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*
@brief select feasible stream for the write, and return # of stream
@param pbFreeActiveBlock:	Allocated block is a free active block
*/
INT32 ActiveBlock_Select(INT32 nLPN, IOTYPE eIOType, BOOL* pbFreeActiveBlock)
{
	*pbFreeActiveBlock = FALSE;

	ACTIVE_BLOCK *pstActiveBlock = NULL;
	INT32 nPartition = PARTITION_ID_FROM_LPN(nLPN);

	pstActiveBlock = ActiveBlock_GetActiveBlockForPartition(nPartition);		// Get Active block for user write
	if (pstActiveBlock != NULL)
	{
		if ((eIOType == IOTYPE_BLOCK_GC) || (eIOType == IOTYPE_STREAM_MERGE))
		{
			// user data 용 active stream을 close 하고 GC/SMerge Active Block을 사용한다.
			DEBUG_ASSERT(pstActiveBlock->nStreamNo != INVALID_STREAM);

			// close current user active block
			CloseActiveStream(pstActiveBlock->nIndex);

			ActiveBlock_MoveToFreeList(pstActiveBlock->nIndex);

			pstActiveBlock = NULL;
		}
		else
		{
			goto result;
		}
	}

	if (((g_stGlobal.nPrevWrittenLPN + 1) == nLPN) && ((nLPN % g_stGlobal.nLPagePerStream) == 0))
	{
		pstActiveBlock = ActiveBlock_GetActiveBlockForPartition(nPartition - 1);		// Lookup previous active block to use continuous block write
		if (pstActiveBlock != NULL)
		{
			if ((eIOType == IOTYPE_BLOCK_GC) || (eIOType == IOTYPE_STREAM_MERGE))
			{
				// user data 용 active stream을 close 하고 GC/SMerge Active Block을 사용한다.
				DEBUG_ASSERT(pstActiveBlock->nStreamNo != INVALID_STREAM);

				// close current user active block
				CloseActiveStream(pstActiveBlock->nIndex);

				ActiveBlock_MoveToFreeList(pstActiveBlock->nIndex);

				pstActiveBlock = NULL;
			}
			else
			{
				goto result;
			}
		}
	}

	// GC용 active block 할당.
	if (eIOType == IOTYPE_BLOCK_GC)
	{
		// GC Active block은 LRU에 포함하지 않는다.
		return g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC->nIndex;
	}
	else if (eIOType == IOTYPE_STREAM_MERGE)
	{
		return g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge->nIndex;
	}

	// there are no available stream, find plan B
	if ( !list_empty(&g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks) )
	{
		pstActiveBlock = list_first_entry(&g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks, ACTIVE_BLOCK, dlUsedOrFree);
		g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks++;

		DEBUG_ASSERT(g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks <= g_stGlobal.nActiveBlockCount);
		*pbFreeActiveBlock = TRUE;
		goto result;
	}

	if (g_pstStreamFTL->stHotCold.m_bEnable == TRUE)
	{
		pstActiveBlock = _HotCold_GetLRUActiveBlock(nPartition);
	}
	else
	{
		// select LRU stream as a victim
		pstActiveBlock = list_last_entry(&g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks, ACTIVE_BLOCK, dlUsedOrFree);
	}

result:
	list_move_head(&pstActiveBlock->dlUsedOrFree, &g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks);
	return pstActiveBlock->nIndex;
}

/*
check active stream for nLPN
*/
INT32 ActiveBlock_Check(INT32 nLPN)
{
	ACTIVE_BLOCK *pstActiveBlock = NULL;
	int nPartition = PARTITION_ID_FROM_LPN(nLPN);

	pstActiveBlock = ActiveBlock_GetActiveBlockForPartition(nPartition);
	if (pstActiveBlock == NULL)
	{
		return INVALID_STREAM;
	}

	return pstActiveBlock->nIndex;
}

VOID ActiveBlock_MoveToFreeList(INT32 nIndex)
{
	// GC, sMerge active block은 free list에 넣지 않는다.
	if ((nIndex == g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC->nIndex) ||
		(nIndex == g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge->nIndex))
	{
		return;
	}

	ACTIVE_BLOCK *pstActiveBlock = GET_ACTIVE_BLOCK(nIndex);

	list_del(&pstActiveBlock->dlUsedOrFree);
	list_add(&pstActiveBlock->dlUsedOrFree, &g_pstStreamFTL->stActiveBlockMgr.dlFreeActiveBlocks);
	g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks--;

	DEBUG_ASSERT(g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks >= 0);
}

///////////////////////////////////////////////////////////////////////////////
//
//	static functions
//

static VOID _AddBufferingLPNHash(INT32 nLPN)
{
	INT32	nBucketIndex = GET_BUCKET_INDEX(nLPN);
	DEBUG_ASSERT(g_pstStreamFTL->stActiveBlockMgr.stBufferingLPN.anBufferingLPNCount[nBucketIndex] <= UINT16_MAX);
	g_pstStreamFTL->stActiveBlockMgr.stBufferingLPN.anBufferingLPNCount[nBucketIndex]++;
}

static BOOL _IsBufferingLPNSuspect(INT32 nLPN)
{
	INT32	nBucketIndex = GET_BUCKET_INDEX(nLPN);
	if (g_pstStreamFTL->stActiveBlockMgr.stBufferingLPN.anBufferingLPNCount[nBucketIndex] > 0)
	{
		return TRUE;
	}

	return FALSE;
}

/*
return NULL: this partition is not active partition
*/
ACTIVE_BLOCK *ActiveBlock_GetActiveBlockForPartition(INT32 nPartition)
{
	PARTITION*	pstPartition = GET_PARTITION(nPartition);
	if (pstPartition->nNumStream == 0)
	{
		DEBUG_ASSERT(list_empty(&pstPartition->dlStream) == TRUE);
		return NULL;
	}

	STREAM * pstStream = list_first_entry(&pstPartition->dlStream, STREAM, dlList);
	if (pstStream->bActive == FALSE)
	{
		return NULL;
	}

	if ((pstStream->bGC == TRUE) || (pstStream->bSMerge == TRUE))
	{
		return NULL;
	}

	ACTIVE_BLOCK *pstActiveBlock = NULL;

	// find a fitting stream first at the active stream list
	list_for_each_entry(ACTIVE_BLOCK, pstActiveBlock, &g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks, dlUsedOrFree)
	{
		// because this is active pool, ActiveBlock should have its LPN
		DEBUG_ASSERT(pstActiveBlock->nRecentLPN != INVALID_LPN);

		// if there are active stream with same cluster, then use it
		if (PARTITION_ID_FROM_LPN(pstActiveBlock->nRecentLPN) == nPartition)
		{
			DEBUG_ASSERT(GET_STREAM(pstActiveBlock->nStreamNo)->bActive == TRUE);
			return pstActiveBlock;
		}
	}

	ASSERT(0);	// never reach here

	return NULL;
}

static ACTIVE_BLOCK* _HotCold_GetLRUActiveBlock(INT32 nPartition)
{
#if (SUPPORT_HOTCOLD == 1)
	// check hot partition or not
	BOOL bHot = _HotCold_IsHotPartition(nPartition);

	// get victim block, backward tracerse
	struct list_head*	pstHead;
	ACTIVE_BLOCK*	pstCur;
	STREAM*			pstStream;
	PARTITION*		pstPartition;

	INT32			nLookupCount;
	ACTIVE_BLOCK*	pstVictim = NULL;

	pstHead = &g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks;
	nLookupCount = _HotCold_GetActiveBlockVictimRange();

	list_for_each_entry_reverse(ACTIVE_BLOCK, pstCur, pstHead, dlUsedOrFree)
	{
		BOOL	bHotCur;

		pstStream = GET_STREAM(pstCur->nStreamNo);
		pstPartition = GET_PARTITION(PARTITION_ID_FROM_LPN(pstStream->nStartLPN));

		bHotCur = _HotCold_IsHotPartition(pstPartition->nPartitionNo);

		if (bHot == bHotCur)		// same hot/cold type, hot & hot, cold & cold
		{
			pstVictim = pstCur;
			break;
		}

		nLookupCount--;
		if (nLookupCount <= 0)
		{
			break;
		}
	}

	if (pstVictim == NULL)
	{
		// select LRU stream as a victim
		pstVictim = list_last_entry(&g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks, ACTIVE_BLOCK, dlUsedOrFree);
	}

	return pstVictim;
#else
	return NULL;
#endif
}

static BOOL _HotCold_IsHotPartition(INT32 nPartition)
{
	float	fOverWriteRatio;

	fOverWriteRatio = StreamFTL_HotCold_GetOverWriteRatio(nPartition);
	if (fOverWriteRatio == 0)
	{
		return FALSE;	// Cold
	}

	// Get index of hot ratio
	INT32	nHotIndex;
	nHotIndex = _HotCold_GetHotIndex();

	float fOverWriteRatioHot;
	fOverWriteRatioHot = (float)pow(2, nHotIndex) * (float)0.1;

	// 1: initial write, 
	if (fOverWriteRatio >= fOverWriteRatioHot)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
Active block LRU list에서 마지막 victim region에 해당하는 block의 수
*/
static INT32 _HotCold_GetActiveBlockVictimRange(VOID)
{
#if (SUPPORT_HOTCOLD == 1)
	INT32	nLookupCount;
	INT32	nActiveBlockCount;

	nActiveBlockCount = g_pstStreamFTL->stActiveBlockMgr.nUsedActiveBlocks;
	// GC, sMerge active block은 used active block list에 포함되지 않음.
	DEBUG_ASSERT((nActiveBlockCount - 2) == list_count(&g_pstStreamFTL->stActiveBlockMgr.dlUsedActiveBlocks));

	nLookupCount = (INT32)(nActiveBlockCount * g_pstStreamFTL->stHotCold.fVictimActiveBlockVictimRange);

	return nLookupCount;
#else
	return 0;
#endif
}

static INT32 _HotCold_GetHotIndex(VOID)
{
#if (SUPPORT_HOTCOLD == 1)
	float	fSum = 0;
	HOTCOLD_MANAGER*	pstHC = &g_pstStreamFTL->stHotCold;

	if (pstHC->m_nCurHotIndex != INVALID_INDEX)
	{
		return pstHC->m_nCurHotIndex;
	}

	// backward lookup
	INT32	nHotIndex = 0;
	for (nHotIndex = (STREAM_FTL_MAX_HOT_RATIO_INDEX - 1); nHotIndex >= 0; nHotIndex--)
	{
		fSum += (float)pstHC->m_anPartitionCount[nHotIndex] / pstHC->m_nNumValidParitions;
		if (fSum >= pstHC->fHotPartitionRatio)
		{
			break;
		}
	}

	pstHC->m_nCurHotIndex = nHotIndex;

	return nHotIndex;
#else
	return 0;
#endif
}

