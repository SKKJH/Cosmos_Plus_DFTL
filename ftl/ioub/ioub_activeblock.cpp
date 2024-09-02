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
//	PROGRAM UNIT
//
///////////////////////////////////////////////////////////////////////////////

PROGRAM_UNIT::PROGRAM_UNIT(VOID)
	:
	m_bFree(TRUE),
	m_bProgramming(FALSE),
	m_bPadding(FALSE),
	m_bReadDone(FALSE),
	m_nLPNCount(0),
	m_pstBufferEntry(NULL)
{
}

BOOL
PROGRAM_UNIT::Add(VOID* pstRequest, UINT32 nVPPN, IOTYPE eBlockType, IOTYPE eRequestType, BOOL* add_new_page, BOOL page_validity)
{
	*add_new_page = FALSE;
	
	if (m_bPadding == TRUE)
		return FALSE;
	if (m_nLPNCount == 0 && m_bReadDone == FALSE)
	{
		DEBUG_ASSERT(m_bProgramming == FALSE);
		DEBUG_ASSERT(m_bFree == TRUE);

		// allocate buffer entry on the first add
		DEBUG_ASSERT(m_pstBufferEntry == NULL);
		if(m_pstBufferEntry == NULL)
			m_pstBufferEntry = IOUB_GLOBAL::GetBufferMgr()->Allocate();
		if(m_pstBufferEntry == NULL)
			ASSERT(0);

		isInIMBuffer = 0;
		m_bFree = FALSE;
		m_nVPPN = nVPPN & (~0x3);

		for (INT32 i = 0; i < LPN_PER_PHYSICAL_PAGE; i++)
		{
			m_anLPN[i] = INVALID_LPN;
		}
		if (page_validity)
		{
			//padding read 
			return FALSE;
		}
		
	}
	else if (m_bReadDone == TRUE)
	{
		m_bReadDone = FALSE;
	}
	
	UINT32	nLPN, lpn_offset;
	void* pBufferingAddr = m_pstBufferEntry->m_pMainBuf;
	if (eBlockType == IOTYPE_PAGE || eBlockType == IOTYPE_BLOCK)
	{
		HIL_REQUEST_PER_WAY* pstHILRequest = static_cast<HIL_REQUEST_PER_WAY*>(pstRequest);
		nLPN = pstHILRequest->GetCurLPN();
		

		if (eBlockType == IOTYPE_PAGE) {
			lpn_offset = m_nLPNCount;
		}
		else {
			lpn_offset = nLPN % LPN_PER_PHYSICAL_PAGE;
		}
		pBufferingAddr = (void*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * lpn_offset));
		
		// host write request, receive data from host 
		HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
		pstHDMA->IssueRxDMA(pstHILRequest->GetHostCmdSlotTag(),  pstHILRequest->GetHDMAOffset() + pstHILRequest->GetIssued_count(), (unsigned int)pBufferingAddr);
		pstHILRequest->SetHDMAIssueInfo(pstHDMA->GetRxDMAIndex(), pstHDMA->GetRxDMAOverFlowCount());
		//Wait explicitly
		m_nDMAReqTail = pstHDMA->GetRxDMAIndex();
		m_nDMAOverFlowCount = pstHDMA->GetRxDMAOverFlowCount();

	}
	else if (eBlockType == IOTYPE_GC)
	{
		pBufferingAddr = (void*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * m_nLPNCount));
		GC_REQUEST* pstGCRequest = static_cast<GC_REQUEST*>(pstRequest);
		UINT32	nLPNOffset = LPN_OFFSET_FROM_VPPN(pstGCRequest->GetVPPN());
		void* pSrcAddr = (char*)pstGCRequest->GetBuffer()->m_pMainBuf + (LOGICAL_PAGE_SIZE * nLPNOffset);
		OSAL_MEMCPY(pBufferingAddr, pSrcAddr, LOGICAL_PAGE_SIZE);
		nLPN = pstGCRequest->GetLPN();
		lpn_offset = m_nLPNCount;
	}
	else
	{
		ASSERT(0);
	}

	if (m_anLPN[lpn_offset] == INVALID_LPN) {

		UINT32 page_offset = PAGE_OFFSET_FROM_LPN(nLPN);
		if (eBlockType == IOTYPE_BLOCK && m_nLPNCount == 0 && page_offset == 0)
			IOUB_GLOBAL::GetVNandMgr()->EraseSimul(nLPN, (nVPPN & 0xfffffffc));
		else if (eBlockType == IOTYPE_PAGE && VPN_FROM_VPPN(nVPPN) == 0)
			IOUB_GLOBAL::GetVNandMgr()->EraseSimul(nLPN, nVPPN);


		IOUB_GLOBAL::GetVNandMgr()->ProgramPageSimul(nLPN, (nVPPN & 0xfffffffc) + lpn_offset);

		m_nLPNCount++;
		*add_new_page = TRUE;
	}
	else if (m_nLPNCount == 0)
	{
		DEBUG_ASSERT(0);
	}
	else if (eBlockType == IOTYPE_BLOCK)
	{
		DEBUG_ASSERT(0);
	}


	m_anLPN[lpn_offset] = nLPN;


#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
	UINT32 nSpareAddr = (UINT32)m_pstBufferEntry->m_pSpareBuf;
	nSpareAddr = nSpareAddr + (lpn_offset * sizeof(UINT32));
	*((UINT32*)nSpareAddr) = m_anLPN[lpn_offset];

	if (eBlockType != IOTYPE_META)
	{
		UINT32 nMainAddr = (UINT32)m_pstBufferEntry->m_pMainBuf;
		nMainAddr = nMainAddr + (lpn_offset * HOST_BLOCK_SIZE);
		*((UINT32*)nMainAddr) = m_anLPN[lpn_offset];
	}
#endif

	DEBUG_ASSERT(m_nLPNCount <= LPN_PER_PHYSICAL_PAGE);

	return TRUE;
}

VOID PROGRAM_UNIT::Add_full_page(UINT32 nLPN, UINT32 nVPPN)
{
	//DEBUG_ASSERT(m_bProgramming == FALSE);
	DEBUG_ASSERT(m_bFree == TRUE);

	DEBUG_ASSERT(nLPN % LPN_PER_PHYSICAL_PAGE == 0);
	DEBUG_ASSERT(nVPPN % LPN_PER_PHYSICAL_PAGE == 0);

	isInIMBuffer = 0;
	m_bFree = FALSE;
	m_nVPPN = nVPPN & (~0x3);
#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
	UINT32 nSpareAddr = (UINT32)m_pstBufferEntry->m_pSpareBuf;
#endif

	HDMA* pstHDMA = IOUB_GLOBAL::GetHDMAMgr();
	m_nDMAReqTail = pstHDMA->GetRxDMAIndex();
	m_nDMAOverFlowCount = pstHDMA->GetRxDMAOverFlowCount();
	for (INT32 i = 0; i < LPN_PER_PHYSICAL_PAGE; i++) {
		m_anLPN[i] = nLPN + i;
#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
		*((UINT32*)nSpareAddr) = m_anLPN[i];
		nSpareAddr = nSpareAddr + (sizeof(UINT32));
#endif
		UINT32 page_offset = PAGE_OFFSET_FROM_LPN(nLPN);
		if (m_nLPNCount == 0 && page_offset == 0)
			IOUB_GLOBAL::GetVNandMgr()->EraseSimul(nLPN, (nVPPN & 0xfffffffc));

		IOUB_GLOBAL::GetVNandMgr()->ProgramPageSimul(nLPN + i, nVPPN + i);
		m_nLPNCount++;
		


		BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(get_channel_from_lpn(m_anLPN[i]), get_way_from_lpn(m_anLPN[i]), IOTYPE_BLOCK);
		pstBufferingLPN->Add(m_anLPN[i]);
	}

	DEBUG_ASSERT(m_nLPNCount <= LPN_PER_PHYSICAL_PAGE);
}

UINT32 PROGRAM_UNIT::Partial_read_page(UINT32 nLPN, UINT32 nVPPN)
{
	UINT32 modify_count = 0;
	for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++) {
		if (m_anLPN[iter] == INVALID_LPN) {
			void* pBufferingAddr, *pSrcAddr;
			m_anLPN[iter] = nLPN + iter;
			if (m_nLPNCount == 0 && iter == 0)
				IOUB_GLOBAL::GetVNandMgr()->EraseSimul(nLPN, (nVPPN & 0xfffffffc));
			IOUB_GLOBAL::GetVNandMgr()->ProgramPageSimul(nLPN + iter, (nVPPN & 0xfffffffc) + iter);
			m_nLPNCount++;

			/*
			//Buffer copy
			pBufferingAddr = m_pstBufferEntry->m_pMainBuf;
			pBufferingAddr = (void*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * iter));

			pSrcAddr = buf->m_pMainBuf;
			pSrcAddr = (void*)((UINT32)pSrcAddr + (LOGICAL_PAGE_SIZE * iter));

			OSAL_MEMCPY(pBufferingAddr, pSrcAddr, LOGICAL_PAGE_SIZE);
			*/
#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
			UINT32 nSpareAddr = (UINT32)m_pstBufferEntry->m_pSpareBuf;
			nSpareAddr = nSpareAddr + (iter * sizeof(UINT32));
			*((UINT32*)nSpareAddr) = m_anLPN[iter];
#endif
			BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(get_channel_from_lpn(m_anLPN[iter]), get_way_from_lpn(m_anLPN[iter]), IOTYPE_BLOCK);
			pstBufferingLPN->Add(m_anLPN[iter]);
			modify_count++;
		}
		DEBUG_ASSERT(m_anLPN[iter] == nLPN + iter);
	}
	DEBUG_ASSERT(m_nLPNCount <= LPN_PER_PHYSICAL_PAGE);
	return modify_count;
}


