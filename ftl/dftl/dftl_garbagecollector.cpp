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

VOID 
GC_MGR::Initialize(UINT32 nGCTh, IOTYPE eIOType, UINT32 channel, UINT32 way)
{
	m_nThreshold	= nGCTh;
	m_eIOType		= eIOType;
	m_channel = channel;
	m_way = way;
	GC_POLICY_GREEDY::Initialize(eIOType);
}

VOID
GC_MGR::Run(VOID)
{
	if (IsGCRunning() == FALSE)
	{
		// nothing to do
		return;
	}

#if (SUPPORT_META_DEMAND_LOADING == 1)
	if (m_eIOType == IOTYPE_GC)
	{
		// Check MetaGC
		if (DFTL_GLOBAL::GetInstance()->isMetaGCing() == TRUE)
		{
			return;
		}
	}
#endif

	_Read();
}

BOOL
GC_MGR::IsGCRunning(VOID)
{
	if (m_nVictimVBN == INVALID_VBN)
	{
		return FALSE;
	}

	return TRUE;
}

VOID
GC_MGR::CheckAndStartGC(VOID)
{
	// check is there anys running GC
	if (IsGCRunning() == TRUE)
	{
		return;
	}
	UINT32 nFreeBlock;

	DFTL_GLOBAL*	pstGlobal = DFTL_GLOBAL::GetInstance();

	nFreeBlock = m_pstBlockMgr->GetFreeBlockCount(m_channel, m_way);

	if (nFreeBlock > m_nThreshold)
	{
		// enough free block
		return;
	}
	m_nVictimVBN = GetVictimVBN(m_channel, m_way);
	m_nVPC = pstGlobal->GetVPagePerVBlock() - DFTL_GLOBAL::GetVBInfoMgr(m_channel, m_way)->GetVBInfo(m_nVictimVBN)->GetInvalidLPNCount();
	m_nCurReadVPageOffset = 0;
	m_nWriteCount = 0;
	m_nIssuedCount = 0;

	if (m_eIOType == IOTYPE_META) {
		DFTL_GLOBAL::GetInstance()->SetMetaGCing();
		DFTL_IncreaseProfile(Prof_CMTGC_count);
	}
	else {
		DFTL_IncreaseProfile(Prof_GC_count);
	}

#if (SUPPORT_GC_DEBUG == 1)
	m_stDebug.Initialize();
#endif

#if (SUPPORT_BLOCK_DEBUG == 1)
	m_pstBlockMgr->CheckVPC(m_channel, m_way, m_nVictimVBN);
#endif
}

VOID
GC_MGR::IncreaseWriteCount(VOID)
{
	m_nWriteCount++;

	if (m_eIOType == IOTYPE_META) {
		DFTL_IncreaseProfile(Prof_CMTGC_write);
	}
	else {
		DFTL_IncreaseProfile(Prof_GC_write);
	}

	DEBUG_ASSERT(m_nWriteCount <= m_nVPC);

	if (m_nWriteCount == m_nVPC)
	{
		// GC Done for current victim
		m_nVictimVBN = INVALID_VBN;
#if (SUPPORT_META_DEMAND_LOADING == 1)
		if (m_eIOType == IOTYPE_META) {
			BOOL	Meta_GCing = FALSE;
			for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
				for (UINT32 way = 0; way < USER_WAYS; way++) {
					if (DFTL_GLOBAL::GetMetaGCMgr(channel, way)->IsGCRunning() == TRUE) {
						Meta_GCing = TRUE;
					}
				}
			}
			if(Meta_GCing == FALSE)
				DFTL_GLOBAL::GetInstance()->ClearMetaGCing();
		}
#endif
	}
}

VOID
GC_MGR::IncreaseIssuedCount(VOID)
{
	m_nIssuedCount++;

	DEBUG_ASSERT(m_nIssuedCount <= m_nVPC);
}

