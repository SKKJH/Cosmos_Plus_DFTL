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

#include "hil.h"

#include "ioub_internal.h"

IOUB_GLOBAL* IOUB_GLOBAL::m_pstInstance;

VIRTUAL VOID 
IOUB_GLOBAL::Initialize(VOID)
{
	m_pstInstance = this;
	_Initialize();

	GetVNandMgr()->Initialize();

	GetMetaMgr(METATYPE_PAGE)->meta_type = METATYPE_PAGE;
	GetMetaMgr(METATYPE_PAGE)->Initialize();
	

	GetMetaMgr(METATYPE_BLOCK)->meta_type = METATYPE_BLOCK;
	GetMetaMgr(METATYPE_BLOCK)->Initialize();

#if (IOUB_STRIPING == 1)
	GetMetaMgr(METATYPE_STRIPING)->meta_type = METATYPE_STRIPING;
	GetMetaMgr(METATYPE_STRIPING)->Initialize();
#endif

	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		for (int way = 0; way < USER_WAYS; way++) {
			GetVBInfoMgr(channel, way)->Initialize();
		}
	}
	GetUserBlockMgr()->Initialize(USER_BLOCK_MGR);		// must be formantted before meta block mgr

	GetRequestMgr()->Initialize();
	GetBufferMgr()->Initialize();
	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		for (int way = 0; way < USER_WAYS; way++) {
			GetActiveBlockMgr(channel, way)->Initialize(channel, way);
			GetGCMgr(channel, way)->Initialize(GetGCTh(), IOTYPE_GC, channel, way);
		}
	}


	_PrintInfo();
	m_MetaGCing = FALSE;

	GetIMBufferMgr()->Initialize();
	GetReadCacheMgr()->Initialize();

	xil_printf("HIL: %d  PADDING: %d  PARTIAL: %d \r\n", FTL_REQUEST_ID_TYPE_HIL_READ, FTL_REQUEST_ID_PADDING_READ, FTL_REQUEST_ID_PARTIAL_READ);
}

VIRTUAL BOOL
IOUB_GLOBAL::Format(VOID)
{
	BOOL	bRet;

	GetUserBlockMgr()->Format();

	bRet = GetMetaMgr(METATYPE_PAGE)->Format();			// THIS MUST BE AT THE END OF FORMAT ROUTINE
	bRet = GetMetaMgr(METATYPE_BLOCK)->Format();			// THIS MUST BE AT THE END OF FORMAT ROUTINE
#if (IOUB_STRIPING == 1)
	bRet = GetMetaMgr(METATYPE_STRIPING)->Format();			// THIS MUST BE AT THE END OF FORMAT ROUTINE
	GetStrinpingMgr()->Initialize();
#endif


	return bRet;
}

VIRTUAL VOID 
IOUB_GLOBAL::Run(VOID)
{
	FIL_Run();
	GetRequestMgr()->Run();

	static int way_counter = 0;
	int way, channel;

	way = way_counter % USER_WAYS;
	channel = way_counter >> NUM_BIT_WAY;

	GetGCMgr(channel, way)->CheckAndStartGC();
	GetGCMgr(channel, way)->Run();

	way_counter = (way_counter + 1) % (1 << (NUM_BIT_WAY + CHANNEL_BITS));
}

/*
	@brief	Add HIL read request to waitQ
*/
VIRTUAL VOID
IOUB_GLOBAL::ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	HIL_REQUEST*	pstRequest;

	do
	{
		// allocate request
		pstRequest = m_stRequestMgr.AllocateHILRequest();
		if (pstRequest == NULL)
		{
			Run();
		}
	} while (pstRequest == NULL);

	pstRequest->Initialize(HIL_REQUEST_READ_WAIT, NVME_CMD_OPCODE_READ, 
						nLPN, nCmdSlotTag, nCount);
	ASSERT(nCount <= REQUEST_LPN_COUNT_MAX);
	IOUB_IncreaseProfile(Prof_Host_read, nCount);
#ifndef WIN32
	//xil_printf("1	%u	%u \r\n", nLPN, nCount);
#endif
	// add to waitQ
	m_stRequestMgr.AddToHILRequestWaitQ(pstRequest);

	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ, pstRequest->GetLPNCount());
	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_READ_REQ);
}

