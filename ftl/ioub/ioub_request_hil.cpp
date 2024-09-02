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
Kyuhwa Han (hgh6877@gmail.com)
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#include "ioub_internal.h"

///////////////////////////////////////////////////////////////////////////////
//
//	HIL_REQUEST_INFO
//
///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include "test_main.h"
#endif

VOID
HIL_REQUEST_INFO::Initialize(VOID)
{
	INIT_LIST_HEAD(&m_dlFree);
	INIT_LIST_HEAD(&m_dlWait);
	INIT_LIST_HEAD(&m_dlIssued);
	INIT_LIST_HEAD(&m_dlDone);

	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		for (int way = 0; way < USER_WAYS; way++) {
			INIT_LIST_HEAD(&m_dlWait_per_way[channel][way]);

		}
	}
	INIT_LIST_HEAD(&m_dlFree_per_way);
	INIT_LIST_HEAD(&m_dlIssued_per_way);
	INIT_LIST_HEAD(&m_dlDone_per_way);

	m_nFreeCount = 0;
	m_nWaitCount = 0;
	m_nIssuedCount = 0;
	m_nDoneCount = 0;

	for (int i = 0; i < HIL_REQUEST_COUNT; i++)
	{
		INIT_LIST_HEAD(&m_astRequestPool[i].m_dlList);
		m_astRequestPool[i].SetRequestIndex(i);

		ReleaseRequest(&m_astRequestPool[i]);

	}
	for (int i = 0; i < HIL_REQUEST_COUNT; i++)
	{
		INIT_LIST_HEAD(&m_astRequestPerWayPool[i].m_dlList);
		m_astRequestPerWayPool[i].SetRequestIndex(i);

		ReleaseRequest_per_way(&m_astRequestPerWayPool[i]);
	}
}

HIL_REQUEST*
HIL_REQUEST_INFO::AllocateRequest(VOID)
{
	if (m_nFreeCount == 0)
	{
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlFree) == FALSE);

	HIL_REQUEST*	pstRequest;

	pstRequest = list_first_entry(&m_dlFree, HIL_REQUEST, m_dlList);

	list_del_init(&pstRequest->m_dlList);

	m_nFreeCount--;

	DEBUG_ASSERT(pstRequest->GetStatus() == HIL_REQUEST_FREE);

	return pstRequest;
}

HIL_REQUEST_PER_WAY*
HIL_REQUEST_INFO::AllocateRequest_per_way() {
	if (m_dlFreeCount_per_way == 0) {
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlFree_per_way) == FALSE);

	HIL_REQUEST_PER_WAY * pstRequest;
	pstRequest = list_first_entry(&m_dlFree_per_way, HIL_REQUEST_PER_WAY, m_dlList);

	list_del_init(&pstRequest->m_dlList);

	m_dlFreeCount_per_way--;

	DEBUG_ASSERT(pstRequest->GetStatus() == HIL_REQUEST_FREE);

	return pstRequest;
}

VOID
HIL_REQUEST_INFO::AddToWaitQ(HIL_REQUEST* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlWait);
	m_nWaitCount++;

	DEBUG_ASSERT(m_nWaitCount <= HIL_REQUEST_COUNT);

	//if(IOUB_GLOBAL::GetInstance()->SSR_start == TRUE)
	//{
	/*if(pstRequest->m_nCmd == NVME_CMD_OPCODE_WRITE)
	{
		if(pstRequest->GetCurLPN() == ADDR_CLOSE_SECTION)
		{
			static int counter = 0;
			xil_printf("SC-%u\r\n", counter++);
		}
		else if(pstRequest->GetCurLPN() == ADDR_INTERNAL_MERGE)
		{
			static int counter_for_IM = 0;
				xil_printf("SC-%u\r\n", counter_for_IM++);
		}
		else if(pstRequest->GetCurLPN() / 8192 > 360)
		{
			xil_printf("HIL-req: insert: %u %u count: %u\r\n", pstRequest->GetCurLPN() / 8192, pstRequest->GetCurLPN() % 8192, pstRequest->GetLPNCount());
		}
	}*/
}


VOID
HIL_REQUEST_INFO::RemoveFromWaitQ(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(m_nWaitCount > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nWaitCount--;
}

VOID
HIL_REQUEST_INFO::AddToIssuedQ(HIL_REQUEST* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlIssued);
	m_nIssuedCount++;

	DEBUG_ASSERT(m_nIssuedCount <= (MAX(HIL_REQUEST_COUNT, GC_REQUEST_COUNT)));
}

VOID
HIL_REQUEST_INFO::RemoveFromIssuedQ(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(m_nIssuedCount > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nIssuedCount--;
}

VOID
HIL_REQUEST_INFO::AddToDoneQ(HIL_REQUEST* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlDone);
	m_nDoneCount++;

	DEBUG_ASSERT(m_nDoneCount <= HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::RemoveFromDoneQ(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(m_nDoneCount > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nDoneCount--;
}

VOID
HIL_REQUEST_INFO::AddToWaitQ_per_way(HIL_REQUEST_PER_WAY* pstRequest, INT32 channel, INT32 way)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlWait_per_way[channel][way]);
	m_nWaitCount_per_way[channel][way]++;

	pstRequest->SetChannel(channel);
	pstRequest->SetWay(way);

	DEBUG_ASSERT(m_nWaitCount_per_way[channel][way] <= HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::RemoveFromWaitQ_per_way(HIL_REQUEST_PER_WAY* pstRequest, INT32 channel, INT32 way)
{
	DEBUG_ASSERT(m_nWaitCount_per_way[channel][way] > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nWaitCount_per_way[channel][way]--;
}

VOID
HIL_REQUEST_INFO::AddToIssuedQ_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlIssued_per_way);
	m_nIssuedCount_per_way++;

	DEBUG_ASSERT(m_nIssuedCount_per_way <= HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::RemoveFromIssuedQ_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	DEBUG_ASSERT(m_nIssuedCount_per_way > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nIssuedCount_per_way--;
}


VOID
HIL_REQUEST_INFO::AddToDoneQ_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlDone_per_way);
	m_nDoneCount_per_way++;

	DEBUG_ASSERT(m_nDoneCount_per_way <= HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::RemoveFromDoneQ_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	DEBUG_ASSERT(m_nDoneCount_per_way > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nDoneCount_per_way--;
}


HIL_REQUEST*
HIL_REQUEST_INFO::GetWaitRequest_per_way(INT32 channel, INT32 way)
{
	BOOL search_needed = FALSE;
	UINT32 section;
	UINT32 pageoff;
	BOOL is_sc;
	HIL_REQUEST_PER_WAY* temp, *pstCurEntry, *pstNextEntry;
	static HIL_REQUEST_PER_WAY* last_selected[USER_CHANNELS][USER_WAYS];
	if (m_nWaitCount_per_way[channel][way] == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlWait_per_way[channel][way]) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlWait_per_way[channel][way]) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = NULL;

	list_for_each_entry_safe(HIL_REQUEST_PER_WAY, pstCurEntry, pstNextEntry, &m_dlWait_per_way[channel][way], m_dlList)
	{
		if (pstCurEntry->isLPN_valid(IOUB_GLOBAL::GetInstance()->Initialized_for_test) == TRUE)
		{
			pstRequest = pstCurEntry;
			break;
		}
	}
	/*if (pstRequest && pstRequest->GetCmd() == NVME_CMD_OPCODE_READ)
	{
		list_for_each_entry_safe(HIL_REQUEST_PER_WAY, pstCurEntry, pstNextEntry, &m_dlWait_per_way[channel][way], m_dlList)
		{
			if (pstCurEntry->GetCmd() == NVME_CMD_OPCODE_READ)
			{
			}
			else if (pstCurEntry->GetStartLPN() / 4 == pstRequest->GetStartLPN() / 4)
			{
				return pstCurEntry;
			}
		}
	}*/


	return pstRequest;
}

HIL_REQUEST*
HIL_REQUEST_INFO::GetIssuedRequest_per_way()
{
	if (m_nIssuedCount_per_way == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlIssued_per_way) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlIssued_per_way) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlIssued_per_way, HIL_REQUEST, m_dlList);

	return pstRequest;
}

HIL_REQUEST*
HIL_REQUEST_INFO::GetDoneRequest_per_way()
{
	if (m_nDoneCount_per_way == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlDone_per_way) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlDone_per_way) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlDone_per_way, HIL_REQUEST, m_dlList);

	return pstRequest;
}



HIL_REQUEST*
HIL_REQUEST_INFO::GetWaitRequest(VOID)
{
	if (m_nWaitCount == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlWait) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlWait) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlWait, HIL_REQUEST, m_dlList);

	return pstRequest;
}

HIL_REQUEST*
HIL_REQUEST_INFO::GetIssuedRequest(VOID)
{
	if (m_nIssuedCount == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlIssued) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlIssued) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlIssued, HIL_REQUEST, m_dlList);

	return pstRequest;
}

HIL_REQUEST*
HIL_REQUEST_INFO::GetDoneRequest(VOID)
{
	if (m_nDoneCount == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlDone) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlDone) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlDone, HIL_REQUEST, m_dlList);

	return pstRequest;
}

VOID
HIL_REQUEST_INFO::ReleaseRequest(HIL_REQUEST* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlFree);
	m_nFreeCount++;
	DEBUG_ASSERT(m_nFreeCount <= HIL_REQUEST_COUNT);

	pstRequest->SetStatus(HIL_REQUEST_FREE);
	DEBUG_ASSERT(m_nFreeCount <= HIL_REQUEST_COUNT);
	DEBUG_ASSERT(_GetRequestIndex(pstRequest) < HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::ReleaseRequest_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlFree_per_way);
	m_dlFreeCount_per_way++;

	DEBUG_ASSERT(m_dlFreeCount_per_way <= HIL_REQUEST_COUNT);

	pstRequest->SetStatus(HIL_REQUEST_FREE);

	DEBUG_ASSERT(_GetRequestIndex_per_way(pstRequest) < HIL_REQUEST_COUNT);
}

BOOL HIL_REQUEST_INFO::IsFreePerWayReq()
{
	if (m_dlFreeCount_per_way == 0)
		return FALSE;
	else
		return TRUE;
}

INT32
HIL_REQUEST_INFO::_GetRequestIndex(HIL_REQUEST* pstRequest)
{
	return pstRequest - &m_astRequestPool[0];
}

INT32
HIL_REQUEST_INFO::_GetRequestIndex_per_way(HIL_REQUEST_PER_WAY* pstRequest)
{
	return pstRequest - &m_astRequestPerWayPool[0];
}

