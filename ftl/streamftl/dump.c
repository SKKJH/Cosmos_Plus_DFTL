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

#include "dump.h"
#include "StreamFTL.h"
#include "error.h"
#include "streamftl_map.h"
#include "streamftl_types.h"
#include "streamftl_vnand.h"
#include "debug.h"
#include "streamftl_main.h"
#include "osal.h"
#include "dump.h"

// static function
static DUMP_TYPE	_GetDumpType(void* pPtr);
static BOOL _IsValidDump(DUMP_HEADER* pstDumpHeader);
static VOID _BuildDumpHeaderFooter(VOID);

// static variables
static DUMP_HEADER		_gstDumpHeader;			// Dump Header
static DUMP_FOOTER		_gstDumpFooter;			// Dump footer
static const char*	_GetDumpFileName(VOID);

VOID* _apDumpTypeAddr[DUMP_TYPE_COUNT];

char	_psDumpFileName[128];

VOID DUMP_Add(DUMP_TYPE eType, INT32 nSize, void* pPtr)
{
	INT32	nIndex = _gstDumpHeader.nCount;

	if (eType == DUMP_TYPE_UNKNOWN)
	{
		eType = _GetDumpType(pPtr);
	}
	else
	{
		ASSERT(eType < DUMP_TYPE_COUNT);
		_apDumpTypeAddr[eType] = pPtr;

		ASSERT(eType == _GetDumpType(pPtr));
	}

	DEBUG_ASSERT(pPtr != NULL);
	DEBUG_ASSERT(eType < DUMP_TYPE_COUNT);

	_gstDumpHeader.astData[nIndex].nSize = nSize;
	_gstDumpHeader.astData[nIndex].eType = eType;

	_gstDumpHeader.nCount++;
	_gstDumpHeader.nDataSize += nSize;

	return;
}

VOID DUMP_Store(VOID)
{
#ifdef WIN32
	// build dump header
	_BuildDumpHeaderFooter();

	FILE* pstFile = fopen(_GetDumpFileName(), "wb");		// open with write mode and truncate it to size 0
	ASSERT(pstFile != NULL);

	size_t	nSize;

	// write header
	nSize = fwrite(&_gstDumpHeader, 1, sizeof(DUMP_HEADER), pstFile);
	ASSERT(nSize == sizeof(DUMP_HEADER));

	// write each memory
	for (int i = 0; i < _gstDumpHeader.nCount; i++)
	{
		void*	pPtr;
		INT32	nDumpSize;

		ASSERT(IS_VALID_DUMP_TYPE(_gstDumpHeader.astData[i].eType) == TRUE);
		pPtr = _apDumpTypeAddr[_gstDumpHeader.astData[i].eType];
		nDumpSize = _gstDumpHeader.astData[i].nSize;

		nSize = fwrite(pPtr, 1, nDumpSize, pstFile);
		ASSERT(nSize == nDumpSize);
	}

	// write header
	nSize = fwrite(&_gstDumpFooter, 1, sizeof(DUMP_FOOTER), pstFile);
	ASSERT(nSize == sizeof(DUMP_FOOTER));

	fclose(pstFile);

	LOG_PRINTF("Precondition dump done \n");
#endif
}

BOOL DUMP_Load(void)
{
#ifdef WIN32
	FILE* pstFile = fopen(_GetDumpFileName(), "rb");
	if (pstFile == NULL)
	{
		LOG_PRINTF("Dump file open fail !!\n");
		return FALSE;
	}

	DUMP_HEADER		stCurDumpHeader;
	size_t			nSize;
	BOOL			bRet = TRUE;

	// read header
	nSize = fread(&stCurDumpHeader, 1, sizeof(DUMP_HEADER), pstFile);
	ASSERT(nSize == sizeof(DUMP_HEADER));

	if (_IsValidDump(&stCurDumpHeader) == FALSE)
	{
		LOG_PRINTF("Dump load fail, Invalid dump file !!\n");
		bRet = FALSE;
		goto out;
	}

	// read each memory
	for (int i = 0; i < stCurDumpHeader.nCount; i++)
	{
		void*	pPtr;
		INT32	nDumpSize;

		ASSERT(IS_VALID_DUMP_TYPE(stCurDumpHeader.astData[i].eType) == TRUE);
		pPtr = _apDumpTypeAddr[stCurDumpHeader.astData[i].eType];
		nDumpSize = stCurDumpHeader.astData[i].nSize;

		nSize = fread(pPtr, 1, nDumpSize, pstFile);
		ASSERT(nSize == nDumpSize);	// invalid dump file
	}

	// restore
	g_pstStreamFTL->stHotCold.m_bEnable = g_stGlobal.bEnableHotColdMgmt;
#if (SUPPORT_HOTCOLD == 1)
	g_pstStreamFTL->stHotCold.fHotPartitionRatio = g_stGlobal.fHotPartitionRatio;
	g_pstStreamFTL->stHotCold.fVictimActiveBlockVictimRange = g_stGlobal.fVictimActiveBlockVictimRange;
#endif

	// read footer
	DUMP_FOOTER		stCurDumpFooter;
	nSize = fread(&stCurDumpFooter, 1, sizeof(DUMP_FOOTER), pstFile);
	ASSERT(nSize == sizeof(DUMP_FOOTER));	// invalid dump file

out:
	fclose(pstFile);
	return bRet;
#else
	return FALSE;
#endif
}

