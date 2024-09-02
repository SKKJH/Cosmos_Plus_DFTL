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

UINT32 
VNAND::GetPPagesPerVBlock(VOID)
{
	return (FIL_GetPagesPerBlock());
}

UINT32 
VNAND::GetVPagesPerVBlock(VOID)
{
	return GetPPagesPerVBlock() * LPN_PER_PHYSICAL_PAGE;
}

UINT32
VNAND::GetVBlockCount(void)
{
	return (TOTAL_BLOCKS_PER_DIE);
}

VOID 
VNAND::Initialize(VOID)
{
	// create the PB map
	UINT32	nVBlockCount = GetVBlockCount();

	INT32	nSize = sizeof(VIRTUAL_BLOCK) * nVBlockCount;
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
		for (UINT32 way = 0; way < USER_WAYS; way++) {
			m_pstVB[channel][way] = (VIRTUAL_BLOCK *)OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT);
			OSAL_MEMSET(m_pstVB[channel][way], 0x00, nSize);

			INT32	nValidBitmapBytePerBlock;
			INT32	nLPagePerVBlock = IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock();
			nValidBitmapBytePerBlock = CEIL(nLPagePerVBlock, NBBY);
			nSize = nValidBitmapBytePerBlock * nVBlockCount;			// Byte count

			UINT8* pstPBValidBitmap = (UINT8*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT);
			OSAL_MEMSET(pstPBValidBitmap, 0x00, nSize);			// clear all bitmap

			nSize = sizeof(INT32) * nLPagePerVBlock * nVBlockCount;
			UINT32* pstP2L = (UINT32*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT);
			ASSERT(pstP2L);

			for (UINT32 i = 0; i < nVBlockCount; i++)
			{
				m_pstVB[channel][way][i].m_pnV2L = &pstP2L[i * nLPagePerVBlock];
				m_pstVB[channel][way][i].m_pbValid = &pstPBValidBitmap[i * nValidBitmapBytePerBlock];

				for (int j = 0; j < nLPagePerVBlock; j++)
				{
					m_pstVB[channel][way][i].m_pnV2L[j] = INVALID_LPN;
				}
			}
		}
	}
	

	
}

UINT32
VNAND::GetV2L(UINT32 nVPPN)
{
	UINT32 nVBN = VBN_FROM_VPPN(nVPPN);
	UINT32 nVPAGE = VPN_FROM_VPPN(nVPPN);
	UINT32 channel = CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way = WAY_FROM_VPPN(nVPPN);

	DEBUG_ASSERT(nVBN < GetVBlockCount());

	return m_pstVB[channel][way][nVBN].m_pnV2L[nVPAGE];
}

/*
	@brief Invalidate an Logical Page of VBLock
			Invalidated page will be be moved while GC operation
*/
VOID
VNAND::Invalidate(UINT32 nVPPN)
{
	INT32	nVBN = VBN_FROM_VPPN(nVPPN);
	INT32	nVPageOffset = VPN_FROM_VPPN(nVPPN);
	UINT32 channel = CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way = WAY_FROM_VPPN(nVPPN);

	DEBUG_ASSERT(ISSET(m_pstVB[channel][way][nVBN].m_pbValid, nVPageOffset) == TRUE);

	CLEARBIT(m_pstVB[channel][way][nVBN].m_pbValid, nVPageOffset);
}

BOOL
VNAND::IsValid(UINT32 channel, UINT32 way, UINT32 nVBN, UINT32 nVPageNo)
{
	DEBUG_ASSERT(nVBN < GetVBlockCount());
	DEBUG_ASSERT(nVPageNo < IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock());

	return ISSET(m_pstVB[channel][way][nVBN].m_pbValid, nVPageNo);
}

BOOL
VNAND::ReadPage(FTL_REQUEST_ID stReqID, UINT32 nVPPN, void * pMainBuf, void * pSpareBuf)
{
	NAND_ADDR	stNandAddr = _GetNandAddrFromVPPN(nVPPN);
	ASSERT(stNandAddr.nBlock < TOTAL_BLOCKS_PER_DIE);

	if (stReqID.stCommon.nType != FTL_REQUEST_ID_TYPE_DEBUG)
	{
		FIL_ReadPage(stReqID, stNandAddr, pMainBuf, pSpareBuf);
		IOUB_IncreaseProfile(Prof_NAND_read);

		IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_NAND_READ);
	}

	return TRUE;
}

VOID
VNAND::ReadPageSimul(UINT32 nVPPN, void * pMainBuf)
{
}