///////////////////////////////////////////////////////////////////////////////
//
//	HIL REQUEST
//
///////////////////////////////////////////////////////////////////////////////

VOID
HIL_REQUEST::Initialize(HIL_REQUEST_STATUS nStatus, NVME_CMD_OPCODE nCmd,
	UINT32 nLPN, UINT32 nHostCmdSlotTag, INT32 nLPNCount)
{
	m_nStatus = nStatus;
	m_nCmd = nCmd;
	m_nLPN = nLPN;
	m_nHostCmdSlotTag = nHostCmdSlotTag;
	m_nLPNCount = nLPNCount;
	m_nDoneLPNCount = 0;
	m_nIssued_Count = 0;
	type = IOTYPE_COUNT;
	IM_readDone = 0;
	IM_readOffset = 0;

#ifndef WIN32
	//xil_printf("IO Command: type: %u start_lpn: %u count: %u \n\r", nStatus, nLPN, nLPNCount);
#endif
	DEBUG_ASSERT(IOUB_GLOBAL::GetInstance()->IsValidLPN(m_nLPN) == TRUE);
	DEBUG_ASSERT(m_nLPNCount <= MAX_NUM_OF_NLB);
}

BOOL
HIL_REQUEST::Run(VOID)
{
	BOOL bSuccess;
	switch (m_nCmd)
	{
	case NVME_CMD_OPCODE_READ:
		bSuccess = _ProcessRead();
		break;

	case NVME_CMD_OPCODE_WRITE:
		bSuccess = _ProcessWrite();
		break;

	case NVME_CMD_INTERNAL_MERGE:
		bSuccess = _ProcessIM();
		break;

	case NVME_CMD_INTERNAL_MERGE_WRITE:
		bSuccess = _ProcessIM_Write();
		break;
	default:
		ASSERT(0);
		bSuccess = FALSE;
		break;
	}

	return bSuccess;
}

FTL_REQUEST_ID
HIL_REQUEST::_GetRquestID(VOID)
{
	FTL_REQUEST_ID stReqID;

	if (m_nCmd == NVME_CMD_OPCODE_READ)
	{
		stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_HIL_READ;
	}
	else if (m_nCmd == NVME_CMD_OPCODE_WRITE)
	{
		stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_WRITE;	// this type will be stored request id of PROGRAM_UNIT
	}
	else
	{
		ASSERT(0);	// Not implemented yet
	}

	stReqID.stHIL.nRequestIndex = m_nRequestIndex;
	DEBUG_ASSERT(stReqID.stHIL.nRequestIndex < HIL_REQUEST_COUNT);

	return stReqID;
}

BOOL
HIL_REQUEST::_ProcessRead(VOID)
{
	BOOL	bSuccess;

	switch (m_nStatus)
	{
	case HIL_REQUEST_READ_WAIT:
		bSuccess = _ProcessRead_Wait();
		break;
	case HIL_REQUEST_READ_NAND_ISSUED:
		bSuccess = _ProcessRead_Issued();
		break;
	case HIL_REQUEST_READ_DONE:
		bSuccess = _ProcessRead_Done();
		break;

	default:
		ASSERT(0);
		break;
	};

	return bSuccess;
}

/*
@brief		check L2P metadata is available for start and end LPN
a meta page(4KB) is enough to have mapping data
for the maximun size(REQUEST_LPN_COUNT_MAX) of request
*/
BOOL
HIL_REQUEST::_CheckAndLoadMeta(VOID)
{
	return TRUE;
}

BOOL
HIL_REQUEST::_CheckMetaWritable(VOID)
{
	return TRUE;
}


BOOL
HIL_REQUEST::_IsReadable(VOID)
{
	BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();

	// check is there enough free buffers
	if (m_nLPNCount > pstBufferMgr->GetFreeCount())
	{
		// not enough free buffer
		return FALSE;
	}

	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessRead_Wait(VOID)
{
	ASSERT(m_nStatus == HIL_REQUEST_READ_WAIT);
	ASSERT(m_nLPNCount <= REQUEST_LPN_COUNT_MAX);

	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();


	HIL_REQUEST_PER_WAY *pstRequest_per_way;

	UINT32			nLPN;
	UINT32			last_ch, last_wy;
	UINT32			lpn_count = 0;

#if (IOUB_STRIPING == 1)
	UINT32			nLPPN;
#endif
	if (type == IOTYPE_COUNT) {
		if (IOUB_GLOBAL::isBlockMappingLPN(m_nLPN))
			type = IOTYPE_BLOCK;
		else
			type = IOTYPE_PAGE;
	}

	nLPN = m_nLPN + m_nIssued_Count;
	//STRIPING POLICY
#if (IOUB_STRIPING == 1)
	nLPPN = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(nLPN);
	nLPN = nLPPN + lpn_count;
#endif

	last_ch = get_channel_from_lpn(nLPN);
	last_wy = get_way_from_lpn(nLPN);



	for (int i = m_nIssued_Count; i < m_nLPNCount; i++) {
		UINT32 channel, way;

		//STRIPING POLICY
#if (IOUB_STRIPING == 1)
		nLPN = nLPPN + lpn_count;
#else
		nLPN = m_nLPN + i;
#endif
		////////////////////

		channel = get_channel_from_lpn(nLPN);
		way = get_way_from_lpn(nLPN);

		if (last_ch == channel && last_wy == way) {
		}
		else {

			pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

			if (!pstRequest_per_way) {
				return FALSE;
			}
			
			//STRIPING POLICY
#if (IOUB_STRIPING == 1)
			nLPN = nLPPN;
#else
			nLPN = m_nLPN + m_nIssued_Count;
#endif
			
			////////////////////
			pstRequest_per_way->Initialize_per_way(HIL_REQUEST_READ_WAIT, NVME_CMD_OPCODE_READ,
				nLPN, m_nHostCmdSlotTag, lpn_count, this, type, 0);

			DEBUG_ASSERT(lpn_count <= LPN_PER_PHYSICAL_PAGE);
			pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
			for (UINT32 j = 0; j < lpn_count; j++) {
				IncIssued_count();
			}
			lpn_count = 0;

#if (IOUB_STRIPING==1)
			nLPN = m_nLPN + m_nIssued_Count;
			nLPPN = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(nLPN);
			channel = get_channel_from_lpn(nLPPN);
			way = get_way_from_lpn(nLPPN);
#endif
		}

		lpn_count++;
		last_ch = channel;
		last_wy = way;
	}

	pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

	if (pstRequest_per_way) {
		

		//STRIPING POLICY
#if (IOUB_STRIPING == 1)
		nLPN = nLPPN;
#else
		nLPN = m_nLPN + m_nIssued_Count;
#endif
		////////////////////

		pstRequest_per_way->Initialize_per_way(HIL_REQUEST_READ_WAIT, NVME_CMD_OPCODE_READ,
			nLPN, m_nHostCmdSlotTag, lpn_count, this, type, 0);

		DEBUG_ASSERT(lpn_count <= LPN_PER_PHYSICAL_PAGE);
		pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
		for (UINT32 j = 0; j < lpn_count; j++) {
			IncIssued_count();
		}
	}
	else {
		return FALSE;
	}


	GoToNextStatus();
	pstHILRequestInfo->RemoveFromWaitQ(this);
	pstHILRequestInfo->AddToIssuedQ(this);	// Issued request will be done by FIL call back

	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessRead_Issued(VOID) {
	return FALSE;
}

BOOL
HIL_REQUEST::_ProcessRead_Done(VOID)
{
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessWrite(VOID)
{
	BOOL	bSuccess;

	switch (m_nStatus)
	{
	case HIL_REQUEST_WRITE_WAIT:
		bSuccess = _ProcessWrite_Wait();
		break;

	case HIL_REQUEST_WRITE_ISSUED:
		bSuccess = _ProcessWrite_Issued();
		break;

	case HIL_REQUEST_WRITE_DONE:
		bSuccess = _ProcessWrite_Done();
		break;

	default:
		ASSERT(0);
		break;
	};

	return bSuccess;
}

BOOL
HIL_REQUEST::_IsWritable(UINT32 channel, UINT32 way)
{
	if (IOUB_GLOBAL::GetGCMgr(channel, way)->IsGCRunning() == TRUE)
	{
		return FALSE;
	}

	// check buffer available
	if (IOUB_GLOBAL::GetBufferMgr()->GetFreeCount() == 0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessWrite_Wait(VOID)
{
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();
	HIL_REQUEST_PER_WAY * pstRequest_per_way;
	int last_ch, last_wy;
	int lpn_count = 0;
	UINT32 nLPN;
#if (IOUB_STRIPING == 1)
	UINT32			nLPPN;
#endif

#if (IOUB_STRIPING == 0)
	if (m_nLPN == ADDR_CLOSE_SECTION || m_nLPN == ADDR_INTERNAL_MERGE) {
		BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];

		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
		UINT32 max_need_req;
		INIT_LIST_HEAD(&m_dlBuffer);
		if (m_nLPN == ADDR_CLOSE_SECTION)
			max_need_req = 1 << (NUM_BIT_WAY + CHANNEL_BITS);
		else
			max_need_req = 512;
		if (pstHILRequestInfo->GetFreeCount_per_way() < max_need_req)
			return FALSE;


		pBufEntry[0] = pstBufferMgr->Allocate();
		if (!pBufEntry[0]) {
			return FALSE;
		}

		list_add_tail(&pBufEntry[0]->m_dlList, &m_dlBuffer);

		HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
		pstHDMA->IssueRxDMA(GetHostCmdSlotTag(), 0, (unsigned int)pBufEntry[0]->m_pMainBuf);
		SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());

#ifdef WIN32
		unsigned int **dp_ptr = &TC_WRITE_BUFFER;

		unsigned int temp = (unsigned int)dp_ptr;
		dp_ptr = (unsigned int**)((temp & 0xfffff000) | 0xd84);
		unsigned int *temp_ptr = *dp_ptr;
		memcpy(pBufEntry[0]->m_pMainBuf, temp_ptr, 4096);

#endif

		IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(m_nDMAReqTail, m_nDMAOverFlowCount);
		if (m_nLPN == ADDR_CLOSE_SECTION) {
			if(((UINT32*)pBufEntry[0]->m_pMainBuf)[0] != 2)
				IOUB_IncreaseProfile(Prof_SC_count);
			else 
				IOUB_IncreaseProfile(Prof_SC_IM_count);
		}
		else if (m_nLPN == ADDR_INTERNAL_MERGE) {
			IOUB_IncreaseProfile(Prof_IM_count);
		}
		/*if(m_nLPN == ADDR_CLOSE_SECTION) {
		xil_printf("SC - %u %u %u\r\n", ((unsigned int*)pBufEntry[0]->m_pMainBuf)[0], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[1], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[2]);
		IOUB_GLOBAL::GetActiveBlockMgr(0, 0)->printActiveBlock();
		}
		else if(m_nLPN == ADDR_INTERNAL_MERGE) {
		xil_printf("IM - %u %u \r\n", ((unsigned int*)pBufEntry[0]->m_pMainBuf)[0], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[1]);
		
		}*/
		if (m_nLPN == ADDR_CLOSE_SECTION) {
			for (INT32 channel = 0; channel < USER_CHANNELS; channel++)
			{
				for (INT32 way = 0; way < USER_WAYS; way++)
				{
					pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

					if (!pstRequest_per_way) {
						ASSERT(0);
					}
					pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
						m_nLPN, m_nHostCmdSlotTag, 1, this, IOTYPE_BLOCK, 0);

					pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, channel, way);
					pstRequest_per_way->SetBufEntry(pBufEntry);
				}
			}
		}
		else
		{
			IOUB_GLOBAL::GetInstance()->IM_start = TRUE;
			UINT32 padding_count = ((UINT32*)pBufEntry[0]->m_pMainBuf)[512];

			//dummy page write
			if(padding_count != 0)
			{
				UINT32 dst_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[513] - padding_count;
				UINT32 dst_ch, dst_wy;
				dst_ch = get_channel_from_lpn(dst_lpn);
				dst_wy = get_way_from_lpn(dst_lpn);

				pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

				if (!pstRequest_per_way) {
					ASSERT(0);
				}
				pstRequest_per_way->Initialize_per_way(HIL_REQUEST_IM_WAIT, NVME_CMD_INTERNAL_MERGE,
					dst_lpn, m_nHostCmdSlotTag, padding_count, this, IOTYPE_BLOCK, 0);

				pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, dst_ch, dst_wy);

				IOUB_IncreaseProfile(Prof_IM_paddings, padding_count);
				//xil_printf("padidng page : %u/%u - %u-%u\r\n", dst_ch, dst_wy, dst_lpn, ((UINT32*)pBufEntry[0]->m_pMainBuf)[512]);
			}

			while (1)
			{
				if (IOUB_MAX_IM_CNT == IM_readOffset)
				{
					pstHILRequestInfo->RemoveFromWaitQ(this);
					pstHILRequestInfo->ReleaseRequest(this);

					BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
					pstBufferMgr->Release(pBufEntry[0]);
					return TRUE;
				}

				//intmg one page
				UINT32 src_lpn, dst_lpn;
				UINT32 dst_lbn;
				UINT32 dst_ch, dst_wy;
				BOOL ret_val = TRUE;
				src_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[IM_readOffset];

				dst_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[513 + (IM_readOffset >> NUM_BIT_LPN_PER_PAGE)];

				
				if (NULL == src_lpn)
				{
					REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
					HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

					//move queue
					pstHILRequestInfo->RemoveFromWaitQ(this);
					pstHILRequestInfo->ReleaseRequest(this);

					BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
					pstBufferMgr->Release(pBufEntry[0]);
					return TRUE;
				}

				dst_ch = get_channel_from_lpn(dst_lpn);
				dst_wy = get_way_from_lpn(dst_lpn);

				pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

				if (!pstRequest_per_way) {
					ASSERT(0);
				}
				pstRequest_per_way->Initialize_per_way(HIL_REQUEST_IM_WAIT, NVME_CMD_INTERNAL_MERGE,
					dst_lpn, m_nHostCmdSlotTag, 1, this, IOTYPE_BLOCK, src_lpn);

				pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, dst_ch, dst_wy);

				IM_readOffset += LPN_PER_PHYSICAL_PAGE;
			}
		}
Success_exit:
		GoToNextStatus();

		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromWaitQ(this);
		pstHILRequestInfo->AddToIssuedQ(this);

		m_nLPNCount = m_nIssued_Count = USER_CHANNELS * USER_WAYS;

		return TRUE;

	}
