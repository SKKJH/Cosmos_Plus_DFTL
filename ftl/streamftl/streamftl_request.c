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

#ifdef STREAMFTL_REQUEST_DEBUG
	#define DEBUG_PRINTF_REQUEST		PRINTF
#else
	#define DEBUG_PRINTF_REQUEST(...)	((void)0)
#endif

HIL_REQUEST_POOL*	g_pstHILRequestPool;
REQUEST_INFO*		g_pstHILRequestInfo;

GC_REQUEST_POOL*	g_pstGCRequestPool;
REQUEST_INFO*		g_pstGCRequestInfo;

static void _InitRequestInfo(REQUEST_INFO* pstRequestInfo, FTL_REQUEST_ID_TYPE nReqType);

static void _AddToRequestFreeQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static void _RemoveFromRequestWaitQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static void _AddToRequestIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static void _RemoveFromRequestIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static void _AddToRequestDoneQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static void _RemoveFromRequestDoneQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static VOID _AddToHDMAIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);
static VOID _ProcessHDMARequestIssuedQ();

static void _ReleaseRequest(REQUEST_INFO* pstRequestInfo, void* pstRequest);

static void _ReleaseHILRequest(HIL_REQUEST* pstRequest);
static void _ReleaseGCRequest(GC_REQUEST* pstRequest);

static BOOL _ProcessHILRequestWaitQ(VOID);
static BOOL _ProcessHILRequestWaitQ_Read(HIL_REQUEST* pstRequest);
static BOOL _ProcessHILRequestWaitQ_Write(HIL_REQUEST* pstRequest);
static VOID _ProcessGCRequestWaitQ(VOID);
static void _ProcessGCRequestDone(GC_REQUEST* pstGCRequest);

static UINT32 _GetHILRequestIndex(HIL_REQUEST* pstRequest);

static void _CallBackHILReadRequest(FTL_REQUEST_ID stReqID);
static void _CallBackStreamMergeReadRequest(FTL_REQUEST_ID stReqID);
static void _CallBackStreamMergeWriteRequest(FTL_REQUEST_ID stReqID);
static VOID _CallBackBlockGCReadRequest(FTL_REQUEST_ID stReqID);
static void _CallBackBlockGCWriteRequest(FTL_REQUEST_ID stReqID);

static void _ReleaseActiveBlockProgramUnit(UINT32 nActiveBlockIndex, UINT32 nBufferingIndex);

void StreamFTL_InitRequestPool(void)
{
	g_pstHILRequestPool = (HIL_REQUEST_POOL*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(HIL_REQUEST_POOL), 0);
	ASSERT(g_pstHILRequestPool);
	
	for (int i = 0; i < HIL_REQUEST_COUNT; i++)
	{
		INIT_LIST_HEAD(&g_pstHILRequestPool->astRequest[i].stCommon.dlList);
		_ReleaseRequest(g_pstHILRequestInfo, &g_pstHILRequestPool->astRequest[i]);
	}

	//GC REQUEST
	g_pstGCRequestPool = (GC_REQUEST_POOL*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(GC_REQUEST_POOL), 0);
	ASSERT(g_pstGCRequestPool);

	ASSERT(GC_REQUEST_COUNT < (1 << GC_REQUST_COUNT_BITS));
	ASSERT(BLOCK_GC_COPY_ENTRY_COUNT < (1 << BLOCK_GC_COPY_ENTRY_COUNT_BITS));

	for (int i = 0; i < GC_REQUEST_COUNT; i++)
	{
		INIT_LIST_HEAD(&g_pstGCRequestPool->astRequest[i].stCommon.dlList);
		g_pstGCRequestPool->astRequest[i].pMainBuf	= (void*)OSAL_MemAlloc(MEM_TYPE_BUF, BYTES_PER_DATA_REGION_OF_PAGE, OSAL_MEMALLOC_BUF_ALIGNMENT);
		g_pstGCRequestPool->astRequest[i].pSpareBuf	= (void*)OSAL_MemAlloc(MEM_TYPE_BUF, BYTES_PER_SPARE_REGION_OF_PAGE, OSAL_MEMALLOC_BUF_ALIGNMENT);
		_ReleaseRequest(g_pstGCRequestInfo, &g_pstGCRequestPool->astRequest[i]);
	}

	return;
}

VOID StreamFTL_InitRequestInfo(void)
{
	g_pstHILRequestInfo = (REQUEST_INFO*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(REQUEST_INFO), 0);
	ASSERT(g_pstHILRequestInfo);

	_InitRequestInfo(g_pstHILRequestInfo, FTL_REQUEST_ID_TYPE_HIL);

	g_pstGCRequestInfo = (REQUEST_INFO*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(REQUEST_INFO), 0);
	ASSERT(g_pstGCRequestInfo);

	_InitRequestInfo(g_pstGCRequestInfo, FTL_REQUEST_ID_TYPE_GC);
}

VOID StreamFTL_ProcessWaitQ(VOID)
{
	BOOL bSuccess;

	_ProcessGCRequestWaitQ();
	do
	{
		bSuccess = _ProcessHILRequestWaitQ();
	} while (bSuccess == TRUE);
}

VOID _CallBackProgram(FTL_REQUEST_ID stReqID)
{
	DEBUG_ASSERT(stReqID.stProgram.nActiveBlockIndex < (UINT32)g_stGlobal.nActiveBlockCount);
	DEBUG_ASSERT(stReqID.stProgram.nBufferingIndex < ACTIVE_BLOCK_BUFFERING_COUNT);

	ACTIVE_BLOCK*	pstActiveBlock = GET_ACTIVE_BLOCK(stReqID.stProgram.nActiveBlockIndex);
	PROGRAM_UNIT*	pstProgram = &pstActiveBlock->astBuffering[stReqID.stProgram.nBufferingIndex];

	if (pstProgram->astRequestID[0].stCommon.nType != FTL_REQUEST_SUB_ID_TYPE_HIL_WRITE)
	{
		for (UINT32 i = 0; i < pstProgram->nLPNCount; i++)
		{
			switch (pstProgram->astRequestID[i].stCommon.nType)
			{
			case FTL_REQUEST_SUB_ID_TYPE_STREAM_MERGE_WRITE:
				_CallBackStreamMergeWriteRequest(pstProgram->astRequestID[i]);
				break;

			case FTL_REQUEST_SUB_ID_TYPE_BLOCK_GC_WRITE:
				_CallBackBlockGCWriteRequest(pstProgram->astRequestID[i]);
				break;

			default:
				ASSERT(0);		//Invalid sub type for program
				break;
			}
		}
	}

	// Release ProgramBuffer
	_ReleaseActiveBlockProgramUnit(stReqID.stProgram.nActiveBlockIndex, stReqID.stProgram.nBufferingIndex);
}