VOID PROGRAM_UNIT::pages_IM_copy(BUFFER_ENTRY * buf, UINT32 src_lpn, UINT32 nLPN, UINT32 nVPPN, UINT32 count)
{
	if (m_nLPNCount == 0)
	{
		DEBUG_ASSERT(m_bProgramming == FALSE);
		DEBUG_ASSERT(m_bFree == TRUE);

		// allocate buffer entry on the first add
		DEBUG_ASSERT(m_pstBufferEntry == NULL);
		if (count == LPN_PER_PHYSICAL_PAGE)
		{
			m_pstBufferEntry = buf;
			buf->refCount++;
			isInIMBuffer = 1;
		}
		else
		{
			m_pstBufferEntry = IOUB_GLOBAL::GetBufferMgr()->Allocate();
			isInIMBuffer = 0;
		}
		m_bFree = FALSE;
		m_nVPPN = nVPPN & (~0x3);

		for (INT32 i = 0; i < LPN_PER_PHYSICAL_PAGE; i++)
		{
			m_anLPN[i] = INVALID_LPN;
		}
	}

	void* pBufferingAddr, *pSrcAddr;
	UINT32 dst_offset = nLPN % LPN_PER_PHYSICAL_PAGE;
	UINT32 src_offset = src_lpn % LPN_PER_PHYSICAL_PAGE;
	for (UINT32 iter = 0; iter < count; iter++)
	{
		m_anLPN[dst_offset + iter] = nLPN + iter;


		if (m_nLPNCount == 0 && PAGE_OFFSET_FROM_LPN(nLPN + iter) == 0)
			IOUB_GLOBAL::GetVNandMgr()->EraseSimul(nLPN + iter, (nVPPN & 0xfffffffc));

		IOUB_GLOBAL::GetVNandMgr()->ProgramPageSimul(nLPN + iter, m_nVPPN + dst_offset + iter);
		m_nLPNCount++;
		BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(get_channel_from_lpn(m_anLPN[dst_offset + iter]), get_way_from_lpn(m_anLPN[dst_offset + iter]), IOTYPE_BLOCK);
		pstBufferingLPN->Add(m_anLPN[dst_offset + iter]);
	}
	if (count != LPN_PER_PHYSICAL_PAGE)
	{
		//Buffer copy
		pBufferingAddr = m_pstBufferEntry->m_pMainBuf;
		pBufferingAddr = (void*)((UINT32)pBufferingAddr + (LOGICAL_PAGE_SIZE * dst_offset));

		pSrcAddr = buf->m_pMainBuf;
		pSrcAddr = (void*)((UINT32)pSrcAddr + (LOGICAL_PAGE_SIZE * src_offset));

		OSAL_MEMCPY(pBufferingAddr, pSrcAddr, LOGICAL_PAGE_SIZE * count);
	}

#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
	for (UINT32 iter = 0; iter < count; iter++)
	{
		UINT32 nSpareAddr = (UINT32)m_pstBufferEntry->m_pSpareBuf;
		nSpareAddr = nSpareAddr + ((dst_offset + iter) * sizeof(UINT32));
		*((UINT32*)nSpareAddr) = m_anLPN[(dst_offset + iter)];

		/*UINT32 nMainAddr = (UINT32)m_pstBufferEntry->m_pMainBuf;
		nMainAddr = nMainAddr + ((dst_offset + iter) * HOST_BLOCK_SIZE);
		*((UINT32*)nMainAddr) = src_lpn + iter;*/
	}

#endif
	

	return;
}



