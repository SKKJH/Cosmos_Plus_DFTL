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

#include "host_lld.h"
#include "streamftl_internal.h"

#define BLOCK_DEBUG		(0)

#if (BLOCK_DEBUG == 1)
	#define BLOCK_DEBUG_PRINTF	PRINTF
#else
	#define BLOCK_DEBUG_PRINTF(...)	((void)0)
#endif

// static function prototype
static RETURN	_Write(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, IOTYPE eIOType, UINT32 nHostCmdSlotTag, INT16 nDMAIndex);
static inline VOID		_CheckAndGC(IOTYPE eIOType);
static RETURN	_CheckGCAvailable(VOID);

static VOID		_IssueBlockGCRead(VOID);
static RETURN	_StartBlockGC(INT32 nVBN);
static VOID		_StartStreamMerge(INT32 nPartition, INT32 nSkipCount, BOOL bWhileBlockGC);
static VOID		_ProcessStreamMerge(VOID);
static VOID		_ProcessBlockGC(VOID);
static VOID		_ProcessGC(VOID);

static VOID		_ReadCounting(IOTYPE eIOType, INT32 nVPPN);

VOID FTL_Run(void)
{
	FIL_Run();

	StreamFTL_ProcessDoneQ();
	StreamFTL_ProcessWaitQ();

	_ProcessGC();
	return;
}

VOID FTL_ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	// allocate request
	while (g_pstHILRequestInfo->nFreeCount == 0)
	{
		FTL_Run();
	}

	HIL_REQUEST*	pstRequest;
	pstRequest = StreamFTL_AllocateHILRequest();

	pstRequest->nCmd		= NVME_CMD_OPCODE_READ;
	pstRequest->nLPN		= nLPN;
	pstRequest->nStatus		= HIL_REQUEST_READ_WAIT;

	pstRequest->nHostCmdSlotTag		= nCmdSlotTag;
	pstRequest->nLPNCount			= nCount;
	pstRequest->nDoneLPNCount		= 0;

	// add to waitQ
	StreamFTL_AddToRequestWaitQ(g_pstHILRequestInfo, &pstRequest->stCommon);

	FTL_DEBUG_PRINTF("[FTL][ReadPage] CmdSlot/BufIndex:%d/%d, LPN:%d, Buf: 0x%X \r\n",
		pstRequest->stRequestID.nRequestIndex, pstRequest->stRequestID.nRequestBufIndex,
		pstRequest->nLPN, (unsigned int)pstRequest->pMainBuf);

	STAT_IncreaseCount(PROFILE_HOST_READ_REQ, 1);
}

/*
	@brief	Read Page for GC
*/
GC_REQUEST* FTL_ReadPageGC(unsigned int nLPN)
{
	// allocate request
	while (g_pstGCRequestInfo->nFreeCount == 0)
	{
		return NULL;
	}

	GC_REQUEST*	pstRequest;
	pstRequest = StreamFTL_AllocateGCRequest();
	if (pstRequest == NULL)
	{
		// no more free request
		return NULL;
	}

	pstRequest->stRequestID.stGC.nType			= FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ;
	pstRequest->stRequestID.stGC.nRequestIndex	= StreamFTL_GetGCRequestIndex(pstRequest);
	pstRequest->stRequestID.stGC.nMoveInfoIndex = GET_GC_MANAGER()->nCurMoveIndex;

	pstRequest->nLPN = nLPN;

	// add to waitQ
	StreamFTL_AddToRequestWaitQ(g_pstGCRequestInfo, &pstRequest->stCommon);

	FTL_DEBUG_PRINTF("[FTL][ReadPageGC] RequestIndex:%d, LPN:%d, Buf: 0x%X \r\n",
		pstRequest->stRequestID.stGC.nRequestIndex, pstRequest->nLPN, (unsigned int)pstRequest->pMainBuf);

	return pstRequest;
}

VOID
FTL_IOCtl(IOCTL_TYPE eType)
{
	switch (eType)
	{
	case IOCTL_INIT_PROFILE_COUNT:
		StreamFTL_InitializeCount();
		break;

	case IOCTL_PRINT_PROFILE_COUNT:
		STAT_Print();
		break;

	default:
		ASSERT(0);		// unknown type
		break;
	}

	return;
}

/*
	@brief read a VPPN
*/
INT32 StreamFTL_ReadVPPN(FTL_REQUEST_ID stReqID, INT32 nVPPN, INT32* pMainBuf, void* pSpareBuf, IOTYPE eIOType)
{
	ASSERT(nVPPN != INVALID_PPN);
	ASSERT(pMainBuf);
	VNAND_ReadPage(stReqID, nVPPN, pMainBuf, pSpareBuf);

	_ReadCounting(eIOType, nVPPN);

	return nVPPN;
}