#else
	if (m_nLPN == ADDR_CLOSE_SECTION || m_nLPN == ADDR_INTERNAL_MERGE || m_nLPN == ADDR_SET_STRIPING) {
		BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];

		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
		UINT32 max_need_req;
		INIT_LIST_HEAD(&m_dlBuffer);
		if (m_nLPN == ADDR_CLOSE_SECTION)
			max_need_req = 1 << (NUM_BIT_WAY + CHANNEL_BITS);
		else if (m_nLPN == ADDR_SET_STRIPING)
			max_need_req = 1;
		else
			max_need_req = 512;
		if (pstHILRequestInfo->GetFreeCount_per_way() < max_need_req)
			return FALSE;


		pBufEntry[0] = pstBufferMgr->Allocate();
		if (!pBufEntry[0]) {
			return FALSE;
		}

		list_add_tail(&pBufEntry[0]->m_dlList, &m_dlBuffer);

		HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
		pstHDMA->IssueRxDMA(GetHostCmdSlotTag(), 0, (unsigned int)pBufEntry[0]->m_pMainBuf);
		SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());

#ifdef WIN32
		unsigned int **dp_ptr = &TC_WRITE_BUFFER;

		unsigned int temp = (unsigned int)dp_ptr;
		dp_ptr = (unsigned int**)((temp & 0xfffff000) | 0xc7c);
		unsigned int *temp_ptr = *dp_ptr;
		memcpy(pBufEntry[0]->m_pMainBuf, temp_ptr, 4096);

#endif

		IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(m_nDMAReqTail, m_nDMAOverFlowCount);
		if (m_nLPN == ADDR_CLOSE_SECTION) {
			if (((UINT32*)pBufEntry[0]->m_pMainBuf)[0] != 2)
				IOUB_IncreaseProfile(Prof_SC_count);
			else
				IOUB_IncreaseProfile(Prof_SC_IM_count);
		}
		else if (m_nLPN == ADDR_INTERNAL_MERGE) {
			IOUB_IncreaseProfile(Prof_IM_count);
		}
		/*if(m_nLPN == ADDR_CLOSE_SECTION) {
		xil_printf("SC - %u %u %u\r\n", ((unsigned int*)pBufEntry[0]->m_pMainBuf)[0], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[1], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[2]);
		IOUB_GLOBAL::GetActiveBlockMgr(0, 0)->printActiveBlock();
		}
		else if(m_nLPN == ADDR_INTERNAL_MERGE) {
		xil_printf("IM - %u %u \r\n", ((unsigned int*)pBufEntry[0]->m_pMainBuf)[0], ((unsigned int*)pBufEntry[0]->m_pMainBuf)[1]);

		}*/
		if (m_nLPN == ADDR_SET_STRIPING)
		{
			UINT32 start_segno, count, ch, wy, blk, lsn;
			start_segno = ((UINT32*)pBufEntry[0]->m_pMainBuf)[0];
			count = ((UINT32*)pBufEntry[0]->m_pMainBuf)[1];
			for (UINT32 iter = 0; iter < count; iter++)
			{
				wy = ((UINT32*)pBufEntry[0]->m_pMainBuf)[2 + iter];
				ch = wy % USER_CHANNELS;
				wy = wy >> CHANNEL_BITS;
				lsn = IOUB_GLOBAL::GetStrinpingMgr()->alloc_and_get_segment(ch, wy);
				ASSERT(IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(start_segno + iter) == 0xffffffff);
				IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(start_segno + iter, lsn);

			}

			pstHILRequestInfo->RemoveFromWaitQ(this);
			pstHILRequestInfo->ReleaseRequest(this);

			BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
			pstBufferMgr->Release(pBufEntry[0]);
			return TRUE;
		}
		else if (m_nLPN == ADDR_FREE_STRIPING)
		{
			UINT32 start_segno, count, ch, wy, blk, lsn;
			start_segno = ((UINT32*)pBufEntry[0]->m_pMainBuf)[0];
			count = ((UINT32*)pBufEntry[0]->m_pMainBuf)[1];
			for (UINT32 iter = 0; iter < count; iter++)
			{
				lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(start_segno + iter);

				ch = get_channel_from_lsn(lsn);
				wy = get_way_from_lsn(lsn);
				blk = get_lbn_from_lsn(lsn);
				IOUB_GLOBAL::GetStrinpingMgr()->release_vsn(ch, wy, blk);
			}

			pstHILRequestInfo->RemoveFromWaitQ(this);
			pstHILRequestInfo->ReleaseRequest(this);

			BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
			pstBufferMgr->Release(pBufEntry[0]);
			return TRUE;
		}
		else if (m_nLPN == ADDR_CLOSE_SECTION) {
			UINT32 lsn, striping_offset;
			UINT32 segs_in_striping_groups;
			UINT32 src_lbn, dst_lbn;
			UINT32 src_wy, dst_wy;
			UINT32 channel, way;

			src_lbn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[2];
			dst_lbn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[3];

			if (src_lbn != 0)
			{
				src_lbn = (src_lbn + IOUB_BLOCK_START_LBN) << NUM_BIT_SEGS_PER_SECTION;
			}
			dst_lbn = (dst_lbn + IOUB_BLOCK_START_LBN) << NUM_BIT_SEGS_PER_SECTION;

			ASSERT(src_lbn % 16 == dst_lbn % 16);

			for (UINT32 iter = 0; iter < SEGS_PER_SECTION; iter++)
			{			
				lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(dst_lbn + iter);
				striping_offset = lsn & STRIPING_OFFSET_MASK;

				ASSERT(striping_offset == iter);

				segs_in_striping_groups = get_secsize_from_lsn(lsn) + 1;

				ASSERT(segs_in_striping_groups == SEGS_PER_SECTION);

				channel = get_channel_from_lsn(lsn);
				way = get_way_from_lsn(lsn);

				pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

				if (!pstRequest_per_way) {
					ASSERT(0);
				}
				pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
					m_nLPN, m_nHostCmdSlotTag, 1, this, IOTYPE_BLOCK, 0);

				pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, channel, way);
				pstRequest_per_way->SetBufEntry(pBufEntry);
			}
		}
		else
		{
			IOUB_GLOBAL::GetInstance()->IM_start = TRUE;
			UINT32 padding_count = ((UINT32*)pBufEntry[0]->m_pMainBuf)[512];

			//dummy page write
			if (padding_count != 0)
			{
				UINT32 dst_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[513] - padding_count;
				UINT32 dst_ch, dst_wy;

				dst_lpn = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(dst_lpn);

				dst_ch = get_channel_from_lpn(dst_lpn);
				dst_wy = get_way_from_lpn(dst_lpn);

				pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

				if (!pstRequest_per_way) {
					ASSERT(0);
				}
				pstRequest_per_way->Initialize_per_way(HIL_REQUEST_IM_WAIT, NVME_CMD_INTERNAL_MERGE,
					dst_lpn, m_nHostCmdSlotTag, padding_count, this, IOTYPE_BLOCK, 0);

				pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, dst_ch, dst_wy);

				IOUB_IncreaseProfile(Prof_IM_paddings, padding_count);
				//xil_printf("padidng page : %u/%u - %u-%u\r\n", dst_ch, dst_wy, dst_lpn, ((UINT32*)pBufEntry[0]->m_pMainBuf)[512]);
			}

			while (1)
			{
				if (IOUB_MAX_IM_CNT == IM_readOffset)
				{
					pstHILRequestInfo->RemoveFromWaitQ(this);
					pstHILRequestInfo->ReleaseRequest(this);

					BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
					pstBufferMgr->Release(pBufEntry[0]);
					return TRUE;
				}

				//intmg one page
				UINT32 src_lpn, dst_lpn;
				UINT32 dst_lbn;
				UINT32 dst_ch, dst_wy;
				BOOL ret_val = TRUE;
				src_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[IM_readOffset];

				dst_lpn = ((UINT32*)pBufEntry[0]->m_pMainBuf)[513 + (IM_readOffset >> NUM_BIT_LPN_PER_PAGE)];


				if (NULL == src_lpn)
				{
					REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
					HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

					//move queue
					pstHILRequestInfo->RemoveFromWaitQ(this);
					pstHILRequestInfo->ReleaseRequest(this);

					BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
					pstBufferMgr->Release(pBufEntry[0]);
					return TRUE;
				}

				src_lpn = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(src_lpn);
				dst_lpn = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(dst_lpn);

				dst_ch = get_channel_from_lpn(dst_lpn);
				dst_wy = get_way_from_lpn(dst_lpn);

				pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

				if (!pstRequest_per_way) {
					ASSERT(0);
				}
				pstRequest_per_way->Initialize_per_way(HIL_REQUEST_IM_WAIT, NVME_CMD_INTERNAL_MERGE,
					dst_lpn, m_nHostCmdSlotTag, 1, this, IOTYPE_BLOCK, src_lpn);

				pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, dst_ch, dst_wy);

				IM_readOffset += LPN_PER_PHYSICAL_PAGE;
			}
		}
	Success_exit:
		GoToNextStatus();

		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromWaitQ(this);
		pstHILRequestInfo->AddToIssuedQ(this);

		m_nLPNCount = m_nIssued_Count = USER_CHANNELS * USER_WAYS;

		return TRUE;

	}