VOID PROGRAM_UNIT::padding_init(UINT32 VPPN) {
	if(m_pstBufferEntry == NULL)
		m_pstBufferEntry = IOUB_GLOBAL::GetBufferMgr()->Allocate();
	if(m_pstBufferEntry == NULL)
		ASSERT(0);
	isInIMBuffer = 0;
	m_bFree = FALSE;
	m_nVPPN = VPPN & (~0x3);

	for (INT32 i = 0; i < LPN_PER_PHYSICAL_PAGE; i++)
	{
		m_anLPN[i] = INVALID_LPN;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	ACTIVE_BLOCK
//
///////////////////////////////////////////////////////////////////////////////

ACTIVE_BLOCK::ACTIVE_BLOCK(VOID)
	:
	m_nCurProgramBuffering(0),
	m_nCurVPPN(INVALID_VPPN),
	m_nVBN(INVALID_VBN),
	m_nLBN(INVALID_VBN)
{
}

BOOL ACTIVE_BLOCK::ReqIssuable()
{
	PROGRAM_UNIT*	pstProgramUnit;
	pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
	if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
	{
		return FALSE;
	}
	if(doPadding)
		return FALSE;

	return TRUE;
}
VOID ACTIVE_BLOCK::Initialize(UINT32 channel, UINT32 way, UINT32 VBN, UINT32 LBN)
{	
	DEBUG_ASSERT(m_nLBN == INVALID_VBN);
	DEBUG_ASSERT(m_nVBN == INVALID_VBN);

	m_nLBN = LBN;
	m_nVBN = VBN;

	m_nOffset = 0;
	m_nCurVPPN = GET_VPPN_FROM_VPN_VBN(channel, way, 0, VBN);

	doPadding = 0;

	OSAL_MEMSET(page_validity, 0, sizeof(UINT8) << (NUM_BIT_PPAGE));
	OpenBySC = 0;
	sec_type = 8;
	return;
}

BOOL
ACTIVE_BLOCK::_IssueProgram(UINT32 bufferOff)
{
	PROGRAM_UNIT* pstProgramUnit;
	UINT32 offset;
	if (bufferOff == 0xffffffff) 
	{ 
		pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
		offset = m_nCurProgramBuffering;
	}
	else {
		pstProgramUnit = &m_astBuffering[bufferOff];
		offset = bufferOff;
	}

	DEBUG_ASSERT(pstProgramUnit->m_bFree == FALSE);
	//DEBUG_ASSERT(pstProgramUnit->m_bProgramming == FALSE);

	FTL_REQUEST_ID	stProgramReqID;
	stProgramReqID.channel = CHANNEL_FROM_VPPN(m_nCurVPPN);
	stProgramReqID.way = WAY_FROM_VPPN(m_nCurVPPN);
	stProgramReqID.stProgram.nType = FTL_REQUEST_ID_TYPE_WRITE;
	stProgramReqID.stProgram.nActiveBlockIndex = Index;
	stProgramReqID.stProgram.nBufferingIndex = offset;
	stProgramReqID.stProgram.nIOType = m_eBlockType;
	
	// do write
	IOUB_GLOBAL::GetVNandMgr()->ProgramPage(stProgramReqID, pstProgramUnit);
	pstProgramUnit->m_bProgramming = TRUE;
	if (bufferOff == 0xffffffff)
	{
		// increase buffering index
		m_nCurProgramBuffering = INCREASE_IN_RANGE(m_nCurProgramBuffering, ACTIVE_BLOCK_BUFFERING_COUNT);
	}

	if (m_eBlockType == IOTYPE_PAGE || m_eBlockType == IOTYPE_BLOCK)
	{
		// Wait Rx DMA Done
		IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(pstProgramUnit->m_nDMAReqTail, pstProgramUnit->m_nDMAOverFlowCount);
	}


	//Read buffer clear
	BUFFER_ENTRY* ReadCacheBuf;
	UINT32 nVPPN = pstProgramUnit->GetVPPN();
	ReadCacheBuf = IOUB_GLOBAL::GetReadCacheMgr()->get_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
	if (ReadCacheBuf != NULL) {

		if (ReadCacheBuf->refCount == 0)
		{
			IOUB_GLOBAL::GetReadCacheMgr()->free_buffer_by_VPPN(nVPPN >> NUM_BIT_LPN_PER_PAGE);
		}
		else
		{
			for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
			{
				ASSERT((((UINT32 *)ReadCacheBuf->m_pSpareBuf)[iter] == pstProgramUnit->GetLPN(iter) || ((UINT32 *)ReadCacheBuf->m_pSpareBuf)[iter] == 0xffffffff));
				((UINT32 *)ReadCacheBuf->m_pSpareBuf)[iter] = pstProgramUnit->GetLPN(iter);
			}
			OSAL_MEMCPY(ReadCacheBuf->m_pMainBuf, pstProgramUnit->GetBufferEntry()->m_pMainBuf, PHYSICAL_PAGE_SIZE);

		}

	}
	m_nOffset++;

	if (m_eBlockType == IOTYPE_BLOCK) {
		list_move_head(&m_dlList, &IOUB_GLOBAL::GetActiveBlockMgr(m_Channel, m_Way)->HostBlock);
	}

	return TRUE;
}

/*
	@param	eIOType: type of pstRequest
*/
BOOL
ACTIVE_BLOCK::Write(VOID* pstRequest, IOTYPE eRequestType)
{
	BOOL add_new_page = FALSE;
	DEBUG_ASSERT(m_nCurVPPN != INVALID_VPPN);
	DEBUG_ASSERT(VBN_FROM_VPPN(m_nCurVPPN) == m_nVBN);
	PROGRAM_UNIT*	pstProgramUnit;
	HIL_REQUEST_PER_WAY* pstHILRequest;

	UINT32 LPN;

	pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
	if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
	{
		if (pstProgramUnit->m_bPadding == FALSE) {
			DEBUG_ASSERT(pstProgramUnit->m_bFree == FALSE);
			DEBUG_ASSERT(pstProgramUnit->m_nLPNCount == LPN_PER_PHYSICAL_PAGE);
		}

		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_ACTIVEBLOCK_PROGRAM_UNIT_FULL);
		// Not enough free request
		return FALSE;
	}

	
	pstHILRequest = static_cast<HIL_REQUEST_PER_WAY*>(pstRequest);
	LPN = pstHILRequest->GetCurLPN();



	//Wait Padding

	if (doPadding) {
		return FALSE;
	}

	//Padding
	if (m_eBlockType == IOTYPE_BLOCK && PAGE_OFFSET_FROM_LPN(LPN) != m_nOffset + doPadding) {
		UINT32 buffer_offset = m_nCurProgramBuffering;
		BOOL buf_alloced = FALSE;
		//Buffer allocation
		if (pstProgramUnit->GetBufferEntry() == NULL) {
			pstProgramUnit->SetBufferEntry(IOUB_GLOBAL::GetBufferMgr()->Allocate());
			if (pstProgramUnit->GetBufferEntry() == NULL)
				return FALSE;
			buf_alloced = TRUE;
		}
		BUFFER_ENTRY *buf = pstProgramUnit->GetBufferEntry();
		UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
		if (oldVPPN == INVALID_VBN) {
			if (pstProgramUnit->IsFull() == TRUE)
			{
				ASSERT(0);
				//_IssueProgram(0xffffffff);
				//IOUB_GLOBAL::GetBufferMgr()->Release(paddingBuffer[buffer_offset]);
				//paddingBuffer[buffer_offset] = NULL;
				return FALSE;
			}
			if(buf_alloced)
				OSAL_MEMSET(buf->m_pMainBuf, 0xffffffff, PHYSICAL_PAGE_SIZE);

			UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, m_nOffset + doPadding, m_nLBN);

			if (pstProgramUnit->m_nLPNCount == 0) {
				pstProgramUnit->Add_full_page(start_lpn, m_nCurVPPN);

				_IssueProgram(0xffffffff);


				m_nCurVPPN += LPN_PER_PHYSICAL_PAGE;
				IOUB_IncreaseProfile(Prof_Block_padding, LPN_PER_PHYSICAL_PAGE);
				IOUB_IncreaseProfile(Prof_Block_padding_cpbk, LPN_PER_PHYSICAL_PAGE);
				// check boundary of VBlock
				DEBUG_ASSERT(VPN_FROM_VPPN(m_nCurVPPN) != 0);
			}
			else {
				UINT32 modify_count = pstProgramUnit->Partial_read_page(start_lpn, m_nCurVPPN);


				_IssueProgram(0xffffffff);

				m_nCurVPPN += modify_count;
				IOUB_IncreaseProfile(Prof_Block_padding, modify_count);
			}

			return FALSE;
		}


		//xil_printf("Write: channel-%u way-%u blk-%u ppn-%u\r\n", m_Channel, m_Way, m_nLBN, m_nOffset + doPadding);
		if (pstProgramUnit->m_nLPNCount == 0)
		{

			ASSERT(page_validity[m_nOffset + doPadding] == 0xff);

			//padding page

			FTL_REQUEST_ID stReqID;
			stReqID.stCommon.nType = FTL_REQUEST_ID_PADDING_READ;
			stReqID.stPadding.nActiveBlockIndex = Index;
			stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
			stReqID.channel = m_Channel;
			stReqID.way = m_Way;

			//UINT32 BLKno = PBN_FROM_VPPN(oldVPPN);

			//oldVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), BLKno);
			oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;

#ifndef WIN32
			//xil_printf("	PD - ch/wy/blk: %u/%u/%u  cur: %u %u/%u/%u/%u    target: %u-%u\r\n", m_Channel, m_Way, m_nLBN,
			//		m_nOffset, isBufferedOffset(0), isBufferedOffset(1), isBufferedOffset(2), isBufferedOffset(3),
			//		PAGE_OFFSET_FROM_LPN(LPN), LPN % LPN_PER_PHYSICAL_PAGE);
#endif
			if (VBN_FROM_VPPN(oldVPPN) >= TOTAL_BLOCKS_PER_DIE) {
				xil_printf("1: %u %u %u %u \r\n", m_Channel, m_Way, VBN_FROM_VPPN(oldVPPN), m_nOffset, doPadding);
				ASSERT(0);
			}
			IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, buf->m_pMainBuf, buf->m_pSpareBuf);

			IOUB_IncreaseProfile(Prof_Block_padding, LPN_PER_PHYSICAL_PAGE);
			IOUB_IncreaseProfile(Prof_SC_real_valid, LPN_PER_PHYSICAL_PAGE);
			IOUB_IncreaseProfile(Prof_Padding_read, 1);

			pstProgramUnit->m_bPadding = TRUE;
			m_nCurProgramBuffering = INCREASE_IN_RANGE(m_nCurProgramBuffering, ACTIVE_BLOCK_BUFFERING_COUNT);
			doPadding++;

			return FALSE;
		}
		else
		{
			UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), m_nLBN);
			UINT32 modify_count = pstProgramUnit->Partial_read_page(start_lpn, m_nCurVPPN);


			_IssueProgram(0xffffffff);

			m_nCurVPPN += modify_count;
			IOUB_IncreaseProfile(Prof_Block_padding, modify_count);
			IOUB_IncreaseProfile(Prof_SC_real_valid, modify_count);

			return FALSE;
		}
	}



	if (pstProgramUnit->IsFull() == TRUE)
	{
		goto program_issue;
	}
	
	// Add to buffering
	if (pstProgramUnit->Add(pstRequest, m_nCurVPPN, m_eBlockType, eRequestType, &add_new_page, page_validity[PAGE_OFFSET_FROM_LPN(LPN)]) == 0) {
		if (pstProgramUnit->m_bPadding != TRUE)
		{ // padding for paritally valid physical page
			UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
			//UINT32 BLKno;
			FTL_REQUEST_ID stReqID;
			stReqID.stCommon.nType = FTL_REQUEST_ID_PARTIAL_READ;
			stReqID.stPadding.nActiveBlockIndex = Index;
			stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
			stReqID.channel = m_Channel;
			stReqID.way = m_Way;

			//BLKno = PBN_FROM_VPPN(oldVPPN);

			//oldVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), BLKno);
			oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;


			//xil_printf("PADDING read1 - C/W - %u/%u   validity: %x %u %u[%u]  %u \r\n", m_Channel, m_Way, page_validity[PAGE_OFFSET_FROM_LPN(LPN)], oldVPPN, m_nLBN, PAGE_OFFSET_FROM_LPN(LPN), m_nCurProgramBuffering);
			//Padding Read
			IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, pstProgramUnit->GetBufferEntry()->m_pMainBuf, pstProgramUnit->GetBufferEntry()->m_pSpareBuf);

			pstProgramUnit->m_bPadding = TRUE;
			IOUB_IncreaseProfile(Prof_Padding_read, 1);
		}
		return FALSE;
	} else if (OpenBySC){
		UINT32 offset = PAGE_OFFSET_FROM_LPN(LPN);
		UINT32 offset_in_page = LPN % LPN_PER_PHYSICAL_PAGE;

		if(pstProgramUnit->m_nLPNCount == 1 && page_validity[offset] != 0) {
			//xil_printf("	PADDING read5 - C/W- %u/%u VPN: %u LPN: %u  %u[%u]     %u  %x \r\n", m_Channel, m_Way, IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN) + (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE, LPN, m_nLBN, PAGE_OFFSET_FROM_LPN(LPN), m_nCurProgramBuffering, page_validity[offset]);
		}

		page_validity[offset] |= (UINT8)1 << offset_in_page;
		page_validity[offset] |= (UINT8)1 << (offset_in_page + LPN_PER_PHYSICAL_PAGE);
	}

	if (m_eBlockType == IOTYPE_PAGE || m_eBlockType == IOTYPE_BLOCK)
	{
		HIL_REQUEST_PER_WAY*	pstHILRequest = static_cast<HIL_REQUEST_PER_WAY*>(pstRequest);
		// Update Mapping
		if (m_eBlockType == IOTYPE_PAGE) {
			IOUB_GLOBAL::GetMetaMgr(METATYPE_PAGE)->SetL2V(pstHILRequest->GetCurLPN(), m_nCurVPPN);
		}
		UINT32 channel = CHANNEL_FROM_VPPN(m_nCurVPPN);
		UINT32 way = WAY_FROM_VPPN(m_nCurVPPN);

		// host request only
		BUFFERING_LPN* pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(channel, way, m_eBlockType);
		pstBufferingLPN->Add(pstHILRequest->GetCurLPN());
	}
	else if (m_eBlockType == IOTYPE_GC)
	{
		GC_REQUEST*	pstGCRequest = static_cast<GC_REQUEST*>(pstRequest);

		// Update Mapping
		UINT32 nLPN = pstGCRequest->GetLPN();

		IOUB_GLOBAL::GetMetaMgr(METATYPE_PAGE)->SetL2V(nLPN, m_nCurVPPN);
	}
	else
	{
		ASSERT(0);
	}


	if(page_validity[PAGE_OFFSET_FROM_LPN(LPN)] == 0xff && pstProgramUnit->IsFull() == FALSE) {
		UINT32 start_lpn = LPN & 0xfffffffc;
		UINT32 modify_count = pstProgramUnit->Partial_read_page(start_lpn, m_nCurVPPN);

		_IssueProgram(0xffffffff);
		m_nCurVPPN += modify_count;
		IOUB_IncreaseProfile(Prof_Block_padding, modify_count);
		goto ISSUE_COMP;
		//xil_printf("WRITE to partial write; C/W- %u/%u  %u[%u] %u %u \r\n", m_Channel, m_Way, m_nLBN, PAGE_OFFSET_FROM_LPN(LPN), m_nCurProgramBuffering, modify_count);
	}
	//xil_printf("	TEST to partial write; page_validity: %x   count: %u [%u] %u \r\n", page_validity[PAGE_OFFSET_FROM_LPN(LPN)], pstProgramUnit->m_nLPNCount,PAGE_OFFSET_FROM_LPN(LPN) + 1, m_nCurProgramBuffering);

	// Add to active block
	if (pstProgramUnit->IsFull() == TRUE)
	{
	program_issue:
		////FOR motivation test
		/*if (sec_type == 2)
		{
			m_nOffset++;
			ProgramDone(m_nCurProgramBuffering);
		}
		else*/
		/////////////////////////////////////////////
		// Issue
		if (!_IssueProgram(0xffffffff))
			return FALSE;
ISSUE_COMP:
		//PADDING
		UINT32 LPN_offset = PAGE_OFFSET_FROM_LPN(LPN) + 1;
		if(LPN_offset < PAGES_PER_BLOCK && page_validity[LPN_offset] != 0 && page_validity[LPN_offset] != 0xff) {
			pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
			if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
			{
				//xil_printf("PADDING read2 fail- [%u]  %u \r\n", LPN_offset, m_nCurProgramBuffering);
			} else {
				UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
				FTL_REQUEST_ID stReqID;

				//UINT32 BLKno = PBN_FROM_VPPN(oldVPPN);

				//oldVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), BLKno);

				oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;

				pstProgramUnit->padding_init(m_nCurVPPN + LPN_PER_PHYSICAL_PAGE - 1);




				stReqID.stCommon.nType = FTL_REQUEST_ID_PARTIAL_READ;
				stReqID.stPadding.nActiveBlockIndex = Index;
				stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
				stReqID.channel = m_Channel;
				stReqID.way = m_Way;



				//xil_printf("PADDING read2 - C/W- %u/%u %u %u[%u]  %u \r\n", m_Channel, m_Way, oldVPPN, m_nLBN, LPN_offset, m_nCurProgramBuffering);
				//Padding Read
				IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, pstProgramUnit->GetBufferEntry()->m_pMainBuf, pstProgramUnit->GetBufferEntry()->m_pSpareBuf);

				pstProgramUnit->m_bPadding = TRUE;
				IOUB_IncreaseProfile(Prof_Padding_read, 1);
			}
		}
	}

	if (m_eBlockType == IOTYPE_PAGE || m_eBlockType == IOTYPE_BLOCK)
	{
		HIL_REQUEST_PER_WAY*	pstHILRequest = static_cast<HIL_REQUEST_PER_WAY*>(pstRequest);

		pstHILRequest->IncreaseDoneCount();
		pstHILRequest->IncIssued_count();
	}

	if(add_new_page)
		m_nCurVPPN++;

	// check boundary of VBlock
	if (VPN_FROM_VPPN(m_nCurVPPN) == 0)
	{
		// end of vblock
		m_nCurVPPN = INVALID_VPPN;		// to go a new active block at the next write
	}
	return TRUE;
}

