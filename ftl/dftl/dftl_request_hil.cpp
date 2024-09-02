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

#include "dftl_internal.h"

///////////////////////////////////////////////////////////////////////////////
//
//	HIL_REQUEST_INFO
//
///////////////////////////////////////////////////////////////////////////////

VOID
HIL_REQUEST_INFO::Initialize(VOID)
{
	INIT_LIST_HEAD(&m_dlFree);
	INIT_LIST_HEAD(&m_dlWait);
	INIT_LIST_HEAD(&m_dlIssued);
	INIT_LIST_HEAD(&m_dlHDMA);
	INIT_LIST_HEAD(&m_dlDone);

	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		for (int way = 0; way < USER_WAYS; way++) {
			
			INIT_LIST_HEAD(&m_dlWait_per_way[channel][way]);
			
		}
	}
	m_nFreeCount = 0;
	m_nWaitCount = 0;
	m_nIssuedCount = 0;
	m_nHDMACount = 0;
	m_nDoneCount = 0;

	INIT_LIST_HEAD(&m_dlFree_per_way);
	INIT_LIST_HEAD(&m_dlIssued_per_way);
	INIT_LIST_HEAD(&m_dlDone_per_way);

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
HIL_REQUEST_INFO::AddToHDMAIssuedQ(HIL_REQUEST* pstRequest)
{
	list_add_tail(&pstRequest->m_dlList, &m_dlHDMA);
	m_nHDMACount++;

	DEBUG_ASSERT(m_nHDMACount <= HIL_REQUEST_COUNT);
}

VOID
HIL_REQUEST_INFO::RemoveFromHDMAIssuedQ(HIL_REQUEST* pstRequest)
{
	DEBUG_ASSERT(m_nHDMACount > 0);

	list_del_init(&pstRequest->m_dlList);
	m_nHDMACount--;
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
	if (m_nWaitCount_per_way[channel][way] == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlWait_per_way[channel][way]) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlWait_per_way[channel][way]) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlWait_per_way[channel][way], HIL_REQUEST, m_dlList);

	return pstRequest;
}

HIL_REQUEST*
HIL_REQUEST_INFO::GetIssuedRequest_per_way(VOID)
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
HIL_REQUEST_INFO::GetDoneRequest_per_way(VOID)
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

HIL_REQUEST*
HIL_REQUEST_INFO::GetHDMARequest(VOID)
{
	if (m_nHDMACount == 0)
	{
		DEBUG_ASSERT(list_empty(&m_dlHDMA) == TRUE);
		return NULL;
	}

	DEBUG_ASSERT(list_empty(&m_dlHDMA) == FALSE);

	HIL_REQUEST*	pstRequest;
	pstRequest = list_first_entry(&m_dlHDMA, HIL_REQUEST, m_dlList);

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

	DEBUG_ASSERT(DFTL_GLOBAL::GetInstance()->IsValidLPN(m_nLPN) == TRUE);
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
#if (SUPPORT_META_DEMAND_LOADING == 1)
	META_MGR*	pstMetaMgr = DFTL_GLOBAL::GetMetaMgr();

	BOOL	bAvailable = TRUE;

	if (pstMetaMgr->IsMetaAvailable(m_nLPN) == FALSE)
	{
		pstMetaMgr->LoadMeta(m_nLPN);
		bAvailable = FALSE;
	}
	else if (pstMetaMgr->IsMetaAvailable(m_nLPN + m_nLPNCount - 1) == FALSE)
	{
		pstMetaMgr->LoadMeta(m_nLPN + m_nLPNCount - 1);
		bAvailable = FALSE;
	}

	return bAvailable;
#else
	return TRUE;
#endif
}

BOOL
HIL_REQUEST::_CheckMetaWritable(VOID)
{
#if (SUPPORT_META_DEMAND_LOADING == 1)
	META_MGR*	pstMetaMgr = DFTL_GLOBAL::GetMetaMgr();

	if (pstMetaMgr->IsMetaWritable(m_nLPN) == FALSE) {
		return FALSE;
	}
	return TRUE;
#else
	return TRUE;
#endif
}