/*
	@brief	FIL to FTL callback function.
			caution: never reissue FIL or some FTL function
*/
VOID FTL_CallBack(FTL_REQUEST_ID stReqID)
{
#if (UNIT_TEST_FIL_PERF == 1)
	return;
#endif

	switch (stReqID.stCommon.nType)
	{
	case FTL_REQUEST_ID_TYPE_HIL_READ:
		_CallBackHILReadRequest(stReqID);
		break;

	case FTL_REQUEST_ID_TYPE_PROGRAM:
		// Pysical page program done
		_CallBackProgram(stReqID);
		break;

	case FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ:
		_CallBackStreamMergeReadRequest(stReqID);
		break;

	case FTL_REQUEST_ID_TYPE_BLOCK_GC_READ:
		_CallBackBlockGCReadRequest(stReqID);
		break;

	default:
		ASSERT(0);
		break;
	}
}

static VOID _AddToHDMAIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	list_add_tail(&pstCommonRequest->dlList, &pstRequestInfo->dlHDMA);
	pstRequestInfo->nHDMACount++;
}

static void _RemoveFromHDMAIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	DEBUG_ASSERT(pstRequestInfo->nHDMACount > 0);

	list_del_init(&pstCommonRequest->dlList);
	pstRequestInfo->nHDMACount--;
}

static VOID _ProcessHILRequestDoneQ_NandRead(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(pstRequest->nStatus == HIL_REQUEST_READ_NAND_DONE);
	DEBUG_ASSERT(pstRequest->nLPNCount == pstRequest->nDoneLPNCount);

	INT32	nDMAIndex = 0;
	BUFFER_ENTRY*		pstBufferEntry;
	UINT32	nBufAddr;

	list_for_each_entry(BUFFER_ENTRY, pstBufferEntry, &pstRequest->dlBuffer, dlList)
	{
		nBufAddr = (UINT32)pstBufferEntry->pMainBuf + (LOGICAL_PAGE_SIZE * pstRequest->anLPNOffsetOfBuffer[nDMAIndex]);
		set_auto_tx_dma(pstRequest->nHostCmdSlotTag, nDMAIndex, nBufAddr, NVME_COMMAND_AUTO_COMPLETION_ON);

#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
		// Set LPN on main buffer to data verification
		if (*((unsigned int *)pstBufferEntry->pSpareBuf) != DATA_VERIFICATION_INVALID_LPN)		// check written page
		{
			//DEBUG_ASSERT(((unsigned int *)pstRequest->pSpareBuf)[nLPNOffset] == pstRequest->nLPN);
			if (((unsigned int *)pstBufferEntry->pSpareBuf)[pstRequest->anLPNOffsetOfBuffer[nDMAIndex]] != (pstRequest->nLPN + nDMAIndex))
			{
				PRINTF("[FTL] LPN mismatch, request LPN: %d, SpareLPN: %d \n\r",
					(pstRequest->nLPN + nDMAIndex), ((unsigned int *)pstBufferEntry->pSpareBuf)[pstRequest->anLPNOffsetOfBuffer[nDMAIndex]]);
				STAT_IncreaseCount(PROFILE_MISCOMAPRE, 1);
			}
		}
#endif
		nDMAIndex++;
	}

	DEBUG_ASSERT(nDMAIndex == pstRequest->nDoneLPNCount);

	pstRequest->nDMAReqTail		= g_hostDmaStatus.fifoTail.autoDmaTx;
	pstRequest->nOverFlowCount	= g_hostDmaAssistStatus.autoDmaTxOverFlowCnt;

	pstRequest->nStatus++;

	// Release Request
	_RemoveFromRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);
	_AddToHDMAIssuedQ(g_pstHILRequestInfo, &pstRequest->stCommon);
}

static VOID _ProcessHILRequestDoneQ_Read(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(pstRequest->nStatus == HIL_REQUEST_READ_DONE);
	DEBUG_ASSERT(pstRequest->nDoneLPNCount == pstRequest->nLPNCount);

	BUFFER_ENTRY*	pstCurEntry;
	BUFFER_ENTRY*	pstNextEntry;

	INT32	nCount = 0;

	// release buffer
	list_for_each_entry_safe(BUFFER_ENTRY, pstCurEntry, pstNextEntry, &pstRequest->dlBuffer, dlList)
	{
		BufferPool_Release(pstCurEntry);
		nCount++;
	}

	DEBUG_ASSERT(nCount == pstRequest->nLPNCount);

	// Release Request
	_RemoveFromRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);
	_ReleaseRequest(g_pstHILRequestInfo, pstRequest);
}

static VOID _ProcessHILRequestDoneQ(VOID)
{
	HIL_REQUEST* pstRequest;

	do
	{
		if (g_pstHILRequestInfo->nDoneCount == 0)
		{
			DEBUG_ASSERT(list_empty(&g_pstHILRequestInfo->dlDone));
			break;
		}

		pstRequest = list_first_entry(&g_pstHILRequestInfo->dlDone, HIL_REQUEST, stCommon.dlList);

		if (pstRequest->nCmd == NVME_CMD_OPCODE_READ)
		{
			if (pstRequest->nStatus == HIL_REQUEST_READ_NAND_DONE)
			{
				// [TODO] stop if there is not enough free HDMA queue
				_ProcessHILRequestDoneQ_NandRead(pstRequest);
			}
			else
			{
				_ProcessHILRequestDoneQ_Read(pstRequest);
			}
		}
		else
		{
			ASSERT(pstRequest->nCmd == NVME_CMD_OPCODE_WRITE);

			// Release Request
			_RemoveFromRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);
			_ReleaseRequest(g_pstHILRequestInfo, pstRequest);
		}

	} while (1);
}

