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

#include "math.h"

#include "streamftl_internal.h"

// static functions
static BOOL _IsActivePartition(INT32 nPartition);
INT32 _SelectVictimBlock_Greedy(VOID);
static INT32 _SelectVictimPartition_Greedy(VOID);
static INT32 _SelectVictimPartition_CostBenefit(VOID);

static INT32 _GetL2PFromCache(INT32 nLPN, INT32* pnStream);
static L2P_CACHE_ENTRY* _GetFreeL2PCacheEntry(VOID);
static VOID _L2PCacheProfile(IOTYPE eIOType, BOOL bHit);

static INT32 _HotCold_GetPartitionHotIndex(INT32 nPartition);

static VOID _RemoveBlockFromActiveBlock(const INT32 nPBN);

static INT32 _STREAM_GetVPPN(INT32 nLPN, INT32 nStream);
static INT32 _STREAM_GetLpnLogicalOffset(INT32 nStream, INT32 nLPN);
static INT32 _STREAM_GetPhysicalOffset(INT32 nStream, INT32 nLogicalOffset);

static BOOL _STREAM_CheckBitmap(STREAM* pstStream, INT32 offset);

/*
	get a new partition structure
*/
INT32 AllocateStream(IOTYPE eIOType)
{
	ASSERT( !list_empty(&g_pstStreamFTL->stStreamMgr.dlFreeStreamPool) );

	STREAM *pstStream = list_first_entry(&g_pstStreamFTL->stStreamMgr.dlFreeStreamPool, STREAM, dlList);
	list_del(&pstStream->dlList);
	g_pstStreamFTL->stStreamMgr.nFreeStreamCount--;

	InitStream(pstStream);

	pstStream->bFree = FALSE;
	pstStream->bActive = TRUE;
	
	switch (eIOType)
	{
		case IOTYPE_BLOCK_GC:
			pstStream->bSMerge = TRUE;
			break;

		case IOTYPE_STREAM_MERGE:
			pstStream->bGC = TRUE;
			break;

		default:
			pstStream->bGC = FALSE;
			pstStream->bSMerge = FALSE;
			break;
	}

	return pstStream->nStreamIndex;
}

/*
	link the allocated partition into the cluster
*/
void InsertStreamIntoPartition(int nPartition, int nStream)
{
	list_add(&GET_STREAM(nStream)->dlList, &GET_PARTITION(nPartition)->dlStream);
	GET_PARTITION(nPartition)->nNumStream++;

#ifdef DEBUG_STREAM	// just for test

	if ((nPartition == -1) || (nStream == 195041))
	{
		PRINTF("(Add)Partition->Stream: %d, %d, NANDWrite: %d\n", nPartition, nStream, g_stGlobal.nStatistics[PROFILE_NAND_WRITE]);
	}

	if (g_stGlobal.nStatistics[PROFILE_NAND_WRITE] == -1)
	{
		PRINTF("Check!");
	}

#endif
}

static VOID FreeStream(INT32 nStream)
{
	STREAM *pstStream = GET_STREAM(nStream);

	list_del(&pstStream->dlList);
	list_add(&pstStream->dlList, &g_pstStreamFTL->stStreamMgr.dlFreeStreamPool);
	g_pstStreamFTL->stStreamMgr.nFreeStreamCount++;

#ifdef DEBUG_STREAM	// just for test
	{
		INT32	nPartition = LPN_TO_PARTITION(pstStream->nStartLPN);
		if ((nPartition == -1) || (nStream == 195041))
		{
			PRINTF("(Remove) Partition x Stream: %d, %d, NANDWrite: %d\n", nPartition, nStream, g_stGlobal.nStatistics[PROFILE_NAND_WRITE]);
		}
	}
#endif

	InitStream(pstStream);
}

VOID RemoveStreamFromPartition(INT32 nStream)
{
	STREAM *pstStream = GET_STREAM(nStream);
	int nPartition = PARTITION_ID_FROM_LPN(pstStream->nStartLPN);
	PARTITION *pstPartition = GET_PARTITION(nPartition);

	// pstPartition->nVPC -= pstStream->nVPC;
	DEBUG_ASSERT(pstStream->nVPC == 0);
	pstPartition->nNumStream--;

	DEBUG_ASSERT(pstPartition->nNumStream >= 0);
	DEBUG_ASSERT(pstPartition->nVPC >= 0);
	DEBUG_ASSERT(pstPartition->nVPC <= g_stGlobal.nLPagePerPartition);

	// partition free
	FreeStream(nStream);
}