VIRTUAL VOID
IOUB_GLOBAL::WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	HIL_REQUEST*	pstRequest;
	do
	{
		// allocate request
		pstRequest = m_stRequestMgr.AllocateHILRequest();
		if (pstRequest == NULL)
		{
			Run();
		}
	} while (pstRequest == NULL);

	pstRequest->Initialize(HIL_REQUEST_WRITE_WAIT, NVME_CMD_OPCODE_WRITE, 
		nLPN, nCmdSlotTag, nCount);
	ASSERT(nCount <= REQUEST_LPN_COUNT_MAX);
	IOUB_IncreaseProfile(Prof_Host_write, nCount);
#ifndef WIN32
	//xil_printf("7	%u	%u  - tag: %u\r\n", nLPN, nCount, nCmdSlotTag);
#endif
	// add to waitQ
	m_stRequestMgr.AddToHILRequestWaitQ(pstRequest);


	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_WRITE, pstRequest->GetLPNCount());
	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_WRITE_REQ);
}

VIRTUAL VOID
IOUB_GLOBAL::CallBack(FTL_REQUEST_ID stReqID)
{
#if (UNIT_TEST_FIL_PERF == 1)
	return;
#endif

	switch (stReqID.stCommon.nType)
	{
	case FTL_REQUEST_ID_TYPE_HIL_READ:
	{
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		HIL_REQUEST_INFO*	pstRequestInfo = pstRequestMgr->GetHILRequestInfo();
		HIL_REQUEST_PER_WAY * pstRequest = pstRequestInfo->GetRequest_per_way(stReqID.stHIL.nRequestIndex);
		//if (pstRequest->GetStartLPN() / 8192 == 15)
		//	xil_printf("ReadDone: Idx: %u\r\n", pstRequest->GetStartLPN());
		pstRequest->IncreaseDoneCount();
		pstRequest->pBufEntry[stReqID.stHIL.bufOffset]->readDone = 1;

		if (pstRequest->GetDoneCount() == pstRequest->GetLPNCount())
		{
			// all read done  
			// remove from issued Q
			pstRequestInfo->RemoveFromIssuedQ_per_way(pstRequest);

			pstRequest->HDMAIssue();
			// add to done Q
			pstRequestInfo->AddToDoneQ_per_way(pstRequest);		// wait for HDMA Issue

			pstRequest->GoToNextStatus();		// NAND Issued -> NAND_DONE
		}

		break;
	}
	case FTL_REQUEST_ID_TYPE_WRITE:
	{
		// Pysical page program done
		INT32	nIndex = stReqID.stProgram.nActiveBlockIndex;
		IOTYPE	eIOType = static_cast<IOTYPE>(stReqID.stProgram.nIOType);

		ACTIVE_BLOCK* pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(stReqID.channel, stReqID.way)->GetActiveBlock(nIndex, eIOType);
		pstActiveBlock->ProgramDone(stReqID.stProgram.nBufferingIndex);
		break;
	}
	case FTL_REQUEST_ID_PADDING_READ:
	{
		// Pysical page program done
		INT32	nIndex = stReqID.stProgram.nActiveBlockIndex;

		ACTIVE_BLOCK* pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(stReqID.channel, stReqID.way)->GetActiveBlock(nIndex, IOTYPE_BLOCK);
		pstActiveBlock->PaddingReadDone(stReqID.stPadding.bufferIndex);
		break;
	}
	case FTL_REQUEST_ID_PARTIAL_READ:
	{
		// Pysical page program done
		INT32	nIndex = stReqID.stProgram.nActiveBlockIndex;

		ACTIVE_BLOCK* pstActiveBlock = IOUB_GLOBAL::GetActiveBlockMgr(stReqID.channel, stReqID.way)->GetActiveBlock(nIndex, IOTYPE_BLOCK);
		pstActiveBlock->PartialReadDone(stReqID.stPadding.bufferIndex);
		break;
	}
	case FTL_REQUEST_ID_IM_READ:
	{

		IOUB_GLOBAL::GetIMBufferMgr()->set_buffer_readDone(stReqID.stIM.bufferIndex);
		
		break;
	}
	case FTL_REQUEST_ID_TYPE_GC_READ:
	{
		REQUEST_MGR*	pstRequestMgr = IOUB_GLOBAL::GetRequestMgr();
		GC_REQUEST_INFO*	pstRequestInfo = pstRequestMgr->GetGCRequestInfo();
		GC_REQUEST * pstRequest = pstRequestInfo->GetRequest(stReqID.stGC.nRequestIndex);
		pstRequest->GCReadDone();
		break;
	}
	default:
		ASSERT(0);
		break;
	}
}

