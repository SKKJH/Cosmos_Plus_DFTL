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

/*
	Virtual Block Manager

	virtualize underling physical channels, ways and NAND flash memories.
	This module has below respoisibility 
	1. Channel interleavin
	2. Block interleaving
	3. Bad block management
	4. Physical Channel and ways to virtual pahysical blocks
*/

#include "streamftl_vnand.h"
#include "streamftl_main.h"
#include "dump.h"
#include "util.h"
#include "debug.h"
#include "osal.h"

#include "streamftl.h"
#include "streamftl_util.h"

#include "fil.h"

static INT32 _GetLpnSimul(UINT32 nVPPN);
static VOID _EraseSimul(INT32 nPBN);
static NAND_ADDR _GetNandAddrFromVPPN(UINT32 nVPPN);

void VNAND_Initialize()
{
	// create the PB map
	INT32	nSize = sizeof(_PB) * g_stGlobal.nVBlockCount;
	g_pstPB = (_PB *)MALLOC(nSize, DUMP_TYPE_PB);
	OSAL_MEMSET(g_pstPB, 0x00, nSize);

	INT32	nValidBitmapBytePerBlock;
	nValidBitmapBytePerBlock = CEIL(g_stGlobal.nLPagesPerVBlock, NBBY);
	nSize = nValidBitmapBytePerBlock * g_stGlobal.nVBlockCount;			// Byte count
	g_pstPBValidBitmap = (BOOL*)MALLOC(nSize, DUMP_TYPE_PB_VALID_BITMAP);
	OSAL_MEMSET(g_pstPBValidBitmap, 0x00, nSize);			// clear all bitmap

	nSize = sizeof(INT32) * g_stGlobal.nLPagesPerVBlock * g_stGlobal.nVBlockCount;
	g_pstPBPPN2LPN		= (INT32*)MALLOC(nSize, DUMP_TYPE_PB_PPN2LPN);
	ASSERT(g_pstPBPPN2LPN);

	for (int i = 0; i < g_stGlobal.nVBlockCount; i++)
	{
		g_pstPB[i].panPPN2LPN = &g_pstPBPPN2LPN[i * g_stGlobal.nLPagesPerVBlock];
		g_pstPB[i].pabValid = &g_pstPBValidBitmap[i * nValidBitmapBytePerBlock];

		for (int j = 0; j < g_stGlobal.nLPagesPerVBlock; j++)
		{
			g_pstPB[i].panPPN2LPN[j] = INVALID_LPN;
		}
	}
}

INT32 VNAND_GetPBlocksPerVBlock(VOID)
{
	return (USER_CHANNELS * USER_WAYS);
}

INT32 VNAND_GetPPagesPerVBlock(VOID)
{
	return (VNAND_GetPBlocksPerVBlock() * FIL_GetPagesPerBlock());
}

INT32 VNAND_GetLPagesPerVBlock(VOID)
{
	return VNAND_GetPPagesPerVBlock() * LPN_PER_PHYSICAL_PAGE;
}

INT32 VNAND_GetVBlockCount(VOID)
{
	return (TOTAL_BLOCKS_PER_DIE);
}

/*
	@get LPN from PPN
*/
INT32 VNAND_GetLPN(INT32 nVPPN)
{
	return _GetLpnSimul(nVPPN);
}

/*
	@brief
	Read a page from NAND flash memory
	ASSUMPTION: NAND spare has LPN

	return: LPN of PPN
*/
INT32 VNAND_ReadPage(FTL_REQUEST_ID stReqID, UINT32 nVPPN, void * pMainBuf, void * pSpareBuf)
{
	NAND_ADDR	stNandAddr = _GetNandAddrFromVPPN(nVPPN);
	if (stReqID.stCommon.nType != FTL_REQUEST_ID_TYPE_DEBUG)
	{
		FIL_ReadPage(stReqID, stNandAddr, pMainBuf, pSpareBuf);

		STAT_IncreaseCount(PROFILE_NAND_READ, 1);
	}

	return _GetLpnSimul(nVPPN);
}

/*
	NAND write 후 spare에 P2L 저장
*/
void VNAND_ProgramPage(FTL_REQUEST_ID stReqID, PROGRAM_UNIT *pstProgram)
{
	NAND_ADDR	stNandAddr = _GetNandAddrFromVPPN(pstProgram->nVPPN);

	FIL_ProgramPage(stReqID, stNandAddr, pstProgram->pstBufferEntry->pMainBuf, pstProgram->pstBufferEntry->pSpareBuf);

	STAT_IncreaseCount(PROFILE_NAND_WRITE, 1);
}