VOID FreeFullInvalidStream(INT32 nStream)
{
	STAT_IncreaseCount(PROFILE_SMERGE_FULL_INVALID, 1);

	STREAM *pstStream = GET_STREAM(nStream);

	// if partition is in active state, close the partition and refresh the stream
	if (pstStream->bActive == TRUE)
	{
		// (x) Active Stream인 경우에는 close 하지 않으면 stream 해제가 되지 않음
		// OLD Stream인 경우 close 해야함.
		for (int i = 0; i < g_stGlobal.nActiveBlockCount; i++)
		{
			if (GET_ACTIVE_BLOCK(i)->nStreamNo == nStream)
			{
				CloseActiveStream(i);

				// because the stream currently has no active partition
				ActiveBlock_MoveToFreeList(i);
			}
		}
	}

	// remove stream link from associated blocks
	UnlinkStreamFromBIT(nStream);

	// remove partition link from associated stream
	RemoveStreamFromPartition(nStream);

	STAT_IncreaseCount(PROFILE_FULL_INVALID_STREAM, 1);
}

VOID LinkStreamToBIT(INT32 nStream, INT32 nBlock)
{
	BLOCK_INFO *pstBIT = GET_BLOCK_INFO(nBlock);

	DEBUG_ASSERT(pstBIT->nStreamCount < (UINT32)g_stGlobal.nLPagesPerVBlock);		// nStreamCount INT8
	pstBIT->nStreamCount++;
	
	if (pstBIT->nStreamCount > 1 )
	{
		pstBIT->bMultiStreams = TRUE;
		
		// add to the tail of g_dlAllocatedBlocks cuz it is hybrid block
		list_del(&pstBIT->dlFreeOrAllocated);
		list_add_tail(&pstBIT->dlFreeOrAllocated, &g_pstStreamFTL->stBlockMgr.dlAllocatedBlocks);
	}

#ifdef DEBUG_STREAM	// just for test
	if ( (pstBIT->nBlockNo == 35390) || (nStream == 195041))
	{
		PRINTF("\n[%d]Link STREAM %d, PBN: %d, StreamCnt: %d\n", nDebugCnt++, nStream, pstBIT->nBlockNo, pstBIT->nStreamCount);
	}
#endif
}

void UnlinkStreamFromBIT(int nStream)
{
	STREAM *pstStream = GET_STREAM(nStream);

	for (INT32 i = 0; i < pstStream->nAllocatedBlocks; i++)
	{
		INT32	nPBN = pstStream->pnVBN[i];

		DEBUG_ASSERT(nPBN >= 0);

		BLOCK_INFO *pstBIT = GET_BLOCK_INFO(nPBN);

		if (pstStream->pnEC[i] != (INT8)VNAND_GetEC(nPBN))
		{
			// reused block for another stream
			continue;
		}

		DEBUG_ASSERT(pstBIT->nStreamCount > 0);
		pstBIT->nStreamCount--;

#ifdef DEBUG_STREAM	// just for test
		if ((nPBN == 35390))
		{
			PRINTF("\n[%d] Unlink STREAM %d, PBN: %d, StreamCnt: %d\n", nDebugCnt++, nStream, nPBN, pstBIT->nStreamCount);
		}

		if (nDebugCnt == 310)
		{
			PRINTF("CHECK");
		}
#endif

		if (pstBIT->nStreamCount == 1)
		{
			pstBIT->bMultiStreams = FALSE;
		}

		pstStream->pnVBN[i] = INVALID_PBN;
	}
}

static BOOL _STREAM_CheckBitmap(STREAM* pstStream, INT32 nLPNOffset)
{
	// return 1 : o return 0 : x
	int nBitmapIndex = nLPNOffset / UINT32_BITS;
	DEBUG_ASSERT(nBitmapIndex < g_stGlobal.nLPagePerStream32);

	int nOffset = nLPNOffset % UINT32_BITS;

	return (pstStream->paBitmap[nBitmapIndex] & (1 << nOffset)) ? TRUE : FALSE;
}


/*
	STREAM의 logical page bitmap update
*/
void insert_bitmap(int nStream, int nLPNOffsetInStream)
{
	DEBUG_ASSERT(nLPNOffsetInStream < g_stGlobal.nLPagePerStream);

	UINT32 nBitmapIndex		= nLPNOffsetInStream / UINT32_BITS;
	UINT32 nOffset			= nLPNOffsetInStream % UINT32_BITS;		// offset in a bitmap word

	GET_STREAM(nStream)->paBitmap[nBitmapIndex] |= (1 << nOffset);
}

