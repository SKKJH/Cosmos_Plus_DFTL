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

#ifndef __STREAMFTL_TYPES_H__
#define __STREAMFTL_TYPES_H__

#include "list.h"
#include "cosmos_types.h"

#ifndef TRUE
	#define TRUE		1
#endif

#ifndef FALSE
	#define FALSE		0
#endif

#define UINT32_BITS			32

typedef enum
{
	SMERGE_POLICY_GREEDY = 0,		// partition that has the most stream 
	SMERGE_POLICY_COST_BENEFIT,		// partition that has minimum copy cost per stream
	SMERGE_POLICY_COUNT,
} SMERGE_POLICY;

typedef enum
{
	IOTYPE_HOST = 0,
	IOTYPE_STREAM_MERGE,
	IOTYPE_BLOCK_GC,
	IOTYPE_TEST,
	IOTYPE_COUNT,
} IOTYPE;

typedef struct
{
	struct list_head	dlList;					// list for queueing
} REQUEST_COMMON;

typedef struct
{
	FTL_REQUEST_ID_TYPE		nReqType;

	struct list_head		dlFree;				// free NVMeRequest
	INT32					nFreeCount;			// count of free request

	struct list_head		dlWait;				// NAND issue wait queue
	INT32					nWaitCount;

	struct list_head		dlIssued;			// issued queue
	INT32					nIssuedCount;

	struct list_head		dlHDMA;				// host DMA issued list
	INT32					nHDMACount;

	struct list_head		dlDone;				// Read done or Program done
	INT32					nDoneCount;
} REQUEST_INFO;

typedef enum
{
	HIL_REQUEST_FREE,

	HIL_REQUEST_READ_WAIT,			// At the wait list
	HIL_REQUEST_READ_NAND_ISSUED,
	HIL_REQUEST_READ_NAND_DONE,
	HIL_REQUEST_READ_HDMA_ISSUE,
	HIL_REQUEST_READ_DONE,

	HIL_REQUEST_WRITE_WAIT,
} HIL_REQUEST_STATUS;

typedef enum
{
	GC_REQUEST_FREE,
	GC_REQUEST_READ_WAIT,
	GC_REQUEST_READ_ISSUE,
	GC_REQUEST_READ_DONE,
	GC_REQUEST_WRITE_WAIT = GC_REQUEST_READ_DONE,
	GC_REQUEST_WRITE_ISSUE,
	GC_REQUEST_WRITE_DONE,
} GC_REQUEST_STATUS;

#define REQUEST_LPN_COUNT_MAX		256			// 1MB

typedef struct
{
	HIL_REQUEST_STATUS	nStatus;
	REQUEST_COMMON		stCommon;				// common type for request
	NVME_CMD_OPCODE		nCmd;					// bool 0:Read, 1:Write, assign 1 more  bit for signed enum type

	// Read & Write
	INT32				nLPN;					// LPN
	UINT32				nHostCmdSlotTag;		// SQ Tag from HIL, This is used for HDMA
	INT32				nLPNCount: 16;			// count of read LPN
	INT32				nDoneLPNCount: 16;		// LPN Count for (read done) or (write buffered)

	// HDMA information
	unsigned int		nDMAReqTail : 8;		// DMA tail request, to check HDMA done
	unsigned int		nOverFlowCount;			// count of DMA index overflow, to check HDMA done

	// read only
	struct list_head	dlBuffer;				// read buffer list
	INT8				anLPNOffsetOfBuffer[REQUEST_LPN_COUNT_MAX];
												// 4K LPN offset in the buffer
} HIL_REQUEST;

#endif	// end of #ifndef __STREAMFTL_TYPES_H__