INT32	Dump_GetSize(DUMP_TYPE	eDumpType)
{
	INT32	nSize = 0;

	for (INT32 i = 0; i < _gstDumpHeader.nCount; i++)
	{
		if (_gstDumpHeader.astData[i].eType == eDumpType)
		{
			nSize = _gstDumpHeader.astData[i].nSize;
			break;
		}
	}

	return nSize;
}


//////////////////////////////////////////////////////////////////////////////////////////\
//
//	static function
//
//////////////////////////////////////////////////////////////////////////////////////////

static DUMP_TYPE	_GetDumpType(void* pPtr)
{
	for (DUMP_TYPE i = 0; i < DUMP_TYPE_COUNT; i++)
	{
		if (_apDumpTypeAddr[i] == pPtr)
		{
			return i;
		}
	}

	ASSERT(0);	// never reach here

	return DUMP_TYPE_UNKNOWN;
}


static BOOL _IsValidDump(DUMP_HEADER* pstDumpHeader)
{
	if ( pstDumpHeader->nSignature != DUMP_SIGNATURE )
	{
		PRINTF("Invalid signature of dump \n");
		return FALSE;
	}

	if ( pstDumpHeader->nVer != STREAM_FTL_VERSION )
	{
		PRINTF("Invalid version of dump, cur/dump version: 0x%X / 0x%X \n", STREAM_FTL_VERSION, pstDumpHeader->nVer);
		return FALSE;
	}

	if ( pstDumpHeader->nCount != _gstDumpHeader.nCount )
	{
		PRINTF("Invalid count of dump \n");
		return FALSE;
	}

	// check memory pool address
	if (pstDumpHeader->pGlobalPtr != &g_stGlobal)
	{
		PRINTF("Invalid dump, memory pool base address is different \n");
		return FALSE;
	}

	if ((pstDumpHeader->stDumpEnv.nStreamCount != g_stGlobal.nStreamCount) ||			// Count of stream
		(pstDumpHeader->stDumpEnv.nStreamSize != g_stGlobal.nStreamSize) ||			// default: 256KB
		(pstDumpHeader->stDumpEnv.nMaxStreamPerPartition != g_stGlobal.nMaxStreamPerPartition) ||		// Max stream per partitio
		(pstDumpHeader->stDumpEnv.nActiveBlockCount != g_stGlobal.nActiveBlockCount) ||	// count of Active Stream, default
		(pstDumpHeader->stDumpEnv.nPartitionSize != g_stGlobal.nPartitionSize) ||			// size of partition, Default 256K
		(pstDumpHeader->stDumpEnv.nPartitionCount != g_stGlobal.nPartitionCount) ||		// count of partition - cover logi
		(pstDumpHeader->stDumpEnv.nLogicalPageSize != LOGICAL_PAGE_SIZE) ||
		(pstDumpHeader->stDumpEnv.nVBlockSize != g_stGlobal.nVBlockSize) ||
		(pstDumpHeader->stDumpEnv.nLogicalFlashSize != g_stGlobal.nLogicalFlashSize) ||		// user space Default 32GB
		(pstDumpHeader->stDumpEnv.nPhysicalFlashSize != g_stGlobal.nPhysicalFlashSize) ||	// size of total blocks
		(pstDumpHeader->stDumpEnv.nOverprovisionSize != g_stGlobal.nOverprovisionSize) ||	// OP
		(pstDumpHeader->stDumpEnv.nVBlockCount != g_stGlobal.nVBlockCount) ||			// Total Block Count
		//(pstDumpHeader->stDumpEnv.nRandomSize != g_stTestGlobal.nRandomSize) ||

		//(pstDumpHeader->stDumpEnv.fRandomRate != g_stTestGlobal.fRandomRate) ||
		//(pstDumpHeader->stDumpEnv.fSeqRate != g_stTestGlobal.fSeqRate) ||
		//(pstDumpHeader->stDumpEnv.fRandomAmount != g_stTestGlobal.fRandomAmount) ||
		//(pstDumpHeader->stDumpEnv.nRandomIncrease != g_stTestGlobal.nRandomIncrease) ||
		(pstDumpHeader->stDumpEnv.nMetaBlockCount != StreamFTL_GetMetaBlockCount()) ||
		(pstDumpHeader->stDumpEnv.fL2PCacheRatio != g_stGlobal.fL2PCacheRatio) ||
		//(pstDumpHeader->stDumpEnv.fHotPartitionRatio != g_pstStreamFTL->stHotCold.fHotPartitionRatio) ||
		//(pstDumpHeader->stDumpEnv.m_fVictimActiveBlockRatio != g_pstStreamFTL->stHotCold.m_fVictimActiveBlockRatio) ||
		FALSE
		)
	{
		PRINTF("Dump load fail - different configuration \n");
		return FALSE;
	}

	return TRUE;
}