int bitCount_u32_HammingWeight(unsigned int n)
{
	int m1 = 0x55555555; //01 01 01 01...
	int m2 = 0x33333333; //00 11 00 11 ...
	int m3 = 0x0f0f0f0f; //00 00 11 11
	int h1 = 0x01010101; //00 00 00 01

	n = n - ((n >> 1) & m1);
	n = (n & m2) + ((n >> 2) & m2);

	return (((n + (n >> 4)) & m3) * h1) >> 24;
}

int get_offset_fast(unsigned int *anBitmap, int nOffset)
{
	int nWordCount = nOffset / UINT32_BITS;
	int nOffsetInWord = nOffset % UINT32_BITS;
	int nValidBitCount = 0;

	for (int i = 0; i < nWordCount; i++)
	{
		nValidBitCount = nValidBitCount + bitCount_u32_HammingWeight(anBitmap[i]);
	}

	if (nOffsetInWord != 0)
	{
		unsigned int nMask = (1 << nOffsetInWord) - 1;
		unsigned int nRest = anBitmap[nWordCount] & nMask;
		nValidBitCount = nValidBitCount + bitCount_u32_HammingWeight(nRest);
	}

	return nValidBitCount;
}

INT32 StreamFTL_AllocateBlock(VOID)
{
	DEBUG_ASSERT( !list_empty(&g_pstStreamFTL->stBlockMgr.dlFreeBlocks) );
	//DEBUG_ASSERT(list_count(&g_pstStreamFTL->stBlockMgr.dlFreeBlocks) == g_pstStreamFTL->stBlockMgr.nFreeBlockCount);

	BLOCK_INFO *pstBIT = NULL;

	pstBIT = list_first_entry(&g_pstStreamFTL->stBlockMgr.dlFreeBlocks, BLOCK_INFO, dlFreeOrAllocated);
	DEBUG_ASSERT(pstBIT->bFree == TRUE);

	pstBIT->bFree = FALSE;

	list_move_head(&pstBIT->dlFreeOrAllocated, &g_pstStreamFTL->stBlockMgr.dlAllocatedBlocks);

	g_pstStreamFTL->stBlockMgr.nFreeBlockCount--;

#if (SUPPORT_AUTO_ERASE == 0)
	VNAND_Erase(pstBIT->nBlockNo);
#endif

	pstBIT->nInvalidPages = 0;
	pstBIT->nStreamCount = 0;		// STREAM에 여러개의 block이 할당되었지만 stream이 유지되는 경우 nStreamCount는 0보다 클 수 있음
	pstBIT->bMultiStreams = FALSE;
	pstBIT->bUser = FALSE;
	pstBIT->bGC = FALSE;
	pstBIT->bSMerge = FALSE;

	return pstBIT->nBlockNo;
}

/*
	Add to free block list
*/
VOID StreamFTL_ReleaseBlock(INT32 nVBN)
{
	BLOCK_INFO *pstBIT = GET_BLOCK_INFO(nVBN);

	DEBUG_ASSERT(pstBIT->nStreamCount >= 0);
	//DEBUG_ASSERT(pstBIT->bMultiStreams == FALSE);

	pstBIT->bFree = TRUE;

	list_del(&pstBIT->dlFreeOrAllocated);
	list_add_tail(&pstBIT->dlFreeOrAllocated, &g_pstStreamFTL->stBlockMgr.dlFreeBlocks);

	g_pstStreamFTL->stBlockMgr.nFreeBlockCount++;

	_RemoveBlockFromActiveBlock(nVBN);

	//DEBUG_ASSERT(list_count(&g_pstStreamFTL->stBlockMgr.dlFreeBlocks) == g_pstStreamFTL->stBlockMgr.nFreeBlockCount);
}

