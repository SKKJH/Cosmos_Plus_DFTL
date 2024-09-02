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

VOID
META_CACHE_ENTRY::Initialize(VOID)
{
	ASSERT(0);
}

VOID
META_CACHE::Initialize(VOID)
{
	ASSERT(0);
}

BOOL
META_CACHE::Format(VOID)
{
	ASSERT(0);

	return FALSE;
}

META_CACHE_ENTRY*
META_CACHE::GetMetaEntry(UINT32 nLPN)
{
	ASSERT(0);
	return NULL;
}

BOOL
META_CACHE::IsMetaAvailable(UINT32 nLPN)
{
	ASSERT(0);
	return FALSE;
}

BOOL
META_CACHE::IsMetaWritable(UINT32 nLPN)
{
	ASSERT(0);

	return FALSE;
}


VOID
META_CACHE::LoadMeta(UINT32 nLPN)
{
	ASSERT(0);
}

UINT32
META_CACHE::GetL2V(UINT32 nLPN)
{
	ASSERT(0);
	return 0;
}

/*
	@brief	set a new VPPN,
	@return	OldVPPN
*/
UINT32
META_CACHE::SetL2V(UINT32 nLPN, UINT32 nVPPN)
{
	ASSERT(0);

	return 0;
}

UINT32
META_CACHE::_GetMetaLPN(UINT32 nLPN)
{
	ASSERT(0);
	return 0;
}

UINT32
META_CACHE::_GetMetaoffset(UINT32 nLPN)
{
	ASSERT(0);
	return 0;
}
VOID
META_CACHE::_Release(META_CACHE_ENTRY* pstEntry)
{
	ASSERT(0);
}

/*
	@brief	allocate a cache entry
*/

META_CACHE_ENTRY*
META_CACHE::_Allocate(VOID)
{
	ASSERT(0);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	Meta Manager
//
///////////////////////////////////////////////////////////////////////////////////

VOID
META_MGR::Initialize(VOID)
{
	if (meta_type == METATYPE_PAGE) 
	{
		UINT32 total_count = IOUB_partition_PMsize;

		m_panL2V = (UINT32*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, total_count * sizeof(UINT32), OSAL_MEMALLOC_FW_ALIGNMENT);
	}
	else if(meta_type == METATYPE_BLOCK)
	{
		UINT32 total_lpn = IOUB_GLOBAL::GetInstance()->GetLPNCount();
		UINT32 total_count = total_lpn >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_PPAGE);
		total_count += USER_WAYS * USER_CHANNELS - 1;
		total_count &= 0xfffffff0;
		m_panL2V = (UINT32*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, total_count * sizeof(UINT32), OSAL_MEMALLOC_FW_ALIGNMENT);
	}
#if (IOUB_STRIPING == 1)
	else if (meta_type == METATYPE_STRIPING)
	{
		UINT32 total_lpn = IOUB_GLOBAL::GetInstance()->GetLPNCount();
		UINT32 total_count = total_lpn >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_PPAGE);
		total_count += USER_WAYS * USER_CHANNELS - 1;
		total_count &= 0xfffffff0;
		m_panL2V = (UINT32*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, total_count * sizeof(UINT32), OSAL_MEMALLOC_FW_ALIGNMENT);
	}
#endif
	else {
		ASSERT(0);
	}

	m_bFormatted = FALSE;
}

BOOL
META_MGR::Format(VOID)
{
	if (m_bFormatted == TRUE)
	{
		return TRUE;
	}

	BOOL	bRet;

	if (meta_type == METATYPE_PAGE) {
		UINT32 total_count = IOUB_partition_PMsize;
		OSAL_MEMSET(m_panL2V, 0xFF, total_count * sizeof(UINT32));		// set invalid LPN
	}
	else if(meta_type == METATYPE_BLOCK) {
		UINT32 total_lpn = IOUB_GLOBAL::GetInstance()->GetLPNCount();
		UINT32 total_count = total_lpn >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_PPAGE);
		total_count += USER_WAYS * USER_CHANNELS - 1;
		total_count &= 0xfffffff0;
		OSAL_MEMSET(m_panL2V, 0xFF, total_count * sizeof(UINT32));		// set invalid LPN
	}
#if (IOUB_STRIPING == 1)
	else if (meta_type == METATYPE_STRIPING) {
		UINT32 total_lpn = IOUB_GLOBAL::GetInstance()->GetLPNCount();
		UINT32 total_count = total_lpn >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_PPAGE);
		total_count += USER_WAYS * USER_CHANNELS - 1;
		total_count &= 0xfffffff0;
		OSAL_MEMSET(m_panL2V, 0xFF, total_count * sizeof(UINT32));		// set invalid LPN
	}