/*
	@brief read a LPN 
*/
INT32 StreamFTL_Read(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pMainBuf, void* pSpareBuf, IOTYPE eIOType, BOOL* pbBufferHit)
{
	INT32	nStream;
	INT32	nVPPN;

	nVPPN = LPN2PPN(nLPN, &nStream, eIOType);
	if (nVPPN != INVALID_PPN)
	{
		// LOOKUP BUFFER ENTRY
		BOOL bHit = ActiveBlock_ReadFromBuffering(nLPN, nVPPN, pMainBuf, pSpareBuf);
		if (bHit == TRUE)
		{
			// read done
			*pbBufferHit = TRUE;
			return nVPPN;
		}

		StreamFTL_ReadVPPN(stReqID, nVPPN, pMainBuf, pSpareBuf, eIOType);
	}

	*pbBufferHit = FALSE;

	return nVPPN;
}

/*
	@brief		write an LPN,
				FTL to HIL callback is postponed until NAND program done.
*/
VOID FTL_WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	// allocate request
	while (g_pstHILRequestInfo->nFreeCount == 0)
	{
		FTL_Run();
	}

	HIL_REQUEST*	pstRequest;
	pstRequest = StreamFTL_AllocateHILRequest();

	pstRequest->nCmd				= NVME_CMD_OPCODE_WRITE;
	pstRequest->nLPN				= nLPN;
	pstRequest->nStatus				= HIL_REQUEST_WRITE_WAIT;

	pstRequest->nHostCmdSlotTag		= nCmdSlotTag;
	pstRequest->nLPNCount			= nCount;
	pstRequest->nDoneLPNCount		= 0;

	// add to waitQ
	StreamFTL_AddToRequestWaitQ(g_pstHILRequestInfo, &pstRequest->stCommon);

	FTL_DEBUG_PRINTF("[FTL][WritePage] Read LPN:%d, Buf: 0x%X \r\n",
		pstRequest->nLPN, (unsigned int)pBuf);

	STAT_IncreaseCount(PROFILE_HOST_WRITE_REQ, 1);
}