static VOID _BuildDumpHeaderFooter(VOID)
{
	_gstDumpHeader.nSignature = DUMP_SIGNATURE;
	_gstDumpHeader.nVer = STREAM_FTL_VERSION;

	_gstDumpHeader.pGlobalPtr = &g_stGlobal;

	_gstDumpHeader.stDumpEnv.nStreamCount = g_stGlobal.nStreamCount;			// Count of stream
	_gstDumpHeader.stDumpEnv.nStreamSize = g_stGlobal.nStreamSize;			// default: 256KB
	_gstDumpHeader.stDumpEnv.nMaxStreamPerPartition = g_stGlobal.nMaxStreamPerPartition;		// Max stream per partitio

	_gstDumpHeader.stDumpEnv.nActiveBlockCount = g_stGlobal.nActiveBlockCount;	// count of Active Stream, default

	_gstDumpHeader.stDumpEnv.nPartitionSize = g_stGlobal.nPartitionSize;			// size of partition, Default 256K
	_gstDumpHeader.stDumpEnv.nPartitionCount = g_stGlobal.nPartitionCount;		// count of partition - cover logi

	_gstDumpHeader.stDumpEnv.nLogicalPageSize = LOGICAL_PAGE_SIZE;
	_gstDumpHeader.stDumpEnv.nVBlockSize		= g_stGlobal.nVBlockSize;
	_gstDumpHeader.stDumpEnv.nLogicalFlashSize = g_stGlobal.nLogicalFlashSize;				// user space Default 32GB
	_gstDumpHeader.stDumpEnv.nPhysicalFlashSize = g_stGlobal.nPhysicalFlashSize;								// size of total blocks
	_gstDumpHeader.stDumpEnv.nOverprovisionSize = g_stGlobal.nOverprovisionSize;							// OP, 3GB
	_gstDumpHeader.stDumpEnv.nVBlockCount = g_stGlobal.nVBlockCount;					// Total Block Count
	_gstDumpHeader.stDumpEnv.nMetaBlockCount = StreamFTL_GetMetaBlockCount();		// Count of metablock

//	_gstDumpHeader.stDumpEnv.fRandomRate = g_stTestGlobal.fRandomRate;
//	_gstDumpHeader.stDumpEnv.fSeqRate = g_stTestGlobal.fSeqRate;
//	_gstDumpHeader.stDumpEnv.fRandomAmount = g_stTestGlobal.fRandomAmount;
//	_gstDumpHeader.stDumpEnv.nRandomIncrease = g_stTestGlobal.nRandomIncrease;
//	_gstDumpHeader.stDumpEnv.nRandomSize = g_stTestGlobal.nRandomSize;

	_gstDumpHeader.stDumpEnv.fL2PCacheRatio = g_stGlobal.fL2PCacheRatio;

	_gstDumpFooter.nSignature = DUMP_SIGNATURE;
}


static const char*	_GetDumpFileName(VOID)
{
	strcpy(_psDumpFileName, DUMP_FILE_NAME);

#ifdef _DEBUG
	strcat(_psDumpFileName, ".debug");
#else
	strcat(_psDumpFileName, ".release");
#endif

	return (const char*)_psDumpFileName;
}