BOOL
HIL_REQUEST::_ProcessRead_Wait(VOID)
{
	ASSERT(m_nStatus == HIL_REQUEST_READ_WAIT);
	ASSERT(m_nLPNCount <= REQUEST_LPN_COUNT_MAX);

	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
	BUFFER_MGR*		pstBufferMgr = DFTL_GLOBAL::GetBufferMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	HIL_REQUEST_PER_WAY *pstRequest_per_way;

	UINT32			nLPN;
	UINT32			last_ch, last_wy;
	UINT32			last_lpn, lpn;
	UINT32			lpn_count = 0;

	last_ch = get_channel_from_lpn(GetStartLPN() + m_nIssued_Count);
	last_wy = get_way_from_lpn(GetStartLPN() + m_nIssued_Count);
	last_lpn = (GetStartLPN() + m_nIssued_Count) >> NUM_BIT_LPN_PER_PAGE;

	for (int i = m_nIssued_Count; i < m_nLPNCount; i++) {
		INT32 channel, way;

		nLPN = GetStartLPN() + i;

		channel = get_channel_from_lpn(nLPN);
		way = get_way_from_lpn(nLPN);
		lpn = nLPN >> NUM_BIT_LPN_PER_PAGE;
		if (last_lpn == lpn) {
		}
		else {
			pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

			if (!pstRequest_per_way) {
				return FALSE;
			}
			pstRequest_per_way->Initialize_per_way(HIL_REQUEST_READ_WAIT, NVME_CMD_OPCODE_READ,
				m_nLPN + m_nIssued_Count, m_nHostCmdSlotTag, lpn_count, this);

			pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
			for (int j = 0; j < lpn_count; j++) {
				IncIssued_count();
			}
			lpn_count = 0;
			

			
		}
		
		lpn_count++;
		last_ch = channel;
		last_wy = way;
		last_lpn = lpn;
	}

	pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

	if (pstRequest_per_way) {
		pstRequest_per_way->Initialize_per_way(HIL_REQUEST_READ_WAIT, NVME_CMD_OPCODE_READ,
			m_nLPN + m_nIssued_Count, m_nHostCmdSlotTag, lpn_count, this);

		pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
		for (int j = 0; j < lpn_count; j++) {
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
	if (m_nLPNCount == m_nDoneLPNCount) {
		GoToNextStatus();

		//Remove from IssuedQ
		REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromIssuedQ(this);
		pstHILRequestInfo->AddToDoneQ(this);
		return TRUE;
	}
	return FALSE;
}

BOOL
HIL_REQUEST::_ProcessRead_Done(VOID)
{
	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();

	pstRequestMgr->GetHILRequestInfo()->RemoveFromDoneQ(this);
	pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(this);

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

	default:
		ASSERT(0);
		break;
	};

	return bSuccess;
}

BOOL
HIL_REQUEST::_IsWritable(UINT32 channel, UINT32 way)
{
	if (DFTL_GLOBAL::GetGCMgr(channel, way)->IsGCRunning() == TRUE)
	{
		return FALSE;
	}

	// check buffer available
	if (DFTL_GLOBAL::GetBufferMgr()->GetFreeCount() == 0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
HIL_REQUEST::_ProcessWrite_Wait(VOID)
{
	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();
	HIL_REQUEST_PER_WAY * pstRequest_per_way;

	// Get ActiveBlock
	ACTIVE_BLOCK*	pstActiveBlock;

	int last_ch, last_wy;
	int last_lpn, lpn;
	int lpn_count = 0;
	last_ch = get_channel_from_lpn(m_nLPN + m_nIssued_Count);
	last_wy = get_way_from_lpn(m_nLPN + m_nIssued_Count);

	last_lpn = (m_nLPN + m_nIssued_Count) >> NUM_BIT_LPN_PER_PAGE;

	for (int i = m_nIssued_Count; i < m_nLPNCount; i++) {
		INT32 channel, way;
		channel = get_channel_from_lpn(m_nLPN + i);
		way = get_way_from_lpn(m_nLPN + i);
		lpn = (m_nLPN + i) >> NUM_BIT_LPN_PER_PAGE;
		if(lpn == last_lpn) 
		{
			lpn_count++;
		}
		else {
			pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

			if (!pstRequest_per_way) {
				break;
			}
			pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
				m_nLPN + m_nIssued_Count, m_nHostCmdSlotTag, lpn_count, this);

			pstHILRequestInfo->AddToWaitQ_per_way(pstRequest_per_way, last_ch, last_wy);
			for (int j = 0; j < lpn_count; j++) {

				IncIssued_count();
			}
			lpn_count = 1;
		}
		last_ch = channel;
		last_wy = way;
		last_lpn = lpn;
		
	}

	pstRequest_per_way = pstHILRequestInfo->AllocateRequest_per_way();

	if (pstRequest_per_way) {
		pstRequest_per_way->Initialize_per_way(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE,
			m_nLPN + m_nIssued_Count, m_nHostCmdSlotTag, lpn_count, this);

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
		REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		//move queue
		pstHILRequestInfo->RemoveFromWaitQ(this);
		pstHILRequestInfo->AddToIssuedQ(this);
		return TRUE;
	}

	return FALSE;
}

VOID HIL_REQUEST_PER_WAY::Initialize_per_way(HIL_REQUEST_STATUS nStatus, NVME_CMD_OPCODE nCmd, UINT32 nLPN, UINT32 nHostCmdSlotTag, INT32 nLPNCount, HIL_REQUEST * parent)
{
	Initialize(nStatus, nCmd, nLPN, nHostCmdSlotTag, nLPNCount);
	
	m_nDMA_offset = parent->GetIssued_count();
	m_parent_req = parent;
	ASSERT(nLPNCount <= LPN_PER_PHYSICAL_PAGE);
	for (int i = 0; i < LPN_PER_PHYSICAL_PAGE; i++) {
		read_VPPN[i] = 0xffffffff;
		pBufEntry[i] = NULL;
	}
}



BOOL
HIL_REQUEST_PER_WAY::_ProcessRead_Wait(VOID)
{

#if (SUPPORT_META_DEMAND_LOADING == 1)

	if (_CheckAndLoadMeta() == FALSE)
	{
		// load metadata
		return FALSE;
	}
#endif
	INT32			nLPN;
	UINT32			nBufferingVPPN;
	INT32			nVPPN;
	UINT32			channel;
	UINT32			way;
	BUFFER_MGR*		pstBufferMgr = DFTL_GLOBAL::GetBufferMgr();

	FTL_REQUEST_ID stReqID = _GetRquestID();
	stReqID.channel = m_channel;
	stReqID.way = m_way;

	// check is there enough free buffers
	if (GetLPNCount()  > pstBufferMgr->GetFreeCount())
	{
		// not enough free buffer
		return FALSE;
	}

	// allocate buffer
	for (int i = GetIssued_count(); i < GetLPNCount(); i++)
	{
		nLPN = GetStartLPN() + i;
		channel = get_channel_from_lpn(nLPN);
		way = get_way_from_lpn(nLPN);


		if (pBufEntry[0] == NULL)
		{
			INIT_LIST_HEAD(&m_dlBuffer);
			

			for (unsigned int i = 0; i < GetLPNCount(); i++) {
				pBufEntry[i] = pstBufferMgr->Allocate();

				if (!pBufEntry[i]) {
					ASSERT(0);
					return FALSE;
				}
				list_add_tail(&pBufEntry[i]->m_dlList, &m_dlBuffer);

			}
		}
		// GetPPN
		nVPPN = DFTL_GLOBAL::GetMetaMgr()->GetL2V(nLPN);
		if (nVPPN == INVALID_PPN)	// unmap read
		{
			FTL_DEBUG_PRINTF("Unmap Read LPN: %d\n\r", nLPN);
			//*BufOffset = 0;
			BufOffset[i] = i * LPN_PER_PHYSICAL_PAGE;
			pBufEntry[i]->nVPPN = nVPPN;

			IncreaseDoneCount();

			DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_UNMAP_READ);

			OSAL_MEMSET((unsigned int*)((UINT32)pBufEntry[i]->m_pMainBuf), 0xffffffff, LOGICAL_PAGE_SIZE);
#if defined(WIN32) && defined(SUPPORT_DATA_VERIFICATION)
			// Set LPN on read buffer
			//((unsigned int*)pBufEntry->m_pSpareBuf)[0] = DATA_VERIFICATION_INVALID_LPN;
			((unsigned int*)pBufEntry[i]->m_pSpareBuf)[0] = DATA_VERIFICATION_INVALID_LPN;
#endif
		}
		else
		{
			BUFFER_ENTRY* ReadCacheBuf;
			ReadCacheBuf = DFTL_GLOBAL::GetReadCacheMgr()->get_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
			if (ReadCacheBuf == NULL) {
			}
			else {
				if (ReadCacheBuf->readDone == 0)
					return false;
				if (ReadCacheBuf->readtype == 1)
				{
					if (((UINT32*)ReadCacheBuf->m_pSpareBuf)[LPN_OFFSET_FROM_VPPN(nVPPN)] != nLPN)
					{
						if (ReadCacheBuf->refCount == 0)
						{
							DFTL_GLOBAL::GetReadCacheMgr()->free_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
							goto skip_readbuf;
						}
						else
						{
							BUFFERING_LPN* pstBufferingLPN = DFTL_GLOBAL::GetActiveBlockBufferingLPN(channel, way);
							nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, ReadCacheBuf);
							ASSERT(nVPPN == nBufferingVPPN);
							return false;
						}
					}
				}

				ReadCacheBuf->refCount++;
				IncreaseDoneCount();
				DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ);

			
				pBufEntry[i]->nVPPN = nVPPN;
				(BufOffset[i]) = i * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);

				goto loop_end;
			}
skip_readbuf:



			//*(BufOffset) = LPN_OFFSET_FROM_VPPN(nVPPN);
			(BufOffset[i]) = i * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);

			// Check buffering 
			BUFFERING_LPN* pstBufferingLPN = DFTL_GLOBAL::GetActiveBlockBufferingLPN(channel, way);
			//nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, pBufEntry);
			nBufferingVPPN = pstBufferingLPN->ReadLPN(nLPN, pBufEntry[i]);

			if (nBufferingVPPN != INVALID_VPPN)
			{
				DEBUG_ASSERT(nVPPN == nBufferingVPPN);

				pBufEntry[i]->nVPPN = nVPPN;
				ReadCacheBuf = DFTL_GLOBAL::GetReadCacheMgr()->change_next_buffer(nLPN >> NUM_BIT_LPN_PER_PAGE, pBufEntry[i]);
				if (ReadCacheBuf == NULL)
					return false;
				IncreaseDoneCount();

				pBufEntry[i]->refCount++;
				pBufEntry[i]->readtype = 1;
				pBufEntry[i]->readDone = 1;

				list_del(&pBufEntry[i]->m_dlList);
				DFTL_GLOBAL::GetBufferMgr()->Release(ReadCacheBuf);

			}
			else
			{
				UINT32 already_read = 0xffffffff;
				for (int iter = 0; iter < GetLPNCount(); iter++) {
					if ((nVPPN >> NUM_BIT_LPN_PER_PAGE) == (read_VPPN[iter] >> NUM_BIT_LPN_PER_PAGE))
						already_read = iter;
				}
				if (already_read == 0xffffffff) {
					pBufEntry[i]->nVPPN = nVPPN;
					ReadCacheBuf = DFTL_GLOBAL::GetReadCacheMgr()->change_next_buffer(nLPN >> NUM_BIT_LPN_PER_PAGE, pBufEntry[i]);
					if (ReadCacheBuf == NULL)
						return false;
					stReqID.stHIL.bufOffset = i;
					DFTL_GLOBAL::GetVNandMgr()->ReadPage(stReqID, nVPPN, pBufEntry[i]->m_pMainBuf, pBufEntry[i]->m_pSpareBuf);
					read_VPPN[i] = nVPPN;

					pBufEntry[i]->refCount++;
					pBufEntry[i]->readtype = 2;


					list_del(&pBufEntry[i]->m_dlList);
					DFTL_GLOBAL::GetBufferMgr()->Release(ReadCacheBuf);

				}
				else {
					pBufEntry[already_read]->refCount++;
					(BufOffset[i]) = already_read * LPN_PER_PHYSICAL_PAGE + (INT8)LPN_OFFSET_FROM_VPPN(nVPPN);
					IncreaseDoneCount();
				}
			}

			DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ);
		}