VIRTUAL VOID 
IOUB_GLOBAL::IOCtl(IOCTL_TYPE eType)
{
	switch (eType)
	{
	case IOCTL_INIT_PROFILE_COUNT:
		m_stProfile.Initialize();
		break;

	case IOCTL_PRINT_PROFILE_COUNT:
		m_stProfile.Print();
		break;

	default:
		ASSERT(0);		// unknown type
		break;
	}

	return;
}

BOOL IOUB_GLOBAL::isBlockMappingLPN(UINT32 nLPN)
{
	if (nLPN >= IOUB_partition_start + IOUB_partition_PMsize)
		return TRUE;
	else
		return FALSE;
}

VOID
IOUB_GLOBAL::SetStatus(IOUB_STATUS eStatus)
{
	m_eStatus = static_cast<IOUB_STATUS>(m_eStatus | eStatus);
}

BOOL
IOUB_GLOBAL::CheckStatus(IOUB_STATUS eStatus)
{
	return (m_eStatus & eStatus) ? TRUE : FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
//	static function
//

VOID 
IOUB_GLOBAL::_Initialize(VOID)
{
	UINT32 nPPagesPerVBlock = m_stVNand.GetPPagesPerVBlock();
	m_nPhysicalFlashSizeKB = USER_CHANNELS * USER_WAYS * m_stVNand.GetVBlockCount() * nPPagesPerVBlock * (PHYSICAL_PAGE_SIZE / KB);

	m_nVBlockSizeKB			= nPPagesPerVBlock * PHYSICAL_PAGE_SIZE;
	m_nVPagesPerVBlock		= m_stVNand.GetVPagesPerVBlock();
	m_nLPagesPerVBlockBits	= UTIL_GetBitCount(m_nVPagesPerVBlock);
	m_nLPagesPerVBlockMask	= (1 << m_nLPagesPerVBlockBits) - 1;

	m_fOverProvisionRatio = (float)OVERPROVISION_RATIO_DEFAULT;
	m_nOverprovisionSizeKB = (INT64)(m_nPhysicalFlashSizeKB * m_fOverProvisionRatio);
	m_nLogicalFlashSizeKB = m_nPhysicalFlashSizeKB - m_nOverprovisionSizeKB;

#if (SUPPORT_STATIC_DENSITY != 0)
	UINT32 nLogicalFlashSizeKB = SUPPORT_STATIC_DENSITY * (GB / KB);

	ASSERT(m_nLogicalFlashSizeKB >= nLogicalFlashSizeKB);

	m_nLogicalFlashSizeKB = nLogicalFlashSizeKB;		// update logical flash size

	ASSERT(m_nLogicalFlashSizeKB >= nLogicalFlashSizeKB);
#endif

	m_nLPNCount			= m_nLogicalFlashSizeKB / LOGICAL_PAGE_SIZE_KB;

	m_nVBlockCount		= m_stVNand.GetVBlockCount();

#if (SUPPORT_META_BLOCK == 1)
	m_bEnableMetaBlock = TRUE;
#else
	m_bEnableMetaBlock = FALSE;
#endif

	m_nGCTh = FREE_BLOCK_GC_THRESHOLD_DEFAULT;

	HIL_SetStorageBlocks(m_nLPNCount);

	m_stProfile.Initialize();
	IM_start = FALSE;
	Initialized_for_test = FALSE;
}

VOID 
IOUB_GLOBAL::_PrintInfo(VOID)
{
	char	psFTL[] = "IOUB";

	PRINTF("[%s] Physical Density: %d MB \n\r", psFTL, m_nPhysicalFlashSizeKB / KB);
	PRINTF("[%s] Logical Density: %d MB \n\r", psFTL, m_nLogicalFlashSizeKB / KB);
}

VOID Read_Cache::Initialize()
{
	for (int i = 0; i < MAX_READ_CACHE_ENTRY; i++)
	{
		source_lpn[i] = 0xffffffff;
		nVPPN[i] = 0xffffffff;
		Buf[i] = IOUB_GLOBAL::GetBufferMgr()->Allocate();
		if (Buf[i] == NULL)
			ASSERT(0);
		Buf[i]->readDone = 1;
	}

	return VOID();
}

UINT32 Read_Cache::get_next_buffer(UINT32 src_lpn)
{
	UINT32 ret;

	UINT32 iter;
	UINT32 channel, way;
	channel = get_channel_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	way = get_way_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;

	for (iter = start_offset; iter < end_offset; iter++)
	{
		if (Buf[iter]->refCount == 0)
		{
			break;
		}
	}
	if (iter == end_offset)
		return 0xffffffff;

	ret = iter;
	source_lpn[iter] = src_lpn;

	return ret;
}

BUFFER_ENTRY * Read_Cache::change_next_buffer(UINT32 src_lpn, BUFFER_ENTRY * input_buf)
{
	BUFFER_ENTRY * ret;

	UINT32 iter;
	UINT32 channel, way;
	channel = get_channel_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	way = get_way_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;

	for (iter = start_offset; iter < end_offset; iter++)
	{
		if (Buf[iter]->refCount == 0)
		{
			break;
		}
	}
	if (iter == end_offset)
		return NULL;

	ret = Buf[iter];
	source_lpn[iter] = src_lpn;
	Buf[iter] = input_buf;
	nVPPN[iter] = input_buf->nVPPN >> NUM_BIT_LPN_PER_PAGE;
	return ret;

}

UINT32 Read_Cache::get_buffer(UINT32 src_lpn)
{
	UINT32 iter;
	UINT32 channel, way;
	channel = get_channel_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	way = get_way_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;

	for (iter = start_offset; iter < end_offset; iter++)
	{
		if (src_lpn == source_lpn[iter])
			return iter;
	}

	return 0xffffffff;
}

BUFFER_ENTRY * Read_Cache::get_buffer_by_idx(UINT32 bufIdx)
{
	return Buf[bufIdx];
}

VOID Read_Cache::set_buffer_readDone(UINT32 bufIdx)
{
	ASSERT(bufIdx < MAX_READ_CACHE_ENTRY);
	Buf[bufIdx]->readDone = 1;
	//xil_printf("ReadDone: Idx: %u\r\n", bufIdx);
	return VOID();
}

BUFFER_ENTRY * Read_Cache::get_buffer_by_VPPN(UINT32 nVPPN_input)
{
	UINT32 channel, way;
	channel = CHANNEL_FROM_VPPN(nVPPN_input << NUM_BIT_LPN_PER_PAGE);
	way = WAY_FROM_VPPN(nVPPN_input << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;
	for (int iter = start_offset; iter < end_offset; iter++)
	{
		if (nVPPN_input == nVPPN[iter])
			return Buf[iter];
	}
	return NULL;
}

VOID Read_Cache::free_buffer_by_VPPN(UINT32 nVPPN_input)
{
	UINT32 channel, way;
	channel = CHANNEL_FROM_VPPN(nVPPN_input << NUM_BIT_LPN_PER_PAGE);
	way = WAY_FROM_VPPN(nVPPN_input << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;
	for (int iter = start_offset; iter < end_offset; iter++)
	{
		if (nVPPN_input == nVPPN[iter])
		{
			Buf[iter]->nVPPN = 0xffffffff;
			nVPPN[iter] = 0xffffffff;
			source_lpn[iter] = 0xffffffff;
			return;
		}
	}

}

VOID Read_Cache::free_buffer_by_Idx(UINT32 bufIdx)
{
	Buf[bufIdx]->nVPPN = 0xffffffff;
	nVPPN[bufIdx] = 0xffffffff;
	source_lpn[bufIdx] = 0xffffffff;
	return VOID();
}

BOOL Read_Cache::has_free_buffer(UINT32 src_lpn)
{
	BOOL ret = FALSE;

	UINT32 iter;
	UINT32 channel, way;
	channel = get_channel_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	way = get_way_from_lpn(src_lpn << NUM_BIT_LPN_PER_PAGE);
	UINT32 start_offset, end_offset;
	way += channel << NUM_BIT_WAY;
	start_offset = way << READ_CAHCE_PER_WAY_BIT;
	end_offset = (way + 1) << READ_CAHCE_PER_WAY_BIT;

	for (iter = start_offset; iter < end_offset; iter++)
	{
		if (Buf[iter]->refCount == 0)
		{
			return TRUE;
		}
	}

	return ret;
}