VOID
ACTIVE_BLOCK::IncreaseVPPN(VOID)
{
	UINT32	nCurLPageOffset = LPN_OFFSET_FROM_VPPN(m_nCurVPPN);

	if (nCurLPageOffset == IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock())
	{
		// end of block, a new vblock will be allocated at the next write
		m_nCurVPPN = INVALID_VPPN;
	}
	else
	{
		m_nCurVPPN++;
	}
}

VOID
ACTIVE_BLOCK::ProgramDone(INT32 nBufferingIndex)
{
	UINT32 channel;
	UINT32 way;
	DEBUG_ASSERT(nBufferingIndex < ACTIVE_BLOCK_BUFFERING_COUNT);
	PROGRAM_UNIT*	pstProgramUnit = &m_astBuffering[nBufferingIndex];

	DEBUG_ASSERT(pstProgramUnit->m_bProgramming == TRUE);
	DEBUG_ASSERT(pstProgramUnit->m_bFree == FALSE);
	DEBUG_ASSERT(pstProgramUnit->m_nLPNCount == LPN_PER_PHYSICAL_PAGE);

	channel = CHANNEL_FROM_VPPN(pstProgramUnit->GetVPPN());
	way = WAY_FROM_VPPN(pstProgramUnit->GetVPPN());

	if ((m_eBlockType == IOTYPE_PAGE) || (m_eBlockType == IOTYPE_BLOCK) || (m_eBlockType == IOTYPE_META))
	{
		BUFFERING_LPN*	pstBufferingLPN;


		pstBufferingLPN = IOUB_GLOBAL::GetActiveBlockBufferingLPN(channel, way, m_eBlockType);

		ASSERT(m_eBlockType != IOTYPE_META);


		// update buffering LPN Hash
		for (UINT32 i = 0; i < pstProgramUnit->m_nLPNCount; i++)
		{
			pstBufferingLPN->Remove(pstProgramUnit->GetLPN(i));
		}
	}

	// release buffer
	if (pstProgramUnit->isInIMBuffer == 0)
		IOUB_GLOBAL::GetBufferMgr()->Release(pstProgramUnit->GetBufferEntry());
	else
		pstProgramUnit->GetBufferEntry()->refCount--;
	pstProgramUnit->SetBufferEntry(NULL);

	// update active block buffering state
	pstProgramUnit->m_bFree = TRUE;
	pstProgramUnit->m_bProgramming = FALSE;
	pstProgramUnit->m_nLPNCount = 0;
	pstProgramUnit->m_bReadDone = FALSE;

}

VOID ACTIVE_BLOCK::PaddingReadDone(UINT32 bufferIndex)
{
	PROGRAM_UNIT*	pstProgramUnit = &m_astBuffering[bufferIndex];
	pstProgramUnit->m_bPadding = FALSE;

	UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, m_nOffset, m_nLBN);

	if (pstProgramUnit->m_nLPNCount == 0) {
		pstProgramUnit->Add_full_page(start_lpn, m_nCurVPPN);

		_IssueProgram(bufferIndex);

		m_nCurVPPN += LPN_PER_PHYSICAL_PAGE;
		IOUB_IncreaseProfile(Prof_Block_padding_cpbk, LPN_PER_PHYSICAL_PAGE);
		// check boundary of VBlock
		//DEBUG_ASSERT(VPN_FROM_VPPN(m_nCurVPPN) != 0);
	}
	else {
		ASSERT(0);
	}

	doPadding--;
	return;
}

VOID ACTIVE_BLOCK::PartialReadDone(UINT32 bufferIndex)
{
	PROGRAM_UNIT*	pstProgramUnit = &m_astBuffering[bufferIndex];
	pstProgramUnit->m_bPadding = FALSE;
	pstProgramUnit->m_bReadDone = TRUE;
	return;
}

VOID ACTIVE_BLOCK::IMReadDone(UINT32 bufIdx)
{
	IOUB_GLOBAL::GetIMBufferMgr()->set_buffer_readDone(bufIdx);
	return VOID();
}


BOOL
ACTIVE_BLOCK::CheckAllProgramUnitIsFree(VOID)
{
	for (INT32 i = 0; i < ACTIVE_BLOCK_BUFFERING_COUNT; i++)
	{
		if ((m_astBuffering[i].m_bFree != TRUE) ||
			(m_astBuffering[i].m_bProgramming != FALSE) || (m_astBuffering[i].m_bPadding != FALSE))
		{
			//DEBUG_ASSERT(0); //Why Assert?
			return FALSE;
		}
	}

	return TRUE;
}

