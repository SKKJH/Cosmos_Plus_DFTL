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
//	VBlock Information 
//
///////////////////////////////////////////////////////////////////////////////
VOID
VBINFO::IncreaseInvalidate(VOID)
{
	m_nInvalidLPN++;
	DEBUG_ASSERT(m_nInvalidLPN <= IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock());
}

BOOL
VBINFO::IsFullInvalid(VOID)
{
	return (m_nInvalidLPN < IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock()) ? FALSE : TRUE;
}

VOID
VBINFO::SetFullInvalid(VOID)
{
	m_nInvalidLPN = IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock();
}

///////////////////////////////////////////////////////////////////////////////
//
//	VBlock Information Manager
//
///////////////////////////////////////////////////////////////////////////////
VOID
VBINFO_MGR::Initialize(VOID)
{
	INT32 nSize = sizeof(VBINFO) * IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount();
	m_pastVBInfo = (VBINFO*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT);

	ASSERT(GetVBSize() == (IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock() * LOGICAL_PAGE_SIZE));
}

VOID
VBINFO_MGR::Format(VOID)
{
	/*UINT32 nVBlockCount = IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount();
	INT32 nSize = sizeof(VBINFO) * nVBlockCount;
	OSAL_MEMSET(m_pastVBInfo, 0x00, nSize);

	VBINFO*		pstVBInfo;
	BOOL		bBad = TRUE;
	for (UINT32 i = 0; i < nVBlockCount; i++)
	{
		//bBad = IOUB_GLOBAL::GetVNandMgr()->IsBadBlock(i);
		if (bBad == FALSE)
		{
			pstVBInfo = GetVBInfo(i);
			INIT_LIST_HEAD(&pstVBInfo->m_dlList);
			pstVBInfo->m_nVBN = i;

			pstVBInfo->SetFullInvalid();	// to avoid debug assert
		}
	}*/
}

VBINFO*
VBINFO_MGR::GetVBInfo(UINT32 nVBN)
{
	DEBUG_ASSERT(nVBN < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());
	return &m_pastVBInfo[nVBN];
}

/*
	@brief return Size of VirtualBlock (byte)
*/
UINT32
VBINFO_MGR::GetVBSize(VOID)
{
	return ((1 << NAND_ADDR_VBN_SHIFT) * LOGICAL_PAGE_SIZE);
}

///////////////////////////////////////////////////////////////////////////////
//
//	Block Manager
//
///////////////////////////////////////////////////////////////////////////////
VOID
BLOCK_MGR::Initialize(BLOCK_MGR_TYPE eType)
{
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
		for (UINT32 way = 0; way < USER_WAYS; way++) {
			// Nothing to do
			INIT_LIST_HEAD(&m_dlFreeBlocks[channel][way]);
			m_nFreeBlocks[channel][way] = 0;

			INIT_LIST_HEAD(&m_dlUsedBlocks[channel][way][METATYPE_PAGE]);
			INIT_LIST_HEAD(&m_dlUsedBlocks[channel][way][METATYPE_BLOCK]);

			m_nUsedBlocks[channel][way][METATYPE_PAGE] = 0;
			m_nUsedBlocks[channel][way][METATYPE_BLOCK] = 0;
#if (IOUB_STRIPING == 1)
			INIT_LIST_HEAD(&m_dlUsedBlocks[channel][way][METATYPE_STRIPING]);
			m_nUsedBlocks[channel][way][METATYPE_STRIPING] = 0;
#endif
		}
	}

	m_eType = eType;

	m_bFormatted = FALSE;
}

VOID
BLOCK_MGR::_FormatUser(VOID)
{
	UINT32 nVBlockCount = IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount();
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
		for (UINT32 way = 0; way < USER_WAYS; way++) {
			VNAND*		pstVNandMgr = IOUB_GLOBAL::GetVNandMgr();
			VBINFO_MGR* pstVBInfoMgr = IOUB_GLOBAL::GetVBInfoMgr(channel, way);

			VBINFO*		pstVBInfo;
			BOOL		bBad;
			for (UINT32 i = 0; i < nVBlockCount; i++)
			{
				pstVBInfo = pstVBInfoMgr->GetVBInfo(i);

				bBad = pstVNandMgr->IsBadBlock(channel, way, i);
				if (bBad == FALSE)
				{
					INIT_LIST_HEAD(&pstVBInfo->m_dlList);
					pstVBInfo->m_nVBN = i;

					pstVBInfo->SetFullInvalid();	// to avoid debug assert

					m_nUsedBlocks[channel][way][METATYPE_PAGE]++;				// to avoid underflow by release without allocation
					Release(channel, way, i, METATYPE_PAGE);
				}
				else
				{
					pstVBInfo->SetBad();
					pstVBInfo->ClearUser();
					pstVBInfo->ClearGC();
				}
			}
		}
	}
}