#endif
	else {
		ASSERT(0);
	}

	bRet = TRUE;


	if (bRet == TRUE)
	{
		m_nVPC = 0;
		m_bFormatted = TRUE;
	}

	return bRet;
}

BOOL
META_MGR::IsMetaAvailable(UINT32 nLPN)
{

	return TRUE;

}

BOOL
META_MGR::IsMetaWritable(UINT32 nLPN)
{
	return TRUE;
}

VOID
META_MGR::LoadMeta(UINT32 nLPN)
{
	return;
}

VOID
META_MGR::LoadDone(META_CACHE_ENTRY* pstMetaEntry, VOID* pBuf)
{
	DEBUG_ASSERT(FALSE == pstMetaEntry->m_bValid);
	DEBUG_ASSERT(FALSE == pstMetaEntry->m_bDirty);
	DEBUG_ASSERT(TRUE == pstMetaEntry->m_bIORunning);

	pstMetaEntry->m_bIORunning = FALSE;
	pstMetaEntry->m_bDirty = FALSE;
	pstMetaEntry->m_bValid = TRUE;

	OSAL_MEMCPY(&pstMetaEntry->m_anL2V[0], pBuf, META_VPAGE_SIZE);
}

VOID
META_MGR::StoreDone(META_CACHE_ENTRY* pstMetaEntry)
{
	DEBUG_ASSERT(TRUE == pstMetaEntry->m_bValid);
	DEBUG_ASSERT(TRUE == pstMetaEntry->m_bDirty);
	DEBUG_ASSERT(TRUE == pstMetaEntry->m_bIORunning);

	pstMetaEntry->m_bIORunning = FALSE;
	pstMetaEntry->m_bDirty = FALSE;
}

UINT32
META_MGR::GetL2V(UINT32 nLPN)
{
	UINT32 nVPPN;
	DEBUG_ASSERT(nLPN < IOUB_GLOBAL::GetInstance()->GetLPNCount());

	if (meta_type == METATYPE_BLOCK)
	{
		UINT32 blockmap_lpn = nLPN;
		UINT32 nLBN = get_lbn_from_lpn(blockmap_lpn);
		UINT32 nChannel = get_channel_from_lpn(blockmap_lpn);
		UINT32 nWay = get_way_from_lpn(blockmap_lpn);
		UINT32 page_offset = PAGE_OFFSET_FROM_LPN(blockmap_lpn);
		UINT32 lpn_offset = blockmap_lpn % LPN_PER_PHYSICAL_PAGE;
		
		IOTYPE type = IOTYPE_BLOCK;

		
		ACTIVE_BLOCK* pstActiveBlk = IOUB_GLOBAL::GetActiveBlockMgr(nChannel, nWay)->GetActiveBlockptr(get_lbn_from_lpn(nLPN), type);
		BOOL isBuffered = FALSE;
		
		if (pstActiveBlk != NULL)
			isBuffered = pstActiveBlk->isBufferedOffset(lpn_offset);
		if (pstActiveBlk != NULL && pstActiveBlk->m_nOffset > page_offset) {
			nVPPN = GET_VPPN_FROM_VPN_VBN(nChannel, nWay, (page_offset * LPN_PER_PHYSICAL_PAGE + lpn_offset), pstActiveBlk->m_nVBN);
		}
		else if (pstActiveBlk != NULL && pstActiveBlk->m_nOffset == page_offset && isBuffered) {
			nVPPN = GET_VPPN_FROM_VPN_VBN(nChannel, nWay, (page_offset * LPN_PER_PHYSICAL_PAGE + lpn_offset), pstActiveBlk->m_nVBN);
		}
		else {
			UINT32 block_offset = (nLBN << (NUM_BIT_WAY + CHANNEL_BITS)) + (nWay << (CHANNEL_BITS)) + nChannel;
			UINT32 L2V_value = m_panL2V[block_offset];

			UINT32 nVBN;


			if (L2V_value != INVALID_VBN) {
				nVBN = (L2V_value >> (NUM_BIT_VPAGE)) & NAND_ADDR_BLOCK_MASK;
				DEBUG_ASSERT(nWay == WAY_FROM_VPPN(L2V_value));
				DEBUG_ASSERT(nChannel == CHANNEL_FROM_VPPN(L2V_value));
				nVPPN = GET_VPPN_FROM_VPN_VBN(nChannel, nWay, (page_offset * LPN_PER_PHYSICAL_PAGE + lpn_offset), nVBN);
				
			}
			else
				nVPPN = INVALID_VBN;
		}

	}
	else if (meta_type == METATYPE_PAGE)
	{
		nVPPN = m_panL2V[nLPN];
	}
	else
	{
		nVPPN = m_panL2V[nLPN];
	}

	return nVPPN;

}