/*
	
*/
INT32 LPN2PPN(int nLPN, int *pnStream, IOTYPE eIOType)
{
	*pnStream = INVALID_STREAM;

	INT32	nVPPN;
	INT32	nvPPN_Test = NOT_CACHED_PPN;			// just for test, backup PPN
	INT32	nStream_Test = INVALID_STREAM;

	nVPPN = _GetL2PFromCache(nLPN, pnStream);	// Check Cache Entry
	if (nVPPN != NOT_CACHED_PPN)
	{
		// Cache Hit
		_L2PCacheProfile(eIOType, TRUE);

		if (eIOType != IOTYPE_TEST)
		{
			return nVPPN;
		}
		// Test type 인 경우에는 debugging을 위해 CachedL2P와 StreamL2P를 비교하기 위해 저장한다.
		nvPPN_Test = nVPPN;
		nStream_Test = *pnStream;
	}
	else
	{
		_L2PCacheProfile(eIOType, FALSE);
	}

	INT32		nPartition = PARTITION_ID_FROM_LPN(nLPN);
	PARTITION*	pstPartition = GET_PARTITION(nPartition);
	STREAM*		pstStream = NULL;

	list_for_each_entry(STREAM, pstStream, &pstPartition->dlStream, dlList)
	{
		INT32 nStartLPN = pstStream->nStartLPN;
		INT32 nLPNOffset = nLPN - nStartLPN;

		DEBUG_ASSERT(nLPNOffset < g_stGlobal.nLPagePerStream);

		// dyseo, LPN이 큰 경우 PVBbuffer overflow 발생하지 않는가? 
		//	문제 없음, 해당 partition에 관련된 cluster만 확인하기 때문
		if ( _STREAM_CheckBitmap(pstStream, nLPNOffset) == TRUE )
		{
			nVPPN = _STREAM_GetVPPN(nLPN, pstStream->nStreamIndex);
			if (nVPPN == INVALID_PPN)
			{
				// there are no available PPN
				// the block might be reclaimed
				// dyseo, CHECKME, reclaim되면 해당 cluster에 new valid가 있어야 하지 않나? 왜 그냥 다음으로 넘어가지?
				continue;
			}

			DEBUG_ASSERT(VNAND_IsValid(VBN_FROM_VPPN(nVPPN), VPAGE_FROM_VPPN(nVPPN)) == TRUE);

			*pnStream = pstStream->nStreamIndex;
			goto out;
		}
	}

	// there are no valid page for LPN
	nVPPN = INVALID_PPN;

out:

	if (nvPPN_Test != NOT_CACHED_PPN)
	{
		DEBUG_ASSERT(nvPPN_Test == nVPPN);
		DEBUG_ASSERT(nStream_Test == *pnStream);
	}

	// ADD TO CACHE
	AddToL2PCache(nLPN, nVPPN, *pnStream);

	return nVPPN;
}

INT32 LPN2Stream(INT32 nLPN)
{
	INT32 nPartition = PARTITION_ID_FROM_LPN(nLPN);

	PARTITION *pstPartition = GET_PARTITION(nPartition);
	STREAM *pstStream = NULL;

	list_for_each_entry(STREAM, pstStream, &pstPartition->dlStream, dlList)
	{
		int nStartLPN = pstStream->nStartLPN;
		int nOffset = nLPN - nStartLPN;

		if (nOffset >= 0)
		{
			if (_STREAM_CheckBitmap(pstStream, nOffset) == TRUE )
			{
				return pstStream->nStreamIndex;
			}
		}
	}

	// there are no valid page for LPN
	return INVALID_STREAM;
}

static INT32 _STREAM_GetVPPN(INT32 nLPN, INT32 nStream)
{
	INT32 nVPPN = INVALID_PPN;

	INT32 nLogicalOffsetFromStream;			// LPN logical offset from start of stream
	INT32 nPhysicalOffsetFromStream;		// LPN Physical offset from start of stream
	INT32 nPhysicalOffsetFrom1stVBlock;		// LPN physical offset from start of VBlock
	INT32 nStreamStartOffset;		// StreamStreamOffset in a VBlock
	INT32 nVPageOffsetFromVBlock;
	INT32 nBlockIndex;

	nLogicalOffsetFromStream		= _STREAM_GetLpnLogicalOffset(nStream, nLPN);
	nPhysicalOffsetFromStream		= _STREAM_GetPhysicalOffset(nStream, nLogicalOffsetFromStream);

	nStreamStartOffset = GET_STREAM(nStream)->nStartVPPN & g_stGlobal.nLPagesPerVBlockMask;

	nPhysicalOffsetFrom1stVBlock = nStreamStartOffset + nPhysicalOffsetFromStream;

	if (nPhysicalOffsetFrom1stVBlock >= g_stGlobal.nLPagesPerVBlock)
	{
		// this LPN is at the 2nd block
		nVPageOffsetFromVBlock = nPhysicalOffsetFrom1stVBlock & g_stGlobal.nLPagesPerVBlockMask;
		nBlockIndex = nPhysicalOffsetFrom1stVBlock >> g_stGlobal.nLPagesPerVBlockBits;		// start를 제외. 0이면 그 다음 block
	}
	else
	{
		// first block
		nVPageOffsetFromVBlock = nPhysicalOffsetFrom1stVBlock;
		nBlockIndex = 0;
	}

	DEBUG_ASSERT(GET_STREAM(nStream)->pnVBN[nBlockIndex] >= 0);

	nVPPN = GET_VPPN_FROM_VPN_VBN(nVPageOffsetFromVBlock, GET_STREAM(nStream)->pnVBN[nBlockIndex]);

	return nVPPN;
}

