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

#include "debug.h"

#include "StreamFTL.h"
#include "error.h"
#include "init.h"
#include "streamftl_main.h"
#include "streamftl_map.h"
#include "streamftl_vnand.h"

#include "osal.h"

VOID check_LPN_MAP(INT32 nLPN, INT32 nVPPN)
{
#if (ERROR_CHECKING_ON == 1)
	// check MAP
	INT32 nStream, nTestVPPN;

	nTestVPPN = LPN2PPN(nLPN, &nStream, IOTYPE_TEST);
	if (nStream == -1)
	{
		DEBUG_ASSERT(0);
	}
	else
	{
		DEBUG_ASSERT(nTestVPPN == nVPPN);
	}
#endif
}

VOID check_OOB_MAP(INT32 nLPN, INT32 nVPPN)
{
#if (ERROR_CHECKING_ON == 1)
	// check RMAP
	DEBUG_ASSERT(g_pstPB[VBN_FROM_VPPN(nVPPN)].panPPN2LPN[VPAGE_FROM_VPPN(nVPPN)] == nLPN);
#endif
}

VOID check_total_MAP(VOID)
{
	if (ERROR_CHECKING_ON)
	{
		// check LPN MAP
		for (INT32 i = 0; i < g_stGlobal.nLPNCount; i++)
		{
			INT32 nStream, nVPPN;

			nVPPN = LPN2PPN(i, &nStream, IOTYPE_TEST);

			if (nStream == INVALID_STREAM)
			{
				continue;
			}

			check_OOB_MAP(i, nVPPN);
		}

		// check OOB MAP
		for (int i = 0; i < g_stGlobal.nPhysicalFlashSize / LOGICAL_PAGE_SIZE; i++)
		{
			int LPN;

			if (VNAND_IsValid(VBN_FROM_VPPN(i), VPAGE_FROM_VPPN(i)))
			{
				LPN = g_pstPB[VBN_FROM_VPPN(i)].panPPN2LPN[VPAGE_FROM_VPPN(i)];

				if (LPN != -1)
				{
					check_LPN_MAP(LPN, i);
				}
			}
			else
			{
				continue;
			}
		}
	}
}

/*
	check validity of partition
*/
VOID _CheckStreamValidity(INT32 nStream)
{
	STREAM *pstStream = GET_STREAM(nStream);

	INT32 nStartOffset, nEndOffset;
	INT32 nVPC = 0;
	INT32 nPartition = PARTITION_ID_FROM_LPN(pstStream->nStartLPN);

	DEBUG_ASSERT(pstStream->nAllocatedBlocks <= BLOCK_PER_STREAM);

	FTL_REQUEST_ID	stReqID;
	stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_DEBUG;

	for (INT32 i = 0; i < pstStream->nAllocatedBlocks; i++)
	{
		INT32 nVBN = pstStream->pnVBN[i];
		if (nVBN == INVALID_PBN)
		{
			continue;
		}

		nStartOffset = 0;
		nEndOffset = g_stGlobal.nLPagesPerVBlock - 1;

		if (i == 0)
		{
			nStartOffset = VPAGE_FROM_VPPN(pstStream->nStartVPPN);
		}

		if (i == (pstStream->nAllocatedBlocks - 1))
		{
			nEndOffset = VPAGE_FROM_VPPN(pstStream->nEndVPPN);
		}

		for (INT32 j = nStartOffset; j <= nEndOffset; j++)
		{
			if ( VNAND_IsValid(nVBN, j) )
			{
				INT32 nVPPN = VPAGE_FROM_VBN_VPN(nVBN, j);
				INT32 nLPN = VNAND_ReadPage(stReqID, nVPPN, NULL, NULL);
				INT32 nCurPartition = PARTITION_ID_FROM_LPN(nLPN);

				if (nPartition != nCurPartition)
				{
					// reused block, skip
					continue;
				}

				if (pstStream->pnEC[i] != (INT8)VNAND_GetEC(nVBN))
				{
					continue;	// reused block
				}

				nVPC++;

				HIL_REQUEST_ID stReqID = { 0xFFFF, 0xFFFF };

				// this page can be accessible through cluster?
				int nSpareLPN= g_pstPB[nVBN].panPPN2LPN[j];
				int nStream;
				int nPPN = LPN2PPN(nSpareLPN, &nStream, IOTYPE_TEST);

				if (nPPN != VPAGE_FROM_VBN_VPN(nVBN, j))
				{
					PRINTF("ERROR: reverse map and StreamFTL map is not same\n");
					DEBUG_ASSERT(0);
				}
			}
		}
	}

	if (nVPC != pstStream->nVPC)
	{
		PRINTF("ERROR: block valid bitmap and StreamFTL valid count is not same\n");
		DEBUG_ASSERT(0);
	}

	if (pstStream->nVPC == 0)
	{
		DEBUG_ASSERT(pstStream->bFree == TRUE);
		DEBUG_ASSERT(pstStream->bActive == FALSE);
		DEBUG_ASSERT(pstStream->bGC == FALSE);
		DEBUG_ASSERT(pstStream->bSMerge == FALSE);
	}
}

VOID CheckStreamValidity(VOID)
{

	for (int i = 0; i < g_stGlobal.nStreamCount; i++)
	{
		STREAM *pstStream = GET_STREAM(i);

		if (pstStream->bFree == FALSE)
		{
			_CheckStreamValidity(i);
		}
		else
		{
			DEBUG_ASSERT(pstStream->bFree == TRUE);
		}
	}
}

VOID __check_block_validity(INT32 nVBN)
{
	BLOCK_INFO *pstBIT = GET_BLOCK_INFO(nVBN);

	int nVPC = 0;

	for (int i = 0; i < g_stGlobal.nLPagesPerVBlock; i++)
	{
		if (VNAND_IsValid(nVBN, i))
		{
			nVPC++;
		}
	}

	if ( pstBIT->bMetaBlock == TRUE )
	{
		ASSERT(nVPC == 0);
		return;
	}

	if (pstBIT->nInvalidPages > (g_stGlobal.nLPagesPerVBlock - nVPC))
	{
		PRINTF("ERROR: block valid bitmap and block invalid count is not matched\n");
		DEBUG_ASSERT(0);
	}

	if ( (pstBIT->bActive == FALSE) && (pstBIT->nInvalidPages != (g_stGlobal.nLPagesPerVBlock - nVPC)) )
	{
		PRINTF("ERROR: block valid bitmap and block invalid count is not matched\n");
		DEBUG_ASSERT(0);
	}
}

VOID CheckBlockValidity(VOID)
{
	for (int i = 0; i < g_stGlobal.nVBlockCount; i++)
	{
		BLOCK_INFO *pstBIT = GET_BLOCK_INFO(i);
		
		if (pstBIT->bFree == FALSE)
		{
			__check_block_validity(i);
		}
	}
}

VOID CheckHotColdMgmt(VOID)
{
#if (SUPPORT_HOTCOLD == 1)
	HOTCOLD_MANAGER*	pstHC = &g_pstStreamFTL->stHotCold;

	ASSERT(pstHC->m_bEnable == g_stGlobal.bEnableHotColdMgmt);

	if (pstHC->m_bEnable == FALSE)
	{
		return;
	}

	INT32	nPartitionCount = 0;
	for (INT32 i = 0; i < STREAM_FTL_MAX_HOT_RATIO_INDEX; i++)
	{
		nPartitionCount += pstHC->m_anPartitionCount[i];
	}

	ASSERT(pstHC->m_nNumValidParitions == nPartitionCount);
#endif
}

