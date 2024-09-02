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

#ifndef __STREAMFTL_MAIN_H__
#define __STREAMFTL_MAIN_H__

#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#endif

#include "streamftl_internal.h"

#define PROGRESS_PRINT_INTERVAL		(5000)

/*
* 	Information with default
*/

// [CHECKME][TODO]		MOVE ALL BELOW GLOBAL VARIABLES to GLOBAL structure

#define SECTOR_SIZE			(512)		// SECTOR SIZE, BYTE

/*
*	Structures
*/
#define MEMORY_SIZE				(256*1024*1024)
#define MEMORY_ALIGNMENT		4					// 4 byte aligned allocation
typedef struct
{
	char	abMemory[MEMORY_SIZE];
	INT32	nCurPosition;
} DYNAMIC_MEMORY;

typedef struct
{
	// FTL Configuration
	float			fOverProvisionRatio;
	BOOL			bEnableHotColdMgmt;			// Enable hot cold managements
	float			fHotPartitionRatio;			// 0.1: 10% of used LPN is overwritten
	float			fVictimActiveBlockVictimRange;		// ratio of victim active block within active block list
	INT32			nMaxStreamPerPartition;

	BOOL			bEnableMetaBlock;			// TRUE: FTL consumes meta block

	SMERGE_POLICY	eSMergePolicy;

	double			fL2PCacheRatio;				// density 대비 L2P Cache의 크기. 1: 100%

	INT64			nPhysicalFlashSize;			// byte
	INT64			nLogicalFlashSize;			// byte
	INT64			nOverprovisionSize;			// byte
	INT32			nLPNCount;					// count of Logical Pages

	INT32			nPartitionSize;				// byte
	INT32			nPartitionCount;			// Partition Count
	INT32			nLPagePerPartition;			// logical page(LPN) per partition

	INT32			nStreamSize;				// byte
	INT32			nStreamCount;				// Stream count
	INT32			nLPagePerStream;			// logical page(LPN) per Stream
	INT32			nLPagePerStream32;			// 32bit word count
	INT32			nStreamPerPartition;
	float			nStreamRatio;				// Ratio of number stream to density

	INT32			nActiveBlockCount;			// count of Active STREAM, default 32

	// Flash Configurations
	INT32			nVBlockSize;				// virtual block size,  byte
	INT32			nVBlockCount;				// count of VBN
	INT32			nLPagesPerVBlock;			// Logical Pages per VBN = LPN Per VBlock
	INT32			nLPagesPerVBlockBits;		// bit count of page per vblock
	INT32			nLPagesPerVBlockMask;		// bit mask for Page No

	INT32			nStreamMergeTh;		// Threshold of Free STREAM GC
	INT32			nBlockGCTh;			// Threshold of Free Block GC

#ifdef WIN32
	DYNAMIC_MEMORY	stMemory;
#endif
	// Debugging information
	INT32*			paPartWrite;				// partition별 write count
	BOOL			bPrintAnalysisInfo;

	INT32			nPrevWrittenLPN;					// for sequential check
} GLOBAL;	// Global configurations

extern GLOBAL	g_stGlobal;

VOID StreamFTL_InitGlobal(VOID);
void StreamFTL_InitializeCount(VOID);
VOID StreamFTL_PrintConfiguration(char* psExecutableBinary, char* psTraceFile);

VOID	IncreasePartitionIOCount(INT32 nLPN);

// Global variables
#ifdef WIN32
	extern FILE*		g_fpTestLog;
#endif

extern INT32	nDebugCnt;

#endif	// end of __STREAMFTL_MAIN_H_
