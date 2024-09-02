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

#ifndef __TEST_MAIN_H__
#define __TEST_MAIN_H__

#include "nvme.h"
#include "cosmos_types.h"

#ifdef STREAM_FTL
	#include "streamftl_types.h"
#endif

#ifdef WIN32
	#include "bsp_windows.hl"
#endif

typedef enum
{
	NVME_QID_ADMIN = 0,
	NVME_QUID_IO = 1,
} NVME_QID;

typedef enum
{
	NVME_IO_CMD_INDEX_LBA0 = 10,
	NVME_IO_CMD_INDEX_LBA1 = 11,
	NVME_IO_CMD_INDEX_INFO = 12,
} NVME_CMD_INDEX;

#ifdef WIN32
	extern char* DATA_BUFFER_BASE_ADDR;
#endif

#if 0		// For debugging
	#define TEST_PRINTF		PRINTF
#else
	#define TEST_PRINTF
#endif

typedef struct
{
	// test configuration
	BOOL			bUsePreconditionDump;
	BOOL			bPreconditionDumpLoaded;
	BOOL			bPreconditionDumpOnly;		// Just generage precondition dump file, do not run test
	INT32			nRunCount;					// workload load run count, iteration

	BOOL			bPrecondition;				// precondition before test

	INT32			nAgingIOCount;
	INT64			nTraceTotalWrite;

	// aging variables
	// aging
	double			fRandomRate;
	double			fSeqRate;
	double			fRandomAmount;
	INT32			nRandomIncrease;
	INT32			nRandomSize;
	char*			psAgingFile;

#ifdef WIN32
	FILE*			fpTestLog;
#endif

	INT32			nDebugCnt;
} TEST_GLOBAL;

void test_main();

extern TEST_GLOBAL	g_stTestGlobal;

#endif		// end of #ifndef __TEST_MAIN_H__
