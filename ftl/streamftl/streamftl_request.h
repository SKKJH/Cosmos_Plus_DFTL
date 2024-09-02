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

#ifndef __STREAMFTL_REQUEST_H__
#define __STREAMFTL_REQUEST_H__

#include "cosmos_types.h"
#include "list.h"

#include "cosmos_plus_system.h"
#include "hil_config.h"
#include "ftl_config.h"

#include "streamftl.h"
#include "streamftl_types.h"

//#define STREAMFTL_REQUEST_DEBUG

#define HIL_REQUEST_COUNT			(HOST_REQUEST_BUF_COUNT)
#define GC_REQUEST_COUNT			(NSC_MAX_CHANNELS * USER_WAYS * LPN_PER_PHYSICAL_PAGE * 2/* double buffering */)
#define GC_REQUST_COUNT_BITS		(16)

typedef struct
{
	HIL_REQUEST			astRequest[HIL_REQUEST_COUNT];
} HIL_REQUEST_POOL;

typedef struct
{
	REQUEST_COMMON		stCommon;				// common type for request
	FTL_REQUEST_ID		stRequestID;

	INT32				nLPN;					// LPN
	INT32				nVPPN;					// to get LPN data offset in pMainBuf

	INT32				nCopyEntryIndex;		// index of GCCopyEntry, for Block Merge

	void*				pMainBuf;
	void*				pSpareBuf;				// GC buffer pointer, not used yet
} GC_REQUEST;

typedef struct
{
	GC_REQUEST			astRequest[GC_REQUEST_COUNT];
} GC_REQUEST_POOL;

extern HIL_REQUEST_POOL*		g_pstHILRequestPool;
extern REQUEST_INFO*			g_pstHILRequestInfo;
extern REQUEST_INFO*			g_pstGCRequestInfo;

VOID StreamFTL_InitRequestPool(VOID);
VOID StreamFTL_InitRequestInfo(VOID);

VOID StreamFTL_ProcessWaitQ(VOID);
VOID StreamFTL_ProcessDoneQ(VOID);

HIL_REQUEST* StreamFTL_AllocateHILRequest(VOID);
GC_REQUEST* StreamFTL_AllocateGCRequest(VOID);
VOID StreamFTL_AddToRequestWaitQ(REQUEST_INFO* pstRequestInfo, REQUEST_COMMON* pstCommonRequest);

UINT32 StreamFTL_GetGCRequestIndex(GC_REQUEST* pstRequest); ;

FTL_REQUEST_ID StreamFTL_GetRequestID(HIL_REQUEST* pstRequest, IOTYPE eIOType);
VOID StreamFTL_IssueStreamMergeWrite(VOID);
VOID StreamFTL_IssueBlockGCWrite(VOID);

#endif		// end of #ifndef __STREAMFTL_REQUEST_H__