static void _ProcessGCRequestDone(GC_REQUEST* pstGCRequest)
{
	if (pstGCRequest->stRequestID.stCommon.nType == FTL_REQUEST_SUB_ID_TYPE_BLOCK_GC_WRITE)
	{
		BLOCK_GC_INFO*	pstBlockGC = GET_BLOCK_GC_INFO(pstGCRequest->stRequestID.stGC.nMoveInfoIndex);
		DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstGCRequest->nCopyEntryIndex].bWriteDone == TRUE);

		pstBlockGC->astCopyInfo[pstGCRequest->nCopyEntryIndex].bFree = TRUE;
	}
}

void _ProcessGCRequestDoneQ(void)
{
	GC_REQUEST* pstRequest;
	GC_REQUEST* pstNextRequest;

	do
	{
		if (g_pstGCRequestInfo->nDoneCount == 0)
		{
			DEBUG_ASSERT(list_empty(&g_pstGCRequestInfo->dlDone));
			break;
		}

		list_for_each_entry_safe(GC_REQUEST, pstRequest, pstNextRequest, &g_pstGCRequestInfo->dlDone, stCommon.dlList)
		{
			_ProcessGCRequestDone(pstRequest);		// nothing to do now

			// remove from DoneQ
			_RemoveFromRequestDoneQ(g_pstGCRequestInfo, &pstRequest->stCommon);

			// Release Request
			_ReleaseGCRequest(pstRequest);
		}
	} while (1);
}

VOID StreamFTL_ProcessDoneQ(VOID)
{
	_ProcessHILRequestDoneQ();
	_ProcessHDMARequestIssuedQ();	// Let's check when there is not enough free buffer
	_ProcessGCRequestDoneQ();
}

HIL_REQUEST* StreamFTL_AllocateHILRequest(VOID)
{
	if (g_pstHILRequestInfo->nFreeCount == 0)
	{
		ASSERT(0);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&g_pstHILRequestInfo->dlFree) == FALSE);

	HIL_REQUEST*	pstRequest;

	pstRequest = list_first_entry(&g_pstHILRequestInfo->dlFree, HIL_REQUEST, stCommon.dlList);

	list_del(&pstRequest->stCommon.dlList);

	g_pstHILRequestInfo->nFreeCount--;

	DEBUG_ASSERT(pstRequest->nStatus == HIL_REQUEST_FREE);

	return pstRequest;
}

GC_REQUEST* StreamFTL_AllocateGCRequest(void)
{
	if (g_pstGCRequestInfo->nFreeCount == 0)
	{
		DEBUG_ASSERT(list_empty(&g_pstGCRequestInfo->dlFree) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&g_pstGCRequestInfo->dlFree) == FALSE);

	GC_REQUEST*	pstRequest;

	pstRequest = list_first_entry(&g_pstGCRequestInfo->dlFree, GC_REQUEST, stCommon.dlList);

	list_del(&pstRequest->stCommon.dlList);

	g_pstGCRequestInfo->nFreeCount--;

#ifdef _DEBUG
	pstRequest->nVPPN = INVALID_PPN;
#endif

	DEBUG_PRINTF_REQUEST("[FTL][REQUEST][GC] Allocate index: %d\n\r", StreamFTL_GetGCRequestIndex(pstRequest));

	return pstRequest;
}

void StreamFTL_AddToRequestWaitQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	list_add_tail(&pstCommonRequest->dlList, &pstRequestInfo->dlWait);
	pstRequestInfo->nWaitCount++;
}

FTL_REQUEST_ID StreamFTL_GetRequestID(HIL_REQUEST* pstRequest, IOTYPE eIOType)
{
	FTL_REQUEST_ID stReqID;

	switch (eIOType)
	{
	case IOTYPE_HOST:
		if (pstRequest->nCmd == NVME_CMD_OPCODE_READ)
		{
			stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_HIL_READ;
		}
		else if (pstRequest->nCmd == NVME_CMD_OPCODE_WRITE)
		{
			stReqID.stCommon.nType = FTL_REQUEST_SUB_ID_TYPE_HIL_WRITE;	// this type will be stored request id of PROGRAM_UNIT
		}
		else
		{
			ASSERT(0);	// Not implemented yet
		}

		stReqID.stHIL.nRequestIndex = _GetHILRequestIndex(pstRequest);
		DEBUG_ASSERT(stReqID.stHIL.nRequestIndex < HIL_REQUEST_COUNT);
		break;

#if 0
	case IOTYPE_STREAM_MERGE:
	case IOTYPE_BLOCK_GC:
		stReqID.nType = REQUEST_TYPE_FTL_GC;
		break;
#endif

	case IOTYPE_TEST:
		stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_DEBUG;
		break;

	default:
		ASSERT(0);		// CHECK IT
		break;
	}

	return stReqID;
}