/*
	@brief	Get physical page offset from start of stream
	count 1's algorithm
*/
static INT32 _STREAM_GetPhysicalOffset(INT32 nStream, INT32 nLogicalOffset)
{
	return get_offset_fast(GET_STREAM(nStream)->paBitmap, nLogicalOffset);
}

/*
	Close Active STREAM for an active block
*/
VOID CloseActiveStream(INT32 nActiveBlockIndex)
{
	ACTIVE_BLOCK *pstActiveBlock = GET_ACTIVE_BLOCK(nActiveBlockIndex);

	INT32 nStream = pstActiveBlock->nStreamNo;
	STREAM *pstStream = GET_STREAM(nStream);

	DEBUG_ASSERT(pstStream->bActive == TRUE);

	// close the partition
	pstStream->bActive	= FALSE;
	pstStream->bGC		= FALSE;
	pstStream->bSMerge	= FALSE;

	// refresh active partition for the stream
	pstActiveBlock->nStreamNo		= INVALID_STREAM;
	pstActiveBlock->nRecentLPN		= INVALID_LPN;
}


/*
	Cluster를 할당된 partition의 개수에 따라 정렬하여 
*/
void InsertPartitionToVictimList(int nPartition)
{
	PARTITION *pstNewPartition = GET_PARTITION(nPartition);
	PARTITION *pstCurPartition = NULL;

	list_del(&pstNewPartition->dlVictimPartition);

	if (list_empty(&g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead))
	{
		list_add(&pstNewPartition->dlVictimPartition, &g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead);
	}
	else
	{
		list_for_each_entry(PARTITION, pstCurPartition, &g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead, dlVictimPartition)
		{
			if ( pstNewPartition->nNumStream >= pstCurPartition->nNumStream )
			{
				list_add_tail(&pstNewPartition->dlVictimPartition, &pstCurPartition->dlVictimPartition);
				return;
			}
		}

		list_add_tail(&pstNewPartition->dlVictimPartition, &g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead);
	}
}


/*
전체 allocated block 중에서 invalid page가 가장 많은 block 선택
*/
INT32 SelectVictimBlock(VOID)
{
	return _SelectVictimBlock_Greedy();;
}


/*
	전체 allocated block 중에서 invalid page가 가장 많은 block 선택
*/
INT32 _SelectVictimBlock_Greedy(VOID)
{
	BLOCK_INFO *pstBIT = NULL;

	int		nVictimBlock;
	int		nMaxInvalid = 0;

	// lookup all used blocks
	// CHECKME, TBD, 성능 개선 필요.
	list_for_each_entry(BLOCK_INFO, pstBIT, &g_pstStreamFTL->stBlockMgr.dlAllocatedBlocks, dlFreeOrAllocated)
	{
		if (pstBIT->bActive == TRUE)
		{
			continue;
		}

		if (pstBIT->nInvalidPages > nMaxInvalid)
		{
			nMaxInvalid		= pstBIT->nInvalidPages;
			nVictimBlock	= pstBIT->nBlockNo;
		}
	}

	return nVictimBlock;
}

INT32 SelectVictimPartition(VOID)
{
	switch (g_pstStreamFTL->stGCMgr.eSMergePolicy)
	{
	case SMERGE_POLICY_GREEDY:
		return _SelectVictimPartition_Greedy();
		break;

	case SMERGE_POLICY_COST_BENEFIT:
		//not implemented yet
		return _SelectVictimPartition_CostBenefit();
		break;

	default:
		ASSERT(0);		// Never reach here
		break;
	}

	return INVALID_PARTITION;

}


