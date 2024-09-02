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

#ifndef _DUMP_H_
#define _DUMP_H_

#include "streamftl_types.h"

#define MAX_DUMP_COUNT			(128)		// Maximum dump count
#define DUMP_FILE_NAME			"precondition_dump.bin"
#define DUMP_SIGNATURE			0x20080620

#define IS_VALID_DUMP_TYPE(_eType)		( (_eType < DUMP_TYPE_COUNT) ? TRUE : FALSE )

typedef enum
{
	DUMP_TYPE_STREAMFTL_GLOBAL,
	DUMP_TYPE_STREAM,
	DUMP_TYPE_STREAM_VALID_BITMAP,
	DUMP_TYPE_STREAM_BLOCK,
	DUMP_TYPE_STREAM_EC,
	DUMP_TYPE_BIT,
	DUMP_TYPE_REVERSE_PARTITION_LIST,
	DUMP_TYPE_ACTIVE_BLOCK,
	DUMP_TYPE_PARTITION,
	DUMP_TYPE_L2PCACHE,
	DUMP_TYPE_L2PCACHE_PPN,
	DUMP_TYPE_L2PCACHE_STREAM,
	DUMP_TYPE_PB,
	DUMP_TYPE_PB_VALID_BITMAP,
	DUMP_TYPE_PB_PPN2LPN,
	DUMP_TYPE_COUNT,			// count of dump type
	DUMP_TYPE_UNKNOWN,			// unknown type, lookup type of pointer
	DUMP_TYPE_NO_DUMP,			// Do not dump this memory
} DUMP_TYPE;


typedef struct
{
	DUMP_TYPE	eType;		// type of data
	INT32		nSize;		// size of data
} DUMP_DATA;

typedef struct
{
	INT32		nStreamCount;			// Count of stream
	INT32		nStreamSize;			// default: 256KB
	INT32		nMaxStreamPerPartition;		// Max stream per partition

	// active stm
	INT32		nActiveBlockCount;	// count of Active STREAM, default 32

	// partition
	INT32		nPartitionSize;			// size of partition, Default 256KB
	INT32		nPartitionCount;		// count of partition - cover logical address area

	// configurations
	INT32		nLogicalPageSize;
	INT32		nVBlockSize;
	INT64		nLogicalFlashSize;	// user space Default 32GB
	INT64		nPhysicalFlashSize;	// size of total blocks, byte
	INT64		nOverprovisionSize;			// OP, 3GB
	INT32		nVBlockCount;		// Total Block Count
	INT32		nMetaBlockCount;		// count of metablock

	double		fRandomRate;
	double		fSeqRate;
	double		fRandomAmount;
	INT32		nRandomIncrease;
	INT32		nRandomSize;

	double		fL2PCacheRatio;

	float		fHotPartitionRatio;
	float		m_fVictimActiveBlockRatio;
} DUMP_ENV;

typedef struct
{
	UINT32		nSignature;		// Signature of dump
	UINT32		nVer;			// Metadata version vsersion
	INT32		nCount;			// Count of dump data
	INT32		nDataSize;		// total dump size

	DUMP_DATA	astData[MAX_DUMP_COUNT];

	VOID*		pGlobalPtr;		// address of g_stGlobal, this variable has memory pool. the base address must be same for dump loading

	// Dump Environment
	DUMP_ENV	stDumpEnv;
} DUMP_HEADER;

typedef struct
{
	UINT32		nSignature;		// Signature of dump
} DUMP_FOOTER;

VOID	DUMP_Initialize(VOID);
VOID	DUMP_Add(DUMP_TYPE eType, INT32 nSize, void* pPtr);
BOOL	DUMP_Load(VOID);
VOID	DUMP_Store(VOID);
INT32	Dump_GetSize(DUMP_TYPE eDumpType);

#endif	// end of _DUMP_H_