/*
@brief	issue write according to the read sequence
*/
VOID StreamFTL_IssueStreamMergeWrite(VOID)
{
	SMERGE_INFO*	pstSMerge = GET_CUR_STREAM_MERGE();
	SMERGE_COPY*	pstCurReadInfo;
	GC_REQUEST*		pstRequest;
	INT32			nReadOffset = pstSMerge->nCurReadLPN - pstSMerge->nStartLPN;

	do
	{
		pstCurReadInfo = &pstSMerge->astCopyInfo[pstSMerge->nWriteOffset];
		if (pstSMerge->nWriteOffset >= nReadOffset)
		{
			break;
		}

		if (pstCurReadInfo->bValid == FALSE)
		{
			pstSMerge->nWriteOffset++;
			continue;
		}

		if (pstCurReadInfo->bReadDone == TRUE)
		{
			DEBUG_ASSERT(pstCurReadInfo->bValid == TRUE);
			DEBUG_ASSERT(pstCurReadInfo->bWriteIssued == FALSE);
			DEBUG_ASSERT(pstCurReadInfo->bWriteDone == FALSE);
			DEBUG_ASSERT(pstCurReadInfo->nGCRequestIndex < GC_REQUEST_COUNT);

			// get request
			pstRequest = &g_pstGCRequestPool->astRequest[pstCurReadInfo->nGCRequestIndex];

			// Re-insert to WaitQ for write
			pstRequest->stRequestID.stGC.nType = FTL_REQUEST_SUB_ID_TYPE_STREAM_MERGE_WRITE;

			_RemoveFromRequestIssuedQ(g_pstGCRequestInfo, &pstRequest->stCommon);
			StreamFTL_AddToRequestWaitQ(g_pstGCRequestInfo, &pstRequest->stCommon);

			pstSMerge->nWriteOffset++;
		}
		else
		{
			// wait read done
			break;
		}
	} while (TRUE);

	if ((pstSMerge->nWriteOffset >= g_stGlobal.nLPagePerPartition) &&		// Write issue done
		(g_pstGCRequestInfo->nWaitCount == 0))								// to avoide interference between bGC and sMerge
	{																		// bGC finish condition is write count goes to the valid page of victim block
		// Stream Merge Done!												// bGC VPC count must not reduced by stream merge or HIL write
		// clear GC type
		g_pstStreamFTL->stGCMgr.eCurGCType = GC_TYPE_NONE;

		// GC active block을 구분한 경우
		//	같은 partition에대한 write가 (1)GC -> (2)user -> (3)GC write 될경우 
		//	(1)에서 할당된 stream이 (3)에서 재사용되어 (2)의 write와 순서가 역전될 수 있다.
		//	그러므로 GC가 끝나면 LPN 정보를 초기화 하여 다시 stream이 할당되도록 한다.
		g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockSMerge->nRecentLPN = INVALID_LPN;
		GET_GC_MANAGER()->nCurMoveIndex = INCREASE_IN_RANGE(GET_GC_MANAGER()->nCurMoveIndex, GC_MANAGER_MOVE_INFO_COUNT);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	STATIC FUNCTIONS
//

static void _AddToRequestFreeQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	list_add_tail(&pstCommonRequest->dlList, &pstRequestInfo->dlFree);
	pstRequestInfo->nFreeCount++;
	DEBUG_ASSERT(pstRequestInfo->nFreeCount <= (MAX(HIL_REQUEST_COUNT, GC_REQUEST_COUNT)));
}

static void _RemoveFromRequestWaitQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	DEBUG_ASSERT(pstRequestInfo->nWaitCount > 0);

	list_del_init(&pstCommonRequest->dlList);
	pstRequestInfo->nWaitCount--;
}

static void _AddToRequestIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	list_add_tail(&pstCommonRequest->dlList, &pstRequestInfo->dlIssued);
	pstRequestInfo->nIssuedCount++;

	DEBUG_ASSERT(pstRequestInfo->nIssuedCount <= (MAX(HIL_REQUEST_COUNT, GC_REQUEST_COUNT)));
}

static void _RemoveFromRequestIssuedQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	DEBUG_ASSERT(pstRequestInfo->nIssuedCount > 0);

	list_del_init(&pstCommonRequest->dlList);
	pstRequestInfo->nIssuedCount--;
}

static void _AddToRequestDoneQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	list_add_tail(&pstCommonRequest->dlList, &pstRequestInfo->dlDone);
	pstRequestInfo->nDoneCount++;

	DEBUG_ASSERT(pstRequestInfo->nDoneCount <= (MAX(HIL_REQUEST_COUNT, GC_REQUEST_COUNT)));
}

static void _RemoveFromRequestDoneQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest)
{
	DEBUG_ASSERT(pstRequestInfo->nDoneCount > 0);

	list_del_init(&pstCommonRequest->dlList);
	pstRequestInfo->nDoneCount--;
}

static void _InitRequestInfo(REQUEST_INFO* pstRequestInfo, FTL_REQUEST_ID_TYPE nReqType)
{
	pstRequestInfo->nReqType = nReqType;

	INIT_LIST_HEAD(&pstRequestInfo->dlWait);
	pstRequestInfo->nWaitCount = 0;

	INIT_LIST_HEAD(&pstRequestInfo->dlIssued);
	pstRequestInfo->nIssuedCount = 0;

	INIT_LIST_HEAD(&pstRequestInfo->dlDone);
	pstRequestInfo->nDoneCount = 0;

	INIT_LIST_HEAD(&pstRequestInfo->dlFree);
	pstRequestInfo->nFreeCount = 0;

	INIT_LIST_HEAD(&pstRequestInfo->dlHDMA);
	pstRequestInfo->nHDMACount = 0;
}

static void _ReleaseRequest(REQUEST_INFO* pstRequestInfo, void* pstRequest)
{
	switch (pstRequestInfo->nReqType)
	{
	case FTL_REQUEST_ID_TYPE_HIL:
		_ReleaseHILRequest(pstRequest);
		break;

	case FTL_REQUEST_ID_TYPE_GC:
		_ReleaseGCRequest(pstRequest);
		break;

	default:
		ASSERT(0);	// never reach here
		break;
	}
}

static void _ReleaseHILRequest(HIL_REQUEST* pstRequest)
{
	_AddToRequestFreeQ(g_pstHILRequestInfo, &pstRequest->stCommon);
	pstRequest->nStatus = HIL_REQUEST_FREE;
	DEBUG_ASSERT(g_pstHILRequestInfo->nFreeCount <= HIL_REQUEST_COUNT);
	DEBUG_ASSERT(_GetHILRequestIndex(pstRequest) < HIL_REQUEST_COUNT);
}