/*
nStream은 write에서만 사용된다. memory 사용량을 줄이기 위해 일단 PPN만 caching 한다.
*/
VOID AddToL2PCache(INT32 nLPN, INT32 nVPPN, INT32 nStream)
{
#if (SUPPORT_L2P_CACHE == 1)
	if (g_pstStreamFTL->stL2PCache.m_nEntryCount == 0)
	{
		// L2P Cache Disabled
		return;
	}

	INT32 nPartition = LPN_TO_PARTITION(nLPN);
	PARTITION*	pstPartition = GET_PARTITION(nPartition);

	L2P_CACHE_ENTRY*		pstCacheEntry;
	if (pstPartition->m_nL2PCacheIndex == INVALID_INDEX)
	{
		pstCacheEntry = _GetFreeL2PCacheEntry();

		INT32	nCacheEntryIndex = INDEX_OF_L2P_CACHE_ENTRY(pstCacheEntry);
		InitializeL2PCacheEntry(nCacheEntryIndex);
		pstCacheEntry->m_nPartition = nPartition;
		pstPartition->m_nL2PCacheIndex = nCacheEntryIndex;
	}
	else
	{
		pstCacheEntry = GET_L2P_CACHE_ENTRY(pstPartition->m_nL2PCacheIndex);
	}

	INT32	nLPNOffset = nLPN - PARTITION_START_LPN(nPartition);

	DEBUG_ASSERT(nLPNOffset >= 0);
	DEBUG_ASSERT(nLPNOffset < g_stGlobal.nLPagePerPartition);

	pstCacheEntry->m_pnL2P[nLPNOffset] = nVPPN;
	pstCacheEntry->m_pnStream[nLPNOffset] = nStream;

	// move LRU head
	list_move_head(&pstCacheEntry->m_dlList, &g_pstStreamFTL->stL2PCache.m_dlUsedLRU);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
//
//	Static fuctions
//
/////////////////////////////////////////////////////////////////////////////////////

/*
	Policy: Partition에 할당된 STREAM의 개수가 많고 valid page가 작은 것 선택
*/
INT32 _SelectVictimPartition_Greedy(VOID)
{
	int nVictimPartition = INVALID_PARTITION;
	int nMinVPC = g_stGlobal.nLPagePerStream + 1;

	if (list_empty(&g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead))
	{
		ASSERT(0);
	}

	PARTITION *pstPartition = NULL;
	int nMaxStreamCount = 0;

	list_for_each_entry(PARTITION, pstPartition, &g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead, dlVictimPartition)
	{
		if( (pstPartition->nNumStream >= nMaxStreamCount)
			&& (pstPartition->nVPC <= nMinVPC) )
		{
			if (_IsActivePartition(pstPartition->nPartitionNo) == TRUE)
			{
				continue;
			}

			nVictimPartition	= pstPartition->nPartitionNo;
			nMaxStreamCount		= pstPartition->nNumStream;
			nMinVPC				= pstPartition->nVPC;
		}
		else
		{
			break;
		}
	}

	DEBUG_ASSERT(nVictimPartition != -1);
	DEBUG_ASSERT(GET_PARTITION(nVictimPartition)->nNumStream > 1);

	return nVictimPartition;
}


/*
	CostBenefit, 
	STREAM당 page copy가 가장 작은 partition을 victim으로 선택한다.
	- Active partition은 제외
*/
static INT32 _SelectVictimPartition_CostBenefit(VOID)
{
	if (list_empty(&g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead))
	{
		ASSERT(0);	// Never reach here
	}

	int nVictimPartition = INVALID_PARTITION;
	int nMinVPCPerStream = g_stGlobal.nLPagePerStream + 1;

	PARTITION *pstPartition = NULL;

	INT32	nCost;		// VPC per STREAM

	list_for_each_entry(PARTITION, pstPartition, &g_pstStreamFTL->stPartitionMgr.dlVictimPartitionHead, dlVictimPartition)
	{
		DEBUG_ASSERT(pstPartition->nNumStream > 0);
		DEBUG_ASSERT(pstPartition->nVPC > 0);

		if (pstPartition->nNumStream <= 1)
		{
			// nothing
			break;
		}

		if (_IsActivePartition(pstPartition->nPartitionNo) == TRUE)
		{
			continue;
		}

		nCost = pstPartition->nVPC / pstPartition->nNumStream;

		if (nCost < nMinVPCPerStream)
		{
			nMinVPCPerStream = nCost;
			nVictimPartition = pstPartition->nPartitionNo;
		}
	}

	DEBUG_ASSERT(nVictimPartition != INVALID_PARTITION);		// active partition만 2개 이상의 stream을 가지고 있으면... 하지만 stream개수가 충분하므로 이렇지는 않을것임.
	DEBUG_ASSERT(GET_PARTITION(nVictimPartition)->nNumStream > 1);

	return nVictimPartition;
}

static BOOL _IsActivePartition(INT32 nPartition)
{
	return (ActiveBlock_GetActiveBlockForPartition(nPartition) != NULL) ? TRUE : FALSE;
}

static L2P_CACHE_ENTRY* _GetFreeL2PCacheEntry(VOID)
{
	L2P_CACHE_ENTRY*	pstCacheEntry;

#if (SUPPORT_L2P_CACHE == 1)
	// check free list
	if (list_empty(&g_pstStreamFTL->stL2PCache.m_dlFreeEntry) == FALSE)
	{
		// get freom free list
		pstCacheEntry = list_first_entry(&g_pstStreamFTL->stL2PCache.m_dlFreeEntry, L2P_CACHE_ENTRY, m_dlList);
	}
	else
	{
		// get from LRU list
		pstCacheEntry = list_last_entry(&g_pstStreamFTL->stL2PCache.m_dlUsedLRU, L2P_CACHE_ENTRY, m_dlList);

		if (pstCacheEntry->m_nPartition != INVALID_PARTITION)
		{
			// Release cache from partition
			PARTITION*	pstPartition = GET_PARTITION(pstCacheEntry->m_nPartition);
			pstPartition->m_nL2PCacheIndex = INVALID_INDEX;
		}
	}

	INT32	nCacheEnteryIndex;
	nCacheEnteryIndex = INDEX_OF_L2P_CACHE_ENTRY(pstCacheEntry);

	InitializeL2PCacheEntry(nCacheEnteryIndex);
#else
	ASSERT(0);
	pstCacheEntry = NULL;
#endif

	return pstCacheEntry;
}

static INT32 _GetL2PFromCache(INT32 nLPN, INT32* pnStream)
{
#if (SUPPORT_L2P_CACHE == 1)

	if (g_pstStreamFTL->stL2PCache.m_nEntryCount == 0)
	{
		// L2P Cache Disabled
		return NOT_CACHED_PPN;
	}

	INT32	nPartition = LPN_TO_PARTITION(nLPN);
	PARTITION*	pstPartition = GET_PARTITION(nPartition);

	if (pstPartition->m_nL2PCacheIndex == INVALID_INDEX)
	{
		return NOT_CACHED_PPN;
	}

	L2P_CACHE_ENTRY* pCacheEntry = GET_L2P_CACHE_ENTRY(pstPartition->m_nL2PCacheIndex);
	INT32	nLPNOffset = nLPN - PARTITION_START_LPN(nPartition);

	DEBUG_ASSERT(nLPNOffset >= 0);
	DEBUG_ASSERT(nLPNOffset < g_stGlobal.nLPagePerPartition);

	if (pCacheEntry->m_pnL2P[nLPNOffset] >= 0)
	{
		*pnStream = pCacheEntry->m_pnStream[nLPNOffset];
	}

	return pCacheEntry->m_pnL2P[nLPNOffset];

#else

	return NOT_CACHED_PPN;

#endif
}

static VOID _L2PCacheProfile(IOTYPE eIOType, BOOL bHit)
{

	switch (eIOType)
	{
		case IOTYPE_HOST:
			bHit ? STAT_IncreaseCount(PROFILE_L2PCACHE_HIT_HOST, 1) : STAT_IncreaseCount(PROFILE_L2PCACHE_MISS_HOST, 1);
			break;
		case IOTYPE_STREAM_MERGE:
			bHit ? STAT_IncreaseCount(PROFILE_L2PCache_Hit_SMERGE, 1) : STAT_IncreaseCount(PROFILE_L2PCache_Miss_SMERGE, 1);
			break;
		case IOTYPE_BLOCK_GC:
			bHit ? STAT_IncreaseCount(PROFILE_L2PCache_Hit_BLOCKGC, 1) : STAT_IncreaseCount(PROFILE_L2PCache_Miss_BLOCKGC, 1);
			break;
		case IOTYPE_TEST:
			// do nothing
			break;
		default:
			ASSERT(0);		// never reach here
			break;
	}

}

/*
	Active Block의 PBN 정보를 초기화 한다.

*/
static VOID _RemoveBlockFromActiveBlock(const INT32 nPBN)
{
	INT32	nCurVBN;
	INT32	nPPN;
	INT32	nCount = 0;		// just for debugging

	ACTIVE_BLOCK*	pstActiveBlock;

	for (int i = 0; i < g_stGlobal.nActiveBlockCount; i++)
	{
		pstActiveBlock = GET_ACTIVE_BLOCK(i);
		nPPN = pstActiveBlock->nRecentVPPN;
		if (nPPN == INVALID_PPN)
		{
			continue;
		}

		nCurVBN = VBN_FROM_VPPN(nPPN);
		if (nPBN == nCurVBN)
		{
			// reset PPN
			pstActiveBlock->nRecentVPPN = INVALID_PPN;
			nCount++;

#ifdef _DEBUG
			break;	// just once, physical block은 1개의 active block에만 할당 가능.
#endif
		}
	}

	DEBUG_ASSERT(nCount < 2);
}

static INT32 _STREAM_GetLpnLogicalOffset(INT32 nStream, INT32 nLPN)
{
	int nLogicalOffset = -1;
	int nStartLPN = GET_STREAM(nStream)->nStartLPN;

	nLogicalOffset = nLPN - nStartLPN;

	DEBUG_ASSERT(nLogicalOffset >= 0);
	DEBUG_ASSERT(nLogicalOffset < g_stGlobal.nLPagePerStream);

	return nLogicalOffset;
}

///////////////////////////////////////////////////////////////////////////////
//
//	HOT COLD MANAGEMENT
//
////////////////////////////////////////////////////////////////////////////////

/*
	Get Overwrite ratio
*/
float StreamFTL_HotCold_GetOverWriteRatio(INT32 nPartition)
{
	PARTITION*	pstPartition;
	float	fOverWriteRatio;

	pstPartition = GET_PARTITION(nPartition);
	if (pstPartition->nVPC == 0)
	{
		return 0;	// Cold
	}

	fOverWriteRatio = (float)pstPartition->nWriteCountAfterGC / pstPartition->nVPC;

	return fOverWriteRatio;
}

/*
	Update HotCold Information
*/
VOID StreamFTL_HotCold_Update(INT32 nPartition, BOOL bOverWrite, IOTYPE eIOType)
{
#if (SUPPORT_HOTCOLD == 1)

	if (g_pstStreamFTL->stHotCold.m_bEnable == FALSE)
	{
		return;
	}

	PARTITION*	pstPartition = GET_PARTITION(nPartition);

	// increase partition write count
	switch (eIOType)
	{
		case IOTYPE_HOST:
			pstPartition->nWriteCountAfterGC++;
			break;

		case IOTYPE_STREAM_MERGE:
		case IOTYPE_BLOCK_GC:
			pstPartition->nWriteCountAfterGC = 0;
			break;

		default:
			// do nothing
			break;
	}

	INT32	nPartCountIndex = _HotCold_GetPartitionHotIndex(nPartition);

	HOTCOLD_MANAGER*	pstHotCold = &g_pstStreamFTL->stHotCold;

	if (pstPartition->m_nOverwriteRatioIndexPrev == INVALID_INDEX)	// First write
	{
		pstHotCold->m_nNumValidParitions++;
	}

	if (nPartCountIndex != pstPartition->m_nOverwriteRatioIndexPrev)
	{
		if (pstPartition->m_nOverwriteRatioIndexPrev != INVALID_INDEX)
		{
			DEBUG_ASSERT(pstHotCold->m_anPartitionCount[pstPartition->m_nOverwriteRatioIndexPrev] > 0);
			pstHotCold->m_anPartitionCount[pstPartition->m_nOverwriteRatioIndexPrev]--;
		}

		pstHotCold->m_anPartitionCount[nPartCountIndex]++;

		pstPartition->m_nOverwriteRatioIndexPrev = nPartCountIndex;
		pstHotCold->m_nCurHotIndex = INVALID_INDEX;
	}
#endif	// end of #if (SUPPORT_HOTCOLD == 1)
}

/*
	HotColdMgmt::m_anPartitionCount[]를 위한 index를 구함.
*/
static INT32 _HotCold_GetPartitionHotIndex(INT32 nPartition)
{
	INT32 nIndex;

	float	fOverWriteRatio = StreamFTL_HotCold_GetOverWriteRatio(nPartition);

	fOverWriteRatio = fOverWriteRatio * 10;		// 0.1 단위로 처리하기 위함.

	nIndex = (INT32)log2(fOverWriteRatio);
	if (nIndex < 0)
	{
		nIndex = 0;
	}
	else if (nIndex >= STREAM_FTL_MAX_HOT_RATIO_INDEX)
	{
		nIndex = STREAM_FTL_MAX_HOT_RATIO_INDEX - 1;
	}

	return nIndex;
}