VOID
VNAND::ProgramPage(FTL_REQUEST_ID stReqID, PROGRAM_UNIT *pstProgram)
{
	NAND_ADDR	stNandAddr = _GetNandAddrFromVPPN(pstProgram->GetVPPN());
	ASSERT(stNandAddr.nBlock < TOTAL_BLOCKS_PER_DIE);

	FIL_ProgramPage(stReqID, stNandAddr, pstProgram->GetBufferEntry()->m_pMainBuf, pstProgram->GetBufferEntry()->m_pSpareBuf);

	IOUB_IncreaseProfile(Prof_NAND_write);
	if (stNandAddr.nPPage == 0) {
		IOUB_IncreaseProfile(Prof_NAND_erase);
	}

	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_NAND_WRITE);

	return;
}

VOID
VNAND::ProgramPageSimul(UINT32 nLPN, UINT32 nVPPN)
{
	UINT32 nVPageNo		= VPN_FROM_VPPN(nVPPN);
	UINT32 nVBN			= VBN_FROM_VPPN(nVPPN);
	UINT32 channel		= CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way			= WAY_FROM_VPPN(nVPPN);


	DEBUG_ASSERT(nLPN < IOUB_GLOBAL::GetInstance()->GetLPNCount());
	DEBUG_ASSERT(nVBN < GetVBlockCount());
	DEBUG_ASSERT(nVPageNo < IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock());

	// this page must be free page
	DEBUG_ASSERT(m_pstVB[channel][way][nVBN].m_pnV2L[nVPageNo] == INVALID_LPN);
	DEBUG_ASSERT(ISSET(m_pstVB[channel][way][nVBN].m_pbValid, nVPageNo) == FALSE);

	SETBIT(m_pstVB[channel][way][nVBN].m_pbValid, nVPageNo);
	m_pstVB[channel][way][nVBN].m_pnV2L[nVPageNo] = nLPN;
}

VOID VNAND::EraseSimul(UINT32 nLPN, UINT32 nVPPN)
{
	UINT32 nVPageNo = VPN_FROM_VPPN(nVPPN);
	UINT32 nVBN = VBN_FROM_VPPN(nVPPN);
	UINT32 channel = CHANNEL_FROM_VPPN(nVPPN);
	UINT32 way = WAY_FROM_VPPN(nVPPN);
	_EraseSimul(channel, way, nVBN);
	return;
}

UINT32
VNAND::GetValidVPNCount(UINT32 channel, UINT32 way, UINT32 nVBN)
{
	UINT32 nVPagePerVBN = IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock();

	UINT32	nVPC = 0;

	for (UINT32 i = 0; i < nVPagePerVBN; i++)
	{
		if (IsValid(channel, way, nVBN, i) == TRUE)
		{
			nVPC++;
		}
	}

	return nVPC;
}

/*
@berief	update erase information
*/
VOID
VNAND::_EraseSimul(INT32 channel, INT32 way, INT32 nVBN)
{
	INT32	nLPagePerVBlock = IOUB_GLOBAL::GetInstance()->GetVPagePerVBlock();

	/* Init PB structure*/
	for (int i = 0; i < nLPagePerVBlock; i++)
	{
		m_pstVB[channel][way][nVBN].m_pnV2L[i] = INVALID_LPN;
		DEBUG_ASSERT(ISCLEAR(m_pstVB[channel][way][nVBN].m_pbValid, i) == TRUE);
	}

	INT32	nValidBitmapBytePerBlock;
	nValidBitmapBytePerBlock = CEIL(nLPagePerVBlock, NBBY);
	OSAL_MEMSET(m_pstVB[channel][way][nVBN].m_pbValid, 0x00, nValidBitmapBytePerBlock);

	m_pstVB[channel][way][nVBN].m_nEC++;		// Increase EC

	IOUB_GLOBAL::GetInstance()->IncreaseProfileCount(PROFILE_NAND_ERASE);
}

NAND_ADDR
VNAND::_GetNandAddrFromVPPN(UINT32 nVPPN)
{
	NAND_ADDR	stNandAddr;

	stNandAddr.nCh		= CHANNEL_FROM_VPPN(nVPPN);
	stNandAddr.nWay		= WAY_FROM_VPPN(nVPPN);
	stNandAddr.nBlock	= PBN_FROM_VPPN(nVPPN);
	stNandAddr.nPPage	= PPAGE_FROM_VPPN(nVPPN);

#if (BITS_PER_CELL == 1)
#define Vpage2PlsbPageTranslation(pageNo) ((pageNo) > (0) ? (2 * (pageNo) - 1): (0))
	INT32	nPPage = stNandAddr.nPPage;

	stNandAddr.nPPage = Vpage2PlsbPageTranslation(nPPage);		// covert to LSB page
#endif

	return stNandAddr;
}