static void _ReleaseGCRequest(GC_REQUEST* pstRequest)
{
	DEBUG_PRINTF_REQUEST("[FTL][REQUEST][GC] Release index: %d\n\r", StreamFTL_GetGCRequestIndex(pstRequest));

	_AddToRequestFreeQ(g_pstGCRequestInfo, &pstRequest->stCommon);
	DEBUG_ASSERT(StreamFTL_GetGCRequestIndex(pstRequest) < GC_REQUEST_COUNT);
	DEBUG_ASSERT(g_pstGCRequestInfo->nFreeCount <= GC_REQUEST_COUNT);
}

static UINT32 _GetHILRequestIndex(HIL_REQUEST* pstRequest)
{
	return pstRequest - &g_pstHILRequestPool->astRequest[0];
}

UINT32 StreamFTL_GetGCRequestIndex(GC_REQUEST* pstRequest)
{
	return pstRequest - &g_pstGCRequestPool->astRequest[0];
}

static VOID _ProcessGCRequestWaitQ(VOID)
{
	if (list_empty(&g_pstGCRequestInfo->dlWait))
	{
		return;		// nothing to do
	}

	GC_REQUEST*		pstRequest;
	RETURN			nRet = RETURN_SUCCESS;
	BOOL			bBufferHit = FALSE;

	pstRequest = list_first_entry(&g_pstGCRequestInfo->dlWait, GC_REQUEST, stCommon.dlList);

	if (pstRequest->stRequestID.stGC.nType == FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ)
	{
		pstRequest->nVPPN = StreamFTL_Read(pstRequest->stRequestID, pstRequest->nLPN, pstRequest->pMainBuf, pstRequest->pSpareBuf, IOTYPE_STREAM_MERGE, &bBufferHit);
		if (pstRequest->nVPPN == INVALID_PPN)	// unmap read
		{
			ASSERT(0);		// never reach here
		}

		//DEBUG_ASSERT(bBufferHit == FALSE);
	}
	else if (pstRequest->stRequestID.stGC.nType == FTL_REQUEST_SUB_ID_TYPE_STREAM_MERGE_WRITE)
	{
		INT32	nLPNOffset	= LPN_OFFSET_FROM_VPPN(pstRequest->nVPPN);
		void	*pBuf		= ((char*)(pstRequest->pMainBuf) + (LOGICAL_PAGE_SIZE * nLPNOffset));

		nRet = StreamFTL_Write(pstRequest->stRequestID, pstRequest->nLPN, pBuf, IOTYPE_STREAM_MERGE, INVALID_INDEX, (INT16)INVALID_INDEX);
		if (nRet == RETURN_SUCCESS)
		{
			INT32			nMoveInfoIndex = pstRequest->stRequestID.stGC.nMoveInfoIndex;
			SMERGE_INFO*	pstSMerge = GET_STREAM_MERGE_INFO(nMoveInfoIndex);
			DEBUG_ASSERT(GET_GC_MANAGER()->astMoveInfo[nMoveInfoIndex].bFree == FALSE);
		}
	}
	else if (pstRequest->stRequestID.stGC.nType == FTL_REQUEST_ID_TYPE_BLOCK_GC_READ)
	{
		DEBUG_ASSERT(GET_BLOCK_GC_INFO(pstRequest->stRequestID.stGC.nMoveInfoIndex)->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteIssued == FALSE);
		DEBUG_ASSERT(GET_BLOCK_GC_INFO(pstRequest->stRequestID.stGC.nMoveInfoIndex)->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteDone == FALSE);
		StreamFTL_ReadVPPN(pstRequest->stRequestID, pstRequest->nVPPN, pstRequest->pMainBuf, pstRequest->pSpareBuf, IOTYPE_STREAM_MERGE);
	}
	else if (pstRequest->stRequestID.stGC.nType == FTL_REQUEST_SUB_ID_TYPE_BLOCK_GC_WRITE)
	{
		BLOCK_GC_INFO*	pstBlockGC = GET_BLOCK_GC_INFO(pstRequest->stRequestID.stGC.nMoveInfoIndex);

		DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteIssued == FALSE);
		DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteDone == FALSE);

		INT32	nLPNOffset	= LPN_OFFSET_FROM_VPPN(pstRequest->nVPPN);
		void	*pBuf		= ((char*)(pstRequest->pMainBuf) + (LOGICAL_PAGE_SIZE * nLPNOffset));

		nRet = StreamFTL_Write(pstRequest->stRequestID, pstRequest->nLPN, pBuf, IOTYPE_BLOCK_GC, INVALID_INDEX, (INT16)INVALID_INDEX);
		if (nRet == RETURN_SUCCESS)
		{
			GET_BLOCK_GC_INFO(pstRequest->stRequestID.stGC.nMoveInfoIndex)->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteIssued = TRUE;
			pstBlockGC->nWriteLPNCount++;
		}
	}
	else
	{
		ASSERT(0);	// unknown(Not supported) type
	}

	if (nRet == RETURN_SUCCESS)
	{
		FTL_DEBUG_PRINTF("[FTL][WAITQ][%s] %s LPN:%d, pBuf: 0x%X \r\n",
			"bGC or sMerge", 
			((pstRequest->stRequestID.stGC.nType == FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ) ? "Read" : "Write"),
			pstRequest->nLPN, (unsigned int)pstRequest->pMainBuf);

		_RemoveFromRequestWaitQ(g_pstGCRequestInfo, &pstRequest->stCommon);
		_AddToRequestIssuedQ(g_pstGCRequestInfo, &pstRequest->stCommon);				// Issued request will be done by FIL call back

		if (bBufferHit == TRUE)
		{
			// process read done
			FTL_CallBack(pstRequest->stRequestID);
		}
	}
}