BOOL ACTIVE_BLOCK::CompactOnePage(VOID)
{
	if (doPadding) {
		return FALSE;
	}
	PROGRAM_UNIT*	pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
	UINT32 LPN = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), m_nLBN);
	if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
	{
		if (pstProgramUnit->m_bPadding == FALSE) {
			DEBUG_ASSERT(pstProgramUnit->m_bFree == FALSE);
			DEBUG_ASSERT(pstProgramUnit->m_nLPNCount == LPN_PER_PHYSICAL_PAGE);
		}
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_ACTIVEBLOCK_PROGRAM_UNIT_FULL);
		// Not enough free request
		return FALSE;
	}
	BOOL buf_alloced = FALSE;
	UINT32 bufferOffset = m_nCurProgramBuffering;
	//Buffer allocation
	if (pstProgramUnit->GetBufferEntry() == NULL) {
		pstProgramUnit->SetBufferEntry(IOUB_GLOBAL::GetBufferMgr()->Allocate());
		if (pstProgramUnit->GetBufferEntry() == NULL)
			return FALSE;
		buf_alloced = TRUE;
	}
	BUFFER_ENTRY *buf = pstProgramUnit->GetBufferEntry();
	UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
	if (oldVPPN == INVALID_VBN) {
		
		if (pstProgramUnit->IsFull() == TRUE)
		{
			ASSERT(0);
			//_IssueProgram(0xffffffff);
			//IOUB_GLOBAL::GetBufferMgr()->Release(buf);
			return FALSE;
		}
		if(buf_alloced)
			OSAL_MEMSET(buf->m_pMainBuf, 0xffffffff, PHYSICAL_PAGE_SIZE);

		UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), m_nLBN);

		if (pstProgramUnit->m_nLPNCount == 0) {
			pstProgramUnit->Add_full_page(start_lpn, m_nCurVPPN);

			_IssueProgram(0xffffffff);

			m_nCurVPPN += LPN_PER_PHYSICAL_PAGE;
			IOUB_IncreaseProfile(Prof_Block_padding, LPN_PER_PHYSICAL_PAGE);
			IOUB_IncreaseProfile(Prof_Block_padding_cpbk, LPN_PER_PHYSICAL_PAGE);
			// check boundary of VBlock
		}
		else {
			UINT32 modify_count = pstProgramUnit->Partial_read_page(start_lpn, m_nCurVPPN);

			_IssueProgram(0xffffffff);

			m_nCurVPPN += modify_count;
			IOUB_IncreaseProfile(Prof_Block_padding, modify_count);
		}

		return TRUE;
	}


	//xil_printf("Compaction: channel-%u way-%u blk-%u ppn-%u\r\n", m_Channel, m_Way, m_nLBN, m_nOffset + doPadding);
	if (pstProgramUnit->m_nLPNCount == 0) {
		//padding page

		FTL_REQUEST_ID stReqID;
		stReqID.stCommon.nType = FTL_REQUEST_ID_PADDING_READ;
		stReqID.stPadding.nActiveBlockIndex = Index;
		stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
		stReqID.channel = m_Channel;
		stReqID.way = m_Way;

		//UINT32 BLKno = PBN_FROM_VPPN(oldVPPN);

		//oldVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), BLKno);
		oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;

#ifndef WIN32
		//xil_printf("	PD - ch/wy/blk: %u/%u/%u  cur: %u %u/%u/%u/%u    target: %u-%u\r\n", m_Channel, m_Way, m_nLBN,
		//		m_nOffset, isBufferedOffset(0), isBufferedOffset(1), isBufferedOffset(2), isBufferedOffset(3),
		//		PAGE_OFFSET_FROM_LPN(LPN), LPN % LPN_PER_PHYSICAL_PAGE);
#endif
		if (VBN_FROM_VPPN(oldVPPN) >= TOTAL_BLOCKS_PER_DIE) {
			xil_printf("1: %u %u %u %u \r\n", m_Channel, m_Way, VBN_FROM_VPPN(oldVPPN), m_nOffset, doPadding);
			ASSERT(0);
		}
		IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, buf->m_pMainBuf, buf->m_pSpareBuf);


		IOUB_IncreaseProfile(Prof_Block_padding, LPN_PER_PHYSICAL_PAGE);
		pstProgramUnit->m_bPadding = TRUE;
		doPadding++;
	}
	else
	{
		UINT32 start_lpn = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, m_nOffset + doPadding, m_nLBN);
		UINT32 modify_count = pstProgramUnit->Partial_read_page(start_lpn, m_nCurVPPN);


		_IssueProgram(0xffffffff);

		m_nCurVPPN += modify_count;
		IOUB_IncreaseProfile(Prof_Block_padding, modify_count);
		IOUB_IncreaseProfile(Prof_SC_real_valid, modify_count);
	}

	return TRUE;
}

BOOL ACTIVE_BLOCK::ProgramPages(UINT32 count)
{
	_IssueProgram(0xffffffff);

	m_nCurVPPN += count;
	return 0;
}

BOOL ACTIVE_BLOCK::IM_page_copies(UINT32 src_lpn, UINT32 dst_lpn, UINT32 count, BUFFER_ENTRY*	IMBuffer)
{
compact_retry:
	PROGRAM_UNIT * pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
	if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
	{
		if (pstProgramUnit->m_bPadding == FALSE) {
			DEBUG_ASSERT(pstProgramUnit->m_bFree == FALSE);
			DEBUG_ASSERT(pstProgramUnit->m_nLPNCount == LPN_PER_PHYSICAL_PAGE);
		}
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_ACTIVEBLOCK_PROGRAM_UNIT_FULL);
		// Not enough free request
		return FALSE;
	}

	if (pstProgramUnit->IsFull() == TRUE)
	{
		goto program_issue;
	}

	if (PAGE_OFFSET_FROM_LPN(dst_lpn) != m_nOffset + doPadding) {
		//if (m_nOffset + doPadding != PAGES_PER_BLOCK)
		//{
		//	if (CompactOnePage())
		//		goto compact_retry;
		//}
		//else
		//{
		//	if (CheckAllProgramUnitIsFree())
		//	{
		//		//Set L2V
		//		UINT32 nLPN = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, 0, m_nLBN);
		//		UINT32 nVPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, 0, m_nVBN);
		//		UINT32 nOldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(nLPN);

		//		IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->SetL2V(nLPN, nVPN);

		//		//return to Free block pool
		//		if (nOldVPPN != INVALID_VBN) {
		//			BLOCK_MGR* pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
		//			VBINFO_MGR* pstVBInfoMgr = IOUB_GLOBAL::GetVBInfoMgr(m_Channel, m_Way);
		//			UINT32 vbn_no = VBN_FROM_VPPN(nOldVPPN);
		//			VBINFO*		pstVBInfo = pstVBInfoMgr->GetVBInfo(vbn_no);
		//			VNAND*	pstVNand = IOUB_GLOBAL::GetVNandMgr();

		//			pstVBInfo->SetFullInvalid();
		//			for (int page = 0; page < IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock(); page++) {
		//				pstVNand->Invalidate(nOldVPPN + page);
		//			}
		//			pstVBInfo->ClearActive();
		//			pstBlockMgr->Release(m_Channel, m_Way, vbn_no, METATYPE_BLOCK);
		//		}

		//		m_nVBN = INVALID_VBN;
		//		m_nCurVPPN = INVALID_VPPN;
		//		m_nLBN = INVALID_VBN;
		//		m_nOffset = 0;
		//		static int comp_count = 0;
		//		xil_printf("%u IM - Compaction occur (Write & IM)\r\n", comp_count++);
		//	}
		//}
		ASSERT(0);
		return FALSE;
	}
	if (doPadding)
		return FALSE;

	if (IMBuffer == NULL)
	{
		ASSERT(0);
	}
	//if (IMBuffer->readDone == 0)
	//	return FALSE;

	//xil_printf("IM_page_copies: src_lpn: %u  dst_lpn: %u   CurVPPN: %u count:%u \r\n", src_lpn, dst_lpn, m_nCurVPPN, count);

	//one page copy
	pstProgramUnit->pages_IM_copy(IMBuffer, src_lpn, dst_lpn, m_nCurVPPN, count);
	if(sec_type < 3)
		IOUB_IncreaseProfile(Prof_IM_data_cpbk);
	else
		IOUB_IncreaseProfile(Prof_IM_node_cpbk);
	IMBuffer->refCount -= count;


	//is full PU
	if (pstProgramUnit->IsFull() == TRUE)
	{

	program_issue:
		// Issue
		_IssueProgram(0xffffffff);
		//m_nOffset++;
		//ProgramDone(m_nCurProgramBuffering);
	}

	m_nCurVPPN += count;

	// check boundary of VBlock
	if (VPN_FROM_VPPN(m_nCurVPPN) == 0)
	{
		// end of vblock
		m_nCurVPPN = INVALID_VPPN;		// to go a new active block at the next write
	}

	return TRUE;
}