loop_end:
		IncIssued_count();
	}

	FTL_DEBUG_PRINTF("[FTL][WAITQ][Read] LPN:%d, Count: %d \r\n", pstRequest->nLPN, pstRequest->nLPNCount);

	GoToNextStatus();

	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
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
	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	for (int i = 0; i < GetLPNCount(); i++) {
		//BUFFER_ENTRY *pstBufferEntry = pBufEntry;
		BUFFER_ENTRY *pstBufferEntry;
		UINT32 nBufAddr;
		UINT32 LPN = GetStartLPN() + i;
		UINT32 LPN_offset = (LPN) % LPN_PER_PHYSICAL_PAGE;

		pstBufferEntry = pBufEntry[(BufOffset[i] >> NUM_BIT_LPN_PER_PAGE)];
		LPN_offset = BufOffset[i] % LPN_PER_PHYSICAL_PAGE;

		BUFFER_ENTRY* ReadCacheBuf;
		ReadCacheBuf = DFTL_GLOBAL::GetReadCacheMgr()->get_buffer_by_VPPN(pstBufferEntry->nVPPN >> NUM_BIT_LPN_PER_PAGE);
		if (ReadCacheBuf == NULL) {
			if (pstBufferEntry->nVPPN != 0xffffffff)
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



		//nBufAddr = (UINT32)pstBufferEntry->m_pMainBuf + (LOGICAL_PAGE_SIZE * *BufOffset);
		nBufAddr = (UINT32)pstBufferEntry->m_pMainBuf + (LOGICAL_PAGE_SIZE * LPN_offset);
		DFTL_GLOBAL::GetHDMAMgr()->IssueTxDMA(m_parent_req->GetHostCmdSlotTag(), m_nDMA_offset + i, nBufAddr);

#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
		// Set LPN on main buffer to data verification
		if (*((UINT32 *)pstBufferEntry->m_pSpareBuf) != DATA_VERIFICATION_INVALID_LPN)		// check written page
		{
			//DEBUG_ASSERT(((unsigned int *)pstRequest->pSpareBuf)[nLPNOffset] == pstRequest->nLPN);
			if (((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset] != (LPN))
			{
				PRINTF("[FTL] (1)LPN mismatch, request LPN: %d, SpareLPN: %d \n\r",
					(LPN), ((UINT32 *)pstBufferEntry->m_pSpareBuf)[LPN_offset]);
				DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_MISCOMAPRE);
			}

			if (*(UINT32 *)nBufAddr != (LPN))
			{

				PRINTF("[FTL] (2)LPN mismatch, request LPN: %d, SpareLPN: %d \n\r",
					(LPN), ((UINT32 *)pstBufferEntry->m_pMainBuf)[LPN_offset]);
				DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_MISCOMAPRE);
			}

		}
#endif
		m_parent_req->IncreaseDoneCount();
		if (m_parent_req->GetDoneCount() == m_parent_req->GetLPNCount()) {
			pstHILRequestInfo->RemoveFromIssuedQ(m_parent_req);
			pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(m_parent_req);
		}
	}

	HDMA*	pstHDMA = DFTL_GLOBAL::GetHDMAMgr();
	SetHDMAIssueInfo(pstHDMA->GetTxDMAIndex(), pstHDMA->GetTxDMAOverFlowCount());

	return 0;
}

