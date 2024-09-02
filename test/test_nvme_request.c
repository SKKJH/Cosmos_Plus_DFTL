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
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#include "xil_types.h"
#include "test_nvme_request.h"
#include "list.h"

#include "osal.h"
#include "debug.h"

#include "cosmos_types.h"
#include "test_main.h"
#ifdef STREAM_FTL
	#include "streamftl.h"
#endif

NVMeRequestPool		*g_pstNVMeRequestPool;

static VOID _AddToFree(NVMeRequest* pstRequest)
{
	list_add_tail(&pstRequest->dlList, &g_pstNVMeRequestPool->dlFree);
	g_pstNVMeRequestPool->m_nFree++;
}

void NVMeRequest_Initialize(void)
{
	g_pstNVMeRequestPool = (NVMeRequestPool*)OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(NVMeRequestPool), 0);

	INIT_LIST_HEAD(&g_pstNVMeRequestPool->dlFree);

	g_pstNVMeRequestPool->m_nFree = 0;
	for (int i = 0; i < NVME_REQUEST_COUNT; i++)
	{
		g_pstNVMeRequestPool->m_astRequest[i].stFIFOReg.cmdSlotTag = i;
		g_pstNVMeRequestPool->m_astRequest[i].nDMACount = 0;

		INIT_LIST_HEAD(&g_pstNVMeRequestPool->m_astRequest[i].dlList);

		_AddToFree(&g_pstNVMeRequestPool->m_astRequest[i]);
	}

	for (int i = 0; i < MAX_UINT8; i++)
	{
		g_pstNVMeRequestPool->m_anHDMAToRequest_Tx[i] = INVALID_INDEX;
		g_pstNVMeRequestPool->m_anHDMAToRequest_Rx[i] = INVALID_INDEX;
	}
}

NVMeRequest* NVMeRequest_Allocate(void)
{
	if (g_pstNVMeRequestPool->m_nFree == 0)
	{
		return NULL;
	}

	assert(list_empty(&g_pstNVMeRequestPool->dlFree) == FALSE);

	NVMeRequest*	pRequest;

	pRequest = list_first_entry(&g_pstNVMeRequestPool->dlFree, NVMeRequest, dlList);

	list_del_init(&pRequest->dlList);

	g_pstNVMeRequestPool->m_nFree--;
	return pRequest;

}

void NVMeRequest_Release(NVMeRequest* pstRequest, int nDMAIndex)
{
	TEST_PRINTF("Release NVME CMD: OPCode / LBA / Count : %d / %d / %d \n\r", 
		pstRequest->stCmd.OPC, pstRequest->stCmd.dword[NVME_IO_CMD_INDEX_LBA0], (pstRequest->stCmdDW12.NLB + 1));

	assert(nDMAIndex <= MAX_UINT8);
	if (pstRequest->stCmd.OPC == NVME_CMD_OPCODE_READ)
	{
		g_pstNVMeRequestPool->m_anHDMAToRequest_Tx[nDMAIndex] = INVALID_INDEX;
	}
	else if (pstRequest->stCmd.OPC == NVME_CMD_OPCODE_WRITE)
	{
		g_pstNVMeRequestPool->m_anHDMAToRequest_Rx[nDMAIndex] = INVALID_INDEX;
	}

	pstRequest->stFIFOReg.cmdValid = NVME_CMD_INVALID;
	pstRequest->stCmd.OPC = (unsigned char)INVALID_INDEX;
	pstRequest->stFIFOReg.qID = INVALID_INDEX;
	//pstRequest->stFIFOReg.cmdSlotTag  = INVALID_INDEX;

	_AddToFree(pstRequest);
}

/*
	nCmdSlotTag : Request index
*/
void NVMeRequest_TxDMAIssue(int nDMAIndex, int nCmdSlotTag)
{
	assert(g_pstNVMeRequestPool->m_astRequest[nCmdSlotTag].stFIFOReg.cmdValid == NVME_CMD_VALID);
	g_pstNVMeRequestPool->m_astRequest[nCmdSlotTag].nDMACount++;
	g_pstNVMeRequestPool->m_anHDMAToRequest_Tx[nDMAIndex] = nCmdSlotTag;
}

void NVMeRequest_RxDMAIssue(int nDMAIndex, int nCmdSlotTag)
{
	assert(g_pstNVMeRequestPool->m_astRequest[nCmdSlotTag].stFIFOReg.cmdValid == NVME_CMD_VALID);
	g_pstNVMeRequestPool->m_astRequest[nCmdSlotTag].nDMACount++;
	g_pstNVMeRequestPool->m_anHDMAToRequest_Rx[nDMAIndex] = nCmdSlotTag;
}

void NVMeRequest_TxDMADone(int nDMAIndex)
{
	DEBUG_ASSERT(nDMAIndex <= MAX_UINT8);

	int		nRequestIndex;
	nRequestIndex = g_pstNVMeRequestPool->m_anHDMAToRequest_Tx[nDMAIndex];
	ASSERT(nRequestIndex < NVME_REQUEST_COUNT);

	NVMeRequest	*pstRequest = &g_pstNVMeRequestPool->m_astRequest[nRequestIndex];

	DEBUG_ASSERT(pstRequest->stFIFOReg.cmdValid == NVME_CMD_VALID);
	DEBUG_ASSERT(pstRequest->nDMACount > 0);
	DEBUG_ASSERT(pstRequest->nRestDMACount > 0);

	pstRequest->nDMACount--;
	pstRequest->nRestDMACount--;

	if (pstRequest->nRestDMACount == 0)
	{
		NVMeRequest_Release(pstRequest, nDMAIndex);
	}
}

void NVMeRequest_RxDMADone(int nDMAIndex)
{
	assert(nDMAIndex <= MAX_UINT8);

	int		nRequestIndex;
	nRequestIndex = g_pstNVMeRequestPool->m_anHDMAToRequest_Rx[nDMAIndex];
	ASSERT(nRequestIndex < NVME_REQUEST_COUNT);

	NVMeRequest	*pstRequest = &g_pstNVMeRequestPool->m_astRequest[nRequestIndex];

	DEBUG_ASSERT(pstRequest->stFIFOReg.cmdValid == NVME_CMD_VALID);
	DEBUG_ASSERT(pstRequest->nDMACount > 0);
	DEBUG_ASSERT(pstRequest->nRestDMACount > 0);

	pstRequest->nDMACount--;
	pstRequest->nRestDMACount--;
	if (pstRequest->nRestDMACount == 0)
	{
		NVMeRequest_Release(pstRequest, nDMAIndex);
	}
}