UINT32
ACTIVE_BLOCK::ReadBufferingLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry)
{
	UINT32	nVPPN = INVALID_VPPN;

	UINT32	nBufferingIndex = (m_nCurProgramBuffering + (ACTIVE_BLOCK_BUFFERING_COUNT - 1)) % ACTIVE_BLOCK_BUFFERING_COUNT;

	// lookup from the latest to oldest
	for (INT32 j = 0; j < ACTIVE_BLOCK_BUFFERING_COUNT; j++)
	{
		if (m_astBuffering[nBufferingIndex].m_bFree == TRUE)
		{
			if (nBufferingIndex == 0)
			{
				nBufferingIndex = (ACTIVE_BLOCK_BUFFERING_COUNT - 1);
			}
			else
			{
				nBufferingIndex--;
			}
			continue;
		}

		for (INT32 nLPNIndex = LPN_PER_PHYSICAL_PAGE - 1; nLPNIndex >= 0; nLPNIndex--)
		{
			if (m_astBuffering[nBufferingIndex].GetLPN(nLPNIndex) == nLPN)
			{
				// HIT
				//PRINTF("Read from buffering LPN: %d\n", nLPN);
				//buffer DMA WAIT
				// Wait Rx DMA Done
				IOUB_GLOBAL::GetHDMAMgr()->WaitRxDMADone(m_astBuffering[nBufferingIndex].m_nDMAReqTail, m_astBuffering[nBufferingIndex].m_nDMAOverFlowCount);

				nVPPN = m_astBuffering[nBufferingIndex].GetVPPN() + nLPNIndex;


				OSAL_MEMCPY(pstBufferEntry->m_pMainBuf, m_astBuffering[nBufferingIndex].GetBufferEntry()->m_pMainBuf, PHYSICAL_PAGE_SIZE);
				for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
				{
					((UINT32*)pstBufferEntry->m_pSpareBuf)[iter] = m_astBuffering[nBufferingIndex].GetLPN(iter);
				}



#if defined(SUPPORT_DATA_VERIFICATION) || defined(WIN32)
#ifndef IOUB
				VOID* pDataAddr = pstBufferEntry->m_pMainBuf;
				for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
				{
					((UINT32*)pDataAddr)[0] = ((UINT32*)pstBufferEntry->m_pSpareBuf)[iter];
					pDataAddr = (VOID*)((UINT32)pDataAddr + (LOGICAL_PAGE_SIZE));
				}
#endif
				
#endif
				return nVPPN;
			}
		}
		if (nBufferingIndex == 0)
		{
			nBufferingIndex = (ACTIVE_BLOCK_BUFFERING_COUNT - 1);
		}
		else
		{
			nBufferingIndex--;
		}
	}

	return nVPPN;
}

UINT32 ACTIVE_BLOCK::GetFirstInvalid(VOID)
{
	for (UINT32 iter = m_nOffset; iter < PAGES_PER_BLOCK; iter++)
	{
		UINT32 offset_in_page;
		if ((page_validity[iter] & 0xf) == MAX_VALID)
		{
			continue;
		}
		for (offset_in_page = 0; offset_in_page, LPN_PER_PHYSICAL_PAGE; offset_in_page++)
		{
			if (!(page_validity[iter] & (1 << offset_in_page)))
				break;
		}
		return (iter << NUM_BIT_LPN_PER_PAGE) + offset_in_page;
	}
	return 1 << NUM_BIT_VPAGE;
}

UINT32 ACTIVE_BLOCK::GetFirstReadInvalid(VOID)
{
	for (UINT32 iter = m_nOffset; iter < PAGES_PER_BLOCK; iter++)
	{
		UINT32 offset_in_page;
		if ((page_validity[iter] & 0xf0) == 0xf0)
		{
			continue;
		}
		for (offset_in_page = 0; offset_in_page, LPN_PER_PHYSICAL_PAGE; offset_in_page++)
		{
			if (!(page_validity[iter] & (1 << (offset_in_page + LPN_PER_PHYSICAL_PAGE))))
				break;
		}
		return (iter << NUM_BIT_LPN_PER_PAGE) + offset_in_page;
	}
	return 1 << NUM_BIT_VPAGE;
}

BOOL ACTIVE_BLOCK::BackgroundWork()
{
	if (m_nLBN == INVALID_VBN)
		return FALSE;
	if(!(OpenBySC))
		return FALSE;
	if(m_nOffset + doPadding == PAGES_PER_BLOCK)
		return FALSE;

	if(doPadding)
		return FALSE;

	//Padding
	if ((page_validity[m_nOffset + doPadding] & 0xf) == MAX_VALID) {
		UINT32 offset = m_nOffset + doPadding;
		//xil_printf("BG-work : %u / %u  : %x\r\n", m_nOffset, doPadding, page_validity[m_nOffset + doPadding]);
		if (CompactOnePage()) {
			IOUB_IncreaseProfile(Prof_BG_padding, 1);
			for (UINT32 iter = 0; iter < LPN_PER_PHYSICAL_PAGE; iter++)
			{
				page_validity[offset] |= 1 << (iter + LPN_PER_PHYSICAL_PAGE);
			}
			return TRUE;
		}
//	} else if(0) {
	} else if((page_validity[m_nOffset+doPadding] & 0xf) != 0) {
		PROGRAM_UNIT*	pstProgramUnit;
		pstProgramUnit = &m_astBuffering[m_nCurProgramBuffering];
		if (pstProgramUnit->m_bProgramming == TRUE || pstProgramUnit->m_bPadding == TRUE)
		{
			//xil_printf("PADDING read3 fail- [%u]  %u \r\n", m_nOffset+doPadding, m_nCurProgramBuffering);
		} else if(pstProgramUnit->m_nLPNCount == 0 && pstProgramUnit->m_bReadDone != TRUE){
			//xil_printf("CAN padding - C/W - %u/%u  %u[%u]  page_validity: %x  \r\n", m_Channel, m_Way, m_nLBN, m_nOffset+doPadding, page_validity[m_nOffset+doPadding]);
			DEBUG_ASSERT(m_nCurVPPN != INVALID_VPPN);
			DEBUG_ASSERT(VBN_FROM_VPPN(m_nCurVPPN) == m_nVBN);
			UINT32 LPN = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), m_nLBN);


			UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
			UINT32 test_free = pstProgramUnit->m_bFree;
			FTL_REQUEST_ID stReqID;
			BUFFER_ENTRY* old_buf_ptr = pstProgramUnit->GetBufferEntry();

			//UINT32 BLKno = PBN_FROM_VPPN(oldVPPN);

			//oldVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, (m_nOffset + doPadding), BLKno);
			oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;

			pstProgramUnit->padding_init(m_nCurVPPN);




			stReqID.stCommon.nType = FTL_REQUEST_ID_PARTIAL_READ;
			stReqID.stPadding.nActiveBlockIndex = Index;
			stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
			stReqID.channel = m_Channel;
			stReqID.way = m_Way;



			//xil_printf("	PADDING read4 - C/W- %u/%u VPN: %u LPN: %u  %u[%u]     %u  %x  new VPPN: %u   old_free = %u  old_buf_ptr: %u \r\n", m_Channel, m_Way, oldVPPN, LPN, m_nLBN, m_nOffset+doPadding, m_nCurProgramBuffering, page_validity[m_nOffset+doPadding], m_nCurVPPN, test_free, old_buf_ptr);
			//Padding Read
			IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, pstProgramUnit->GetBufferEntry()->m_pMainBuf, pstProgramUnit->GetBufferEntry()->m_pSpareBuf);

			pstProgramUnit->m_bPadding = TRUE;
			IOUB_IncreaseProfile(Prof_Padding_read, 1);
			return TRUE;

			/*UINT32 LPN = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, m_nOffset + doPadding, m_nLBN);
			UINT32 oldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(LPN);
			UINT32 test_free = pstProgramUnit->m_bFree;
			FTL_REQUEST_ID stReqID;
			BUFFER_ENTRY* old_buf_ptr = pstProgramUnit->GetBufferEntry();
			oldVPPN += (m_nOffset + doPadding) * LPN_PER_PHYSICAL_PAGE;

			pstProgramUnit->padding_init(m_nCurVPPN);




			stReqID.stCommon.nType = FTL_REQUEST_ID_PARTIAL_READ;
			stReqID.stPadding.nActiveBlockIndex = Index;
			stReqID.stPadding.bufferIndex = m_nCurProgramBuffering;
			stReqID.channel = m_Channel;
			stReqID.way = m_Way;



			xil_printf("	PADDING read4 - C/W- %u/%u VPN: %u LPN: %u  %u[%u]     %u  %x  new VPPN: %u   old_free = %u  old_buf_ptr: %u \r\n", m_Channel, m_Way, oldVPPN, LPN, m_nLBN, m_nOffset+doPadding, m_nCurProgramBuffering, page_validity[m_nOffset+doPadding], m_nCurVPPN, test_free, old_buf_ptr);
			//Padding Read
			IOUB_GLOBAL::GetVNandMgr()->ReadPage(stReqID, oldVPPN, pstProgramUnit->GetBufferEntry()->m_pMainBuf, pstProgramUnit->GetBufferEntry()->m_pSpareBuf);

			pstProgramUnit->m_bPadding = TRUE;
			IOUB_IncreaseProfile(Prof_Padding_read, 1);
			return TRUE;*/
		}
	}

	return FALSE;
}

VOID ACTIVE_BLOCK::Relase()
{
	m_nVBN = INVALID_VBN;
	m_nLBN = INVALID_VBN;
	m_nCurVPPN = INVALID_PPN;
	return;
}

///////////////////////////////////////////////////////////////////////////////
//
//	BUFFERING_LPN
//

