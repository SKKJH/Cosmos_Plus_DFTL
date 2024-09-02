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

#ifdef __cplusplus
extern "C" {
#endif

	// ftl interface
	VOID FTL_Initialize(VOID);
	VOID FTL_Run(VOID);
	BOOL FTL_Format(VOID);

	VOID FTL_ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VOID FTL_WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VOID FTL_CallBack(FTL_REQUEST_ID stReqID);
	VOID FTL_IOCtl(IOCTL_TYPE eType);

#define SyncAllLowLevelReqDone()				// do nothing, just for GREEDY_FTL_3_0_0

#ifdef __cplusplus
}
#endif

IOUB_GLOBAL	*g_pstIOUB;

VOID FTL_Initialize(VOID)
{
	void* p = OSAL_MemAlloc(MEM_TYPE_FW_DATA, sizeof(IOUB_GLOBAL), OSAL_MEMALLOC_ALIGNMENT);
	g_pstIOUB = new(p) IOUB_GLOBAL();

	g_pstIOUB->Initialize();

	return;
}

BOOL FTL_Format(VOID)
{
	UINT32 ret_val = g_pstIOUB->Format();
	IOUB_Profile_Initialize();
	return ret_val;
}

VOID FTL_ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	g_pstIOUB->ReadPage(nCmdSlotTag, nLPN, nCount);
}

VOID FTL_WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount)
{
	g_pstIOUB->WritePage(nCmdSlotTag, nLPN, nCount);
}

VOID FTL_Run(VOID)
{
	g_pstIOUB->Run();
}

VOID FTL_CallBack(FTL_REQUEST_ID stReqID)
{
	g_pstIOUB->CallBack(stReqID);
}

VOID FTL_IOCtl(IOCTL_TYPE eType)
{
	g_pstIOUB->IOCtl(eType);
}