BOOL
HIL_REQUEST_PER_WAY::_ProcessRead_Done(VOID)
{
	// Release Request
	//Remove from DoneQ
	REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
	HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

	BOOL bDMADone;
	bDMADone = DFTL_GLOBAL::GetHDMAMgr()->CheckTxDMADone(m_nDMAReqTail, m_nDMAOverFlowCount);
	if (bDMADone == TRUE) {
		BUFFER_MGR*		pstBufferMgr = DFTL_GLOBAL::GetBufferMgr();

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

	if (GetStartLPN() == ADDR_PRINT_PROFILE) {
		//DMA transfer
		BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];
		BUFFER_MGR*		pstBufferMgr = DFTL_GLOBAL::GetBufferMgr();
		
		pBufEntry[0] = pstBufferMgr->Allocate();
		if (!pBufEntry[0]) {
			return FALSE;
		}

		HDMA* pstHDMA = DFTL_GLOBAL::GetHDMAMgr();
		pstHDMA->IssueRxDMA(GetHostCmdSlotTag(), 0, (unsigned int)pBufEntry[0]->m_pMainBuf);
		SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());

		//DMA wait
		DFTL_GLOBAL::GetHDMAMgr()->WaitRxDMADone(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());
		DFTL_GLOBAL::GetBufferMgr()->Release(pBufEntry[0]);


		DFTL_PrintProfile();


		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		pstHILRequestInfo->RemoveFromIssuedQ(m_parent_req);
		pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(m_parent_req);
		//per way
		pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
		pstHILRequestInfo->ReleaseRequest_per_way(this);

		return TRUE;
	}


	//Normal I/O
	do
	{
		if (_IsWritable(m_channel, m_way) == FALSE)
		{
			bSuccess = FALSE;
			break;
		}


#if (SUPPORT_META_DEMAND_LOADING == 1)
		if (_CheckAndLoadMeta() == FALSE)
		{
			bSuccess = FALSE;
			break;
		}
		if (_CheckMetaWritable() == FALSE)
		{
			bSuccess = FALSE;
			break;
		}
#endif

		
		pstActiveBlock = DFTL_GLOBAL::GetActiveBlockMgr(m_channel, m_way)->GetActiveBlock(IOTYPE_HOST);

		DEBUG_ASSERT(m_channel == CHANNEL_FROM_VPPN(pstActiveBlock->m_nCurVPPN));
		DEBUG_ASSERT(m_channel == get_channel_from_lpn(GetCurLPN()));
		DEBUG_ASSERT(m_way == WAY_FROM_VPPN(pstActiveBlock->m_nCurVPPN));
		DEBUG_ASSERT(m_way == get_way_from_lpn(GetCurLPN()));

		bSuccess = pstActiveBlock->Write(this, IOTYPE_HOST);

	} while ((GetDoneCount() < GetLPNCount()) && (bSuccess == TRUE));

	if (bSuccess == TRUE)
	{
		DEBUG_ASSERT(GetLPNCount() == GetDoneCount());
		GoToNextStatus();

		//Remove from WaitQ
		REQUEST_MGR*	pstRequestMgr = DFTL_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstHILRequestInfo = pstRequestMgr->GetHILRequestInfo();

		for (int i = 0; i < GetLPNCount(); i++) {
			m_parent_req->IncreaseDoneCount();
		}

		if (m_parent_req->GetDoneCount() == m_parent_req->GetLPNCount()) {
			pstHILRequestInfo->RemoveFromIssuedQ(m_parent_req);
			pstRequestMgr->GetHILRequestInfo()->ReleaseRequest(m_parent_req);

		}

		//per way
		pstHILRequestInfo->RemoveFromWaitQ_per_way(this, m_channel, m_way);
		pstHILRequestInfo->ReleaseRequest_per_way(this);

	}

	return bSuccess;
}