static VOID _ProcessHDMARequestIssuedQ()
{
	if (g_pstHILRequestInfo->nHDMACount == 0)
	{
		return;
	}

	HIL_REQUEST*	pstRequest;
	HIL_REQUEST*	pstNextRequest;
	BOOL			bDMADone;

	list_for_each_entry_safe(HIL_REQUEST, pstRequest, pstNextRequest, &g_pstHILRequestInfo->dlHDMA, stCommon.dlList)
	{
		DEBUG_ASSERT(pstRequest->nStatus == HIL_REQUEST_READ_HDMA_ISSUE);
		bDMADone = check_auto_tx_dma_partial_done(pstRequest->nDMAReqTail, pstRequest->nOverFlowCount);
		if (bDMADone == TRUE)
		{
			_RemoveFromHDMAIssuedQ(g_pstHILRequestInfo, &pstRequest->stCommon);
			pstRequest->nStatus++;
			_AddToRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);					// Issued request will be done by FIL call back
		}
	}
}

static BOOL _ProcessHILRequestWaitQ_Write(HIL_REQUEST* pstRequest)
{
	FTL_REQUEST_ID	stReqID = StreamFTL_GetRequestID(pstRequest, IOTYPE_HOST);

	ASSERT(pstRequest->nStatus == HIL_REQUEST_WRITE_WAIT);
	ASSERT(pstRequest->nLPNCount <= REQUEST_LPN_COUNT_MAX);

	RETURN			nRet;

	// allocate buffer
	for (int i = pstRequest->nDoneLPNCount; i < pstRequest->nLPNCount; i++)
	{
		ASSERT(pstRequest->nCmd == NVME_CMD_OPCODE_WRITE);
		ASSERT(pstRequest->nStatus == HIL_REQUEST_WRITE_WAIT);

		nRet = StreamFTL_Write(stReqID, (pstRequest->nLPN + i), NULL, IOTYPE_HOST, pstRequest->nHostCmdSlotTag, pstRequest->nDoneLPNCount);
		if (nRet != RETURN_SUCCESS)
		{
			break;
		}

		pstRequest->nDoneLPNCount++;
	}

	FTL_DEBUG_PRINTF("[FTL][WAITQ][Write] LPN:%d, Count: %d \r\n", pstRequest->nLPN, pstRequest->nLPNCount);

	if (pstRequest->nDoneLPNCount < pstRequest->nLPNCount)
	{
		DEBUG_ASSERT(nRet != RETURN_SUCCESS);
		return FALSE;
	}

	DEBUG_ASSERT(nRet == RETURN_SUCCESS);

	// write buffering done
	_RemoveFromRequestWaitQ(g_pstHILRequestInfo, &pstRequest->stCommon);
	_ReleaseHILRequest(pstRequest);
	return TRUE;
}

static BOOL _ProcessHILRequestWaitQ(VOID)
{
	HIL_REQUEST*	pstRequest;

	if (g_pstHILRequestInfo->nWaitCount == 0)
	{
		DEBUG_ASSERT(list_empty(&g_pstHILRequestInfo->dlWait) == TRUE);
		return FALSE;		// nothing to do
	}

	BOOL	bDone = FALSE;
	BOOL	bSuccess = TRUE;

	pstRequest = list_first_entry(&g_pstHILRequestInfo->dlWait, HIL_REQUEST, stCommon.dlList);

	if (pstRequest->nCmd == NVME_CMD_OPCODE_READ)
	{
		return _ProcessHILRequestWaitQ_Read(pstRequest);
	}
	else
	{
		return _ProcessHILRequestWaitQ_Write(pstRequest);
	}
}

static void _CallBackHILReadRequest(FTL_REQUEST_ID stReqID)
{
	// get request index
	HIL_REQUEST * pstRequest = &g_pstHILRequestPool->astRequest[stReqID.stHIL.nRequestIndex];

	pstRequest->nDoneLPNCount++;

	if (pstRequest->nLPNCount == pstRequest->nDoneLPNCount)		// all LPN read done
	{
		_RemoveFromRequestIssuedQ(g_pstHILRequestInfo, &pstRequest->stCommon);
		_AddToRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);
		pstRequest->nStatus++;
	}
}

static void _ReleaseActiveBlockProgramUnit(UINT32 nActiveBlockIndex, UINT32 nBufferingIndex)
{
	DEBUG_ASSERT(nActiveBlockIndex < (UINT32)g_stGlobal.nActiveBlockCount);
	ACTIVE_BLOCK*	pstActiveBlock = GET_ACTIVE_BLOCK(nActiveBlockIndex);

	DEBUG_ASSERT(nBufferingIndex < ACTIVE_BLOCK_BUFFERING_COUNT);
	PROGRAM_UNIT*	pstProgram = &pstActiveBlock->astBuffering[nBufferingIndex];

	DEBUG_ASSERT(pstProgram->bProgramming == TRUE);

	// update buffering LPN Hash
	for (UINT32 i = 0; i < pstProgram->nLPNCount; i++)
	{
		ActiveBlock_RemoveBufferingLPNHash(pstProgram->anLPN[i]);
	}

	// release buffer
	BufferPool_Release(pstProgram->pstBufferEntry);
	pstProgram->pstBufferEntry = NULL;

	// update active block buffering state
	pstProgram->bFree = TRUE;
	pstProgram->bProgramming = FALSE;
	pstProgram->nLPNCount = 0;
}

static void _CallBackStreamMergeReadRequest(FTL_REQUEST_ID stReqID)
{
	GC_REQUEST*	pstRequest;

	// get request
	pstRequest = &g_pstGCRequestPool->astRequest[stReqID.stGC.nRequestIndex];

	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nType == FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ);

	// Set Read Done
	SMERGE_INFO*	pstSMerge = GET_STREAM_MERGE_INFO(stReqID.stGC.nMoveInfoIndex);

	INT32	nLPNOffset = pstRequest->nLPN - pstSMerge->nStartLPN;

	DEBUG_ASSERT(nLPNOffset < g_stGlobal.nLPagePerPartition);
	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].bValid == TRUE);
	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].nGCRequestIndex != INVALID_INDEX);

	pstSMerge->astCopyInfo[nLPNOffset].bReadDone = TRUE;

	StreamFTL_IssueStreamMergeWrite();
}