VOID
GC_MGR::_Read(VOID)
{
	DFTL_GLOBAL*	pstGlobal = DFTL_GLOBAL::GetInstance();
	VNAND*			pstVNand = pstGlobal->GetVNandMgr();
	if (m_nIssuedCount != m_nWriteCount  && m_nIssuedCount != m_nWriteCount + 1) {
		return;
	}
	do
	{
		if (m_nCurReadVPageOffset >= pstGlobal->GetVPagePerVBlock())
		{
			// end of block, GC Read Done
			return;
		}

		BOOL bValid = pstVNand->IsValid(m_channel, m_way, m_nVictimVBN, m_nCurReadVPageOffset);
		if (bValid == TRUE)
		{
			break;
		}

		m_nCurReadVPageOffset++;
		DEBUG_ASSERT(m_nCurReadVPageOffset <= pstGlobal->GetVPagePerVBlock());
	} while (1);

	REQUEST_MGR*	pstReqMgr = pstGlobal->GetRequestMgr();

	// Issue Read
	GC_REQUEST*	pstRequest;

	pstRequest = pstReqMgr->AllocateGCRequest();
	if (pstRequest == NULL)
	{
		// no more free request
		return;
	}

	UINT32	nVPPN = VPPN_FROM_VBN_VPN(m_channel, m_way, m_nVictimVBN, m_nCurReadVPageOffset);

	pstRequest->Initialize(GC_REQUEST_READ_WAIT, nVPPN, m_eIOType);

	pstReqMgr->AddToGCRequestWaitQ(pstRequest);

	m_nCurReadVPageOffset++;
	IncreaseIssuedCount();

	if (m_eIOType == IOTYPE_META) {
		DFTL_IncreaseProfile(Prof_CMTGC_read);
	}
	else {
		DFTL_IncreaseProfile(Prof_GC_read);
	}
	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//	GREEDY VICTIM SELECTION
//
///////////////////////////////////////////////////////////////////////////////

VOID 
GC_POLICY_GREEDY::Initialize(IOTYPE eIOType)
{
	switch (eIOType)
	{
	case IOTYPE_GC:
		m_pstBlockMgr = DFTL_GLOBAL::GetUserBlockMgr();
		break;

#if (SUPPORT_META_DEMAND_LOADING == 1)
	case IOTYPE_META:
		m_pstBlockMgr = DFTL_GLOBAL::GetMetaBlockMgr();
		break;
#endif

	default:
		ASSERT(0);
		break;
	}
}

/*
	@brief	choose a maximum invalid page block
			this function can be imrpoved an InvalidPageCount hash data structure
				to decrease MaxInvalidPageBlock lookup time
*/
UINT32
GC_POLICY_GREEDY::GetVictimVBN(UINT32 channel, UINT32 way)
{
	VBINFO*	pstVBInfo;
	UINT32	nVictimVBN = INVALID_VBN;
	UINT32	nMaxInvalid = 0;

	// lookup all used blocks
	// CHECKME, TBD, 성능 개선 필요.
	list_for_each_entry(VBINFO, pstVBInfo, &m_pstBlockMgr->m_dlUsedBlocks[channel][way], m_dlList)
	{
		DEBUG_ASSERT(pstVBInfo->IsFree() == FALSE);

		if (pstVBInfo->IsActive() == TRUE)
		{
			continue;
		}

		if (pstVBInfo->GetInvalidLPNCount() > nMaxInvalid)
		{
			nMaxInvalid	= pstVBInfo->GetInvalidLPNCount();
			nVictimVBN	= pstVBInfo->m_nVBN;
		}
	}

	DEBUG_ASSERT(nVictimVBN != INVALID_VBN);

	DFTL_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_BGC);

	return nVictimVBN;
}


///////////////////////////////////////////////////////////////////////////////
//
//	DEBUGGING
//
///////////////////////////////////////////////////////////////////////////////

VOID
GC_DEBUG::Initialize(VOID)
{
	INT32 nSize;
	nSize = DFTL_GLOBAL::GetInstance()->GetVPagePerVBlock() * sizeof(GC_DEBUG_FLAG);

	if (m_apstVPNStatus == NULL)
	{
		m_apstVPNStatus = static_cast<GC_DEBUG_FLAG*>(OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT));
	}

	OSAL_MEMSET(m_apstVPNStatus, 0x00, nSize);
	m_nRead = 0;
	m_nWrite = 0;
}

VOID
GC_DEBUG::SetFlag(UINT32 nVPPN, GC_DEBUG_FLAG eFlag)
{
	UINT32 modVPPN = CHANNEL_VPPN(nVPPN);
	UINT32 nVPageOffset = VPN_FROM_VPPN(modVPPN);
	UINT32 channel = CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way = WAY_FROM_VPPN(nVPPN);

	DEBUG_ASSERT((m_apstVPNStatus[nVPageOffset] & eFlag) == 0);

	m_apstVPNStatus[nVPageOffset] = static_cast<GC_DEBUG_FLAG>((UINT32)m_apstVPNStatus[nVPageOffset] | (UINT32)eFlag);

	switch (eFlag)
	{
	case GC_DEBUG_FLAG_READ:
		m_nRead++;
		break;

	case GC_DEBUG_FLAG_WRITE_ISSUE:
		m_nWrite++;
		break;

	case GC_DEBUG_FLAG_WRITE_DONE:
		DEBUG_ASSERT(m_apstVPNStatus[nVPageOffset] == GC_DEBUG_FLAG_MASK);
		break;
	}
}