UINT32 META_MGR::GetL2V_value(UINT32 nLPN)
{
	UINT32 blockmap_lpn = nLPN;
	UINT32 nLBN = get_lbn_from_lpn(blockmap_lpn);
	UINT32 nChannel = get_channel_from_lpn(blockmap_lpn);
	UINT32 nWay = get_way_from_lpn(blockmap_lpn);
	IOTYPE type = IOTYPE_BLOCK;

	UINT32 block_offset = (nLBN << (NUM_BIT_WAY + CHANNEL_BITS)) + (nWay << (CHANNEL_BITS)) + nChannel;
	UINT32 L2V_value = m_panL2V[block_offset];

	return L2V_value;
}

VOID
META_MGR::SetL2V(UINT32 nLPN, UINT32 nVPPN)
{
	DEBUG_ASSERT(nLPN < IOUB_GLOBAL::GetInstance()->GetLPNCount());

	UINT32	nOldVPPN;


	if (meta_type == METATYPE_BLOCK)
	{

		UINT32 blockmap_lpn = nLPN;
		UINT32 nLBN = get_lbn_from_lpn(blockmap_lpn);
		UINT32 nChannel = get_channel_from_lpn(blockmap_lpn);
		UINT32 nWay = get_way_from_lpn(blockmap_lpn);
		UINT32 page_offset = PAGE_OFFSET_FROM_LPN(blockmap_lpn);
		UINT32 lpn_offset = blockmap_lpn % LPN_PER_PHYSICAL_PAGE;


		UINT32 block_offset = (nLBN << (NUM_BIT_WAY + CHANNEL_BITS)) + (nWay << (CHANNEL_BITS)) + nChannel;



		nOldVPPN = m_panL2V[block_offset];
		m_panL2V[block_offset] = nVPPN;
	
	}
	else if(meta_type == METATYPE_PAGE)
	{
		nOldVPPN = m_panL2V[nLPN];
		m_panL2V[nLPN] = nVPPN;
	}
	else {
		nOldVPPN = m_panL2V[nLPN];
		m_panL2V[nLPN] = nVPPN;
	}

	if (nOldVPPN == INVALID_PPN)
	{
		m_nVPC++;
	}
	else
	{
		if (meta_type == METATYPE_PAGE)
		{

			// Invalidate OLD PPN
			VNAND*	pstVNand = IOUB_GLOBAL::GetVNandMgr();
			DEBUG_ASSERT(pstVNand->GetV2L(nOldVPPN) == nLPN);
			IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_HOST_OVERWRITE);

			pstVNand->Invalidate(nOldVPPN);
			IOUB_GLOBAL::GetInstance()->GetUserBlockMgr()->Invalidate(nLPN, nOldVPPN);
		}
	}
}

/*
@brief	return L2p SIZE IN BYTE
*/
INT32
META_MGR::_GetL2PSize(VOID)
{
	return sizeof(UINT32) * IOUB_GLOBAL::GetInstance()->GetLPNCount();
}

///////////////////////////////////////////////////////////////////////////////////
//
//	Meta L2V Manager
//
///////////////////////////////////////////////////////////////////////////////////

VOID
META_L2V_MGR::Initialize(VOID)
{
	ASSERT(0);
}

BOOL
META_L2V_MGR::Format(VOID)
{
	ASSERT(0);
	return TRUE;
}

UINT32
META_L2V_MGR::GetMetaLPNCount(VOID)
{
	ASSERT(0);
	return 0;
}

UINT32
META_L2V_MGR::GetL2V(UINT32 nLPN)
{
	ASSERT(0);
	return	m_panL2V[nLPN];
}

VOID
META_L2V_MGR::SetL2V(UINT32 nLPN, UINT32 nVPPN)
{
	ASSERT(0);		// check option
}

INT32
META_L2V_MGR::_GetL2PSize(VOID)
{
	ASSERT(0);
	return sizeof(UINT32) * GetMetaLPNCount();
}