static void _CallBackStreamMergeWriteRequest(FTL_REQUEST_ID stReqID)
{
	GC_REQUEST*	pstRequest;

	// get request
	pstRequest = &g_pstGCRequestPool->astRequest[stReqID.stGC.nRequestIndex];

	_RemoveFromRequestIssuedQ(g_pstGCRequestInfo, &pstRequest->stCommon);

	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nType == FTL_REQUEST_SUB_ID_TYPE_STREAM_MERGE_WRITE);
	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nType == stReqID.stGC.nType);
	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nRequestIndex == stReqID.stGC.nRequestIndex);
	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nMoveInfoIndex == stReqID.stGC.nMoveInfoIndex);

	// Set Read Done
	INT32			nMoveInfoIndex = stReqID.stGC.nMoveInfoIndex;
	SMERGE_INFO*	pstSMerge = GET_STREAM_MERGE_INFO(nMoveInfoIndex);
	INT32			nLPNOffset = pstRequest->nLPN - pstSMerge->nStartLPN;

	DEBUG_ASSERT(pstRequest->nLPN >= pstSMerge->nStartLPN);
	DEBUG_ASSERT(nLPNOffset < g_stGlobal.nLPagePerPartition);
	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].bValid == TRUE);
	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].bReadDone == TRUE);
	DEBUG_ASSERT(pstSMerge->astCopyInfo[nLPNOffset].nGCRequestIndex != INVALID_INDEX);

	pstSMerge->astCopyInfo[nLPNOffset].bWriteDone = TRUE;

	DEBUG_ASSERT(GET_GC_MANAGER()->astMoveInfo[nMoveInfoIndex].bFree == FALSE);

	pstSMerge->nWriteDoneLPNCount++;

	DEBUG_ASSERT(pstSMerge->nWriteDoneLPNCount <= g_stGlobal.nLPagePerStream);
	DEBUG_ASSERT(GET_GC_MANAGER()->astMoveInfo[nMoveInfoIndex].bFree == FALSE);

	// Insert to WaitQ for write
	_AddToRequestDoneQ(g_pstGCRequestInfo, &pstRequest->stCommon);

	// check write done
	// check all write is done
	if (pstSMerge->nVPC == pstSMerge->nWriteDoneLPNCount)
	{
		// move operation done, release move infos
		MOVE_INFO*	pstMoveInfo = &GET_GC_MANAGER()->astMoveInfo[nMoveInfoIndex];
		pstMoveInfo->bFree = TRUE;
	}
}

/*
	@brief issue write according to the read issue sequence, to guarantee stream page sequence
*/
VOID StreamFTL_IssueBlockGCWrite(VOID)
{
	BLOCK_GC_INFO*	pstBlockGC = GET_CUR_BLOCK_GC();
	BLOCK_GC_COPY*	pstCurCopyInfo;
	GC_REQUEST*		pstRequest;

	do
	{
		if (pstBlockGC->nWriteIndex == pstBlockGC->nReadIndex)
		{
			// nothing to do
			break;
		}

		// check current write index is ready for write
		pstCurCopyInfo = &pstBlockGC->astCopyInfo[pstBlockGC->nWriteIndex];

		if (pstCurCopyInfo->bReadDone == TRUE)
		{
			DEBUG_ASSERT(pstCurCopyInfo->bWriteIssued == FALSE);
			DEBUG_ASSERT(pstCurCopyInfo->bWriteDone == FALSE);
			DEBUG_ASSERT(pstCurCopyInfo->nGCRequestIndex < GC_REQUEST_COUNT);

			// get request
			pstRequest = &g_pstGCRequestPool->astRequest[pstCurCopyInfo->nGCRequestIndex];

			// Re-insert to WaitQ for write
			pstRequest->stRequestID.stGC.nType = FTL_REQUEST_SUB_ID_TYPE_BLOCK_GC_WRITE;

			_RemoveFromRequestIssuedQ(g_pstGCRequestInfo, &pstRequest->stCommon);
			StreamFTL_AddToRequestWaitQ(g_pstGCRequestInfo, &pstRequest->stCommon);

			pstBlockGC->nWriteIndex = INCREASE_IN_RANGE(pstBlockGC->nWriteIndex, BLOCK_GC_COPY_ENTRY_COUNT);
		}
		else
		{
			// wait read done
			break;
		}
	} while (TRUE);

	// All GC write is issued
	if ((pstBlockGC->nCurReadLPageOffset == g_stGlobal.nLPagesPerVBlock) &&		// Read Done
		(pstBlockGC->nVPC == pstBlockGC->nWriteLPNCount) &&						// Write Done
		(g_pstGCRequestInfo->nWaitCount == 0))									// no wait request (all issued)
	{
		// Stream Merge Done!
		// clear GC type
		g_pstStreamFTL->stGCMgr.eCurGCType = GC_TYPE_NONE;

		// GC active block을 구분한 경우
		//	같은 partition에대한 write가 (1)GC -> (2)user -> (3)GC write 될경우 
		//	(1)에서 할당된 stream이 (3)에서 재사용되어 (2)의 write와 순서가 역전될 수 있다.
		//	그러므로 GC가 끝나면 LPN 정보를 초기화 하여 다시 stream이 할당되도록 한다.
		g_pstStreamFTL->stActiveBlockMgr.pstActiveBlockGC->nRecentLPN = INVALID_LPN;

		GET_GC_MANAGER()->nCurMoveIndex = INCREASE_IN_RANGE(GET_GC_MANAGER()->nCurMoveIndex, GC_MANAGER_MOVE_INFO_COUNT);
	}
}