#endif
	if (type == IOTYPE_COUNT) {
		if (IOUB_GLOBAL::isBlockMappingLPN(m_nLPN))
			type = IOTYPE_BLOCK;
		else
			type = IOTYPE_PAGE;
	}

	nLPN = m_nLPN + m_nIssued_Count;
	//STRIPING POLICY
#if (IOUB_STRIPING == 1)
	if (!IOUB_GLOBAL::GetStrinpingMgr()->isalloced(nLPN))
		return FALSE;
	nLPPN = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(nLPN);
	nLPN = nLPPN + lpn_count;
#endif
	////////////////////

	last_ch = get_channel_from_lpn(nLPN);
	last_wy = get_way_from_lpn(nLPN);
	for (int i = m_nIssued_Count; i < m_nLPNCount; i++) {
		INT32 channel, way;

		//STRIPING POLICY
#if (IOUB_STRIPING == 1)
		nLPN = nLPPN + lpn_count;
#else
		nLPN = m_nLPN + i;
#endif
		////////////////////

		channel = get_channel_from_lpn(nLPN);
		way = get_way_from_lpn(nLPN);

		if (last_ch == channel && last_wy == way)
		{
			lpn_count++;
		}
		else {
			pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

			if (!pstRequest_per_way) {
				break;
			}
			//STRIPING POLICY
#if (IOUB_STRIPING == 1)
			nLPN = nLPPN;

#else
			nLPN = m_nLPN + m_nIssued_Count;
#endif
			////////////////////
		
			pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
				nLPN, m_nHostCmdSlotTag, lpn_count, this, type, 0);

			DEBUG_ASSERT(lpn_count <= LPN_PER_PHYSICAL_PAGE);
			pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
			for (int j = 0; j < lpn_count; j++) {

				IncIssued_count();
			}
			lpn_count = 1;

#if (IOUB_STRIPING==1)
			nLPN = m_nLPN + m_nIssued_Count;
			nLPPN = IOUB_GLOBAL::GetStrinpingMgr()->get_striped_lpn(nLPN);
			channel = get_channel_from_lpn(nLPPN);
			way = get_way_from_lpn(nLPPN);
#endif
		}
		last_ch = channel;
		last_wy = way;

	}

	pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

	if (pstRequest_per_way) {
		//STRIPING POLICY
#if (IOUB_STRIPING == 1)
		nLPN = nLPPN;
#else
		nLPN = m_nLPN + m_nIssued_Count;
#endif
		////////////////////

		pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
			nLPN, m_nHostCmdSlotTag, lpn_count, this, type, 0);

		DEBUG_ASSERT(lpn_count <= LPN_PER_PHYSICAL_PAGE);
		pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
		for (int j = 0; j < lpn_count; j++) {

			IncIssued_count();
		}
	}


	if (m_nIssued_Count == m_nLPNCount)
	{
		DEBUG_ASSERT(m_nLPNCount == m_nIssued_Count);
		GoToNextStatus();

		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromWaitQ(this);
		pstHILRequestInfo->AddToIssuedQ(this);
		return TRUE;
	}

	return FALSE;
}

BOOL
HIL_REQUEST::_ProcessWrite_Issued(VOID)
{
	if (GetLPNCount() == GetDoneCount()) {
		GoToNextStatus();

		//Remove from IssuedQ
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromIssuedQ(this);
		pstHILRequestInfo->AddToDoneQ(this);
		return TRUE;
	}
	return FALSE;
}