/*
	@param	pBuf: buffer for write, sizeof(LOGICAL_PAGE_SIZE), bGC & sMerge only
	@param	nHostCmdSlotTag: SQ index
	@param	nDMAIndex: index of DMA, LPN index of Host request
*/
RETURN StreamFTL_Write(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, IOTYPE eIOType, UINT32 nHostCmdSlotTag, INT16 nDMAIndex)
{
	DEBUG_ASSERT(nLPN < g_stGlobal.nLPNCount);

	RETURN nRet;

	_CheckAndGC(eIOType);	// do gc before write

	nRet = _Write(stReqID, nLPN, pBuf, eIOType, nHostCmdSlotTag, nDMAIndex);
	if ( nRet == RETURN_FAIL_MAX_STREAMS_PER_PARTITION )
	{
		DEBUG_ASSERT(SUPPORT_LIMIT_STREAM_PER_PARTITION == 1);
		DEBUG_ASSERT(eIOType == IOTYPE_HOST);

		STAT_IncreaseCount(PROFILE_SMERGE_meet_max_stream, 1);

		int nPartition = PARTITION_ID_FROM_LPN(nLPN);
		_StartStreamMerge(nPartition, STREAM_MERGE_SKIP_STREAM_COUNT, FALSE);
	}

	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
//
//	static functions
//
///////////////////////////////////////////////////////////////////////////////
static BOOL _CheckMaxStreamsPerPartition(int nLPN)
{
#if (SUPPORT_LIMIT_STREAM_PER_PARTITION == 0)
	return FALSE;
#endif

	INT32 nPartition = PARTITION_ID_FROM_LPN(nLPN);
	PARTITION *pstPartition = GET_PARTITION(nPartition);

	BOOL	bNewStreamAllocate = FALSE;

	// Select the Stream
	INT32 nActiveBlock = ActiveBlock_Check(nLPN);
	if (nActiveBlock == INVALID_STREAM)
	{
		bNewStreamAllocate = TRUE;
	}
	else
	{
		ACTIVE_BLOCK *pstActiveBlock = &g_pstStreamFTL->stActiveBlockMgr.pstActiveBlock[nActiveBlock];

		if ((pstActiveBlock->nRecentLPN == INVALID_LPN)		||		// Closed stream
			(nLPN <= pstActiveBlock->nRecentLPN)			||			// backward write
			(PARTITION_ID_FROM_LPN(nLPN) != PARTITION_ID_FROM_LPN(pstActiveBlock->nRecentLPN)))	// Partition이 변경되는 경우 stream 다시 할당.
		{
			bNewStreamAllocate = TRUE;
		}
		else if (IS_VBLOCK_FULL(pstActiveBlock->nRecentVPPN) == TRUE)
		{
			bNewStreamAllocate = TRUE;
		}
	}

	if ( bNewStreamAllocate == TRUE )
	{
		// check count of current stream for partition
		if (pstPartition->nNumStream >= g_stGlobal.nMaxStreamPerPartition)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void _IssueProgram(ACTIVE_BLOCK* pstActiveBlock, PROGRAM_UNIT* pstProgram)
{
	DEBUG_ASSERT(pstProgram->bFree == FALSE);
	DEBUG_ASSERT(pstProgram->bProgramming == FALSE);

	FTL_REQUEST_ID	stProgramReqID;
	stProgramReqID.stProgram.nType = FTL_REQUEST_ID_TYPE_PROGRAM;
	stProgramReqID.stProgram.nActiveBlockIndex = pstActiveBlock->nIndex;
	stProgramReqID.stProgram.nBufferingIndex = pstActiveBlock->nCurProgramBuffering;

	// do write
	VNAND_ProgramPage(stProgramReqID, pstProgram);

	pstProgram->bProgramming = TRUE;
}

static void _CheckAndIssueProgram(ACTIVE_BLOCK* pstActiveBlock, IOTYPE eIOType)
{
	PROGRAM_UNIT*	pstProgram = &pstActiveBlock->astBuffering[pstActiveBlock->nCurProgramBuffering];
	DEBUG_ASSERT(pstProgram->bFree == FALSE);
	DEBUG_ASSERT(pstProgram->bProgramming == FALSE);

	VNAND_ProgramPageSimul(pstActiveBlock, (pstProgram->nLPNCount - 1));

	if (pstProgram->nLPNCount < LPN_PER_PHYSICAL_PAGE)
	{
		return;
	}

	if (eIOType == IOTYPE_HOST)
	{
		// Wait DMA Done
		BOOL bDMADone;
		do
		{
			bDMADone = check_auto_rx_dma_partial_done(pstProgram->nDMAReqTail, pstProgram->nDMAOverFlowCount);
#if defined(WIN32) || (NVME_UNIT_TEST == 1)
			// process virtual DMA done by force
			SYSTEM_Run();
#endif
		} while (bDMADone == FALSE);
	}

	// bGC or sMerge
	_IssueProgram(pstActiveBlock, pstProgram);

	pstActiveBlock->nCurProgramBuffering = INCREASE_IN_RANGE(pstActiveBlock->nCurProgramBuffering, ACTIVE_BLOCK_BUFFERING_COUNT);
}

static void _InvalidateOldVPPN(IOTYPE eIOType, INT32 nLPN, INT32 nOverWritePPN, INT32 nOverWriteStream)
{
	if (eIOType == IOTYPE_HOST)
	{
		STAT_IncreaseCount(PROFILE_HOST_OVERWRITE, 1);
	}

	BLOCK_INFO* pstBIT = GET_BLOCK_INFO(VBN_FROM_VPPN(nOverWritePPN));
	STREAM* pstOldStream = GET_STREAM(nOverWriteStream);

	// error check
	check_OOB_MAP(nLPN, nOverWritePPN);

	// unset page valid bit, NAND invalidate 처리하여 GC시 copy 하지 않도록 함.
	VNAND_Invalidate(nOverWritePPN);

	pstOldStream->nVPC--;

	// invalid g_pstStreams
	if (pstOldStream->nVPC == 0 )		// CHECKME, dyseo, hot spot write에서 0이 될 수 있음.
	{
		FreeFullInvalidStream(nOverWriteStream);
	}

	// BIT
	pstBIT->nInvalidPages++;
	if ( pstBIT->nInvalidPages == g_stGlobal.nLPagesPerVBlock )		// check full free block
	{
		StreamFTL_ReleaseBlock(pstBIT->nBlockNo);
		STAT_IncreaseCount(PROFILE_FULL_INVALID_BLOCK, VNAND_GetPBlocksPerVBlock());
	}
}

static void _WriteCounting(IOTYPE eIOType, INT32 nLPN, BOOL bNewBlockAllocated, BLOCK_INFO* pstBIT)
{
	if ( eIOType == IOTYPE_HOST )
	{
		STAT_IncreaseCount(PROFILE_HOST_WRITE, 1);
		IncreasePartitionIOCount(nLPN);
	}
	else if ( eIOType == IOTYPE_STREAM_MERGE )
	{
		STAT_IncreaseCount(PROFILE_SMERGE_write, 1);
	}
	else
	{
		STAT_IncreaseCount(PROFILE_BGC_WRITE, 1);
	}

	if (bNewBlockAllocated == TRUE)
	{
		if (eIOType == IOTYPE_STREAM_MERGE)
		{
			STAT_IncreaseCount(PROFILE_SMERGE_erase, 1);
		}
		else if (eIOType == IOTYPE_BLOCK_GC)
		{
			STAT_IncreaseCount(PROFILE_BGC_ERASE, 1);
		}

		// just for information
		switch (eIOType)
		{
		case IOTYPE_HOST:
			pstBIT->bUser = TRUE;
			break;

		case IOTYPE_BLOCK_GC:
			pstBIT->bGC = TRUE;
			break;

		case IOTYPE_STREAM_MERGE:
			pstBIT->bSMerge = TRUE;
			break;

		default:
			// do nothing
			break;
		}
	}
}

static void _UpdateMapping(INT32 nLPN, ACTIVE_BLOCK* pstActiveBlock, INT32 nStream, STREAM* pstStream, INT32 nOverWriteStream,
		BLOCK_INFO* pstBIT, INT32 nOverWritePPN, IOTYPE eIOType, INT32 nPartition, PARTITION* pstPartition)
{
	pstActiveBlock->nRecentLPN = nLPN;

	// Set g_pstStreams
	insert_bitmap(nStream, nLPN - pstStream->nStartLPN);
	pstStream->nVPC++;

	pstStream->nEndVPPN = pstActiveBlock->nRecentVPPN; //temporal nEndVPPN, will be changed if the partition is not closed 

	DEBUG_ASSERT((pstBIT->bUser + pstBIT->bGC + pstBIT->bSMerge) <= 1);

	BOOL bOverWrite = ((nOverWritePPN != INVALID_PPN) ? TRUE : FALSE);
	if (bOverWrite == TRUE)
	{
		// invalid old data
		_InvalidateOldVPPN(eIOType, nLPN, nOverWritePPN, nOverWriteStream);
	}
	else
	{
		pstPartition->nVPC++;
		g_pstStreamFTL->nVPC++;
	}

	StreamFTL_HotCold_Update(nPartition, bOverWrite, eIOType);

	// ADD TO CACHE
	AddToL2PCache(nLPN, pstActiveBlock->nRecentVPPN, nStream);
}

static void _AllocateBlock(ACTIVE_BLOCK* pstActiveBlock)
{
	BLOCK_INFO*	pstBIT;
	INT32		nBlock;

	// unset previous block as inactive
	if (pstActiveBlock->nRecentVPPN != INVALID_PPN)
	{
		nBlock = VBN_FROM_VPPN(pstActiveBlock->nRecentVPPN);
		if (GET_BLOCK_INFO(nBlock)->bActive == FALSE )
		{
			PRINTF("ERROR:: the block should be active\n");
			getchar();
			DEBUG_ASSERT(0);
		}

		GET_BLOCK_INFO(nBlock)->bActive = FALSE;
	}

	pstActiveBlock->nRecentVPPN = INVALID_PPN;

	nBlock = StreamFTL_AllocateBlock();
	pstBIT = GET_BLOCK_INFO(nBlock);

	pstBIT->bActive = TRUE;
	pstActiveBlock->nRecentVPPN = nBlock * g_stGlobal.nLPagesPerVBlock;

	BLOCK_DEBUG_PRINTF("[BLOCK] ALLOC :%d, ActiveBlock: %d \r\n", nBlock, pstActiveBlock->nIndex);
}

static RETURN _CheckWritable(ACTIVE_BLOCK* pstActiveBlock, IOTYPE eIOType)
{
	if ((eIOType == IOTYPE_HOST) && (g_pstStreamFTL->stGCMgr.eCurGCType != GC_TYPE_NONE))
	{
		// GC Running, GC prefer processing.
		// Incremental GC does not implemented yet.
		return RETURN_GC_RUNNING;
	}

	PROGRAM_UNIT*	pstProgram = &pstActiveBlock->astBuffering[pstActiveBlock->nCurProgramBuffering];

	if (pstProgram->bProgramming == TRUE)
	{
		DEBUG_ASSERT(pstProgram->bFree == FALSE);

		// busy, on programming
		return RETURN_NOT_ENOUGH_BUFFER_POOL;
	}

	// need to allocate a new buffer pool
	if (BufferPool_IsEmpty() == TRUE)
	{
		return RETURN_NOT_ENOUGH_BUFFER_POOL;
	}

	return RETURN_SUCCESS;
}

/*
	@brief StreamFTL WRITE BODY, Write a LPN
	@param	pHostBuf: buffer pointer for host data, from HIL
*/
static RETURN _Write(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, IOTYPE eIOType, UINT32 nHostCmdSlotTag, INT16 nDMAIndex)
{
#if (SUPPORT_LIMIT_STREAM_PER_PARTITION == 1)
	if ( eIOType == IOTYPE_HOST )
	{
		if ( _CheckMaxStreamsPerPartition(nLPN) == TRUE )
		{
			return RETURN_FAIL_MAX_STREAMS_PER_PARTITION;
		}
	}
#endif

	BOOL	bFreeActiveBlock;

	// Select the STREAM
	INT32 nActiveBlock = ActiveBlock_Select(nLPN, eIOType, &bFreeActiveBlock);
	ACTIVE_BLOCK *pstActiveBlock = GET_ACTIVE_BLOCK(nActiveBlock);

	RETURN nRet;
	nRet = _CheckWritable(pstActiveBlock, eIOType);
	if (nRet != RETURN_SUCCESS)
	{
		if (bFreeActiveBlock == TRUE)
		{
			// return to free list
			ActiveBlock_MoveToFreeList(nActiveBlock);
		}

		return nRet;
	}

	INT32		nOverWriteStream	= INVALID_STREAM;
	INT32		nOverWritePPN		= INVALID_PPN;

	INT32		nStream				= INVALID_STREAM;
	STREAM		*pstStream			= NULL;

	INT32		nBlock				= INVALID_PBN;				// = VBLOCK
	BLOCK_INFO	*pstBIT				= NULL;

	INT32		nPartition			= PARTITION_ID_FROM_LPN(nLPN);
	PARTITION	*pstPartition		= GET_PARTITION(nPartition);

	INT32 nPreviousStreamCount = pstPartition->nNumStream;

	// Check Overwrite
	nOverWritePPN = LPN2PPN(nLPN, &nOverWriteStream, eIOType);

	BOOL bNewBlockAllocated = FALSE;

	// Check the stream status for a write
	// 1. do we need to allocate a new physical block?
	if ( IS_VBLOCK_FULL(pstActiveBlock->nRecentVPPN) == TRUE )
	{
		_AllocateBlock(pstActiveBlock);
		bNewBlockAllocated = TRUE;
	}
	else
	{
		pstActiveBlock->nRecentVPPN++;			// Increase page offset
	}

	nBlock = VBN_FROM_VPPN(pstActiveBlock->nRecentVPPN);
	pstBIT = GET_BLOCK_INFO(nBlock);

	_WriteCounting(eIOType, nLPN, bNewBlockAllocated, pstBIT);	// for statistics and debugging

	BOOL	bNewStream = FALSE;

	if ((pstActiveBlock->nRecentLPN == INVALID_LPN) ||		// Closed stream
		(nLPN <= pstActiveBlock->nRecentLPN) ||				// Backward write
		(STREAM_START_LPN(nLPN) != STREAM_START_LPN(pstActiveBlock->nRecentLPN)))	// Partition이 변경되는 경우 stream 다시 할당.
	{
		bNewStream = TRUE;
	}

	// 2. do we need to allocate a new partition?
	if ((bNewBlockAllocated == TRUE) || (bNewStream == TRUE))
	{
		// close previous partition if exist
		if (bNewStream == TRUE)
		{
			if (pstActiveBlock->nStreamNo != INVALID_STREAM)
			{
				CloseActiveStream(nActiveBlock);
			}

			nStream = AllocateStream(eIOType);
			pstActiveBlock->nStreamNo = nStream;
			pstStream = GET_STREAM(nStream);

			pstStream->nStartLPN = ROUND_DOWN(nLPN, g_stGlobal.nLPagePerStream);
			pstStream->nStartVPPN = pstActiveBlock->nRecentVPPN;

			// for aligned partition alloc
			// for future, startLPN can be deleted
			InsertStreamIntoPartition(nPartition, nStream);
		}
		else
		{
			//DEBUG_ASSERT(pstActiveBlock->nRecentLPN == INVALID_LPN);
			nStream = pstActiveBlock->nStreamNo;
			pstStream = GET_STREAM(nStream);
		}

		LinkStreamToBIT(nStream, nBlock);

		pstStream->pnVBN[pstStream->nAllocatedBlocks]	= nBlock;
		pstStream->pnEC[pstStream->nAllocatedBlocks]	= (INT8)VNAND_GetEC(nBlock);
#if (SUPPORT_AUTO_ERASE == 1)
		if (bNewBlockAllocated == TRUE)
		{
			pstStream->pnEC[pstStream->nAllocatedBlocks]++;		// this block will be erased on the first page program
		}
#endif
		pstStream->nAllocatedBlocks++;

		DEBUG_ASSERT(pstStream->nAllocatedBlocks <= BLOCK_PER_STREAM);
	}
	else
	{
		nStream = pstActiveBlock->nStreamNo;
		pstStream = GET_STREAM(nStream);
	}

	DEBUG_ASSERT(pstStream->nAllocatedBlocks > 0);

	_UpdateMapping(nLPN, pstActiveBlock, nStream, pstStream, nOverWriteStream, pstBIT, nOverWritePPN, eIOType, nPartition, pstPartition);

	ActiveBlock_AddBufferingLPN(pstActiveBlock, stReqID, nLPN, pBuf, nHostCmdSlotTag, nDMAIndex);
	_CheckAndIssueProgram(pstActiveBlock, eIOType);

#if (ERROR_CHECKING_ON == 1)
	// test
	check_LPN_MAP(nLPN, pstActiveBlock->nRecentVPPN);
	check_OOB_MAP(nLPN, pstActiveBlock->nRecentVPPN);
#endif

	if (nPreviousStreamCount != pstPartition->nNumStream )
	{
		// if mean valid pages of cluster are changed, then re-sort the victim list
		InsertPartitionToVictimList(nPartition);
	}

	// check sequential and current LPN is start of stream
	g_stGlobal.nPrevWrittenLPN = nLPN;

	return RETURN_SUCCESS;
}

static inline VOID _CheckAndGC(IOTYPE eIOType)
{
	if (eIOType == IOTYPE_HOST) 
	{
		BOOL bNeedBlockGC = FALSE;
		BOOL bNeedStreamMerge = FALSE;

		if (g_pstStreamFTL->stStreamMgr.nFreeStreamCount < g_stGlobal.nStreamMergeTh)
		{
			bNeedStreamMerge = TRUE;
		}

		if (g_pstStreamFTL->stBlockMgr.nFreeBlockCount < g_stGlobal.nBlockGCTh)
		{
			bNeedBlockGC = TRUE;
		}

		if (bNeedBlockGC == TRUE)		// block 부족시
		{
			RETURN nRet = _StartBlockGC(INVALID_PBN);
			if (nRet == RETURN_NOT_ENOUGH_STREAM)
			{
				_StartStreamMerge(INVALID_PARTITION, STREAM_MERGE_SKIP_STREAM_COUNT, FALSE);
			}
		}
		else if (bNeedStreamMerge == TRUE)		// stream 개수 부족시
		{
			_StartStreamMerge(INVALID_PARTITION, STREAM_MERGE_SKIP_STREAM_COUNT, FALSE);
		}

		DEBUG_ASSERT(g_pstStreamFTL->stBlockMgr.nFreeBlockCount > 0);
	}
}

// WAF 실험을 위해 metablock의 필요량 만큼 free block을 감소시킨다.
// TBD. target에서 구현할 때에는 실제 meta write가 되도록 구현해야함.
VOID StreamFTL_Format(VOID)
{
	if ( g_stGlobal.bEnableMetaBlock == FALSE )
	{
		return;
	}

	INT32	nMetaBlocks = StreamFTL_GetMetaBlockCount();
	
	for (int i = 0; i < nMetaBlocks; i++)
	{
		INT32 nPBN = StreamFTL_AllocateBlock();	// throw out free blocks
		GET_BLOCK_INFO(nPBN)->bMetaBlock = TRUE;
	}

	LOG_PRINTF("[DEBUG] METABLOCK Count : %d\n", nMetaBlocks);
}

// this function returns on NAND metadata size
INT32 StreamFTL_GetMetaBlockCount(VOID)
{
	INT32	nTotalSize = 0;

	for (DUMP_TYPE i = 0; i < DUMP_TYPE_COUNT; i++)
	{
		INT32 nSize;

		if ((i == DUMP_TYPE_PB_PPN2LPN) ||		// spare data of user area
			(i == DUMP_TYPE_L2PCACHE) ||
			(i == DUMP_TYPE_L2PCACHE_PPN) ||
			(i == DUMP_TYPE_L2PCACHE_STREAM))
		{
			continue;
		}

		nSize = Dump_GetSize(i);
		DEBUG_ASSERT(nSize >= 0);

		nTotalSize += nSize;
	}

	ASSERT(nTotalSize > 0);

	INT32	nBlockCount;
	static const MIRROR = 2;
	static const SPO = 2;
	INT32	nSLCBlockSize;

	nSLCBlockSize = g_stGlobal.nVBlockSize / BITS_PER_CELL;
	nBlockCount = CEIL(nTotalSize, nSLCBlockSize);
	nBlockCount = nBlockCount * MIRROR * SPO;

	return nBlockCount;
}


///////////////////////////////////////////////////////////////////////////////
//
//	static functions
//
///////////////////////////////////////////////////////////////////////////////

/*
	Partition GC: Free stream을 확보하기 위해 partition을 선택하여 stream들을 합친다.
	nSkipCount:	Latest (stream 부터) skip할 stream의 개수.
*/
static VOID _StartStreamMerge(INT32 nPartition, INT32 nSkipCount, BOOL bWhileBlockGC)
{
	RETURN	nRet = _CheckGCAvailable();
	if (nRet != RETURN_SUCCESS)
	{
		// there is on-going GC
		return;
	}

	/*******COUNTING******/
	STAT_IncreaseCount(PROFILE_SMERGE, 1);
	if (bWhileBlockGC == TRUE)
	{
		STAT_IncreaseCount(PROFILE_SMERAGE_WHILE_BGC, 1);
	}
	/*********************/

	SMERGE_INFO*		pstSMerge = GET_CUR_STREAM_MERGE();

	OSAL_MEMSET(pstSMerge, 0x00, sizeof(SMERGE_INFO));

	if (nPartition == INVALID_PARTITION)
	{
		pstSMerge->nPartition = SelectVictimPartition();
	}
	else
	{
		pstSMerge->nPartition = nPartition;
	}

	PARTITION *pstPartition			= GET_PARTITION(pstSMerge->nPartition);
	pstSMerge->nCurReadLPN			= PARTITION_START_LPN(pstPartition->nPartitionNo);
	pstSMerge->nWriteOffset			= 0;
	pstSMerge->nStartLPN			= PARTITION_START_LPN(pstSMerge->nPartition);
	pstSMerge->nEndLPN				= pstSMerge->nStartLPN + g_stGlobal.nLPagePerPartition - 1;
	pstSMerge->nWriteDoneLPNCount	= 0;
	pstSMerge->nVPC					= pstPartition->nVPC;

#ifdef _DEBUG
	for (int i = 0; i < g_stGlobal.nLPagePerPartition; i++)
	{
		pstSMerge->astCopyInfo[i].nGCRequestIndex = INVALID_INDEX;
	}
#endif

	DEBUG_ASSERT( (bWhileBlockGC == FALSE) ? (pstPartition->nNumStream > 1) : TRUE);

	g_pstStreamFTL->stGCMgr.eCurGCType = GC_TYPE_STREAM_MERGE;
	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bFree = FALSE;
	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bBlockGC = FALSE;
	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bSMerge = TRUE;

	// done. next operation is done on _ProcessStreamMerge()
}

static VOID _IssueReadForStreamMerge(VOID)
{
	SMERGE_INFO* pstSMerge = GET_CUR_STREAM_MERGE();

	INT32	nVPPN;
	INT32	nStream;

	do
	{
		if (pstSMerge->nCurReadLPN > pstSMerge->nEndLPN)
		{
			// all read done
			return;
		}

		nVPPN = LPN2PPN(pstSMerge->nCurReadLPN, &nStream, IOTYPE_STREAM_MERGE);
		if (nVPPN != INVALID_PPN)
		{
			break;
		}

		pstSMerge->nCurReadLPN++;
	} while (1);

	DEBUG_ASSERT(nVPPN != INVALID_PPN);

	// Issue Read
	GC_REQUEST*	pstRequest;
	pstRequest = FTL_ReadPageGC(pstSMerge->nCurReadLPN);
	if (pstRequest == NULL)
	{
		return;		// Fail to issue
	}

	INT32	nLPNOffset = pstSMerge->nCurReadLPN - pstSMerge->nStartLPN;

	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].bValid == FALSE);

	pstSMerge->astCopyInfo[nLPNOffset].bValid			= TRUE;
	pstSMerge->astCopyInfo[nLPNOffset].nGCRequestIndex	= pstRequest->stRequestID.stGC.nRequestIndex;

	pstSMerge->nCurReadLPN++;
	return;
}

/*
	@brief this function creates read request for stream merge
*/
static void _ProcessStreamMerge(VOID)
{
	// Issue read LPN
	_IssueReadForStreamMerge();

	// Issue write for the read pages
	StreamFTL_IssueStreamMergeWrite();
}

static void _ReadCounting(IOTYPE eIOType, INT32 nVPPN)
{
	/****** COUNTING ******/
	if (eIOType == IOTYPE_HOST)
	{
		STAT_IncreaseCount(PROFILE_HOST_READ, 1);
	}
	else if (eIOType == IOTYPE_STREAM_MERGE)
	{
		STAT_IncreaseCount(PROFILE_SMERGE_read, 1);
	}
	else if (eIOType == IOTYPE_BLOCK_GC)
	{
		STAT_IncreaseCount(PROFILE_BGC_READ, 1);
	}

	if ((nVPPN == INVALID_PPN) && (eIOType != IOTYPE_TEST))
	{
		STAT_IncreaseCount(PROFILE_HOST_UNMAP_READ, 1);
	}
}

/*
	@brief check and update free move info index, 
			StreamFTL has two type of Garbage collection (sMerge, bGC).
			Two GC have it's own active block.
			if one active block does not be full for program(4LPN, 16KB), then it is remained on buffering state. 
			and the move info's bFree state is FALSE.
			this function update nCurMoveIndex to a free index to proceed another type of GC.
*/
static BOOL _LookupFreeMoveInfo(VOID)
{
	// lookup free move info index
	for (INT32 i = 0; i < GC_MANAGER_MOVE_INFO_COUNT; i++)
	{
		if (GET_GC_MANAGER()->astMoveInfo[i].bFree == TRUE)
		{
			GET_GC_MANAGER()->nCurMoveIndex = i;
			return TRUE;
		}
	}

	return FALSE;
}

static RETURN _CheckGCAvailable(VOID)
{
	if (g_pstStreamFTL->stGCMgr.eCurGCType != GC_TYPE_NONE)
	{
		// there is on-going GC
		return RETURN_GC_RUNNING;
	}

	// check free move info
	if (GET_GC_MANAGER()->astMoveInfo[GET_GC_MANAGER()->nCurMoveIndex].bFree == FALSE)
	{
		if (_LookupFreeMoveInfo() == FALSE)
		{
			return RETURN_GC_RUNNING;
		}
	}

	return RETURN_SUCCESS;
}

/*
	@brief start block garbage collection
		Process
		1. select victim block
		2. Set merge inforation on STREAMFTL_GLOBAL
		(below is on _ProcessBlockGC())
		3. Get a valid VPPN
		4. Issue Read, Increase page offset
		5. (On Read Done) Re-write valid LPN
		6. goto 3 until last page of VBlocks

	@param	nVBN:	Victim Block Number, 
				INVALID_VBN: choose a block by victim selection policy
*/
static RETURN _StartBlockGC(INT32 nVBN)
{
	RETURN	nRet = _CheckGCAvailable();
	if (nRet != RETURN_SUCCESS)
	{
		// there is on-going GC
		return nRet;
	}

	int nVictimVBN = 0;

	/* Select victim block */
	if (nVBN == -1)
	{
		nVictimVBN = SelectVictimBlock();
	}
	else
	{
		nVictimVBN = nVBN;
	}

	BLOCK_INFO *pstBIT = GET_BLOCK_INFO(nVictimVBN);

	DEBUG_ASSERT(pstBIT->nInvalidPages != g_stGlobal.nLPagesPerVBlock);

	// check is there enough stream 
	if ( (pstBIT->nStreamCount + 2) >= (UINT32)g_pstStreamFTL->stStreamMgr.nFreeStreamCount )
	{
		return RETURN_NOT_ENOUGH_STREAM;
	}

	g_pstStreamFTL->stGCMgr.eCurGCType = GC_TYPE_BLOCK_GC;

	BLOCK_GC_INFO*		pstGC = GET_CUR_BLOCK_GC();

	pstGC->nVictimVBN			= nVictimVBN;
	pstGC->nCurReadLPageOffset	= 0;
	pstGC->nVPC					= g_stGlobal.nLPagesPerVBlock - pstBIT->nInvalidPages;
	pstGC->nWriteLPNCount		= 0;
	pstGC->nWriteDoneLPNCount	= 0;
	pstGC->nReadIndex			= 0;
	pstGC->nWriteIndex			= 0;

	OSAL_MEMSET(&pstGC->astCopyInfo[0], 0xFF, sizeof(pstGC->astCopyInfo));

	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bFree = FALSE;
	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bBlockGC = TRUE;
	g_pstStreamFTL->stGCMgr.astMoveInfo[g_pstStreamFTL->stGCMgr.nCurMoveIndex].bSMerge = FALSE;

#ifdef _DEBUG
	for (int i = 0; i < BLOCK_GC_COPY_ENTRY_COUNT; i++)
	{
		pstGC->astCopyInfo[i].nGCRequestIndex = INVALID_INDEX;
	}
#endif

	/*******COUNTING******/
	STAT_IncreaseCount(PROFILE_BGC, 1);
	/*********************/

	return RETURN_SUCCESS;
}

static VOID _IssueBlockGCRead(VOID)
{
	BLOCK_GC_INFO* pstBlockGC = GET_CUR_BLOCK_GC();

	INT32	nNextQueueIndex = INCREASE_IN_RANGE(pstBlockGC->nReadIndex, BLOCK_GC_COPY_ENTRY_COUNT);
	// Check is there free entry
	if (pstBlockGC->astCopyInfo[nNextQueueIndex].bFree == FALSE)
	{
		// queue full, wait until write done
		return;
	}

	// check Page offset is valid

	do
	{
		if (pstBlockGC->nCurReadLPageOffset >= g_stGlobal.nLPagesPerVBlock)
		{
			// end of block
			return;
		}

		BOOL bValid = VNAND_IsValid(pstBlockGC->nVictimVBN, pstBlockGC->nCurReadLPageOffset);
		if (bValid == TRUE)
		{
			break;
		}
		else
		{
			pstBlockGC->nCurReadLPageOffset++;
			DEBUG_ASSERT(pstBlockGC->nCurReadLPageOffset <= g_stGlobal.nLPagesPerVBlock);
		}
	} while (1);

	// Issue Read
	GC_REQUEST*	pstRequest;

	pstRequest = StreamFTL_AllocateGCRequest();
	if (pstRequest == NULL)
	{
		// no more free request
		return;
	}

	pstRequest->stRequestID.stGC.nType			= FTL_REQUEST_ID_TYPE_BLOCK_GC_READ;
	pstRequest->stRequestID.stGC.nRequestIndex	= StreamFTL_GetGCRequestIndex(pstRequest);
	pstRequest->stRequestID.stGC.nMoveInfoIndex	= GET_GC_MANAGER()->nCurMoveIndex;

	pstRequest->nVPPN = GET_VPPN_FROM_VPN_VBN(pstBlockGC->nCurReadLPageOffset, pstBlockGC->nVictimVBN);

	//pstRequest->nLPN = nLPN;			// Unknown now, LPN is stored at NAND spare

	StreamFTL_AddToRequestWaitQ(g_pstGCRequestInfo, &pstRequest->stCommon);

	pstBlockGC->nCurReadLPageOffset++;

	BLOCK_GC_COPY*	pstCopy = &pstBlockGC->astCopyInfo[pstBlockGC->nReadIndex];

	DEBUG_ASSERT(pstCopy->bFree == TRUE);

	pstCopy->nGCRequestIndex = StreamFTL_GetGCRequestIndex(pstRequest);
	pstCopy->bFree			= FALSE;
	pstCopy->bReadDone		= FALSE;
	pstCopy->bWriteIssued	= FALSE;
	pstCopy->bWriteDone		= FALSE;

	pstRequest->nCopyEntryIndex = pstBlockGC->nReadIndex;

	pstBlockGC->nReadIndex = INCREASE_IN_RANGE(pstBlockGC->nReadIndex, BLOCK_GC_COPY_ENTRY_COUNT);			// increase queue index

	FTL_DEBUG_PRINTF("[FTL][BlockGC][Read] RequestIndex:%d, Buf: 0x%X \r\n",
		pstRequest->stRequestID.stGC.nRequestIndex, (UINT32)pstRequest->pMainBuf);

	return;
}

/*
	@brief	Issue Read/Write LPage for page copy
*/
static VOID _ProcessBlockGC(VOID)
{
	DEBUG_ASSERT(g_pstStreamFTL->stGCMgr.eCurGCType == GC_TYPE_BLOCK_GC);

	// Issue read LPN
	_IssueBlockGCRead();

	// Issue write the read pages
	StreamFTL_IssueBlockGCWrite();
}

static VOID _ProcessGC(VOID)
{
	switch (g_pstStreamFTL->stGCMgr.eCurGCType)
	{
	case GC_TYPE_NONE:
		// Nothing to do
		break;

	case GC_TYPE_STREAM_MERGE:
		_ProcessStreamMerge();
		break;

	case GC_TYPE_BLOCK_GC:
		_ProcessBlockGC();
		break;

	default:
		ASSERT(0);		// unknown type
		break;
	}
}