/*
	@brief	Get LPN from spare buffer and update write information
*/
static VOID _CallBackBlockGCReadRequest(FTL_REQUEST_ID stReqID)
{
	GC_REQUEST*	pstRequest;

	// get request
	pstRequest = &g_pstGCRequestPool->astRequest[stReqID.stGC.nRequestIndex];

	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nType == FTL_REQUEST_ID_TYPE_BLOCK_GC_READ);

	// Set Read Done
	BLOCK_GC_INFO*	pstBlockGC	= GET_BLOCK_GC_INFO(stReqID.stGC.nMoveInfoIndex);

	// Get LPN from spare but current FW does not store LPN to spare area,
	//	(TODO) I need to figure out the layout of spare area, and then store LPN to Spare
#ifdef WIN32
	// Get LPN from spare
	INT32	nLPNOffset = LPN_OFFSET_FROM_VPPN(pstRequest->nVPPN);
	pstRequest->nLPN = ((INT32*)pstRequest->pSpareBuf)[nLPNOffset];

	DEBUG_ASSERT(pstRequest->nLPN < g_stGlobal.nLPNCount);		// Check Spare LPN
#else
	// on target
	pstRequest->nLPN = VNAND_GetLPN(pstRequest->nVPPN);
#endif
	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bReadDone == FALSE);
	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteIssued == FALSE);
	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteDone == FALSE);

	pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bReadDone = TRUE;

	StreamFTL_IssueBlockGCWrite();
}

static void _CallBackBlockGCWriteRequest(FTL_REQUEST_ID stReqID)
{
	GC_REQUEST*	pstRequest;

	// get request
	pstRequest = &g_pstGCRequestPool->astRequest[stReqID.stGC.nRequestIndex];

	DEBUG_ASSERT(pstRequest->stRequestID.stGC.nType == FTL_REQUEST_SUB_ID_TYPE_BLOCK_GC_WRITE);

	// Set Read Done
	BLOCK_GC_INFO*	pstBlockGC= GET_BLOCK_GC_INFO(stReqID.stGC.nMoveInfoIndex);

	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bReadDone == TRUE);
	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteIssued == TRUE);
	DEBUG_ASSERT(pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].nGCRequestIndex < GC_REQUEST_COUNT);

	pstBlockGC->astCopyInfo[pstRequest->nCopyEntryIndex].bWriteDone = TRUE;

	pstBlockGC->nWriteDoneLPNCount++;

	DEBUG_ASSERT(pstBlockGC->nWriteDoneLPNCount <= pstBlockGC->nWriteLPNCount);
	DEBUG_ASSERT(GET_GC_MANAGER()->astMoveInfo[stReqID.stGC.nMoveInfoIndex].bFree == FALSE);

	// Insert to WaitQ for write
	_RemoveFromRequestIssuedQ(g_pstGCRequestInfo, &pstRequest->stCommon);
	_AddToRequestDoneQ(g_pstGCRequestInfo, &pstRequest->stCommon);

	// check all write is done
	if ( pstBlockGC->nVPC == pstBlockGC->nWriteDoneLPNCount )
	{
		// move operation done, release move infos
		MOVE_INFO*	pstMoveInfo = &GET_GC_MANAGER()->astMoveInfo[stReqID.stGC.nMoveInfoIndex];
		pstMoveInfo->bFree = TRUE;
	}
}

static BOOL _ProcessHILRequestWaitQ_Read(HIL_REQUEST* pstRequest)
{
	FTL_REQUEST_ID	stReqID = StreamFTL_GetRequestID(pstRequest, IOTYPE_HOST);

	ASSERT(pstRequest->nStatus == HIL_REQUEST_READ_WAIT);
	ASSERT(pstRequest->nLPNCount <= REQUEST_LPN_COUNT_MAX);

	// check is there enough free buffers
	if (pstRequest->nLPNCount > BufferPool_GetFreeCount())
	{
		// not enough free buffer
		return FALSE;
	}

	BOOL bBufferHit;

	INIT_LIST_HEAD(&pstRequest->dlBuffer);
	BUFFER_ENTRY	*pBufEntry;

	INT32			nVPPN;
	INT32			nLPN;
	// allocate buffer
	for (int i = 0; i < pstRequest->nLPNCount; i++)
	{
		pBufEntry = BufferPool_Allocate();
		list_add_tail(&pBufEntry->dlList, &pstRequest->dlBuffer);

		nLPN = pstRequest->nLPN + i;
		nVPPN = StreamFTL_Read(stReqID, nLPN, pBufEntry->pMainBuf, pBufEntry->pSpareBuf, IOTYPE_HOST, &bBufferHit);
		if (nVPPN == INVALID_PPN)	// unmap read
		{
			FTL_DEBUG_PRINTF("Unmap Read LPN: %d\n\r", nLPN);
			pstRequest->anLPNOffsetOfBuffer[i] = 0;
			pstRequest->nDoneLPNCount++;

#if defined(WIN32) && defined(SUPPORT_DATA_VERIFICATION)
			// Set LPN on read buffer
			((unsigned int*)pBufEntry->pSpareBuf)[0] = DATA_VERIFICATION_INVALID_LPN;
#endif
		}
		else
		{
			if (bBufferHit == TRUE)
			{
				pstRequest->nDoneLPNCount++;
			}

			pstRequest->anLPNOffsetOfBuffer[i] = LPN_OFFSET_FROM_VPPN(nVPPN);
		}
	}

	FTL_DEBUG_PRINTF("[FTL][WAITQ][Read] LPN:%d, Count: %d \r\n", pstRequest->nLPN, pstRequest->nLPNCount);

	pstRequest->nStatus++;

	_RemoveFromRequestWaitQ(g_pstHILRequestInfo, &pstRequest->stCommon);
	if (pstRequest->nLPNCount == pstRequest->nDoneLPNCount)
	{
		pstRequest->nStatus++;
		_AddToRequestDoneQ(g_pstHILRequestInfo, &pstRequest->stCommon);					// Issued request will be done by FIL call back
	}
	else
	{
		_AddToRequestIssuedQ(g_pstHILRequestInfo, &pstRequest->stCommon);				// Issued request will be done by FIL call back
	}

	return TRUE;
}