BOOL
HIL_REQUEST::_ProcessWrite_Done(VOID)
{
	// Release Request
	//Remove from DoneQ
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	//move queue
	pstHILRequestInfo->RemoveFromDoneQ(this);

	pstHILRequestInfo->ReleaseRequest(this);

	//xil_printf("End	%u	%u  - tag: %u\r\n", m_nLPN, m_nLPNCount, m_nHostCmdSlotTag);
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM(VOID)
{
	BOOL	bSuccess;

	switch (m_nStatus)
	{
	case HIL_REQUEST_IM_WAIT:
		bSuccess = _ProcessIM_Wait();
		break;

	case HIL_REQUEST_IM_ISSUED:
		bSuccess = _ProcessIM_Issued();
		break;

	case HIL_REQUEST_IM_DONE:
		bSuccess = _ProcessIM_Done();
		break;

	default:
		ASSERT(0);
		break;
	};

	return bSuccess;
}

BOOL
HIL_REQUEST::_ProcessIM_Wait(VOID) {
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM_Issued(VOID) {
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM_Done(VOID)
{
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM_Write(VOID)
{
	BOOL	bSuccess;

	switch (m_nStatus)
	{
	case HIL_REQUEST_IM_WRITE_WAIT:
		bSuccess = _ProcessIM_Write_Wait();
		break;

	case HIL_REQUEST_IM_WRITE_ISSUED:
		bSuccess = _ProcessIM_Write_Issued();
		break;

	case HIL_REQUEST_IM_WRITE_DONE:
		bSuccess = _ProcessIM_Write_Done();
		break;

	default:
		ASSERT(0);
		break;
	};

	return bSuccess;
}

BOOL
HIL_REQUEST::_ProcessIM_Write_Wait(VOID) {
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM_Write_Issued(VOID) {
	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessIM_Write_Done(VOID)
{
	return TRUE;
}


VOID HIL_REQUEST_PER_WAY::Initialize_per_way(HIL_REQUEST_STATUS nStatus, NVME_CMD_OPCODE nCmd, UINT32 nLPN, UINT32 nHostCmdSlotTag, INT32 nLPNCount, HIL_REQUEST * parent, IOTYPE input_type, UINT32 src_lpn)
{
	Initialize(nStatus, nCmd, nLPN, nHostCmdSlotTag, nLPNCount);

	if(parent != NULL)
		m_nDMA_offset = parent->GetIssued_count();
	m_parent_req = parent;
	intmg_iter = 0;
	IM_src_lpn = src_lpn;
	IM_first_success = 0;
	read_issue = FALSE;
	type = input_type;
	for (int i = 0; i < LPN_PER_PHYSICAL_PAGE; i++) {
		read_VPPN[i] = 0xffffffff;
		pBufEntry[i] = NULL;
	}
	//if(nLPN / 8192 > 1894)
		//xil_printf("HIL-req-per-way: insert: %u %u   count: %u\r\n", nLPN / 8192, nLPN % 8192, nLPNCount);
	if(IOUB_GLOBAL::GetInstance()->IM_start == TRUE)
	{
		if(nCmd == NVME_CMD_OPCODE_WRITE)
		{
			if(nLPN == ADDR_CLOSE_SECTION)
			{
				/*
				xil_printf("HIL-req-per-way: SC - %u-%u  %u-> %u\r\n",
						((UINT32*)pBufEntry[0]->m_pMainBuf)[0],
						((UINT32*)pBufEntry[0]->m_pMainBuf)[1],
						((UINT32*)pBufEntry[0]->m_pMainBuf)[2],
						((UINT32*)pBufEntry[0]->m_pMainBuf)[3]);
						*/
			}
			else
			{
				//xil_printf("HIL-req-per-way: insert: %u %u   count: %u\r\n", nLPN / 8192, nLPN % 8192, nLPNCount);
			}
		}
	}
}



BOOL
HIL_REQUEST_PER_WAY::_ProcessRead_Wait(VOID)
{

	UINT32			nLPN;
	UINT32			nBufferingVPPN;
	UINT32			nVPPN;
	UINT32			channel;
	UINT32			way;
	UINT32			LPNOffset;

	BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
	FTL_REQUEST_ID stReqID = _GetRquestID();
	stReqID.channel = m_channel;
	stReqID.way = m_way;


	if (GetLPNCount()  > pstBufferMgr->GetFreeCount())
	{
		// not enough free buffer
		return FALSE;
	}
	

	if (pBufEntry[0] == NULL && type == IOTYPE_BLOCK) {
		INIT_LIST_HEAD(&m_dlBuffer);
		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();

		//buffer allocation
		pBufEntry[0] = pstBufferMgr->Allocate();

		if (!pBufEntry[0]) {

			ASSERT(0);

			return FALSE;

		}
		list_add_tail(&pBufEntry[0]->m_dlList, &m_dlBuffer);

		pBufEntry[1] = pstBufferMgr->Allocate();

		if (!pBufEntry[1]) {
			pstBufferMgr->Release(pBufEntry[0]);
			pBufEntry[0] = NULL;
			ASSERT(0);

			return FALSE;

		}
		list_add_tail(&pBufEntry[1]->m_dlList, &m_dlBuffer);
	}
	else if (pBufEntry[0] == NULL) {
		INIT_LIST_HEAD(&m_dlBuffer);
		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();

		for (INT32 i = 0; i < GetLPNCount(); i++) {
			pBufEntry[i] = pstBufferMgr->Allocate();

			if (!pBufEntry[i]) {
				ASSERT(0);
				return FALSE;
			}
			list_add_tail(&pBufEntry[i]->m_dlList, &m_dlBuffer);

		}

	}

	for (int i = GetIssued_count(); i < GetLPNCount(); i++)
	{
		nLPN = GetStartLPN() + i;
		LPNOffset = nLPN % LPN_PER_PHYSICAL_PAGE;
		channel = get_channel_from_lpn(nLPN);
		way = get_way_from_lpn(nLPN);

		// GetPPN
		if (type == IOTYPE_BLOCK) {
			nVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V(nLPN);
		}
		else {
			nVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_PAGE)->GetL2V(nLPN);
		}

		if (nVPPN == INVALID_PPN)	// unmap read
		{
			UINT32 buf_offset;
			if (type == IOTYPE_BLOCK) {
				buf_offset = 0;
				BufOffset[i] = (INT8)0;
				pBufEntry[0]->nVPPN = nVPPN;
			}
			else {
				buf_offset = i;
				LPNOffset = 0;
				BufOffset[i] = i * LPN_PER_PHYSICAL_PAGE;
				pBufEntry[i]->nVPPN = nVPPN;
			}
			FTL_DEBUG_PRINTF("Unmap Read LPN: %d\n\r", nLPN);


			IncreaseDoneCount();

			IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_UNMAP_READ);

			OSAL_MEMSET((unsigned int*)((UINT32)pBufEntry[buf_offset]->m_pMainBuf + LPNOffset * LOGICAL_PAGE_SIZE), 0xffffffff, LOGICAL_PAGE_SIZE);
#if defined(WIN32) && defined(SUPPORT_DATA_VERIFICATION)
			// Set LPN on read buffer
			((unsigned int*)pBufEntry[buf_offset]->m_pSpareBuf)[LPNOffset] = DATA_VERIFICATION_INVALID_LPN;
#endif
		}
		/*else if (nLPN / 8192 == 7)
		{
			UINT32 buf_offset;
			if (type == IOTYPE_BLOCK) {
				buf_offset = 0;
				BufOffset[i] = (INT8)0;
				pBufEntry[0]->nVPPN = INVALID_PPN;
			}
			else {
				buf_offset = i;
				LPNOffset = 0;
				BufOffset[i] = i * LPN_PER_PHYSICAL_PAGE;
				pBufEntry[i]->nVPPN = INVALID_PPN;
			}

			IncreaseDoneCount();

			IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_UNMAP_READ);
		}*/
		else
		{
			BUFFER_ENTRY* ReadCacheBuf;
			ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->get_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
			if (ReadCacheBuf == NULL) {
			}
			else {
				if (ReadCacheBuf->readDone == 0)
					return false;
				if (ReadCacheBuf->readtype == 1)
				{
					if(((UINT32*)ReadCacheBuf->m_pSpareBuf)[LPN_OFFSET_FROM_VPPN(nVPPN)] != nLPN)
					{
						if (ReadCacheBuf->refCount == 0)
						{
							IOUB_GLOBAL::GetReadCacheMgr()->free_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
							goto skip_readbuf;
						}
						else
						{
							BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(channel, way, type);
							nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, ReadCacheBuf);
							ASSERT(nVPPN == nBufferingVPPN);
							return false;
						}
					}
				}

				ReadCacheBuf->refCount++;
				IncreaseDoneCount();
				IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ);

				if (type == IOTYPE_BLOCK) {
					ACTIVE_BLOCK *pstActiveBlock;
					UINT32 lbn = get_lbn_from_lpn(GetCurLPN());
					pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(lbn, IOTYPE_BLOCK);
					if (pstActiveBlock == NULL)
					{
						pBufEntry[0]->nVPPN = nVPPN;
						(BufOffset[i]) = 0;
					}
					else if (PBN_FROM_VPPN(nVPPN) == pstActiveBlock->m_nVBN)
					{
						pBufEntry[1]->nVPPN = nVPPN;
						(BufOffset[i]) = 1;
					}
					else
					{
						pBufEntry[0]->nVPPN = nVPPN;
						(BufOffset[i]) = 0;
					}

				}
				else
				{
					pBufEntry[i]->nVPPN = nVPPN;
					(BufOffset[i]) = i * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);
				}
				goto loop_end;
				continue;
			}
skip_readbuf:

			// Check buffering 
			BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(channel, way, type);


			if (type == IOTYPE_PAGE) {
				(BufOffset[i]) = i * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);
				nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, pBufEntry[i]);
			}
			else {
				nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, pBufEntry[1]);
			}

			if (nBufferingVPPN != INVALID_VPPN)
			{
				UINT32 bufoffset;
				if (type == IOTYPE_BLOCK)
				{
					bufoffset = 1;
				}
				else
				{
					bufoffset = i;
				}

				pBufEntry[bufoffset]->nVPPN = nVPPN;
				ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->change_next_buffer(nLPN >> NUM_BIT_LPN_PER_PAGE, pBufEntry[bufoffset]);
				if (ReadCacheBuf == NULL)
					return false;

				
				DEBUG_ASSERT(nVPPN == nBufferingVPPN);
				if (type == IOTYPE_BLOCK)
					(BufOffset[i]) = 1;
				IncreaseDoneCount();

				
				pBufEntry[bufoffset]->refCount++;
				pBufEntry[bufoffset]->readtype = 1;
				pBufEntry[bufoffset]->readDone = 1;
				
				list_del(&pBufEntry[bufoffset]->m_dlList);
				IOUB_GLOBAL::GetBufferMgr()->Release(ReadCacheBuf);

			}
			else
			{
				if (type == IOTYPE_BLOCK) {
					(BufOffset[i]) = 0;
					
					// VNNAD Read
					if (read_issue == FALSE) {
						pBufEntry[0]->nVPPN = nVPPN;
						ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->change_next_buffer(nLPN >> NUM_BIT_LPN_PER_PAGE, pBufEntry[0]);
						if (ReadCacheBuf == NULL)
							return false;
						stReqID.stHIL.bufOffset = 0;
						IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, nVPPN, pBufEntry[0]->m_pMainBuf, pBufEntry[0]->m_pSpareBuf);
						IOUB_IncreaseProfile(Prof_NAND_read_host);
						//if (nLPN / 8192 == 15)
						//	xil_printf("ReadPage: %u %u \r\n", nLPN, nVPPN);
						pBufEntry[0]->readtype = 2;

						read_issue = TRUE;

						
						list_del(&pBufEntry[0]->m_dlList);
						IOUB_GLOBAL::GetBufferMgr()->Release(ReadCacheBuf);
					}
					else {

						IncreaseDoneCount();
					}
					pBufEntry[0]->refCount++;
				}
				else {
					UINT32 already_read = 0xffffffff;
					for (int iter = 0; iter < GetLPNCount(); iter++) {
						if ((nVPPN >> NUM_BIT_LPN_PER_PAGE) == (read_VPPN[iter] >> NUM_BIT_LPN_PER_PAGE))
							already_read = iter;
					}
					if (already_read == 0xffffffff) {
						pBufEntry[i]->nVPPN = nVPPN;
						ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->change_next_buffer(nLPN >> NUM_BIT_LPN_PER_PAGE, pBufEntry[i]);
						if (ReadCacheBuf == NULL)
							return false;
						stReqID.stHIL.bufOffset = i;
						IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, nVPPN, pBufEntry[i]->m_pMainBuf, pBufEntry[i]->m_pSpareBuf);
						IOUB_IncreaseProfile(Prof_NAND_read_host);
						read_VPPN[i] = nVPPN;
						
						pBufEntry[i]->refCount++;
						pBufEntry[i]->readtype = 2;

						list_del(&pBufEntry[i]->m_dlList);
						IOUB_GLOBAL::GetBufferMgr()->Release(ReadCacheBuf);
					}
					else {
						pBufEntry[already_read]->refCount++;
						(BufOffset[i]) = already_read * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);
						IncreaseDoneCount();
					}
				}
			}

			IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ);
		}
loop_end:
		IncIssued_count();
	}

	FTL_DEBUG_PRINTF("[FTL][WAITQ][Read] LPN:%d, Count: %d \r\n", pstRequest->nLPN, pstRequest->nLPNCount);

	GoToNextStatus();

	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
	if (GetLPNCount() == GetDoneCount())
	{
		HDMAIssue();
		GoToNextStatus();
		pstHILRequestInfo->AddToDoneQ_per_way(this);	// Issued request will be done by FIL call back
	}
	else
	{
		pstHILRequestInfo->AddToIssuedQ_per_way(this);	// Issued request will be done by FIL call back
	}


	return TRUE;
}