/*
@brief	ProgramPage for simulator
*/
void VNAND_ProgramPageSimul(ACTIVE_BLOCK* pstActiveBlock, int nBufferedLPNIndex)
{
	PROGRAM_UNIT* pstProgram = &pstActiveBlock->astBuffering[pstActiveBlock->nCurProgramBuffering];

	INT32	nVBN = VBN_FROM_VPPN(pstProgram->nVPPN);
	INT32	nVPageNo;									// page number in a VBN
	INT32	nLPN;

	nVPageNo = VPAGE_FROM_VPPN(pstProgram->nVPPN) + nBufferedLPNIndex;
	nLPN = pstProgram->anLPN[nBufferedLPNIndex];

#if SUPPORT_AUTO_ERASE == 1
	if (nVPageNo == 0)
	{
		_EraseSimul(nVBN);
	}
#endif

	DEBUG_ASSERT(nLPN < g_stGlobal.nLPNCount);
	DEBUG_ASSERT(nVBN < g_stGlobal.nVBlockCount);
	DEBUG_ASSERT(nVPageNo < g_stGlobal.nLPagesPerVBlock);

	// this page must be free page
	DEBUG_ASSERT(g_pstPB[nVBN].panPPN2LPN[nVPageNo] == -1);
	DEBUG_ASSERT(ISSET(g_pstPB[nVBN].pabValid, nVPageNo) == FALSE);

	SETBIT(g_pstPB[nVBN].pabValid, nVPageNo);
	g_pstPB[nVBN].panPPN2LPN[nVPageNo] = nLPN;
}

/*
	LPN overwrite시 invalidate 시킴.
	GC 수행중 copy 하지 않도록 하기 위함.
*/
void VNAND_Invalidate(INT32 nVPN)
{
	INT32	nVBN = VBN_FROM_VPPN(nVPN);
	INT32	nPageOffset = VPAGE_FROM_VPPN(nVPN);
	DEBUG_ASSERT(ISSET(g_pstPB[nVBN].pabValid, nPageOffset) == TRUE);
	CLEARBIT(g_pstPB[nVBN].pabValid, nPageOffset);
}

INT32 VNAND_IsValid(INT32 nVBN, INT32 nVPage)
{
	DEBUG_ASSERT(nVBN < VNAND_GetVBlockCount());
	return ISSET(g_pstPB[nVBN].pabValid, nVPage);
}

VOID VNAND_Erase(INT32 nVBN)
{
	_EraseSimul(nVBN);

#ifdef COSMOS_PLUS
	ASSERT(0);		// not implemented yet
#endif
}

INT32 VNAND_GetEC(INT32 nPBN)
{
	return g_pstPB[nPBN].nEC;
}

// TEMPORAL BAD CHECK FUNCTION
BOOL VNAND_IsBad(INT32 nVBN)
{
	return FIL_IsBad(nVBN);
}

////////////////////////////////////////////////////////////////////////////////
//
//	static functions
//

/*
	@berief	update erase information
*/
static VOID _EraseSimul(INT32 nPBN)
{
	/* Init PB structure*/
	for (int i = 0; i < g_stGlobal.nLPagesPerVBlock; i++)
	{
		g_pstPB[nPBN].panPPN2LPN[i] = INVALID_LPN;
		DEBUG_ASSERT(ISCLEAR(g_pstPB[nPBN].pabValid, i) == TRUE);
	}

	INT32	nValidBitmapBytePerBlock;
	nValidBitmapBytePerBlock = CEIL(g_stGlobal.nLPagesPerVBlock, NBBY);
	OSAL_MEMSET(g_pstPB[nPBN].pabValid, 0x00, nValidBitmapBytePerBlock);

	g_pstPB[nPBN].nEC++;		// Increase EC

	STAT_IncreaseCount(PROFILE_NAND_ERASE, (USER_CHANNELS * USER_WAYS));
}

/*
	@brief
	Read a page from NAND flash memory
	ASSUMPTION: NAND spare has LPN

	return: LPN of PPN
*/
static INT32 _GetLpnSimul(UINT32 nVPPN)
{
	INT32	nVBN		= VBN_FROM_VPPN(nVPPN);
	INT32	nVPageNo	= VPAGE_FROM_VPPN(nVPPN);			// page number in a VBN

	DEBUG_ASSERT(nVBN < g_stGlobal.nVBlockCount);
	DEBUG_ASSERT(nVPageNo < g_stGlobal.nLPagesPerVBlock);
	DEBUG_ASSERT(g_pstPB[nVBN].panPPN2LPN[nVPageNo] < g_stGlobal.nLPNCount);

	return g_pstPB[nVBN].panPPN2LPN[nVPageNo];
}

static NAND_ADDR _GetNandAddrFromVPPN(UINT32 nVPPN)
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