VOID
BUFFERING_LPN::Initialize(IOTYPE eIOType)
{
	OSAL_MEMSET(&m_anBufferingLPNCount[0], 0x00, sizeof(m_anBufferingLPNCount));
	m_eIOType = eIOType;
}

VOID
BUFFERING_LPN::Add(UINT32 nLPN)
{
	UINT16	nIndex = BUFFERING_LPN::GET_BUCKET_INDEX(nLPN);

	m_anBufferingLPNCount[nIndex]++;
}

VOID
BUFFERING_LPN::Remove(UINT32 nLPN)
{
	UINT16	nIndex = BUFFERING_LPN::GET_BUCKET_INDEX(nLPN);
	DEBUG_ASSERT(m_anBufferingLPNCount[nIndex] > 0);

	m_anBufferingLPNCount[nIndex]--;
}

/*
	@Brief read LPn from buffering active block
*/
UINT32
BUFFERING_LPN::ReadLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry)
{
	UINT16	nIndex = BUFFERING_LPN::GET_BUCKET_INDEX(nLPN);
	UINT32 channel, way;

	if (m_anBufferingLPNCount[nIndex] == 0)
	{
		return INVALID_PPN;
	}
	channel = get_channel_from_lpn(nLPN);
	way = get_way_from_lpn(nLPN);
	// Get Active Block
	ACTIVE_BLOCK_MGR*	pstActiveBlockMgr = IOUB_GLOBAL::GetActiveBlockMgr(channel, way);
	return pstActiveBlockMgr->ReadBufferingLPN(nLPN, pstBufferEntry, m_eIOType);
}

///////////////////////////////////////////////////////////////////////////////
//
//	ACTIVE_BLOCK_MGR
//
///////////////////////////////////////////////////////////////////////////////
VOID
ACTIVE_BLOCK_MGR::Initialize(UINT32 channel, UINT32 way)
{
	m_nCurActiveBlockPage	= 0;
	m_nCurActiveBlockBlock = 0;
	m_nCurActiveBlockGC		= 0;
	m_nCurActiveBlockMeta	= 0;
	Comp_Block_Index = INVALID_INDEX;

	for (int i = 0; i < ACTIVE_BLOCK_COUNT_PER_STREAM; i++)
	{
		m_astActiveBlockHost[LBNTYPE_PAGE][i].SetIOType(IOTYPE_PAGE);
		m_astActiveBlockHost[LBNTYPE_PAGE][i].m_Channel = channel;
		m_astActiveBlockHost[LBNTYPE_PAGE][i].m_Way = way;
		m_astActiveBlockHost[LBNTYPE_PAGE][i].Index = i;

		m_astActiveBlockHost[LBNTYPE_BLOCK][i].SetIOType(IOTYPE_BLOCK);
		m_astActiveBlockHost[LBNTYPE_BLOCK][i].m_Channel = channel;
		m_astActiveBlockHost[LBNTYPE_BLOCK][i].m_Way = way;
		m_astActiveBlockHost[LBNTYPE_BLOCK][i].Index = i;

		m_astActiveBlockGC[i].SetIOType(IOTYPE_GC);
		m_astActiveBlockMeta[i].SetIOType(IOTYPE_META);
		m_astActiveBlockMeta[i].Index = i;
	}
	m_pstCurActiveBlockPage = &m_astActiveBlockHost[LBNTYPE_PAGE][m_nCurActiveBlockPage];
	m_pstCurActiveBlockBlock = &m_astActiveBlockHost[LBNTYPE_BLOCK][m_nCurActiveBlockBlock];
	m_pstCurActiveBlockGC = &m_astActiveBlockGC[m_nCurActiveBlockGC];
	m_pstCurActiveBlockMeta = &m_astActiveBlockMeta[m_nCurActiveBlockMeta];

	m_stBufferingMgr[LBNTYPE_PAGE].Initialize(IOTYPE_PAGE);
	m_stBufferingMgr[LBNTYPE_BLOCK].Initialize(IOTYPE_BLOCK);
	
	m_Channel = channel;
	m_Way = way;

	INIT_LIST_HEAD(&HostBlock);

	for (int i = 0; i < ACTIVE_BLOCK_COUNT_PER_STREAM; i++) {
		list_add_tail(&m_astActiveBlockHost[LBNTYPE_BLOCK][i].m_dlList, &HostBlock);
	}
}

ACTIVE_BLOCK*
ACTIVE_BLOCK_MGR::_GetCurActiveBlock(IOTYPE eIOType)
{
	ACTIVE_BLOCK*		pstActiveBlock;

	switch (eIOType)
	{
	case IOTYPE_PAGE:
		pstActiveBlock = m_pstCurActiveBlockPage;
		break;

	case IOTYPE_BLOCK:
		pstActiveBlock = m_pstCurActiveBlockBlock;
		break;

	case IOTYPE_GC:
		pstActiveBlock = m_pstCurActiveBlockGC;
		break;

	case IOTYPE_META:
		pstActiveBlock = m_pstCurActiveBlockMeta;
		break;

	default:
		ASSERT(0);		// unknown type
		break;
	}

	return pstActiveBlock;
}

ACTIVE_BLOCK*
ACTIVE_BLOCK_MGR::_GoToNextActiveBlock(IOTYPE eIOType)
{
	ACTIVE_BLOCK*		pstActiveBlock;
	BOOL				bUser = FALSE;
	BOOL				bGC = FALSE;
	BOOL				bMeta = FALSE;

	BLOCK_MGR* pstBlockMgr;

	switch (eIOType)
	{
	case IOTYPE_PAGE:
		m_nCurActiveBlockPage = INCREASE_IN_RANGE(m_nCurActiveBlockPage, ACTIVE_BLOCK_COUNT_PER_STREAM);
		m_pstCurActiveBlockPage = &m_astActiveBlockHost[LBNTYPE_PAGE][m_nCurActiveBlockPage];
		pstActiveBlock = m_pstCurActiveBlockPage;
		bUser = TRUE;

		pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
		break;

	case IOTYPE_BLOCK:
		ASSERT(0);

	case IOTYPE_GC:
		m_nCurActiveBlockGC = INCREASE_IN_RANGE(m_nCurActiveBlockGC, ACTIVE_BLOCK_COUNT_PER_STREAM);
		m_pstCurActiveBlockGC = &m_astActiveBlockGC[m_nCurActiveBlockGC];
		pstActiveBlock = m_pstCurActiveBlockGC;
		bGC = TRUE;

		pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
		break;

	default:
		ASSERT(0);
		break;
	}
	DEBUG_ASSERT(pstActiveBlock->m_nCurVPPN == INVALID_PPN);

	// Clear current active status
	if (pstActiveBlock->m_nVBN != INVALID_VBN)
	{
		VBINFO* pstVBInfo = IOUB_GLOBAL::GetVBInfoMgr(m_Channel, m_Way)->GetVBInfo(pstActiveBlock->m_nVBN);
		DEBUG_ASSERT(pstVBInfo->IsActive() == TRUE);

		pstVBInfo->ClearActive();
#if (SUPPORT_BLOCK_DEBUG == 1)
		pstBlockMgr->CheckVPC(m_Channel, m_Way, pstActiveBlock->m_nVBN);
#endif
	}

	// Allocate New Block
	UINT32 nVBN = pstBlockMgr->Allocate(m_Channel, m_Way, bUser, bGC, bMeta, IOTYPE_PAGE);

	pstActiveBlock->m_nVBN = nVBN;
	pstActiveBlock->m_nCurVPPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, 0, nVBN);
	pstActiveBlock->SetIOType(eIOType);

	DEBUG_ASSERT(pstActiveBlock->CheckAllProgramUnitIsFree() == TRUE);

	switch (eIOType)
	{
	case IOTYPE_PAGE:
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_IO_ERASE, 1);
		break;

	case IOTYPE_GC:
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_GC_ERASE, 1);
		break;

	case IOTYPE_META:
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_META_ERASE, 1);
		break;

	default:
		ASSERT(0);
		break;
	}

	return pstActiveBlock;
}

/*
	@brief get active block for writes
*/
ACTIVE_BLOCK* 
ACTIVE_BLOCK_MGR::GetActiveBlock(IOTYPE eIOType)
{
	BOOL	bNeedNewBlock = FALSE;

	ACTIVE_BLOCK*		pstActiveBlock;
	pstActiveBlock = _GetCurActiveBlock(eIOType);

	if (pstActiveBlock->m_nCurVPPN == INVALID_PPN)
	{
		pstActiveBlock = _GoToNextActiveBlock(eIOType);
	}

	return pstActiveBlock;
}

/*
	@brief get active block data structure
*/
ACTIVE_BLOCK*
ACTIVE_BLOCK_MGR::GetActiveBlock(INT32 nIndex, IOTYPE eIOType)
{
	DEBUG_ASSERT(nIndex < ACTIVE_BLOCK_COUNT_PER_STREAM);

	ACTIVE_BLOCK* pstActiveBlock;
	switch (eIOType)
	{
	case IOTYPE_PAGE:
		pstActiveBlock = &m_astActiveBlockHost[LBNTYPE_PAGE][nIndex];
		break;

	case IOTYPE_BLOCK:
		pstActiveBlock = &m_astActiveBlockHost[LBNTYPE_BLOCK][nIndex];
		break;

	case IOTYPE_GC:
		pstActiveBlock = &m_astActiveBlockGC[nIndex];
		break;

	case IOTYPE_META:
		pstActiveBlock = &m_astActiveBlockMeta[nIndex];
		break;

	default:
		ASSERT(0);
		pstActiveBlock = NULL;
		break;
	}

	return pstActiveBlock;
}