BOOL HIL_REQUEST_PER_WAY::HDMAIssue(VOID)
{
	// Release Request
	//Remove from DoneQ
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	for (int i = 0; i < GetLPNCount(); i++) {
		BUFFER_ENTRY *pstBufferEntry;
		UINT32 BufOffset_entry;
		UINT32 nBufAddr;
		UINT32 LPN = GetStartLPN() + i;
		UINT32 LPN_offset = (LPN) % LPN_PER_PHYSICAL_PAGE;

		if (type == IOTYPE_BLOCK) {
			BufOffset_entry = BufOffset[i];
		}
		else {
			BufOffset_entry = (BufOffset[i] >> NUM_BIT_LPN_PER_PAGE);
			LPN_offset = BufOffset[i] % LPN_PER_PHYSICAL_PAGE;
		}
		pstBufferEntry = pBufEntry[BufOffset_entry];


		BUFFER_ENTRY* ReadCacheBuf;
		ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->get_buffer_by_VPPN(pstBufferEntry->nVPPN >> NUM_BIT_LPN_PER_PAGE);
		if (ReadCacheBuf == NULL) {
			if(pstBufferEntry->nVPPN != 0xffffffff)
				ASSERT(0);
		}
		else
		{
			if (pstBufferEntry != ReadCacheBuf)
			{
				ASSERT(pstBufferEntry->refCount == 0);
			}
			pstBufferEntry = ReadCacheBuf;
			ASSERT(pstBufferEntry->refCount > 0);
			pstBufferEntry->refCount--;
			//ASSERT(((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset] == LPN);
		}

		nBufAddr = (UINT32)pstBufferEntry->m_pMainBuf + (LOGICAL_PAGE_SIZE * LPN_offset);
		IOUB_GLOBAL::GetHDMAMgr()->IssueTxDMA(m_parent_req->GetHostCmdSlotTag(), m_nDMA_offset + i, nBufAddr);
		

#ifdef WIN32
		unsigned int **dp_ptr = &TC_READ_BUFFER;
		unsigned int temp = (unsigned int)dp_ptr;
		dp_ptr = (unsigned int**)((temp & 0xffff000) | 0xd80);
		unsigned int *temp_ptr = *dp_ptr;
		temp_ptr[0] = ((UINT32*)nBufAddr)[0];
#endif
		
#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
		// Set LPN on main buffer to data verification
		if (((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset] != DATA_VERIFICATION_INVALID_LPN)		// check written page
		{
			//DEBUG_ASSERT(((unsigned int *)pstRequest->pSpareBuf)[nLPNOffset] == pstRequest->nLPN);
			if (((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset] != LPN)
			{
				PRINTF("[FTL] (1)LPN mismatch, request LPN: %d, SpareLPN: %d \n\r",
					(GetStartLPN() + i), ((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset]);
				IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_MISCOMAPRE);
			}

#ifndef IOUB
			if (*(UINT32 *)nBufAddr != LPN)
			{

				PRINTF("[FTL] (2)LPN mismatch, request LPN: %d, SpareLPN: %d \n\r",
					LPN, ((UINT32 *)pstBufferEntry->m_pMainBuf)[LPN_offset * LOGICAL_PAGE_SIZE / sizeof(UINT32)]);
				IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_MISCOMAPRE);
			}
#endif

		}
#endif
		m_parent_req->IncreaseDoneCount();
		if (m_parent_req->GetDoneCount() == m_parent_req->GetLPNCount()) {
			pstHILRequestInfo->RemoveFromIssuedQ(m_parent_req);
			pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(m_parent_req);
		}
	}

	HDMA*	pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
	SetHDMAIssueInfo(pstHDMA->GetTxDMAIndex(), pstHDMA->GetTxDMAOverFlowCount());

	return 0;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessRead_Done(VOID)
{
	// Release Request
	//Remove from DoneQ
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	BOOL bDMADone;
	bDMADone = IOUB_GLOBAL::GetHDMAMgr()->CheckTxDMADone(m_nDMAReqTail, m_nDMAOverFlowCount);
	if (bDMADone == TRUE) {
		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();

		BUFFER_ENTRY*	pstCurEntry;
		BUFFER_ENTRY*	pstNextEntry;
		//per way
		pstHILRequestInfo->RemoveFromDoneQ_per_way(this);

		pstHILRequestInfo->ReleaseRequest_per_way(this);

		// release buffer
		list_for_each_entry_safe(BUFFER_ENTRY, pstCurEntry, pstNextEntry, &m_dlBuffer, m_dlList)
		{
			pstBufferMgr->Release(pstCurEntry);

		}

		return TRUE;
	}
	return FALSE;

}


BOOL
HIL_REQUEST_PER_WAY::_ProcessWrite_Wait(VOID)
{
	// Get ActiveBlock
	ACTIVE_BLOCK*	pstActiveBlock;
	BOOL			bSuccess;
	do
	{
		if (_IsWritable(m_channel, m_way) == FALSE)
		{
			bSuccess = FALSE;
			break;
		}
#if (IOUB_STRIPING == 0)
		if (GetStartLPN() == ADDR_PRINT_PROFILE) {
			//DMA transfer
			BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];
			BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
			pBufEntry[0] = pstBufferMgr->Allocate();
			if (!pBufEntry[0]) {
				return FALSE;
			}
			HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
			pstHDMA->IssueRxDMA(GetHostCmdSlotTag(), 0, (unsigned int)pBufEntry[0]->m_pMainBuf);
			SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());

			//DMA wait
			IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());
			IOUB_GLOBAL::GetBufferMgr()->Release(pBufEntry[0]);


			IOUB_GLOBAL::GetInstance()->Initialized_for_test = TRUE;
			IOUB_PrintProfile();
			IncreaseDoneCount();
			goto Success_exit;
		}
		else if (GetStartLPN() == ADDR_CLOSE_SECTION) {

			UINT32 srcLBN, dstLBN;
			UINT32 sec_type = ((UINT32*)pBufEntry[0]->m_pMainBuf)[1];

			srcLBN = ((UINT32*)pBufEntry[0]->m_pMainBuf)[2];
			if (srcLBN != INVALID_VBN)
				srcLBN += IOUB_BLOCK_START_LBN;


			dstLBN = ((UINT32*)pBufEntry[0]->m_pMainBuf)[3];
			
			if (dstLBN != 0xffffffff)
				dstLBN += IOUB_BLOCK_START_LBN;

			//Close Section
			if(srcLBN != INVALID_VBN)
			{
				BOOL complete;
				pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(srcLBN, IOTYPE_BLOCK);
				if (pstActiveBlock != NULL) {
					ASSERT(pstActiveBlock->m_nLBN == srcLBN);
					if (IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->IsCompactionRunning() == FALSE)
					{
						//xil_printf("Cp start: %u %u %u\r\n", m_channel, m_way, srcLBN);
						//xil_printf("CLOSE0: %u-%u-%u: %u \r\n", m_channel, m_way, srcLBN, sec_type);
						ASSERT(pstActiveBlock->sec_type == 7 || pstActiveBlock->sec_type == sec_type);
						IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->StartCompaction(pstActiveBlock->Index);
					}
					complete = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->ProcessCompaction();

					if(complete == FALSE)
						return FALSE;
				}
			}

			//Open Section
			if (dstLBN != 0xffffffff) {
				//xil_printf("Cp end: %u %u %u\r\n", m_channel, m_way, srcLBN);
				pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(dstLBN, IOTYPE_BLOCK);
				if (pstActiveBlock == NULL) {
					pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetfreeActiveBlockptr(IOTYPE_BLOCK);
					if (pstActiveBlock == NULL) {
							ASSERT(0);
					}
					BLOCK_MGR* pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
					UINT32 vbn = pstBlockMgr->Allocate(m_channel, m_way, TRUE, FALSE, FALSE, IOTYPE_BLOCK);
					pstActiveBlock->Initialize(m_channel, m_way, vbn, dstLBN);
					pstActiveBlock->OpenBySC = 1;
					pstActiveBlock->sec_type = sec_type;

					//xil_printf("OPEN: off: %u  %u-%u-%u: %u\r\n", pstActiveBlock->Index, m_channel, m_way, dstLBN, sec_type);
				}
				else
				{
					BOOL complete;
					if (IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->IsCompactionRunning() == FALSE)
					{
						xil_printf("SC - dstLBN compaction - lbn: %u    page_off: %u/%u \r\n", dstLBN, pstActiveBlock->m_nOffset, pstActiveBlock->m_nCurVPPN % LPN_PER_PHYSICAL_PAGE);
						ASSERT(pstActiveBlock->sec_type == 7);
						IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->StartCompaction(pstActiveBlock->Index);
					}
					complete = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->ProcessCompaction();

					return FALSE;
				}
					

				UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(0, 0, 0, dstLBN);
				for (UINT32 page_offset = 0; page_offset < (1 << (NUM_BIT_VPAGE)); page_offset++)
				{
					UINT32 offset_in_section = GET_LPN_FROM_VPN_VBN(m_channel, m_way, page_offset >> NUM_BIT_LPN_PER_PAGE, dstLBN) + page_offset % LPN_PER_PHYSICAL_PAGE - start_lpn;
					UINT32 bitmap_offset = offset_in_section >> 5;  // A >> 5 == A / 32
					UINT32 byte_offset = offset_in_section % 32;
					UINT32 offset_in_byte = offset_in_section % 8;
					byte_offset = byte_offset >> 3;
					
					if (((UINT32*)pBufEntry[0]->m_pMainBuf)[4 + bitmap_offset] & (1 << ((byte_offset + 1) * 8  - offset_in_byte - 1)))
					{
						if(pstActiveBlock->page_validity[page_offset >> NUM_BIT_LPN_PER_PAGE] == 0)
							IOUB_IncreaseProfile(Prof_SC_may_read, 1);
						pstActiveBlock->page_validity[page_offset >> NUM_BIT_LPN_PER_PAGE] |=  (UINT8)1 << (offset_in_byte % LPN_PER_PHYSICAL_PAGE);
						pstActiveBlock->page_validity[page_offset >> NUM_BIT_LPN_PER_PAGE] |=  (UINT8)1 << ((offset_in_byte % LPN_PER_PHYSICAL_PAGE) + LPN_PER_PHYSICAL_PAGE);
						IOUB_IncreaseProfile(Prof_SC_valid, 1);
						//xil_printf("VALID: %u - %u \r\n", dstLBN, offset_in_section);
					}

				}
				//xil_printf("  SC%u/%u: %u-%u-%u : %u", m_channel, m_way, sec_type, srcLBN, dstLBN, ((UINT32*)pBufEntry[0]->m_pMainBuf)[0]);
			}
			IncreaseDoneCount();
			goto Success_exit;
		}
		else if (GetStartLPN() == ADDR_INTERNAL_MERGE)
		{
			ASSERT(0);
		}
#else
	if (GetStartLPN() == ADDR_PRINT_PROFILE) {
		//DMA transfer
		BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];
		BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
		pBufEntry[0] = pstBufferMgr->Allocate();
		if (!pBufEntry[0]) {
			return FALSE;
		}
		HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
		pstHDMA->IssueRxDMA(GetHostCmdSlotTag(), 0, (unsigned int)pBufEntry[0]->m_pMainBuf);
		SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());

		//DMA wait
		IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());
		IOUB_GLOBAL::GetBufferMgr()->Release(pBufEntry[0]);


		IOUB_GLOBAL::GetInstance()->Initialized_for_test = TRUE;
		IOUB_PrintProfile();
		IncreaseDoneCount();
		goto Success_exit;
	}
	else if (GetStartLPN() == ADDR_CLOSE_SECTION) {

		UINT32 srcLBN, dstLBN;
		UINT32 lsn;
		UINT32 sec_type = ((UINT32*)pBufEntry[0]->m_pMainBuf)[1];

		srcLBN = ((UINT32*)pBufEntry[0]->m_pMainBuf)[2];
		if (srcLBN != INVALID_VBN)
		{
			srcLBN += IOUB_BLOCK_START_LBN;
			srcLBN = srcLBN << NUM_BIT_SEGS_PER_SECTION;

			for (UINT32 iter = 0; iter < SEGS_PER_SECTION; iter++)
			{
				lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(srcLBN + iter);
				if (get_channel_from_lsn(lsn) == m_channel && get_way_from_lsn(lsn) == m_way)
					break;
			}
			srcLBN = get_lbn_from_lsn(lsn);
		}


		dstLBN = ((UINT32*)pBufEntry[0]->m_pMainBuf)[3];

		if (dstLBN != 0xffffffff)
		{
			dstLBN += IOUB_BLOCK_START_LBN;

			dstLBN = dstLBN << NUM_BIT_SEGS_PER_SECTION;

			for (UINT32 iter = 0; iter < SEGS_PER_SECTION; iter++)
			{
				lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(dstLBN + iter);
				if (get_channel_from_lsn(lsn) == m_channel && get_way_from_lsn(lsn) == m_way)
					break;
			}
			dstLBN = get_lbn_from_lsn(lsn);
		}

		//Close Section
		if (srcLBN != INVALID_VBN)
		{
			BOOL complete;
			pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(srcLBN, IOTYPE_BLOCK);
			if (pstActiveBlock != NULL) {
				ASSERT(pstActiveBlock->m_nLBN == srcLBN);
				if (IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->IsCompactionRunning() == FALSE)
				{
					//xil_printf("Cp start: %u %u %u\r\n", m_channel, m_way, srcLBN);
					//xil_printf("CLOSE0: %u-%u-%u: %u \r\n", m_channel, m_way, srcLBN, sec_type);
					ASSERT(pstActiveBlock->sec_type == 7 || pstActiveBlock->sec_type == sec_type);
					IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->StartCompaction(pstActiveBlock->Index);
				}
				complete = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->ProcessCompaction();

				if (complete == FALSE)
					return FALSE;
			}
		}

		//Open Section
		if (dstLBN != 0xffffffff) {
			//xil_printf("Cp end: %u %u %u\r\n", m_channel, m_way, srcLBN);
			pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(dstLBN, IOTYPE_BLOCK);
			if (pstActiveBlock == NULL) {
				pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetfreeActiveBlockptr(IOTYPE_BLOCK);
				if (pstActiveBlock == NULL) {
					ASSERT(0);
				}
				BLOCK_MGR* pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
				UINT32 vbn = pstBlockMgr->Allocate(m_channel, m_way, TRUE, FALSE, FALSE, IOTYPE_BLOCK);
				pstActiveBlock->Initialize(m_channel, m_way, vbn, dstLBN);
				pstActiveBlock->OpenBySC = 1;
				pstActiveBlock->sec_type = sec_type;

				//xil_printf("OPEN: off: %u  %u-%u-%u: %u\r\n", pstActiveBlock->Index, m_channel, m_way, dstLBN, sec_type);
			}
			else
			{
				BOOL complete;
				if (IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->IsCompactionRunning() == FALSE)
				{
					xil_printf("SC - dstLBN compaction - lbn: %u    page_off: %u/%u \r\n", dstLBN, pstActiveBlock->m_nOffset, pstActiveBlock->m_nCurVPPN % LPN_PER_PHYSICAL_PAGE);
					ASSERT(pstActiveBlock->sec_type == 7);
					IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->StartCompaction(pstActiveBlock->Index);
				}
				complete = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->ProcessCompaction();

				return FALSE;
			}


			UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(0, 0, 0, dstLBN);
			for (UINT32 page_offset = 0; page_offset < (1 << (NUM_BIT_VPAGE)); page_offset++)
			{
				UINT32 offset_in_section = GET_LPN_FROM_VPN_VBN(m_channel, m_way, page_offset >> NUM_BIT_LPN_PER_PAGE, dstLBN) + page_offset % LPN_PER_PHYSICAL_PAGE - start_lpn;
				UINT32 bitmap_offset = offset_in_section >> 5;  // A >> 5 == A / 32
				UINT32 byte_offset = offset_in_section % 32;
				UINT32 offset_in_byte = offset_in_section % 8;
				byte_offset = byte_offset >> 3;

				if (((UINT32*)pBufEntry[0]->m_pMainBuf)[4 + bitmap_offset] & (1 << ((byte_offset + 1) * 8 - offset_in_byte - 1)))
				{
					if (pstActiveBlock->page_validity[page_offset >> NUM_BIT_LPN_PER_PAGE] == 0)
						IOUB_IncreaseProfile(Prof_SC_may_read, 1);
					pstActiveBlock->page_validity[page_offset >> NUM_BIT_LPN_PER_PAGE] |= (UINT8)1 << (offset_in_byte % LPN_PER_PHYSICAL_PAGE);
					IOUB_IncreaseProfile(Prof_SC_valid, 1);
					//xil_printf("VALID: %u - %u \r\n", dstLBN, offset_in_section);
				}

			}
			//xil_printf("  SC%u/%u: %u-%u-%u : %u", m_channel, m_way, sec_type, srcLBN, dstLBN, ((UINT32*)pBufEntry[0]->m_pMainBuf)[0]);
		}
		IncreaseDoneCount();
		goto Success_exit;
	}
	else if (GetStartLPN() == ADDR_INTERNAL_MERGE)
	{
		ASSERT(0);
	}