VOID
BLOCK_MGR::_FormatMeta(VOID)
{

	UINT32			nMetaVBlockCount = 0;
	if (m_mType == METATYPE_PAGE) {
		// 36864 pages
	}
	else {
		//8192 blocks
	}

	

	nMetaVBlockCount = MAX(nMetaVBlockCount, META_VBLOCK_COUNT_MIN); 
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
		for (UINT32 way = 0; way < USER_WAYS; way++) {
			VBINFO_MGR* pstVBInfoMgr = IOUB_GLOBAL::GetVBInfoMgr(channel, way);

			// Get blocks from user block
			BLOCK_MGR*	pstUserBlockMgr = IOUB_GLOBAL::GetUserBlockMgr();
			UINT32		nVBN;
			VBINFO*		pstVBInfo;

			for (UINT32 i = 0; i < nMetaVBlockCount; i++)
			{
				nVBN = pstUserBlockMgr->Allocate(channel, way, TRUE, FALSE, FALSE, IOTYPE_PAGE); //allocated for metadata block

				DEBUG_ASSERT(IOUB_GLOBAL::GetVNandMgr()->IsBadBlock(channel, way,  nVBN) == FALSE);

				pstVBInfo = pstVBInfoMgr->GetVBInfo(nVBN);
				list_del_init(&pstVBInfo->m_dlList);

				pstVBInfo->SetFullInvalid();	// to avoid debug assert

				m_nUsedBlocks[channel][way][METATYPE_PAGE]++;				// to avoid underflow by release without allocation
				Release(channel, way, nVBN, METATYPE_PAGE);
			}
		}
	}
}

VOID
BLOCK_MGR::Format(VOID)
{
	if (m_bFormatted == TRUE)
	{
		return;
	}

	if (m_eType == META_BLOCK_MGR)
	{
		_FormatMeta();
	}
	else
	{
		DEBUG_ASSERT(m_eType == USER_BLOCK_MGR);
		_FormatUser();
	}

	m_bFormatted = TRUE;
}

UINT32
BLOCK_MGR::Allocate(UINT32 channel, UINT32 way, BOOL bUser, BOOL bGC, BOOL bMeta, IOTYPE mtype)
{
	DEBUG_ASSERT(m_nFreeBlocks[channel][way] > 0);
	DEBUG_ASSERT(m_nUsedBlocks[channel][way][mtype] < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());
	DEBUG_ASSERT((bMeta == TRUE) ? (m_eType == META_BLOCK_MGR) : TRUE);

	VBINFO* pstVBInfo = list_first_entry(&m_dlFreeBlocks[channel][way], VBINFO, m_dlList);


	list_del_init(&pstVBInfo->m_dlList);
	list_add_tail(&pstVBInfo->m_dlList, &m_dlUsedBlocks[channel][way][mtype]);

	pstVBInfo->ClearFree();
	pstVBInfo->ClearUser();
	pstVBInfo->ClearGC();
	pstVBInfo->ClearMeta();

	if (bUser == TRUE)
	{
		pstVBInfo->SetUser();
	}
	else if (bGC == TRUE)
	{
		pstVBInfo->SetGC();
	}
	else if (bMeta == TRUE)
	{
		pstVBInfo->SetMeta();
	}
	else
	{
		ASSERT(0);
	}

	pstVBInfo->SetActive();
	pstVBInfo->SetInvalidPageCount(0);

	m_nFreeBlocks[channel][way]--;
	m_nUsedBlocks[channel][way][mtype]++;

	return pstVBInfo->m_nVBN;
}

VOID
BLOCK_MGR::Release(UINT32 channel, UINT32 way, UINT32 nVBN, METATYPE mtype)
{
	DEBUG_ASSERT(nVBN < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());
	DEBUG_ASSERT(m_nUsedBlocks[channel][way][mtype] > 0);
	DEBUG_ASSERT(m_nFreeBlocks[channel][way] < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());

	VBINFO*		pstVBInfo;

	pstVBInfo = IOUB_GLOBAL::GetVBInfoMgr(channel, way)->GetVBInfo(nVBN);

	DEBUG_ASSERT(pstVBInfo->IsFullInvalid() == TRUE);

	pstVBInfo->SetFree();

	list_del_init(&pstVBInfo->m_dlList);
	m_nUsedBlocks[channel][way][mtype]--;

	// Add to free block list
	list_add_tail(&pstVBInfo->m_dlList, &m_dlFreeBlocks[channel][way]);
	m_nFreeBlocks[channel][way]++;
}

VOID
BLOCK_MGR::Invalidate(UINT32 nLPN, UINT32 nVPPN)
{
	UINT32 nVBN = VBN_FROM_VPPN(nVPPN);
	UINT32 channel = CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way = WAY_FROM_VPPN(nVPPN);

	DEBUG_ASSERT(nVBN < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());

	VBINFO*		pstVBInfo;
	pstVBInfo = IOUB_GLOBAL::GetVBInfoMgr(channel, way)->GetVBInfo(nVBN);

	pstVBInfo->IncreaseInvalidate();

	if (pstVBInfo->IsFullInvalid() == TRUE)
	{
		Release(channel, way, nVBN, METATYPE_PAGE);
		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_FULL_INVALID_BLOCK, 1);
	}
}

VOID
BLOCK_MGR::CheckVPC(UINT32 channel, UINT32 way, UINT32 nVBN)
{
	DEBUG_ASSERT(nVBN < IOUB_GLOBAL::GetVNandMgr()->GetVBlockCount());

	VBINFO*		pstVBInfo;
	pstVBInfo = IOUB_GLOBAL::GetVBInfoMgr(channel, way)->GetVBInfo(nVBN);

	UINT32 nValidVPN_VBInfo;

	nValidVPN_VBInfo = IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock() - pstVBInfo->GetInvalidLPNCount();

	VNAND* pstVNandMgr = IOUB_GLOBAL::GetVNandMgr();
	UINT32 nValidVPN_VNand = pstVNandMgr->GetValidVPNCount(channel, way, nVBN);

	DEBUG_ASSERT(nValidVPN_VBInfo == nValidVPN_VNand);
}