ACTIVE_BLOCK * ACTIVE_BLOCK_MGR::GetActiveBlockptr(UINT32 nLBN, IOTYPE eIOType)
{
	UINT32 iter;

	for (iter = 0; iter < ACTIVE_BLOCK_COUNT_PER_STREAM; iter++) 
	{
		if (m_astActiveBlockHost[eIOType][iter].m_nLBN == nLBN) 
		{
			return &m_astActiveBlockHost[eIOType][iter];
		}
	}
	return NULL;
}

ACTIVE_BLOCK * ACTIVE_BLOCK_MGR::GetfreeActiveBlockptr(IOTYPE eIOType)
{
	UINT32 iter;

	for (iter = 0; iter < ACTIVE_BLOCK_COUNT_PER_STREAM; iter++)
	{
		if (m_astActiveBlockHost[eIOType][iter].m_nLBN == INVALID_VBN)
		{
			return &m_astActiveBlockHost[eIOType][iter];
		}
	}
	return NULL;
}

ACTIVE_BLOCK* ACTIVE_BLOCK_MGR::GetActiveBlockType(IOTYPE eIOType, UINT32 type)
{
	UINT32 iter;
	UINT32 off = ACTIVE_BLOCK_COUNT_PER_STREAM;
	UINT32 page_count = 0;
	for (iter = 0; iter < ACTIVE_BLOCK_COUNT_PER_STREAM; iter++)
	{
		if (m_astActiveBlockHost[eIOType][iter].sec_type == type)
		{
			if(page_count < m_astActiveBlockHost[eIOType][iter].m_nOffset)
			{
				off = iter;
				page_count = m_astActiveBlockHost[eIOType][iter].m_nOffset;
			}

		}
	}
	if(off != ACTIVE_BLOCK_COUNT_PER_STREAM)
	{
		return &m_astActiveBlockHost[eIOType][off];
	}
	return NULL;
}

VOID ACTIVE_BLOCK_MGR::printActiveBlock() {
	ACTIVE_BLOCK*	pstActiveBlock;
	for (int i = 0; i < ACTIVE_BLOCK_COUNT_PER_STREAM; i++)
	{
		pstActiveBlock = GetActiveBlock(i, IOTYPE_BLOCK);
		xil_printf("%u ", pstActiveBlock->m_nLBN);
	}
	xil_printf("\r\n");
}

VOID ACTIVE_BLOCK_MGR::Background_work()
{
	static UINT32 cur_offset[USER_CHANNELS][USER_WAYS] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};
	BOOL ret;
	int loop_count = 0;

	//return VOID();

	do {
		ret = m_astActiveBlockHost[IOTYPE_BLOCK][cur_offset[m_Channel][m_Way]].BackgroundWork();

		cur_offset[m_Channel][m_Way] = (cur_offset[m_Channel][m_Way] + 1) % ACTIVE_BLOCK_COUNT_PER_STREAM;
		loop_count++;
	}while(ret == FALSE && loop_count < ACTIVE_BLOCK_COUNT_PER_STREAM);


	return VOID();
}

UINT32
ACTIVE_BLOCK_MGR::ReadBufferingLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry, IOTYPE eIOType)
{
	ACTIVE_BLOCK*	pstActiveBlock;
	UINT32			nVPPN;
	if (eIOType == IOTYPE_PAGE) {
		UINT32 targetVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_PAGE)->GetL2V(nLPN);
		nVPPN = m_pstCurActiveBlockPage->ReadBufferingLPN(nLPN, pstBufferEntry);

		if (nVPPN != targetVPPN) {
			nVPPN = m_pstCurActiveBlockGC->ReadBufferingLPN(nLPN, pstBufferEntry);
		}
		ASSERT((nVPPN == INVALID_VPPN) || nVPPN == targetVPPN);
	}
	else {
		for (int i = 0; i < ACTIVE_BLOCK_COUNT_PER_STREAM; i++)
		{
			pstActiveBlock = GetActiveBlock(i, eIOType);
			nVPPN = pstActiveBlock->ReadBufferingLPN(nLPN, pstBufferEntry);
			if (nVPPN != INVALID_VPPN)
			{
				break;
			}
		}
	}
	return nVPPN;
}

UINT32
ACTIVE_BLOCK_MGR::GetActiveBlockIndex(IOTYPE eIOType)
{
	switch (eIOType)
	{
	case IOTYPE_PAGE:
		return m_nCurActiveBlockPage;

	case IOTYPE_BLOCK:
		ASSERT(0);

	case IOTYPE_GC:
		return m_nCurActiveBlockGC;

	case IOTYPE_META:
		return m_nCurActiveBlockMeta;

	default:
		ASSERT(0);
		break;
	}

	return INVALID_INDEX;
}

BOOL ACTIVE_BLOCK_MGR::IsCompactionRunning(VOID)
{
	if (Comp_Block_Index == INVALID_INDEX)
		return FALSE;
	return TRUE;
}

VOID ACTIVE_BLOCK_MGR::StartCompaction(UINT32 input_index)
{
	if (Comp_Block_Index == INVALID_INDEX) {
		if (input_index == INVALID_INDEX) {
			Comp_Block_Index = GetCompactBlock();

			xil_printf("start_compaction: %u %u select: %u \r\n", m_Channel, m_Way, Comp_Block_Index);
			printActiveBlock();
		}
		else {
			Comp_Block_Index = input_index;
		}
		/*if(m_astActiveBlockHost[IOTYPE_BLOCK][Comp_Block_Index].m_nOffset != PAGES_PER_BLOCK)
			xil_printf("startCompaction - off: %u lbn: %u page_offset: %u sec_type: %u \r\n", Comp_Block_Index,
				m_astActiveBlockHost[IOTYPE_BLOCK][Comp_Block_Index].m_nLBN,
				m_astActiveBlockHost[IOTYPE_BLOCK][Comp_Block_Index].m_nOffset,
				m_astActiveBlockHost[IOTYPE_BLOCK][Comp_Block_Index].sec_type);*/
	}
	return;
}

BOOL ACTIVE_BLOCK_MGR::ProcessCompaction(VOID)
{
	ACTIVE_BLOCK *pstActiveBlock = &m_astActiveBlockHost[IOTYPE_BLOCK][Comp_Block_Index];
compact_retry:
	if (pstActiveBlock->m_nOffset + pstActiveBlock->doPadding != PAGES_PER_BLOCK)
	{
		if (pstActiveBlock->CompactOnePage())
			goto compact_retry;
	}
	else 
	{
		if(pstActiveBlock->CheckAllProgramUnitIsFree()) 
		{
			//Set L2V
			UINT32 nLPN = GET_LPN_FROM_VPN_VBN(m_Channel, m_Way, 0, pstActiveBlock->m_nLBN);
			UINT32 nVPN = GET_VPPN_FROM_VPN_VBN(m_Channel, m_Way, 0, pstActiveBlock->m_nVBN);
			UINT32 nOldVPPN = IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->GetL2V_value(nLPN);

			IOUB_GLOBAL::GetMetaMgr(METATYPE_BLOCK)->SetL2V(nLPN, nVPN);

			//return to Free block pool
			if (nOldVPPN != INVALID_VBN) {
				BLOCK_MGR* pstBlockMgr = IOUB_GLOBAL::GetInstance()->GetUserBlockMgr();
				VBINFO_MGR* pstVBInfoMgr = IOUB_GLOBAL::GetVBInfoMgr(m_Channel, m_Way);
				UINT32 vbn_no = VBN_FROM_VPPN(nOldVPPN);
				VBINFO*		pstVBInfo = pstVBInfoMgr->GetVBInfo(vbn_no);
				VNAND*	pstVNand = IOUB_GLOBAL::GetVNandMgr();

				pstVBInfo->SetFullInvalid();
				for (int page = 0; page < IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock(); page++) {
					pstVNand->Invalidate(nOldVPPN + page);
				}
				pstVBInfo->ClearActive();
				pstBlockMgr->Release(m_Channel, m_Way, vbn_no, METATYPE_BLOCK);

			}

			pstActiveBlock->m_nVBN = INVALID_VBN;
			pstActiveBlock->m_nCurVPPN = INVALID_VPPN;
			pstActiveBlock->m_nLBN = INVALID_VBN;
			pstActiveBlock->m_nOffset = 0;
			pstActiveBlock->sec_type = 8;
			Comp_Block_Index = INVALID_INDEX;
			return TRUE;
		}
	}
	return FALSE;
}

UINT32 ACTIVE_BLOCK_MGR::GetCompactBlock(VOID)
{
	ACTIVE_BLOCK* pstActiveBlock;

	list_for_each_entry_reverse(ACTIVE_BLOCK, pstActiveBlock, &HostBlock, m_dlList)
	{
		if (pstActiveBlock->sec_type == 7)
			return pstActiveBlock->Index;
	}
	ASSERT(0);
	return NULL;
}