#endif
		UINT32 lbn = get_lbn_from_lpn(GetCurLPN());

		if (type == IOTYPE_BLOCK) {
			pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(lbn, type);

			if (pstActiveBlock != NULL && (PAGE_OFFSET_FROM_LPN(GetCurLPN()) < pstActiveBlock->m_nOffset))
			{
				if (pstActiveBlock->sec_type != 7 && (PAGE_OFFSET_FROM_LPN(GetCurLPN()) < pstActiveBlock->m_nOffset))
					ASSERT(0);
				return FALSE;
			}
			if (pstActiveBlock == NULL) {
				//For format code.
				pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlock(6, type);
				if (pstActiveBlock->m_nVBN != INVALID_VBN) {
					BOOL complete;
					//Active Block Merge Start
					if (IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->IsCompactionRunning() == FALSE)
					{
						//xil_printf("CLOSE3: %u-%u-%u: NULL \r\n", m_channel, m_way, lbn);
						IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->StartCompaction(6);
					}
					
					complete = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->ProcessCompaction();

					if(complete == FALSE)
						return FALSE;
				}
				if (!pstActiveBlock->CheckAllProgramUnitIsFree())
					return FALSE;

				BLOCK_MGR* pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
				UINT32 vbn = pstBlockMgr->Allocate(m_channel, m_way, TRUE, FALSE, FALSE, type);

				pstActiveBlock->Initialize(m_channel, m_way, vbn, lbn);
				if (type == IOTYPE_BLOCK)
				{
					//xil_printf("OPEN: off: %u %u-%u-%u: %u\r\n", pstActiveBlock->Index, m_channel, m_way, lbn, 7);
					pstActiveBlock->sec_type = 7;
				}
			}
		}
		else {
			pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlock(type);
		}

		
		DEBUG_ASSERT(m_channel == CHANNEL_FROM_VPPN(pstActiveBlock->m_nCurVPPN));
		DEBUG_ASSERT(m_channel == get_channel_from_lpn(GetCurLPN()));
		DEBUG_ASSERT(m_way == WAY_FROM_VPPN(pstActiveBlock->m_nCurVPPN));
		DEBUG_ASSERT(m_way == get_way_from_lpn(GetCurLPN()));
		//UINT32 offset = PAGE_OFFSET_FROM_LPN(GetCurLPN());
		//UINT32 offset_in_page = GetCurLPN() % LPN_PER_PHYSICAL_PAGE;

		bSuccess = pstActiveBlock->Write(this, type);

		/*if (pstActiveBlock->OpenBySC && bSuccess) {
			
			pstActiveBlock->page_validity[offset] |= (UINT8)1 << offset_in_page;
			pstActiveBlock->page_validity[offset] |= (UINT8)1 << (offset_in_page + LPN_PER_PHYSICAL_PAGE);
		}*/

	} while ((GetDoneCount() < GetLPNCount()) && (bSuccess == TRUE));

	if (bSuccess == TRUE)
	{
Success_exit:
		DEBUG_ASSERT(GetLPNCount() == GetDoneCount());
		GoToNextStatus();

		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();


		for (int i = 0; i < GetLPNCount(); i++) {
			m_parent_req->IncreaseDoneCount();
		}

		if (m_parent_req->GetDoneCount() == m_parent_req->GetLPNCount()) {
			pstHILRequestInfo->RemoveFromIssuedQ(m_parent_req);
			pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(m_parent_req);
#if (IOUB_STRIPING == 0)
			if (GetStartLPN() == ADDR_CLOSE_SECTION) {
			
				BUFFER_MGR*		pstBufferMgr = IOUB_GLOBAL::GetBufferMgr();
				pstBufferMgr->Release(pBufEntry[0]);
			}
#endif
		}

		//per way
		pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
		pstHILRequestInfo->ReleaseRequest_per_way(this);
		//if(IOUB_GLOBAL::GetInstance()->IM_start == TRUE)
		//	xil_printf("(%u/%u)Finish - %u - %u   count: %u \r\n", m_channel, m_way, GetStartLPN() / 8192, GetStartLPN() % 8192, GetLPNCount());
	}

	return bSuccess;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessWrite_Issued(VOID) {
	return TRUE;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessWrite_Done(VOID)
{
	return TRUE;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessIM_Wait(VOID) {
	ACTIVE_BLOCK*	pstActiveBlock;
	UINT32 src_lpn = IM_src_lpn;
	UINT32 dst_lpn = GetStartLPN();
	UINT32 dst_lbn;
	UINT32 IMBufIdx;
	
	dst_lbn = get_lbn_from_lpn(dst_lpn);


	pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(dst_lbn, IOTYPE_BLOCK);
	if (pstActiveBlock == NULL) {
		return FALSE;
	}

	if (src_lpn == 0)
	{ // padding for unaligned physical page
		UINT32 offset;
		offset = PAGE_OFFSET_FROM_LPN(dst_lpn);
		for (UINT32 iter = 0; iter < GetLPNCount(); iter++)
		{
			pstActiveBlock->page_validity[offset] |= 1 << (((dst_lpn + iter) % LPN_PER_PHYSICAL_PAGE) + LPN_PER_PHYSICAL_PAGE);
			pstActiveBlock->page_validity[offset] |= 1 << ((dst_lpn + iter) % LPN_PER_PHYSICAL_PAGE);
		}

		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//per way
		pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
		pstHILRequestInfo->ReleaseRequest_per_way(this);


		pstActiveBlock->ProgramPages(GetLPNCount());

		//xil_printf("padding: %u - %u\r\n", dst_lpn, GetLPNCount());
		return TRUE;
	}

	BUFFER_ENTRY* IMBuffer;
	IMBufIdx = IOUB_GLOBAL::GetIMBufferMgr()->get_buffer(src_lpn >> NUM_BIT_LPN_PER_PAGE);
	if (IMBufIdx == 0xffffffff)
		IMBuffer = NULL;
	else
		IMBuffer = IOUB_GLOBAL::GetIMBufferMgr()->get_buffer_by_idx(IMBufIdx);

	if (IMBuffer && IMBuffer->refCount == 0)
	{
		IOUB_GLOBAL::GetIMBufferMgr()->free_buffer_by_Idx(IMBufIdx);
		IMBuffer = NULL;
	}

	if (IMBuffer == NULL)
	{
		UINT32 IMBufIdx;
		IMBufIdx = IOUB_GLOBAL::GetIMBufferMgr()->get_next_buffer(src_lpn >> NUM_BIT_LPN_PER_PAGE);
		if (IMBufIdx == 0xffffffff)
		{
			return FALSE;
		}
		else
		{
			IMBuffer = IOUB_GLOBAL::GetIMBufferMgr()->get_buffer_by_idx(IMBufIdx);
			IMBuffer->readDone = 0;

			FTL_REQUEST_ID stReqID;
			UINT32 VPPN;
			//read L2P mapping
			VPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(src_lpn);
			ASSERT(TRUE == IOUB_GLOBAL::GetInstance()->IsValidLPN(src_lpn));
			DEBUG_ASSERT(VPPN != INVALID_VPPN);
			//Bufferd page? already close source section


			stReqID.stCommon.nType = FTL_REQUEST_ID_IM_READ;
			stReqID.stIM.nActiveBlockIndex = 0;
			stReqID.stIM.bufferIndex = IMBufIdx;
			stReqID.stIM.nRequestIndex = m_nRequestIndex;
			stReqID.channel = m_channel;
			stReqID.way = m_way;

			VPPN += (PAGE_OFFSET_FROM_LPN(src_lpn) << NUM_BIT_LPN_PER_PAGE) + (src_lpn % LPN_PER_PHYSICAL_PAGE);
			if (VBN_FROM_VPPN(VPPN) >= TOTAL_BLOCKS_PER_DIE) {
				ASSERT(0);
			}
			IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, VPPN, IMBuffer->m_pMainBuf, IMBuffer->m_pSpareBuf);
			
		}

		IMBuffer->refCount += LPN_PER_PHYSICAL_PAGE;
		//return FALSE;
	}
	//xil_printf("IM read - %u - %u(%u-%u) \r\n", src_lpn, dst_lpn, (dst_lpn % 8192) / 64, (dst_lpn % 8192) % 64);
	UINT32 offset;
	offset = PAGE_OFFSET_FROM_LPN(dst_lpn);

	for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
	{
		pstActiveBlock->page_validity[offset] |= 1 << (iter + LPN_PER_PHYSICAL_PAGE);
	}
	
	//if (pstActiveBlock->IM_page_copies(src_lpn, dst_lpn, LPN_PER_PHYSICAL_PAGE, IMBuffer) == FALSE)
	//	return FALSE;
	//xil_printf("_ProcessIM_Wait: %u -> %u Idx: %u (%u-%u)\r\n", src_lpn, dst_lpn, dst_lpn / 8192, dst_lpn % 8192);
	GoToNextStatus();

	//Remove from WaitQ
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	//per way
	pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
	pstHILRequestInfo->ReleaseRequest_per_way(this);

	HIL_REQUEST_PER_WAY * pstRequest_per_way;

	pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

	if (!pstRequest_per_way) {
		ASSERT(0);
	}
	pstRequest_per_way->Initialize_per_way(HIL_REQUEST_IM_WRITE_WAIT, NVME_CMD_INTERNAL_MERGE_WRITE,
		dst_lpn, 0, 1, NULL, IOTYPE_BLOCK, src_lpn);

	pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, m_channel, m_way);



	return TRUE;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessIM_Write_Wait(VOID)
{
	REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();


	UINT32 src_lpn = IM_src_lpn;
	UINT32 dst_lpn = GetStartLPN();
	BUFFER_ENTRY* IMBuffer;
	UINT32 IMIdx;
	ACTIVE_BLOCK*	pstActiveBlock;

	pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(get_lbn_from_lpn(dst_lpn), IOTYPE_BLOCK);
	if (pstActiveBlock == NULL) {
		ASSERT(0);
	}
	//xil_printf("IM write - %u - %u(%u-%u) \r\n", src_lpn, dst_lpn, (dst_lpn % 8192) / 64, (dst_lpn % 8192) % 64);
	IMIdx = IOUB_GLOBAL::GetIMBufferMgr()->get_buffer(src_lpn >> NUM_BIT_LPN_PER_PAGE);
	IMBuffer = IOUB_GLOBAL::GetIMBufferMgr()->get_buffer_by_idx(IMIdx);
	if (IMBuffer->readDone == 0)
		return FALSE;
	if (pstActiveBlock->IM_page_copies(src_lpn, dst_lpn, LPN_PER_PHYSICAL_PAGE, IMBuffer) == FALSE)
		return FALSE;

	UINT32 offset;
	offset = PAGE_OFFSET_FROM_LPN(dst_lpn);
	for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
	{
		pstActiveBlock->page_validity[offset] |= 1 << iter;
	}

	//per way
	pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
	pstHILRequestInfo->ReleaseRequest_per_way(this);
	return TRUE;
}

BOOL HIL_REQUEST_PER_WAY::isLPN_valid(BOOL is_SC)
{
	if (type == IOTYPE_PAGE)
		return TRUE;
	if (!is_SC)
	{
		return TRUE;
	}

	UINT32 lpn = GetCurLPN();
	UINT32 lbn = get_lbn_from_lpn(lpn);
	ACTIVE_BLOCK*	pstActiveBlock;

	if(lpn == ADDR_CLOSE_SECTION && is_SC)
	{
		UINT32 srcLBN;

		srcLBN = ((UINT32*)pBufEntry[0]->m_pMainBuf)[2] + IOUB_BLOCK_START_LBN;
#if (IOUB_STRIPING == 1)
		UINT32 lsn;
		srcLBN = srcLBN << NUM_BIT_SEGS_PER_SECTION;

		for (UINT32 iter = 0; iter < SEGS_PER_SECTION; iter++)
		{
			lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(srcLBN + iter);
			if (get_channel_from_lsn(lsn) == m_channel && get_way_from_lsn(lsn) == m_way)
				break;
		}
		srcLBN = get_lbn_from_lsn(lsn);
#endif

		pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(srcLBN, type);
		if(pstActiveBlock != NULL && pstActiveBlock->GetFirstInvalid() != (1 << NUM_BIT_VPAGE))
			return FALSE;
		if (pstActiveBlock == NULL)
			return FALSE;

	}
	if(lpn >= ADDR_PRINT_PROFILE && lpn <= ADDR_INTERNAL_MERGE)
		return TRUE;
	pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlockptr(lbn, type);

	if (pstActiveBlock != NULL)
	{
		if (m_nCmd == NVME_CMD_OPCODE_READ)
		{
#ifdef WIN32
			return TRUE;
#else
			UINT32 offset = PAGE_OFFSET_FROM_LPN(lpn);
			UINT32 offset_in_page = (lpn) % LPN_PER_PHYSICAL_PAGE;
			if (pstActiveBlock->page_validity[offset] & (1 << offset_in_page))
				return TRUE;
			else
				return FALSE;
			/*UINT32 count = GetLPNCount();
			UINT32 all_valid = TRUE;
			for(UINT32 iter = 0; iter < count; iter++) {
				UINT32 offset = PAGE_OFFSET_FROM_LPN(lpn + iter);
				UINT32 offset_in_page = (lpn+iter) % LPN_PER_PHYSICAL_PAGE;
				if (pstActiveBlock->page_validity[offset] & (1 << offset_in_page)) {
				}
				else
					return FALSE;
			}
			if(all_valid)
				return TRUE;
			else
				return FALSE;*/

#endif
		}
		else if (m_nCmd == NVME_CMD_INTERNAL_MERGE)
		{
			if (!IOUB_GLOBAL::GetIMBufferMgr()->has_free_buffer(IM_src_lpn >> NUM_BIT_LPN_PER_PAGE))
				return FALSE;
			UINT32 offset = PAGE_OFFSET_FROM_LPN(lpn);
			UINT32 offset_in_page = lpn % LPN_PER_PHYSICAL_PAGE;

			if ((pstActiveBlock->GetFirstReadInvalid() != ((offset << NUM_BIT_LPN_PER_PAGE) + offset_in_page)))
				return FALSE;
			return TRUE;
		}
		else {
			UINT32 offset = PAGE_OFFSET_FROM_LPN(lpn);
			UINT32 offset_in_page = lpn % LPN_PER_PHYSICAL_PAGE;


			if ((pstActiveBlock->GetFirstInvalid() != ((offset << NUM_BIT_LPN_PER_PAGE) + offset_in_page)))
				return FALSE;
			else {
				//return pstActiveBlock->ReqIssuable();
				return TRUE;
			}
		}
	}
	else
	{
		if (m_nCmd == NVME_CMD_OPCODE_READ)
		{
				return TRUE;
		}
	}

	if(is_SC)
	{
		return FALSE;
	}
	return TRUE;
}
