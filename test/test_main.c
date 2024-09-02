﻿/*******************************************************
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

#ifdef WIN32
	#include <time.h>
#endif

#include "assert.h"

#include "debug.h"
#include "xenv_standalone.h"
#include "xil_types.h"

#include "host_lld.h"
#include "test_main.h"
#include "cosmos_plus_system.h"

#ifdef GREEDY_FTL
	#include "request_schedule.h"
	#include "memory_map.h"
#elif defined(STREAM_FTL)
	#include "streamftl.h"
	#include "streamftl_main.h"
	#include "error.h"
	#include "dump.h"
#elif defined(DFTL)
	#include "dftl_types.h"
#elif defined(IOUB)
	#include "ioub_types.h"
	//#include "ioub_global.h"
#endif

#include "random.h"
#include "util.h"
#include "fil.h"
#include "fil_nand.h"
#include "hil.h"
#include "ftl.h"

#include "osal.h"
#include "cosmos_types.h"

#include "test_nvme_request.h"

#include "test_main.h"

#define WORKLOAD_PATH		"E:\\workload\\snia\\msr-cambridge\\"

#ifndef GREEDY_FTL
	#define Vpage2PlsbPageTranslation(pageNo) ((pageNo) > (0) ? (2 * (pageNo) - 1): (0))
#endif

extern void nvme_run();
static void _CheckTimer(int nDurationSec);

// Global variables
#ifdef WIN32
	FILE*		g_fpTestLog;
#endif

typedef enum
{
	TEST_BUF_OFFSET_CHANNEL,
	TEST_BUF_OFFSET_WAY,
	TEST_BUF_OFFSET_BLOCK,
	TEST_BUF_OFFSET_PAGE,
	TEST_BUF_OFFSET_SEQUENCE,
	TEST_BUF_OFFSET_COUNT,
} TEST_BUF_OFFSET;

static VOID		_PrintTestProgress(INT32 nProgressPercent);
static VOID		command_setting(int argc, char *argv[]);
#ifdef WIN32
	static time_t	_PrintCurrentTime(VOID);
#else
	#define _PrintCurrentTime()

#endif
static INT32	parsing_size(char * str);
static VOID		print_count(char *psWorkload, INT64 trace_total_write);

static VOID _DoAging(VOID);
static void _SeqFill(void);
static void _PreconditionRandomWrite(void);
static INT32 _RunWorkload(char* pstTraceFileName);

#ifdef WIN32
static VOID _OpenTestLogFile(char* pstTraceFileName);
static VOID _CloseTestLogFile(VOID);
#endif

static VOID _CheckMetadataValidity(VOID);
static VOID _PrintAnalysisInfo(char* psTraceFileName);
#ifdef WIN32
	static FILE* _OpenAnalysisLogFile(char* pstTraceFileName);
	static char* _GetFileName(char* psPath);
#endif
static VOID _PrintFTLInfo(char* psWorkload);


#ifdef WIN32
static TRACE_TYPE _ParseMSRTrace(FILE *fp, int *start_LPN, int *count);
#endif

static INT32 _GetTraceLineCount(char* psWorkloadPath);

static void _ReadLPN(INT32 nLBA, INT32 nBlockCount);
static void _WriteLPN(INT32 nLBA, INT32 nBlockCount);

TEST_GLOBAL	g_stTestGlobal;

#ifdef WIN32
extern int cosmos_plus_main();

#if defined(STREAM_FTL) || defined(DFTL)
int main(int argc, char *argv[])
{
	command_setting(argc, argv);

	char *pstTraceFileName = argv[1];
	
	cosmos_plus_main();

	//firmware test
	return;

	g_stTestGlobal.bUsePreconditionDump = FALSE;
	g_stTestGlobal.bPreconditionDumpLoaded = FALSE;
	g_stTestGlobal.nRunCount = 1;
	g_stTestGlobal.nAgingIOCount = 0;

	if (g_stTestGlobal.bPrecondition == TRUE)
	{
		if (g_stTestGlobal.psAgingFile == NULL)
		{
			LOG_PRINTF("[DEBUG] SEQ RATE: %f\r\n", g_stTestGlobal.fSeqRate);
			LOG_PRINTF("[DEBUG] RANDOM RATE : %f\r\n", g_stTestGlobal.fRandomRate);
			LOG_PRINTF("[DEBUG] RANDOM MOUNT : %f\r\n", g_stTestGlobal.fRandomAmount);
			LOG_PRINTF("[DEBUG] g_stTestGlobal.nRandomIncrease : %d\r\n", g_stTestGlobal.nRandomIncrease);
			LOG_PRINTF("[DEBUG] g_stTestGlobal.nRandomSize : %d\r\n", g_stTestGlobal.nRandomSize);
		}
		else
		{
			LOG_PRINTF("[DEBUG] Aging file : %s\r\n", g_stTestGlobal.psAgingFile);
		}
	}
	else
	{
		LOG_PRINTF("[DEBUG] No precondition \r\n");
	}

	// open test log file
	_OpenTestLogFile(pstTraceFileName);

	time_t nStartTime = _PrintCurrentTime();

	_DoAging();					// precondition

	LOG_PRINTF("\n[main] AGING FINN\r\n");
	_PrintCurrentTime();

	_CheckMetadataValidity();

	g_stTestGlobal.nTraceTotalWrite = 0;

	FTL_IOCtl(IOCTL_INIT_PROFILE_COUNT);

	if (g_stTestGlobal.bPreconditionDumpOnly == TRUE)
	{
		goto out;
	}

	INT32	nRunCount = g_stTestGlobal.nRunCount;

	do
	{
		_RunWorkload(pstTraceFileName);
		print_count(pstTraceFileName, g_stTestGlobal.nTraceTotalWrite);
		_PrintFTLInfo(pstTraceFileName);

		nRunCount --;
	} while (nRunCount > 0);

	LOG_PRINTF("fin\n");

out:
	LOG_PRINTF("Finish =======================================\n");

	time_t nFinishTime;;
	nFinishTime = _PrintCurrentTime();

	double fDiff;
	fDiff = difftime(nFinishTime, nStartTime);
	LOG_PRINTF("Elapsed Time: %.2lf Sec \n", fDiff);

	if (g_stTestGlobal.bPreconditionDumpOnly == FALSE)
	{
		_PrintAnalysisInfo(pstTraceFileName);
		_CheckMetadataValidity();
	}

#ifdef WIN32
	_CloseTestLogFile();
#endif

	//	getch();
	exit(1);
}
#endif	// end of #ifdef STREAM_FTL
#endif	// end of #ifdef WIN32

void NVME_SetCCEN()
{
	// NVME_TASK_WAIT_CC_EN
	DEV_IRQ_REG* pDevReg = (DEV_IRQ_REG *)DEV_IRQ_STATUS_REG_ADDR;
	pDevReg->nvmeCcEn = 1;

	NVME_STATUS_REG* pNvmeReg = (NVME_STATUS_REG *)NVME_STATUS_REG_ADDR;
	pNvmeReg->ccEn = 1;

	dev_irq_handler();		// interrupt processing
}

int NVME_SetIOCmd(int nStartLBA, unsigned short nLBACount, NVME_CMD_OPCODE nCmdOPCode)
{
	DEBUG_ASSERT(nCmdOPCode != NVME_CMD_OPCODE_FLUASH);	// not support yet
	DEBUG_ASSERT(nLBACount > 0);
	NVMeRequest*	pstRequest;
	pstRequest = NVMeRequest_Allocate();
	if (pstRequest == NULL)
	{
		return FALSE;
	}

	pstRequest->stFIFOReg.cmdValid		= NVME_CMD_VALID;
	pstRequest->stFIFOReg.qID			= NVME_SQID_USER;
	//pRequest->stFIFOReg.cmdSlotTag		remain original
	pstRequest->stFIFOReg.cmdSeqNum		= 0;

	NVME_CMD_FIFO_REG* pNVMeFifoReg = (NVME_CMD_FIFO_REG *)NVME_CMD_FIFO_REG_ADDR;
	*pNVMeFifoReg = pstRequest->stFIFOReg;

	pstRequest->stCmd.OPC = nCmdOPCode;
	pstRequest->stCmd.dword[NVME_IO_CMD_INDEX_LBA0] = nStartLBA;		// startLBA0
	pstRequest->stCmd.dword[NVME_IO_CMD_INDEX_LBA1] = 0;		// startLBA1
	pstRequest->stCmd.PRP1[0] = 0;
	pstRequest->stCmd.PRP2[0] = 0;
	pstRequest->stCmd.PRP1[1] = 0;
	pstRequest->stCmd.PRP2[1] = 0;

	NVME_IO_COMMAND* pNVMeCmd = (NVME_IO_COMMAND *)(NVME_CMD_SRAM_ADDR + (pNVMeFifoReg->cmdSlotTag * 64));
	*pNVMeCmd = pstRequest->stCmd;

	pstRequest->stCmdDW12.NLB = nLBACount - 1;
	pstRequest->stCmdDW12.PRINFO = 0;
	pstRequest->stCmdDW12.FUA = 0;			// not support
	pstRequest->stCmdDW12.LR = 0;			// Limited Retry

	// read/write same structure
	IO_READ_COMMAND_DW12* pNVMeCmdInfo = (IO_READ_COMMAND_DW12*)&pNVMeCmd->dword[NVME_IO_CMD_INDEX_INFO];
	*pNVMeCmdInfo = pstRequest->stCmdDW12;

	pstRequest->nRestDMACount = nLBACount;

	TEST_PRINTF("Set NVME CMD: OPCode / LBA / Count : %d / %d / %d \n\r", nCmdOPCode, nStartLBA, nLBACount);

	return TRUE;
}

void NAND_Program_Verification(int nCh, int nWay, int nBlock, int nPage, void* pMainBuf, void* pSpareBuf)
{
#ifdef STREAM_FTL
	static unsigned int nWriteCount = 0;

	// setup verification data
	unsigned int *pDataBuf = (unsigned int*)pMainBuf;
	pDataBuf[TEST_BUF_OFFSET_CHANNEL]	= nCh;
	pDataBuf[TEST_BUF_OFFSET_WAY]		= nWay;
	pDataBuf[TEST_BUF_OFFSET_BLOCK]		= nBlock;
	pDataBuf[TEST_BUF_OFFSET_PAGE]		= nPage;
	pDataBuf[TEST_BUF_OFFSET_SEQUENCE]	= nWriteCount;

	NAND_IssueProgram(nCh, nWay, nBlock, nPage, pMainBuf, pSpareBuf, TRUE);

	xil_printf("NAND Program Data: ");
	for (int i = 0; i < TEST_BUF_OFFSET_COUNT; i++)
	{
		xil_printf("0x%X, ", pDataBuf[i]);
	}
	xil_printf("\r\n");

	XENV_MEM_FILL(pMainBuf, 0x00, 64);

	NAND_IssueRead(nCh, nWay, nBlock, nPage, pMainBuf, pSpareBuf, TRUE, TRUE);

	xil_printf("NAND Read Data: ");
	for (int i = 0; i < TEST_BUF_OFFSET_COUNT; i++)
	{
		xil_printf("0x%X, ", pDataBuf[i]);
	}
	xil_printf("\r\n");

	nWriteCount++;
#endif
}
int IO_LBA[7] = { 0, 1, 64, 65, 193, 194, 8130 };
void PDTest()
{
	int		nLBA, nBlockCount;
	int		bSync = FALSE;

	for (int i = 0; i < 7; i++)
	{
		nLBA = IO_LBA[i];

		nBlockCount = 1;


		while (NVME_SetIOCmd(nLBA, nBlockCount, NVME_CMD_OPCODE_WRITE) == FALSE)
		{
			nvme_run();
		}

		do
		{
			nvme_run();
			if (bSync == FALSE)
			{
				break;
			}
		} while (g_pstNVMeRequestPool->m_nFree != NVME_REQUEST_COUNT);
	}

	PRINTF("\r\n");
}
#define ADDR_PRINT_PROFILE (8288)
#define ADDR_CLOSE_SECTION (8352)
#define ADDR_INTERNAL_MERGE (8416)

#ifdef IOUB_STRIPING
static UINT32 IOUB_partition_start[4 + 1] = { 0, 4194304, 8388608, 16777216, 0xffffffff };
static UINT32 IOUB_partition_PMsize[4] = { 29696, 29696, 30720, 32768 };
#else
static UINT32 IOUB_partition_start[4 + 1] = { 0, 0xffffffff };
static UINT32 IOUB_partition_PMsize[4] = { 114688 };
#endif

void RunTest(int nMaxLBA, int nLBACount, NVME_CMD_OPCODE nOPCode, int bRand, int nRandIOSize, int nMaxIOLBASize, int bPrintProgress, int mixed)
{
	int		nLBA;
	int		nBlockCount = nMaxIOLBASize;
	int		bSync = FALSE;
	NVME_CMD_OPCODE code = nOPCode;
	for (int i = 0; i < nLBACount; i += nBlockCount)
	{
		nLBA = i;
		if (nLBA == 29695)
			i = nLBA;
		if (bRand == TRUE)
		{
			nLBA = UTIL_Random() % nMaxLBA;
		}


		if (nRandIOSize == TRUE)
		{
			nBlockCount = UTIL_Random() % nMaxIOLBASize;
			if (nBlockCount == 0)
			{
				nBlockCount = 1;
			}
		}
		if (mixed == TRUE) {
			int isWrite = UTIL_Random() % 2;
			if (isWrite)
				code = NVME_CMD_OPCODE_WRITE;
			else
				code = NVME_CMD_OPCODE_READ;
		}
		while (((nLBA <= ADDR_PRINT_PROFILE && (nLBA + nBlockCount >= ADDR_PRINT_PROFILE)))) {
			nLBA = UTIL_Random() % nMaxLBA;
		}
#ifdef IOUB
		while (!((nLBA < ADDR_CLOSE_SECTION && (nLBA + nBlockCount < ADDR_CLOSE_SECTION)) || (nLBA > ADDR_INTERNAL_MERGE && (nLBA + nBlockCount > ADDR_INTERNAL_MERGE)))) {
			nLBA = UTIL_Random() % nMaxLBA;
		}
		UINT32 part_off;
		for (part_off = 0; part_off < 4; part_off++) {
			if (nLBA < IOUB_partition_start[part_off + 1])
				break;
		}

		while ((nLBA < IOUB_partition_start[part_off] + IOUB_partition_PMsize[part_off]) && ((nLBA + nBlockCount - 1) >= IOUB_partition_start[part_off] + IOUB_partition_PMsize[part_off]))
		{
			nBlockCount--;
		}

#endif
		
		if ((nLBA + nBlockCount) >= nMaxLBA)
		{
			nBlockCount = nMaxLBA - nLBA;
		}

		while (NVME_SetIOCmd(nLBA, nBlockCount, code) == FALSE)
		{
			nvme_run();
		}

		do
		{
			nvme_run();
			if (bSync == FALSE)
			{
				break;
			}
		} while (g_pstNVMeRequestPool->m_nFree != NVME_REQUEST_COUNT);

		if (bPrintProgress == TRUE)
		{
			_PrintTestProgress(((long long)i * 100) / nLBACount);
		}
	}

	PRINTF("\r\n");
}

static void _Sync()
{
#ifdef GREEDY_FTL
	SyncAllLowLevelReqDone();		// for GreedyFTL
#else
	while (g_pstNVMeRequestPool->m_nFree != NVME_REQUEST_COUNT)
	{
		nvme_run();
	}
#endif
}

#define MAX_ITER_TEST (4194304 * 2)

#define TC_SECTION_START_LPN (49152)

#define TC_NUM_OF_SECTION	(400)

#define TC_MAX_LPN	(3316120)			//utilization 80%
#define TC_SEGS_PER_SECTION (16)
#define TC_PAGES_PER_SECTION (8192)

#define PAGES_PER_SUPERPAGE (4)

UINT32 TC_write_buffer_offset = 0;



UINT32 tc_get_l2p(UINT32 lpn) {
	return TC_L2P_ADDR[lpn];
}

void tc_set_l2p(UINT32 lpn, UINT32 ppn) {
	TC_L2P_ADDR[lpn] = ppn;
}
UINT32 tc_get_p2l(UINT32 ppn) {
	return TC_P2L_ADDR[ppn];
}

void tc_set_p2l(UINT32 ppn, UINT32 lpn) {
	TC_P2L_ADDR[ppn] = lpn;
}

struct tc_blk_list_struct {
	UINT32 offset;
	UINT32 lbn;
	void* next;
};
struct tc_free_blk_list {
	UINT32 count;
	struct tc_blk_list_struct *head;
};
struct tc_blk_list_struct tc_blk_list[TC_NUM_OF_SECTION];
struct tc_free_blk_list tc_list_head;
struct tc_free_blk_list tc_hot_list;

UINT32 tc_get_free_blk() {
	if (tc_list_head.count) {
		struct tc_blk_list_struct* temp = tc_list_head.head;
		tc_list_head.head = (struct tc_blk_list_struct*)temp->next;
		temp->next = NULL;
		tc_list_head.count--;
		return temp->offset;
	}
	ASSERT(0);
}
void tc_set_free_blk(UINT32 offset) {
	if (tc_list_head.head == NULL) {
		tc_list_head.head = &tc_blk_list[offset];
	}
	else {
		struct tc_blk_list_struct *temp = tc_list_head.head;
		for (; temp->next != NULL; temp = (struct tc_blk_list_struct*)temp->next);
		tc_blk_list[offset].next = NULL;
		temp->next = &tc_blk_list[offset];
	}
	tc_list_head.count++;
}

UINT32 tc_get_victim_blk() {
	struct tc_blk_list_struct *temp = tc_hot_list.head;
	tc_hot_list.head = (struct tc_blk_list_struct*)temp->next;
	tc_hot_list.count--;
	temp->next = NULL;
	return temp->offset;
}
void tc_set_hot_seg_blk(UINT32 offset) {
	if (tc_hot_list.head == NULL) {
		tc_hot_list.head = &tc_blk_list[offset];
	}
	else {
		struct tc_blk_list_struct *temp = tc_hot_list.head;
		for (; temp->next != NULL; temp = (struct tc_blk_list_struct*)temp->next);
		tc_blk_list[offset].next = NULL;
		temp->next = &tc_blk_list[offset];
	}
	tc_hot_list.count++;
}

UINT32 TC_read_and_test() {
	UINT32 lpn;
	UINT32 error_occur = 0;
	for (lpn = 0; lpn < TC_MAX_LPN; lpn++) {
		UINT32 sample_data = TC_DATA_ADDR[lpn];
		UINT32 read_data;
		if (lpn == 2733466)
			lpn = lpn;
		if (sample_data) {
			UINT32 ppn = tc_get_l2p(lpn);
			if (ppn == 0xffffffff)
				continue;
			if (ppn == 8192)
				printf("d");
			while (NVME_SetIOCmd(TC_SECTION_START_LPN + ppn, 1, NVME_CMD_OPCODE_READ) == FALSE)
			{
				nvme_run();
			}
			_Sync();

			

			read_data = TC_READ_BUFFER[0];
			if (sample_data != read_data) {
				PRINTF("lpn: %u - ppn: %u\n", lpn, ppn + TC_SECTION_START_LPN);
				PRINTF("sample: %u  read: %u\n", sample_data, read_data);
				error_occur = 1;

			}
		}
	}
	if (error_occur)
		DEBUG_ASSERT(0);
	return 1;
}
#define TC_IM_TEST
//#define TC_SSR_MODE
#define NUM_HOT_SEC (2)

//IOUB ADDRESS
#define TC_IOUB_START_ADDR 8224 // excpet partition table
#define TC_IOUB_ADDR_CNT   (16 * 1024 * 2 - TC_IOUB_START_ADDR) // use first 16MB space

#define TC_CMD_TRANSFER_UNIT       32
#define TC_ADDR_CLOSE_SECTION      (TC_IOUB_START_ADDR + TC_CMD_TRANSFER_UNIT * 4) // 
#define TC_ADDR_INTERNAL_MERGE     (TC_IOUB_START_ADDR + TC_CMD_TRANSFER_UNIT * 6) // 

static void IOUBTest() {
	unsigned int* temp_addr;
	//malloc L2P & P2L
	

	UINT32 iter;
	UINT32 hot_seg_cusor[NUM_HOT_SEC], cold_seg_cusor;
	UINT32 offset;
	UINT32 test_data = 0;
	UINT32 section_alloc = 0;
	UINT32 ssr_count = 0;
	UINT32 next_test = 6879499;
	UINT32 SSR_l2p_out, SSR_p2l_out;
	static int SC_seq = 0;

	tc_list_head.count = 0;
	tc_list_head.head = NULL;
	tc_hot_list.count = 0;
	tc_hot_list.head = NULL;
	for (iter = 0; iter < TC_NUM_OF_SECTION; iter++) {
		tc_set_free_blk(iter);
		tc_blk_list[iter].offset = iter;
	}

	
	TC_L2P_ADDR = malloc(TC_MAX_LPN * sizeof(unsigned int));
	TC_P2L_ADDR = malloc(TC_NUM_OF_SECTION * TC_PAGES_PER_SECTION * sizeof(unsigned int));
	TC_DATA_ADDR = malloc(HIL_GetStorageBlocks() * sizeof(unsigned int));
	TC_RDATA_ADDR = malloc(HIL_GetStorageBlocks() * sizeof(unsigned int));

	TC_READ_BUFFER = malloc(4096);
	TC_WRITE_BUFFER = malloc(4096);

	OSAL_MEMSET(TC_L2P_ADDR, 0xffffffff, TC_MAX_LPN * sizeof(unsigned int));
	OSAL_MEMSET(TC_P2L_ADDR, 0xffffffff, TC_NUM_OF_SECTION * TC_PAGES_PER_SECTION * sizeof(unsigned int));
	OSAL_MEMSET(TC_DATA_ADDR, 0xffffffff, HIL_GetStorageBlocks() * sizeof(unsigned int));
	OSAL_MEMSET(TC_RDATA_ADDR, 0xffffffff, HIL_GetStorageBlocks() * sizeof(unsigned int));


	offset = tc_get_free_blk();
	hot_seg_cusor[0] = offset * TC_PAGES_PER_SECTION;

	OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
	TC_WRITE_BUFFER[0] = SC_seq++;
	TC_WRITE_BUFFER[1] = 0;
	TC_WRITE_BUFFER[2] = 0xffffffff;
	TC_WRITE_BUFFER[3] = offset;
	while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	nvme_run();


	offset = tc_get_free_blk();
	hot_seg_cusor[1] = offset * TC_PAGES_PER_SECTION;

	OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
	TC_WRITE_BUFFER[0] = SC_seq++;
	TC_WRITE_BUFFER[1] = 1;
	TC_WRITE_BUFFER[2] = 0xffffffff;
	TC_WRITE_BUFFER[3] = offset;
	while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	nvme_run();

	offset = tc_get_free_blk();
	cold_seg_cusor = offset * TC_PAGES_PER_SECTION;

	OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
	TC_WRITE_BUFFER[0] = SC_seq++;
	TC_WRITE_BUFFER[1] = 2;
	TC_WRITE_BUFFER[2] = 0xffffffff;
	TC_WRITE_BUFFER[3] = offset;
	while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	nvme_run();


	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();

	while (1) {
		UINT32 lpn, start_ppn;
		UINT32 hot_sec_type = rand() % NUM_HOT_SEC;
		BOOL loop_for_SSR = FALSE;
		static UINT32 IM_counter_global = 0;

		do {
			//write one page
			if (iter < 2816120)
				lpn = iter;
			else
				lpn = (rand()*rand()) % TC_MAX_LPN;
			loop_for_SSR = FALSE;
			for (UINT32 iter = 0; iter < NUM_HOT_SEC; iter++)
			{
				if (tc_get_l2p(lpn) / 8192 == hot_seg_cusor[iter] / 8192)
					loop_for_SSR = TRUE;
			}
		} while (loop_for_SSR);

		start_ppn = hot_seg_cusor[hot_sec_type];


		TC_DATA_ADDR[lpn] = TC_SECTION_START_LPN + start_ppn;

		//hot data write
		tc_set_l2p(lpn, hot_seg_cusor[hot_sec_type]);
		tc_set_p2l(hot_seg_cusor[hot_sec_type], lpn);

		//	uart_printf("%u W - l-%u->p-%u %u - D: %u %x ", iter, lpn, hot_seg_cusor[hot_sec_type], cmd.sector_count, test_data - 1, (hot_seg_cusor[hot_sec_type] % PAGES_PER_SUPERPAGE) * BYTES_PER_SUB_PAGE);
		//	delay(300000);
#ifndef TC_SSR_MODE
		hot_seg_cusor[hot_sec_type]++;
#endif

		//	printf("iter: %u L: %u  P: %u\n", iter, lpn, hot_seg_cusor[hot_sec_type] + TC_SECTION_START_LPN);

		while (NVME_SetIOCmd(start_ppn + TC_SECTION_START_LPN, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
		{
			nvme_run();
		}
		nvme_run();



#ifdef TC_SSR_MODE
		do {
			SSR_l2p_out = 0xffffffff;
			//cusor update
			hot_seg_cusor[hot_sec_type]++;

#endif

		//hot section change
		if (hot_seg_cusor[hot_sec_type] % TC_PAGES_PER_SECTION == 0) { //section close
			UINT32 seg_offset, page_offset;
			UINT32 old_offset = hot_seg_cusor[hot_sec_type] / TC_PAGES_PER_SECTION - 1;
			
#ifdef TC_SSR_MODE
			if (tc_list_head.count < 1) {
				offset = tc_get_victim_blk();
			}
			else
#endif
			offset = tc_get_free_blk();

			hot_seg_cusor[hot_sec_type] = offset * TC_PAGES_PER_SECTION;
			tc_set_hot_seg_blk(old_offset);

			OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
			TC_WRITE_BUFFER[0] = SC_seq++;
			TC_WRITE_BUFFER[1] = hot_sec_type;
			TC_WRITE_BUFFER[2] = old_offset;
			TC_WRITE_BUFFER[3] = offset;
			if (offset == 0)
				printf("d");
			//bitmap 추가
			for (seg_offset = 0; seg_offset < TC_SEGS_PER_SECTION; seg_offset++) {
				for (page_offset = 0; page_offset < 512; page_offset++) {
					UINT32 ppn = offset * TC_PAGES_PER_SECTION + seg_offset * TC_PAGES_PER_SECTION / TC_SEGS_PER_SECTION + page_offset;
					UINT32 p2l_output;
					UINT32 l2p_output;
					UINT32 offset_in_the_section = seg_offset * TC_PAGES_PER_SECTION / TC_SEGS_PER_SECTION + page_offset;


					p2l_output = tc_get_p2l(ppn);
					l2p_output = tc_get_l2p(p2l_output);
					if (l2p_output == ppn) {

						UINT32 bitmap_offset = offset_in_the_section >> 5;  // A >> 5 == A / 32
						UINT32 byte_offset = offset_in_the_section % 32;
						UINT32 offset_in_byte = offset_in_the_section % 8;
						byte_offset = byte_offset >> 3;

						TC_WRITE_BUFFER[4 + bitmap_offset] ^= (1 << ((byte_offset + 1) * 8 - offset_in_byte - 1));
					}
				}
			}
		

			while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
			{
				nvme_run();
			}
			nvme_run();

			//uart_printf("section close - %u %u %u", 1, old_offset + TC_SECTION_START_LPN / TC_PAGES_PER_SECTION, offset + TC_SECTION_START_LPN / TC_PAGES_PER_SECTION);
			//uart_printf("HOT_SEG ALLOC %u: %u ", section_alloc++, offset);
		}

#ifdef TC_SSR_MODE

			SSR_p2l_out = tc_get_p2l(hot_seg_cusor[hot_sec_type]);
			if (SSR_p2l_out != 0xffffffff) {
				SSR_l2p_out = tc_get_l2p(SSR_p2l_out);
			}
		} while (SSR_l2p_out == hot_seg_cusor[hot_sec_type]);
#endif
#ifdef TC_IM_TEST
		//check GC with IM
#ifndef TC_SSR_MODE
		if (tc_list_head.count <= 2) {


			UINT32 victim_section;
			UINT32 seg_offset;
			UINT32 page_offset;

			//TC_read_and_test();
			//select_victim
			victim_section = tc_get_victim_blk();
			//uart_printf("IM_START - %u", victim_section);
			for (seg_offset = 0; seg_offset < TC_SEGS_PER_SECTION; seg_offset++) {
				UINT32 IM_count = 0;
				UINT32 write_lpn_in_GC[512];
				UINT32 write_data_in_GC[512];
				UINT32 max_some_write_cnt;
				UINT32 some_write_cnt = 0;
				UINT32 iner_iter;
				UINT32 section_chaged = 0;
				UINT32 old_section_no = cold_seg_cusor / TC_PAGES_PER_SECTION;
				UINT32 cur_physical_page = 0;
				UINT32 cur_page_valid_count = 0;

				max_some_write_cnt = 0;// rand() % 16;
				//insert to some write page
				for (page_offset = 0; page_offset < 512; page_offset++)
				{
					UINT32 ppn = victim_section * TC_PAGES_PER_SECTION + seg_offset * TC_PAGES_PER_SECTION / TC_SEGS_PER_SECTION + page_offset;
					UINT32 p2l_output;
					UINT32 l2p_output;
					UINT32 SSR_p2l_out;
					UINT32 SSR_l2p_out;

					if (page_offset / 4 != cur_physical_page)
					{
						if (cur_page_valid_count == 4)
						{
							max_some_write_cnt -= 4;
						}
						cur_page_valid_count = 0;
					}

					p2l_output = tc_get_p2l(ppn);
					l2p_output = tc_get_l2p(p2l_output);
					if (l2p_output == ppn) {
						write_lpn_in_GC[max_some_write_cnt] = p2l_output;
						write_data_in_GC[max_some_write_cnt] = TC_DATA_ADDR[write_lpn_in_GC[max_some_write_cnt]];
						max_some_write_cnt++;
						cur_page_valid_count++;
					}
					if (p2l_output == 400)
						printf(" ");
					cur_physical_page = page_offset / 4;
				}

				while (cold_seg_cusor % 4 != 0)
				{
					while (NVME_SetIOCmd(cold_seg_cusor + TC_SECTION_START_LPN, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
					{
						nvme_run();
					}
					nvme_run();
					//printf("cold_seg_write0: %u-%u\n", cold_seg_cusor / 8192, cold_seg_cusor % 8192);
					//TODO: data copy


					TC_DATA_ADDR[write_lpn_in_GC[some_write_cnt]] = TC_SECTION_START_LPN + cold_seg_cusor;
					tc_set_l2p(write_lpn_in_GC[some_write_cnt], cold_seg_cusor);
					tc_set_p2l(cold_seg_cusor, write_lpn_in_GC[some_write_cnt]);
					some_write_cnt++;

					cold_seg_cusor++;
					if (cold_seg_cusor % TC_PAGES_PER_SECTION == 0) {
						UINT32 old_offset = cold_seg_cusor / TC_PAGES_PER_SECTION - 1;
						tc_set_hot_seg_blk(cold_seg_cusor / TC_PAGES_PER_SECTION - 1);
						offset = tc_get_free_blk();
						cold_seg_cusor = offset * TC_PAGES_PER_SECTION;
						//printf("COLD_SEG ALLOC %u: %u ", section_alloc++, offset);
						section_chaged = 1;

						OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
						TC_WRITE_BUFFER[0] = SC_seq++;
						TC_WRITE_BUFFER[1] = 2;
						TC_WRITE_BUFFER[2] = old_offset;
						TC_WRITE_BUFFER[3] = offset;

						while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
						{
							nvme_run();
						}
						nvme_run();
					}



				}
				//uart_printf("IM_NO: %u", IM_counter_global++);
				

				//make IM_struct
				OSAL_MEMSET(TC_WRITE_BUFFER, 0xffffffff, 4096);
				
				for (page_offset = 0; page_offset < 512; page_offset++) {
					UINT32 ppn = victim_section * TC_PAGES_PER_SECTION + seg_offset * TC_PAGES_PER_SECTION / TC_SEGS_PER_SECTION + page_offset;
					UINT32 p2l_output;
					UINT32 l2p_output;
					UINT32 SSR_p2l_out;
					UINT32 SSR_l2p_out;
					BOOL is_in_somewrite = 0;

					p2l_output = tc_get_p2l(ppn);
					l2p_output = tc_get_l2p(p2l_output);
					if (l2p_output == ppn) {
						for (iner_iter = 0; iner_iter < max_some_write_cnt; iner_iter++)
						{
							if (write_lpn_in_GC[iner_iter] == p2l_output)
							{
								is_in_somewrite = 1;
								break;
							}
						}
						if (is_in_somewrite) {
							continue;
						}
						//valid page
						TC_WRITE_BUFFER[IM_count * 2] = ppn + TC_SECTION_START_LPN;
						TC_WRITE_BUFFER[IM_count * 2 + 1] = cold_seg_cusor + TC_SECTION_START_LPN;
						//if(iter >= 5341740)
						//	printf("iter: %u L: %u  P: %u-> %u\n", iter, p2l_output, ppn + TC_SECTION_START_LPN, cold_seg_cusor + TC_SECTION_START_LPN);
						IM_count++;
						//printf("IM write: %u-%u -> %u-%u    lpn: %u\n", ppn / 8192, ppn % 8192, cold_seg_cusor / 8192, cold_seg_cusor % 8192, p2l_output);
						//for free block
						tc_set_l2p(p2l_output, cold_seg_cusor);
						tc_set_p2l(cold_seg_cusor, p2l_output);

						//delay(300000);


						cold_seg_cusor++;
						if (cold_seg_cusor % TC_PAGES_PER_SECTION == 0) {
							if (IM_count != 512)
								TC_WRITE_BUFFER[IM_count * 2] = NULL;

							if (IM_count != 0)
							{
								while (NVME_SetIOCmd(TC_ADDR_INTERNAL_MERGE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
								{
									nvme_run();
								}
								nvme_run();
							}
							IM_count = 0;


							UINT32 old_offset = cold_seg_cusor / TC_PAGES_PER_SECTION - 1;
							tc_set_hot_seg_blk(cold_seg_cusor / TC_PAGES_PER_SECTION - 1);
							offset = tc_get_free_blk();
							cold_seg_cusor = offset * TC_PAGES_PER_SECTION;
							//printf("COLD_SEG ALLOC %u: %u ", section_alloc++, offset);
							section_chaged = 1;

							OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
							TC_WRITE_BUFFER[0] = SC_seq++;
							TC_WRITE_BUFFER[1] = 2;
							TC_WRITE_BUFFER[2] = old_offset;
							TC_WRITE_BUFFER[3] = offset;

							while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
							{
								nvme_run();
							}
							nvme_run();
						}
						//SSR mode 
					}
				}
				if(IM_count != 512)
					TC_WRITE_BUFFER[IM_count * 2] = NULL;

				if (IM_count != 0)
				{
					while (NVME_SetIOCmd(TC_ADDR_INTERNAL_MERGE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
					{
						nvme_run();
					}
					nvme_run();
				}


				//write some pages
				for (; some_write_cnt < max_some_write_cnt; some_write_cnt++) {

					while (NVME_SetIOCmd(cold_seg_cusor + TC_SECTION_START_LPN, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
					{
						nvme_run();
					}
					nvme_run();
					//printf("cold_seg_write1: %u-%u\n", cold_seg_cusor / 8192, cold_seg_cusor % 8192);
					//TODO: data copy

					TC_DATA_ADDR[write_lpn_in_GC[some_write_cnt]] = TC_SECTION_START_LPN + cold_seg_cusor;
					tc_set_l2p(write_lpn_in_GC[some_write_cnt], cold_seg_cusor);
					tc_set_p2l(cold_seg_cusor, write_lpn_in_GC[some_write_cnt]);

					cold_seg_cusor++;
					if (cold_seg_cusor % TC_PAGES_PER_SECTION == 0) {
						UINT32 old_offset = cold_seg_cusor / TC_PAGES_PER_SECTION - 1;
						tc_set_hot_seg_blk(cold_seg_cusor / TC_PAGES_PER_SECTION - 1);
						offset = tc_get_free_blk();
						cold_seg_cusor = offset * TC_PAGES_PER_SECTION;
						//uart_printf("COLD_SEG ALLOC %u: %u ", section_alloc++, offset);
						section_chaged = 1;

						OSAL_MEMSET(TC_WRITE_BUFFER, 0, 4096);
						TC_WRITE_BUFFER[0] = SC_seq++;
						TC_WRITE_BUFFER[1] = 2;
						TC_WRITE_BUFFER[2] = old_offset;
						TC_WRITE_BUFFER[3] = offset;

						while (NVME_SetIOCmd(TC_ADDR_CLOSE_SECTION, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
						{
							nvme_run();
						}
						nvme_run();
					}


					


				}
			}
			tc_set_free_blk(victim_section);

			_Sync();
			//test_with_flush
			//uart_printf("TEST - %u ", iter);
			//TC_read_and_test();			

		}
#endif
#endif
		nvme_run();
		if(next_test < iter) {
		next_test += 100000;
		TC_read_and_test();
		}
		if (iter++ > MAX_ITER_TEST) {
			xil_printf("\n");
			TC_read_and_test();
			break;
		}
		if (iter % (MAX_ITER_TEST / 1000) == 0) {
			xil_printf("-", iter / (MAX_ITER_TEST / 1000));
		}

	}

	
}

void IOUB_log_test() {
	
	struct file* fp;

	fp= fopen("log2.file", "r");
	if (!fp)
		return;
	unsigned int test_count = 0;
	for (int i = 0; i < 138043; i ++)
	{
		UINT32 type, nLBA, nCount;
		NVME_CMD_OPCODE nOPCode;
		fscanf(fp, "%u	%u	%u", &type, &nLBA, &nCount);
		
		if (type == 1) {
			nOPCode = NVME_CMD_OPCODE_READ;
		}
		else {
			nOPCode = NVME_CMD_OPCODE_WRITE;
		}

		while (NVME_SetIOCmd(nLBA, nCount, nOPCode) == FALSE)
		{
			nvme_run();
		}

		nvme_run();
	}

	_Sync();
	PRINTF("\r\n");
	fclose(fp);
}

void test_basic()
{
	int nMaxLBA;
	int nLBACount;

#ifdef GREEDY_FTL
	nMaxLBA = SLICES_PER_SSD;
	nLBACount = SLICES_PER_SSD;
#else
	nMaxLBA = HIL_GetStorageBlocks() - 1;
#endif
	int nMaxIOSize = 256;		// lba count

#ifdef IOUB
	IOUBTest();
#endif
	//80% seq write
	nLBACount = (int)(nMaxLBA * 0.3);		// 80% write
	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_WRITE, FALSE, TRUE, 256, TRUE, FALSE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");

	//80% seq read
	_Sync();
	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, FALSE, TRUE, 256, TRUE, FALSE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");

	//50% rand write
	nLBACount = (int)(nMaxLBA * 0.5);		// 50% write
	_Sync();

	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_WRITE, TRUE, TRUE, 256, TRUE, FALSE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");

	

	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();
	//80% seq read
	nLBACount = (int)(nMaxLBA * 0.8);		// 80% write
	_Sync();
	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, FALSE, TRUE, 256, TRUE, FALSE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");

	//80% rand read+write
	_Sync();
	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, FALSE, TRUE, 256, TRUE, TRUE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");

	nLBACount = (int)(nMaxLBA * 0.8);		// 0.3: 30% write

	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();
	//50% rand read+write
	nLBACount = (int)(nMaxLBA * 0.5);		// 50% write
	_Sync();
	PRINTF("Seq. Read Start, #LBA: %d \r\n", nLBACount);
	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, FALSE, TRUE, 256, TRUE, TRUE);		// Sequential Read
	PRINTF("Seq. Read Done \r\n");


	nLBACount = (int)(nMaxLBA * 0.8);		// 0.3: 30% write

	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();

	while (NVME_SetIOCmd(ADDR_PRINT_PROFILE, 1, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}
	_Sync();
}

/*
	@brief	range of random write
*/
void test_gc(float fWriteRatio)
{
	int nMaxLBA;
	int nLBACount;

#ifdef GREEDY_FTL
	nMaxLBA = SLICES_PER_SSD;
	nLBACount = SLICES_PER_SSD;
#else
	nMaxLBA = HIL_GetStorageBlocks() - 1;
#endif

	int nMaxIOSize = 128;		// lba count

	nLBACount = (int)(nMaxLBA * fWriteRatio);

	int nTestCount = 0;

	while (1)
	{
		PRINTF("GC Test Index : %d \r\n", nTestCount++);

		_Sync();
		PRINTF("Rand. Write Start, #LBA: %d \r\n", nLBACount);
		RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_WRITE, TRUE, TRUE, nMaxIOSize, FALSE, FALSE);		// Random Write
		PRINTF("Rand. Write Done \r\n");

		_Sync();
		PRINTF("Rand. Read Start, #LBA: %d \r\n", nLBACount);
		RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, TRUE, TRUE, nMaxIOSize, FALSE, FALSE);		// Random Read
		PRINTF("Rand. Read Done \r\n");
	}
}

#if defined(STREAM_FTL) || defined(DFTL)
void _PerfSeqWrite(int nWriteMB)
{
	unsigned int nStartTimeUS, nEndTimeUS;
	int nMaxLBA = HIL_GetStorageBlocks() - 1;
	int nLBACount;
	int nIOSize = 128 * KB / HOST_BLOCK_SIZE;			// 128KB
	nLBACount = nWriteMB * MB / HOST_BLOCK_SIZE;

	nStartTimeUS = OSAL_GetCurrentTickUS();

	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_WRITE, FALSE, FALSE, nIOSize, FALSE, FALSE);

	nEndTimeUS = OSAL_GetCurrentTickUS();

	float fThroughput = ((float)nLBACount*HOST_BLOCK_SIZE) / ((nEndTimeUS - nStartTimeUS) / (float)1000000);
	int 	nMB = (int)fThroughput / 1000000;
	int 	n100KBPosition = ((int)fThroughput / 100000) % 10;

	static INT32 nCount = 0;

	PRINTF("[%d] Seq. Write Throughput: %d.%d MB/s\r\n", nCount++, nMB, n100KBPosition);
}

void _PerfSeqRead(int nWriteMB)
{
	unsigned int nStartTimeUS, nEndTimeUS;
	int nMaxLBA = HIL_GetStorageBlocks() - 1;
	int nLBACount;
	int nIOSize = 128 * KB / HOST_BLOCK_SIZE;			// 128KB
	nLBACount = nWriteMB * MB / HOST_BLOCK_SIZE;

	nStartTimeUS = OSAL_GetCurrentTickUS();

	RunTest(nMaxLBA, nLBACount, NVME_CMD_OPCODE_READ, FALSE, FALSE, nIOSize, FALSE, FALSE);

	nEndTimeUS = OSAL_GetCurrentTickUS();

	float fThroughput = ((float)nLBACount*HOST_BLOCK_SIZE) / ((nEndTimeUS - nStartTimeUS) / (float)1000000);
	int 	nMB = (int)fThroughput / 1000000;
	int 	n100KBPosition = ((int)fThroughput / 100000) % 10;

	static INT32 nCount = 0;

	PRINTF("[%d] Seq. Read Throughput: %d.%d MB/s\r\n", nCount++, nMB, n100KBPosition);
}

void _PerfFTLProgram(int nWriteMB)
{
	UINT32	nCmdSlotTag = 0;

	char* pBuf = (char*)OSAL_MemAlloc(MEM_TYPE_BUF, HOST_BLOCK_SIZE, HOST_BLOCK_SIZE);

	unsigned int nStartTimeUS, nEndTimeUS;

	nStartTimeUS = OSAL_GetCurrentTickUS();

	int nLBACount = nWriteMB * MB / HOST_BLOCK_SIZE;

	for (int i = 0; i < nLBACount; i++)
	{
		FTL_WritePage(nCmdSlotTag, i, 1);
	}

	nEndTimeUS = OSAL_GetCurrentTickUS();

	float fThroughput = ((float)nLBACount * HOST_BLOCK_SIZE) / ((nEndTimeUS - nStartTimeUS) / (float)1000000);
	int 	nMB = (int)fThroughput / 1000000;
	int 	n100KBPosition = ((int)fThroughput / 100000) % 10;

	PRINTF("FTL Program Throughput: %d.%d MB/s\r\n", nMB, n100KBPosition);
}

void _PerfFILProgram(void)
{
#ifdef STREAM_FTL
	FTL_REQUEST_ID	stReqID = { 0 };	// don't care
	stReqID.stCommon.nType = FTL_REQUEST_ID_TYPE_PROGRAM;

	char* pMainBuf = (char*)OSAL_MemAlloc(MEM_TYPE_BUF, PHYSICAL_PAGE_SIZE, PHYSICAL_PAGE_SIZE);
	char* pSpareBuf = (char*)OSAL_MemAlloc(MEM_TYPE_BUF, PHYSICAL_PAGE_SIZE, PHYSICAL_PAGE_SIZE);

	INT32	nCh = 0;
	INT32	nWay = 0;
	INT32	nBlock = 100;
	INT32	nPage = 0;
	INT32	nPPage = 0;

	unsigned int nStartTimeUS, nEndTimeUS;
	nStartTimeUS = OSAL_GetCurrentTickUS();

	NAND_ADDR		stNandAddr;
	INT32	nBlockIndex;

	for (nBlockIndex = 0; nBlockIndex < 10; nBlockIndex++)
	{
		stNandAddr.nBlock = nBlock + nBlockIndex;

		for (nPage = 0; nPage < PAGES_PER_BLOCK; nPage++)
		{
			stNandAddr.nPPage = Vpage2PlsbPageTranslation(nPage);

			for (nCh = 0; nCh < USER_CHANNELS; nCh++)
			{
				stNandAddr.nCh = nCh;
				FIL_ProgramPage(stReqID, stNandAddr, pMainBuf, pSpareBuf);
			}
		}
	}

	nEndTimeUS = OSAL_GetCurrentTickUS();

	float fThroughput = ((float)PHYSICAL_PAGE_SIZE * PAGES_PER_BLOCK * USER_CHANNELS * nBlockIndex) / ((nEndTimeUS - nStartTimeUS) / (float)1000000);
	int 	nMB = (int)fThroughput / 1000000;
	int 	n100KBPosition = ((int)fThroughput / 100000) % 10;

	PRINTF("FIL Program Throughput: %d.%d MB/s\r\n", nMB, n100KBPosition);
#endif
}

void _PerfNANDProgram(void)
{
	char* pMainBuf = (char*)OSAL_MemAlloc(MEM_TYPE_BUF, PHYSICAL_PAGE_SIZE, PHYSICAL_PAGE_SIZE);
	char* pSpareBuf = (char*)OSAL_MemAlloc(MEM_TYPE_BUF, PHYSICAL_PAGE_SIZE, PHYSICAL_PAGE_SIZE);

	INT32	nCh = 0;
	INT32	nWay = 0;
	INT32	nBlock = 100;
	INT32	nPage = 0;
	INT32	nPPage = 0;

	NAND_RESULT		anResult[USER_CHANNELS];
	INT32	nDoneCount;
	unsigned int nStartTimeUS, nEndTimeUS;
	nStartTimeUS = OSAL_GetCurrentTickUS();

	INT32	nBlockCount;
	for (nBlockCount = 0; nBlockCount < 10; nBlockCount++)
	{
		for (nPage = 0; nPage < PAGES_PER_BLOCK; nPage++)
		{
			nPPage = Vpage2PlsbPageTranslation(nPage);
			nDoneCount = 0;

			if (nPage == 0)
			{
				while (nDoneCount < USER_CHANNELS)
				{
					anResult[nCh] = NAND_ProcessErase(nCh, nWay, (nBlock + nBlockCount), FALSE);
					if (anResult[nCh] == NAND_RESULT_DONE)
					{
						nDoneCount++;
					}
					nCh = (nCh + 1) % USER_CHANNELS;
				}
			}

			nCh = 0;
			while (nDoneCount < USER_CHANNELS)
			{
				anResult[nCh] = NAND_ProcessProgram(nCh, nWay, (nBlock + nBlockCount), nPPage, pMainBuf, pSpareBuf, FALSE);
				if (anResult[nCh] == NAND_RESULT_DONE)
				{
					nDoneCount++;
				}
				nCh = (nCh + 1) % USER_CHANNELS;
			}
		}
	}

	nEndTimeUS = OSAL_GetCurrentTickUS();

	float fThroughput = ((float)PHYSICAL_PAGE_SIZE * PAGES_PER_BLOCK * USER_CHANNELS * nBlockCount) / ((nEndTimeUS - nStartTimeUS) / (float)1000000);
	int 	nMB = (int)fThroughput / 1000000;
	int 	n100KBPosition = ((int)fThroughput / 100000) % 10;

	PRINTF("NAND Program Throughput: %d.%d MB/s\r\n", nMB, n100KBPosition);
}
#endif		// end of #ifdef STREAM_FTL

void test_main()
{
	NVMeRequest_Initialize();

	// NVME_TASK_WAIT_CC_EN
	NVME_SetCCEN();
	nvme_run();

	//RunTest(1, 1, NVME_CMD_OPCODE_WRITE, FALSE, FALSE, 1, TRUE);
	//RunTest(1, 1, NVME_CMD_OPCODE_READ, FALSE, FALSE, 1, TRUE);

	// EraseAllBlock();

	//ProgramAllPages();
	//ProgramBlock(0, 0, 0);
	//NAND_Erase(0, 0, 0);
	//ReadBlock(0, 0, 0);

	/*
		RunTest(1, 1, NVME_CMD_OPCODE_READ, FALSE, TRUE, 1, TRUE);		// Sequential Read
		nvme_run();
		nvme_run();
		nvme_run();
		nvme_run();
		nvme_run();
	*/

	test_basic();

	//test_gc(0.001);

#if (UNIT_TEST_PERFORMANCE == 1)
	//_CheckTimer(60);

	INT32	nWriteMB = 1024;

	_PerfSeqWrite(nWriteMB);

	while (1)
	{
		_PerfSeqRead(nWriteMB);
	}
#endif

#if (UNIT_TEST_FTL_PERF == 1)
	while (1)
	{
		_PerfFTLProgram(128);
	}

#endif
#if (UNIT_TEST_FIL_PERF == 1)
	while (1)
	{
		_PerfFILProgram();
	}
#endif

#if (UNIT_TEST_NAND_PERF == 1)
	while (1)
	{
		_PerfNANDProgram();
	}
#endif

#ifndef WIN32
	//test_basic();
	//test_rand_rw_verification();

	xil_printf("Test Done \r\n");
#endif
}

VOID _PrintTestProgress(INT32 nProgressPercent)
{
	static int nCount;
	nCount++;
	static int PRINT_INTERVAL = 0xFFF;
	static int nPrevProgressPercent;

	if ((nCount % PRINT_INTERVAL) == 0)
	{
		xil_printf("-");
	}

	if (nPrevProgressPercent != nProgressPercent)
	{
		xil_printf(" %d %% ", nProgressPercent);
		nPrevProgressPercent = nProgressPercent;
	}
}

#if defined(WIN32) && defined(STREAM_FTL)
static VOID _ParseCommandLineParameter_StreamFTL(char* argv[], INT32* pnIndex)
{
	if (strcmp(argv[*pnIndex], "-abc") == 0)		// active block count
	{
		*pnIndex++;
		g_stGlobal.nActiveBlockCount = atoi(argv[*pnIndex]);
	}

	// stream size
	if (strcmp(argv[*pnIndex], "-streamsize") == 0)
	{
		*pnIndex++;
		g_stGlobal.nStreamSize = (INT32)(atoi(argv[*pnIndex]) * KB);
	}

	if (strcmp(argv[*pnIndex], "-partsize") == 0)
	{
		// CLUSTER SIZE -> PARTITION SIZE 濡� �씠由� 蹂�寃�(�끉臾� �몴湲� �뵲由�)
		*pnIndex++;
		g_stGlobal.nPartitionSize = (INT32)(atoi(argv[*pnIndex]) * KB);
	}

	if (strcmp(argv[*pnIndex], "-stc") == 0)		// stream count
	{
		*pnIndex++;
		//g_stGlobal.nStreamCount = atoi(argv[i]);
		ASSERT(0);		// use -str
	}

	if (strcmp(argv[*pnIndex], "-str") == 0)		// stream count
	{
		*pnIndex++;
		g_stGlobal.nStreamRatio = (float)atof(argv[*pnIndex]);
	}

	if (strcmp(argv[*pnIndex], "-bs") == 0)
	{
		*pnIndex++;
		g_stGlobal.nVBlockSize = (INT32)(atoi(argv[*pnIndex]) * KB);
	}

	if (strcmp(argv[*pnIndex], "-ss") == 0)
	{
		*pnIndex++;
		g_stGlobal.nLogicalFlashSize = atoi(argv[*pnIndex]) * GB;
	}

	if (strcmp(argv[*pnIndex], "-op") == 0)
	{
		*pnIndex++;
		g_stGlobal.nOverprovisionSize = atoi(argv[*pnIndex]) * GB;
	}

	// max stream per partition
	if (strcmp(argv[*pnIndex], "-mspp") == 0)
	{
		*pnIndex++;
		g_stGlobal.nMaxStreamPerPartition = atoi(argv[*pnIndex]);
	}

	// use preconditino dump
	if (strcmp(argv[*pnIndex], "-pcd") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stTestGlobal.bUsePreconditionDump = atoi(argv[*pnIndex]);

		ASSERT((g_stTestGlobal.bUsePreconditionDump == 0) || (g_stTestGlobal.bUsePreconditionDump == 1));
		ASSERT(SUPPORT_PERCONDITION_DUMP == 1);
	}

	// use preconditino dump
	if (strcmp(argv[*pnIndex], "-pcd_only") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stTestGlobal.bPreconditionDumpOnly = atoi(argv[*pnIndex]);
		ASSERT(SUPPORT_PERCONDITION_DUMP == 1);
	}

	// use preconditino dump
	if (strcmp(argv[*pnIndex], "-metablock") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stGlobal.bEnableMetaBlock = atoi(argv[*pnIndex]);
	}

	// use preconditino dump
	if (strcmp(argv[*pnIndex], "-smerge_policy") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stGlobal.eSMergePolicy = atoi(argv[*pnIndex]);
		ASSERT(g_stGlobal.eSMergePolicy < SMERGE_POLICY_COUNT);
	}

	// use preconditino dump
	if (strcmp(argv[*pnIndex], "-l2p_cache_ratio") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stGlobal.fL2PCacheRatio = atof(argv[*pnIndex]);

	#if (SUPPORT_L2P_CACHE == 0)
		if (g_stGlobal.fL2PCacheRatio != 0)
		{
			ASSERT(0);		// Invalid parameter
		}
	#endif
	}

	if (strcmp(argv[*pnIndex], "-print_analysis_info") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		if (atoi(argv[*pnIndex]) > 0)
		{
			g_stGlobal.bPrintAnalysisInfo = TRUE;
		}
	}

	if (strcmp(argv[*pnIndex], "-hotcold") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stGlobal.fHotPartitionRatio = (float)atof(argv[*pnIndex]);
		if (g_stGlobal.fHotPartitionRatio > 0)
		{
			g_stGlobal.bEnableHotColdMgmt = TRUE;
		}
	}

	// hot cold active block victim range
	if (strcmp(argv[*pnIndex], "-hc_abvr") == 0)
	{
		// 0: disable
		// 1: enable
		*pnIndex++;
		g_stGlobal.fVictimActiveBlockVictimRange = (float)atof(argv[*pnIndex]);
	}

	return;
}
#endif	// end of #if defined(WIN32) && defined(STREAM_FTL)

static VOID _ParseCommandLineParameter(int argc, char* argv[])
{
#if defined(WIN32) && (defined(STREAM_FTL) || defined(DFTL))
	for (INT32 i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-aging") == 0)
		{
			i++;
			g_stTestGlobal.psAgingFile = argv[i];
		}

		if (strcmp(argv[i], "-init") == 0)
		{
			i++;
			g_stTestGlobal.bPrecondition = atoi(argv[i]);
		}

		if (strcmp(argv[i], "-randr") == 0)		// Range for Rand.Write 
		{
			i++;
			g_stTestGlobal.fRandomRate = atof(argv[i]);
		}

		if (strcmp(argv[i], "-seqr") == 0)		// Range for Seq.Write
		{
			i++;
			g_stTestGlobal.fSeqRate = atof(argv[i]);
		}

		if (strcmp(argv[i], "-randm") == 0)		// UTIL_Random write�쓽 �뼇, density�뿉�꽌�쓽 鍮꾩쑉
		{
			i++;
			g_stTestGlobal.fRandomAmount = atof(argv[i]);
		}

		if (strcmp(argv[i], "-randi") == 0)		// 
		{
			i++;
			g_stTestGlobal.nRandomIncrease = atoi(argv[i]);
		}

		if (strcmp(argv[i], "-rands") == 0)		// UTIL_Random size, # of pages
		{
			i++;
			g_stTestGlobal.nRandomSize = atoi(argv[i]);
		}

		// workload run count
		if (strcmp(argv[i], "-run_count") == 0)
		{
			// 0: disable
			// 1: enable
			i++;
			g_stTestGlobal.nRunCount = (INT32)atoi(argv[i]);
		}

#ifdef STREAM_FTL
		_ParseCommandLineParameter_StreamFTL(argv, &i);
#endif
	}
#endif	// end of #ifdef WIN32
}

static VOID command_setting(int argc, char *argv[])
{
#if defined(WIN32) && (defined(STREAM_FTL) || defined(DFTL))
	int j = 0;

	//test configuration
	g_stTestGlobal.fRandomRate		= 0.3;
	g_stTestGlobal.fSeqRate			= 0.5;
	g_stTestGlobal.fRandomAmount	= 0.7;
	g_stTestGlobal.nRandomIncrease	= 1;
	g_stTestGlobal.nRandomSize		= 1;
	g_stTestGlobal.psAgingFile		= NULL;

	g_stTestGlobal.bPrecondition	= FALSE;

	_ParseCommandLineParameter(argc, argv);

	char cwd[512];
	if (getcwd(cwd, sizeof(cwd)) != 0)
	{
		LOG_PRINTF("[DEBUG] Current working dir: %s\n", cwd);
	}
#endif
}

static INT32 parsing_size(char * str)
{
	if (strstr(str, "K") != NULL)
	{
		return (INT32)KB;
	}
	else if (strstr(str, "M") != NULL)
	{
		return (INT32)MB;
	}
	else if (strstr(str, "G") != NULL)
	{
		return (INT32)GB;
	}
	else
	{
		PRINTF("ERROR in parsing_size\n");
		ASSERT(0);
	}
}

static void _ReadLPN(INT32 nLBA, INT32 nBlockCount)
{
	while (NVME_SetIOCmd(nLBA, nBlockCount, NVME_CMD_OPCODE_READ) == FALSE)
	{
		nvme_run();
	}

	nvme_run();		// run current command
}

/*
	@brief	write blocks, block size is HOST_BLOCK_SIZE(4K)
*/
static void _WriteLPN(INT32 nLBA, INT32 nBlockCount)
{
	g_stTestGlobal.nTraceTotalWrite += nBlockCount;

	while (NVME_SetIOCmd(nLBA, nBlockCount, NVME_CMD_OPCODE_WRITE) == FALSE)
	{
		nvme_run();
	}

	nvme_run();		// run current command
}

static void _PreconditionRandomWrite(void)
{
#ifdef STREAM_FTL
	PRINTF("\n[main] RAND WRITE\n");
	RandomInit(1);		// initialize UTIL_Random seed

	int anWriteSize[4] = { 8, 16, 32, 64 };
	INT32	i = 0;
	INT32	nLPN;
	INT32	nWriteSizeIndex;
	INT32	nLPNCount;		// Total LPN Count to write
	INT32	nLastLPN;		// last LPN for test range

	nLPNCount = (INT32)(g_stGlobal.nLPNCount * g_stTestGlobal.fRandomAmount);
	nLastLPN = (INT32)(g_stGlobal.nLPNCount * g_stTestGlobal.fRandomRate);

	//	random_increase
	if (g_stTestGlobal.nRandomIncrease == 1)
	{
		INT32 nPreviousLPN = 0;

		if (g_stTestGlobal.nRandomSize == 0)
		{
			while (i <= nLPNCount)
			{
				nLPN = IRandom(nPreviousLPN, nLastLPN);
				nWriteSizeIndex = IRandom(0, 3);

				_WriteLPN(nLPN, anWriteSize[nWriteSizeIndex]);

				nPreviousLPN = nLPN;

				if (nPreviousLPN >= nLastLPN)
				{
					nPreviousLPN = 0;
				}

				i = i + anWriteSize[nWriteSizeIndex];
				g_stTestGlobal.nAgingIOCount = g_stTestGlobal.nAgingIOCount + anWriteSize[nWriteSizeIndex];

				_PrintTestProgress(i * 100 / nLPNCount);
			}
		}
		else
		{
			while (i <= nLPNCount)
			{
				nLPN = IRandom(nPreviousLPN, nLastLPN);

				_WriteLPN(nLPN, g_stTestGlobal.nRandomSize);

				nPreviousLPN = nLPN;
				if (nPreviousLPN >= nLastLPN)
				{
					nPreviousLPN = 0;
				}

				i = i + g_stTestGlobal.nRandomSize;
				g_stTestGlobal.nAgingIOCount = g_stTestGlobal.nAgingIOCount + g_stTestGlobal.nRandomSize;

				_PrintTestProgress(i * 100 / nLPNCount);
			}
		}
	}
	else
	{
		if (g_stTestGlobal.nRandomSize == 0)
		{
			while (i <= nLPNCount)
			{
				int nLPN = IRandom(0, nLastLPN);
				int nWriteSizeIndex = IRandom(0, 3);

				_WriteLPN(nLPN, anWriteSize[nWriteSizeIndex]);

				i = i + anWriteSize[nWriteSizeIndex];

				g_stTestGlobal.nAgingIOCount = g_stTestGlobal.nAgingIOCount + anWriteSize[nWriteSizeIndex];

				_PrintTestProgress(i * 100 / nLPNCount);
			}
		}
		else
		{
			while (i <= nLPNCount)
			{
				int nLPN = IRandom(0, nLastLPN);

				_WriteLPN(nLPN, g_stTestGlobal.nRandomSize);
				i = i + g_stTestGlobal.nRandomSize;

				g_stTestGlobal.nAgingIOCount = g_stTestGlobal.nAgingIOCount + g_stTestGlobal.nRandomSize;

				_PrintTestProgress(i * 100 / nLPNCount);
			}
		}
	}
#endif
}

static VOID _DoAging(VOID)
{
#if defined(STREAM_FTL) || defined(DFTL)
	/* Aging */
	if (g_stTestGlobal.bPrecondition)
	{
		if (g_stTestGlobal.psAgingFile == NULL)
		{
			do
			{
	#if defined(STREAM_FTL)
				if (g_stTestGlobal.bUsePreconditionDump == TRUE)
				{
					// try to load precondition
					g_stTestGlobal.bPreconditionDumpLoaded = DUMP_Load();

					if (g_stTestGlobal.bPreconditionDumpLoaded == TRUE)
					{
						LOG_PRINTF("Precondition dump load success \n");
						// bypass precondition
						break;
					}
				}
	#endif	// end of #if defined(STREAM_FTL)

				// seq
				PRINTF("[main] SEQ WRITE\n");
				_SeqFill();

				_CheckMetadataValidity();

				PRINTF("\n");
				_PrintCurrentTime();

				_PreconditionRandomWrite();

	#if defined(STREAM_FTL)
				if ((g_stTestGlobal.bUsePreconditionDump == TRUE) && (g_stTestGlobal.bPreconditionDumpLoaded == FALSE))
				{
					DUMP_Store();	// store new 
				}
	#endif
			} while (0);

			_PrintFTLInfo("Seq/Rand");
		}
		else
		{
			if (strcmp(g_stTestGlobal.psAgingFile, "all") != 0)
			{
				_RunWorkload(g_stTestGlobal.psAgingFile);
				_PrintFTLInfo(g_stTestGlobal.psAgingFile);
			}
			else
			{
				// run all workload
				char	psAgingWorkload[][128] =
				{
					"hm_0.csv", "hm_1.csv",
					"mds_0.csv", "mds_1.csv",
					"prn_0.csv", "prn_1.csv",
					"proj_0.csv", "proj_1.csv", "proj_2.csv", "proj_3.csv", "proj_4.csv",
					"prxy_0.csv", "prxy_1.csv",
					"rsrch_0.csv", "rsrch_1.csv", "rsrch_2.csv",
					"src1_0.csv", "src1_1.csv", "src1_2.csv", "src2_0.csv", "src2_1.csv", "src2_2.csv",
					"stg_0.csv", "stg_1.csv",
					"ts_0.csv",
					"usr_0.csv", "usr_1.csv", "usr_2.csv",
					"wdev_0.csv", "wdev_1.csv", "wdev_2.csv", "wdev_3.csv",
					"web_0.csv", "web_1.csv", "web_2.csv", "web_3.csv",
				};

				INT32	nWorkloadCount = sizeof(psAgingWorkload) / sizeof(psAgingWorkload[0]);

				for (INT32 i = 0; i < nWorkloadCount; i++)
				{
					_RunWorkload(psAgingWorkload[i]);
					g_stTestGlobal.psAgingFile = psAgingWorkload[i];
					print_count("all_workload", g_stTestGlobal.nTraceTotalWrite);
					_PrintFTLInfo(psAgingWorkload[i]);
				}
			}
		}
	}
#endif	// end of #ifdef STREAM_FTL
	return;
}

static VOID _PrintFTLInfo(char* psWorkload)
{
#ifdef STREAM_FTL
	LOG_PRINTF("\nWorkload,Written(GB),LPN_Used,LPN_Total,LPN_Used_Ratio,Block_Free,Block_Total,Block_Free_ratio,Stream_Free,Stream_Total,Stream_Free_ratio\n");
	LOG_PRINTF("%s,", psWorkload);
	LOG_PRINTF("%.1f,", (float)g_stTestGlobal.nTraceTotalWrite * LOGICAL_PAGE_SIZE / GB);
	LOG_PRINTF("%d,%d,%.1f,", g_pstStreamFTL->nVPC, (INT32)g_stGlobal.nLPNCount, (float)g_pstStreamFTL->nVPC / g_stGlobal.nLPNCount);
	LOG_PRINTF("%d,%d,%.1f,", g_pstStreamFTL->stBlockMgr.nFreeBlockCount, g_stGlobal.nVBlockCount, (float)g_pstStreamFTL->stBlockMgr.nFreeBlockCount / g_stGlobal.nVBlockCount);
	LOG_PRINTF("%d,%d,%.1f,", g_pstStreamFTL->stStreamMgr.nFreeStreamCount, g_stGlobal.nStreamCount, (float)g_pstStreamFTL->stStreamMgr.nFreeStreamCount / g_stGlobal.nStreamCount);

	LOG_PRINTF("\n");
#endif
}

/*
	
*/
static INT32 _RunWorkload(char* pstTraceFileName)
{
//#define LPN_DEBUG

#if defined(WIN32) && (defined(STREAM_FTL) || defined(DFTL))
	INT32 nStartLBA		= INVALID_LPN;
	INT32 nLBACount		= INVALID_PPN;

	PRINTF("\r\n Workload: %s \r\n", pstTraceFileName);

	char	psWorkload[256];
	strncpy(psWorkload, WORKLOAD_PATH, sizeof(psWorkload));
	strncat(psWorkload, pstTraceFileName, strlen(pstTraceFileName));

	FILE *fp;
	fp = fopen(psWorkload, "r");
	if (fp == NULL)
	{
		PRINTF("Fail to open workload \n");
		ASSERT(0);
	}

	fseek(fp, 0L, SEEK_END);
	INT32 nFileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	g_stTestGlobal.nTraceTotalWrite = 0;

	INT32	nRestLBACount;

	while (!feof(fp))
	{
		TRACE_TYPE nIOType;
		nIOType = _ParseMSRTrace(fp, &nStartLBA, &nLBACount);

#ifdef LPN_DEBUG		// just for test
		//PRINTF("\n\IOType: %s, LPN: %d, Count: %d\n\r", nIOType == TRACE_WRITE ? "W" : "R", nStartLBA, nLBACount);

		INT32	nCheckLBA = 3314217;
		static INT32	nCheckCount = 0;

		if ( (nCheckLBA >= nStartLBA) && (nCheckLBA < (nStartLBA + nLBACount)))
		{
			if (nCheckCount >= 7628)
			{
				PRINTF(" CHECK");
			}

			PRINTF("\n\rCHECK: %d, IOType: %s\n\r", nCheckCount++, nIOType == TRACE_WRITE ? "W" : "R");
		}
#endif

		nRestLBACount = nLBACount;
		do
		{
			if (nLBACount > MAX_NUM_OF_NLB)
			{
				nLBACount = MAX_NUM_OF_NLB;
			}

			if (nIOType == TRACE_WRITE)	// write
			{
				if ((nStartLBA + nLBACount) < storageCapacity_L)
				{
					//g_stTestGlobal.nTraceTotalWrite = g_stTestGlobal.nTraceTotalWrite + count;
					_WriteLPN(nStartLBA, nLBACount);
				}
			}
			else if (nIOType == TRACE_READ)	// read
			{
				if ((nStartLBA + nLBACount) < storageCapacity_L)
				{
					_ReadLPN(nStartLBA, nLBACount);
				}
			}
			nRestLBACount -= nLBACount;
			nStartLBA += nLBACount;
		} while (nRestLBACount > 0);

#ifdef LPN_DEBUG		// just for test
		INT32	nCount = 32;
		while (nCount > 0)
		{
			nvme_run();		// run current command
			nCount--;
		}
#endif

		_PrintTestProgress((UINT64)ftell(fp) * 100 / nFileSize);
		//_PrintTestProgress(0);
	}

	fclose(fp);
#endif		// end of #ifdef WIN32
	return TRUE;
}


static VOID print_count(char * psWorkload, INT64 trace_total_write)
{
#if defined(WIN32) && defined(STREAM_FTL)
	printf("\nPRINT_COUNT\n");
	FILE *fp;
	char file_name[1024];
	char * extension = ".txt";
	char * underbar = "_result";

	strcpy(file_name,  _GetFileName(psWorkload));
	strtok(file_name, ".");
	strcat(file_name,  underbar);
	strcat(file_name,  extension);

	// trace_partition_cluster_st.txt
	//fopen_s(&fp, file_name, "r");
	fp = fopen(file_name, "r");

	if (fp == NULL)
	{
		fp = fopen(file_name, "a+");
		//fopen_s(&fp, file_name, "a+");
		fprintf(fp, "Date, Aging, ACTIVE_BLOCK_COUNT, g_stGlobal.nStreamSize, g_stGlobal.nPartitionSize, STREAM_COUNT, MAP_SIZE, ");
		fprintf(fp, "NAND_WRITE, ");
		fprintf(fp, "NAND_READ, ");
		fprintf(fp, "NAND_ERASE, ");

		fprintf(fp, "IO_WRITE, ");
		fprintf(fp, "IO_READ, ");
		fprintf(fp, "IO_OVERWRITE, ");
		fprintf(fp, "IO_NULLREAD, ");
		fprintf(fp, "IO_WRITE_REQ, ");
		fprintf(fp, "IO_READ_REQ, ");

		fprintf(fp, "BGC_WRITE, ");
		fprintf(fp, "BGC_READ, ");
		fprintf(fp, "BGC_ERASE, ");
		fprintf(fp, "BGC_CNT, ");

		fprintf(fp, "sMerge_WRITE, ");
		fprintf(fp, "sMerge_READ, ");
		fprintf(fp, "sMerge_ERASE, ");
		fprintf(fp, "sMerge_CNT, ");
		fprintf(fp, "sMerge_VICTIM, ");
		fprintf(fp, "sMerge_MEET_MAX_STREAM, ");

		fprintf(fp, "L2PCache_Hit_USER_RW, ");
		fprintf(fp, "L2PCache_Miss_USER_RW, ");
		fprintf(fp, "L2PCache_Hit_SMERGE, ");
		fprintf(fp, "L2PCache_Miss_SMERGE, ");
		fprintf(fp, "L2PCache_Hit_BLOCKGC, ");
		fprintf(fp, "L2PCache_Miss_BLOCKGC, ");

		fprintf(fp, "BGC_WITH_SMERGE, ");
		fprintf(fp, "SMERGE_BY_BGC, ");

		fprintf(fp, "Meta_Read, ");
		fprintf(fp, "Meta_Write, ");

		fprintf(fp, "#FullInvalidBlock, ");
		fprintf(fp, "#FullInvalidStream, ");

		fprintf(fp, "\n");
	}
	else
	{
		fclose(fp);
		fp = fopen(file_name, "a+");
		//fopen_s(&fp, file_name, "a+");
	}

	INT32	map_size = 0;

	// Map size with bit
	if (g_stGlobal.nStreamSize < 1 * MB)
	{
		INT32	startLPN = 23;
		INT32	nStartVPPN = 23;
		INT32	endPBN = 16;
		INT32	bitmap = g_stGlobal.nLPagePerStream;
		double	valid_page_d = log(g_stGlobal.nLPagePerStream) / log(2);
		INT32	valid_page = (INT32)ceil(valid_page_d);
		double	seq_d = log(g_stGlobal.nStreamCount) / log(2);
		INT32	seq = (INT32)ceil(seq_d);		// pointer
		INT32	num_partition = 8;

		map_size = (startLPN + nStartVPPN + endPBN + bitmap + valid_page + seq + seq)*g_stGlobal.nStreamCount + (seq + num_partition)*g_stGlobal.nPartitionCount;
		if (map_size % 8 != 0)
		{
			map_size = map_size / 8 + 1;
		}
		else
		{
			map_size = map_size / 8;
		}
	}

	struct tm *t;
	time_t timer;
	timer = time(NULL);
	t = localtime(&timer);

	fprintf(fp, "%02d%02d%02d_%02d%02d%02d, ", (t->tm_year + 1900), (t->tm_mon + 1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	if (g_stTestGlobal.bPrecondition == TRUE)
	{
		if (g_stTestGlobal.psAgingFile != NULL)
		{
			fprintf(fp, "%s, ", g_stTestGlobal.psAgingFile);
		}
		else
		{
			fprintf(fp, "s(%f)r(%f m:%f i:%d s:%d), ", g_stTestGlobal.fSeqRate, g_stTestGlobal.fRandomRate, g_stTestGlobal.fRandomAmount, g_stTestGlobal.nRandomIncrease, g_stTestGlobal.nRandomSize);
		}
	}
	else
	{
		fprintf(fp, "%s, ", "None");
	}

	fprintf(fp, "%d, ", g_stGlobal.nActiveBlockCount);
	fprintf(fp, "%d, ", g_stGlobal.nStreamSize);
	fprintf(fp, "%d, ", g_stGlobal.nPartitionSize);
	fprintf(fp, "%d, ", g_stGlobal.nStreamCount);
	fprintf(fp, "%d, ", map_size);
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_NAND_WRITE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_NAND_READ));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_NAND_ERASE));

	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_WRITE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_READ));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_OVERWRITE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_UNMAP_READ));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_WRITE_REQ));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_HOST_READ_REQ));

	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_BGC_WRITE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_BGC_READ));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_BGC_ERASE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_BGC));

	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE_write));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE_read));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE_erase));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE_victim));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERGE_meet_max_stream));

	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCACHE_HIT_HOST));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCACHE_MISS_HOST));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCache_Hit_SMERGE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCache_Miss_SMERGE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCache_Hit_BLOCKGC));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_L2PCache_Miss_BLOCKGC));

	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_BGC_WITH_SMERGE));
	fprintf(fp, "%d, ", STAT_GetCount(PROFILE_SMERAGE_WHILE_BGC));

	fprintf(fp, "%d,", STAT_GetCount(PROFILE_META_READ));			// meta page read
	fprintf(fp, "%d,", STAT_GetCount(PROFILE_META_WRITE));		// meta page write

	fprintf(fp, "%d,", STAT_GetCount(PROFILE_FULL_INVALID_BLOCK));
	fprintf(fp, "%d,", STAT_GetCount(PROFILE_FULL_INVALID_STREAM));

	fprintf(fp, "\n");

	fclose(fp);
#endif
}


//////////////////////////////////////////////////////////////////
//
//	Debugging
//
//////////////////////////////////////////////////////////////////
static VOID _CheckMetadataValidity(VOID)
{
#if (ERROR_CHECKING_ON == 1) && defined(STREAM_FTL)
	_PrintCurrentTime();

	// debug
	CheckStreamValidity();
	CheckBlockValidity();
	check_total_MAP();
	CheckHotColdMgmt();

	LOG_PRINTF("\n[DEBUG] Metadata validity check PASS!!\n");

	_PrintCurrentTime();
#endif
}

static VOID _PrintAnalysisInfo(char* psTraceFileName)
{
#ifdef STREAM_FTL
	if (g_stGlobal.bPrintAnalysisInfo == FALSE)
	{
		return;
	}

#if !defined(COSMOS_PLUS)
	FILE* pFP = _OpenAnalysisLogFile(psTraceFileName);

	INT32	nMaxIndex = 0;

	nMaxIndex = MAX(nMaxIndex, g_stGlobal.nPartitionCount);
	nMaxIndex = MAX(nMaxIndex, g_stGlobal.nStreamCount);
	nMaxIndex = MAX(nMaxIndex, g_stGlobal.nVBlockCount);

	LOG_PRINTFILE_ANALYSIS("Index,");
	LOG_PRINTFILE_ANALYSIS("Partition.StreamNo,");
	LOG_PRINTFILE_ANALYSIS("Partition.VPC,");
	LOG_PRINTFILE_ANALYSIS("Part.WriteCnt,");
	LOG_PRINTFILE_ANALYSIS("Part.OverwriteRatio,");		// over write ratio
	LOG_PRINTFILE_ANALYSIS("Stream.VPC,");
	LOG_PRINTFILE_ANALYSIS("Block.VPC,");
	LOG_PRINTFILE_ANALYSIS("Block.StreamCount,");
	LOG_PRINTFILE_ANALYSIS("Block.EC,");
	LOG_PRINTFILE_ANALYSIS("BLK.bFree,");
	LOG_PRINTFILE_ANALYSIS("BLK.bUser,");
	LOG_PRINTFILE_ANALYSIS("BLK.bGC,");
	LOG_PRINTFILE_ANALYSIS("BLK.bSMerge,");
	LOG_PRINTFILE_ANALYSIS("BLK.bMeta,");	
	LOG_PRINTFILE_ANALYSIS("\n");

	for (INT32 i = 0; i < nMaxIndex; i++)
	{
		LOG_PRINTFILE_ANALYSIS("%d,", i);

		if (i < g_stGlobal.nPartitionCount)
		{
			LOG_PRINTFILE_ANALYSIS("%d,%d,", GET_PARTITION(i)->nNumStream, GET_PARTITION(i)->nVPC);
			// print partition write count
			LOG_PRINTFILE_ANALYSIS("%d,", g_stGlobal.paPartWrite[i]);
			LOG_PRINTFILE_ANALYSIS("%.2lf,", StreamFTL_HotCold_GetOverWriteRatio(i));	// HotRatio
		}
		else
		{
			LOG_PRINTFILE_ANALYSIS(",,");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty, Part. Write Count
			LOG_PRINTFILE_ANALYSIS(",");	// empty, HotRatio
		}

		if (i < g_stGlobal.nStreamCount)
		{
			LOG_PRINTFILE_ANALYSIS("%d,", g_pstStreamFTL->m_pstStreams[i].nVPC);
		}
		else
		{
			LOG_PRINTFILE_ANALYSIS(",");	// empty
		}

		if (i < g_stGlobal.nVBlockCount)
		{
			LOG_PRINTFILE_ANALYSIS("%d,%d,%d,", (PAGE_PER_BLOCK - GET_BLOCK_INFO(i)->nInvalidPages), GET_BLOCK_INFO(i)->nStreamCount, g_pstPB[i].nEC);
			LOG_PRINTFILE_ANALYSIS("%d,", GET_BLOCK_INFO(i)->bFree);
			LOG_PRINTFILE_ANALYSIS("%d,", GET_BLOCK_INFO(i)->bUser);
			LOG_PRINTFILE_ANALYSIS("%d,", GET_BLOCK_INFO(i)->bGC);
			LOG_PRINTFILE_ANALYSIS("%d,", GET_BLOCK_INFO(i)->bSMerge);			
			LOG_PRINTFILE_ANALYSIS("%d,", GET_BLOCK_INFO(i)->bMetaBlock);
		}
		else
		{
			LOG_PRINTFILE_ANALYSIS(",,,");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty
			LOG_PRINTFILE_ANALYSIS(",");	// empty
		}

		LOG_PRINTFILE_ANALYSIS("\n");
	}

	fclose(pFP);
#endif
#endif	// end if #ifdef STREAM_FTL
}

#ifdef WIN32
static time_t _PrintCurrentTime(VOID)
{
#ifndef  GREEDY_FTL
	struct tm *pstCurTime;
	time_t nCurTime = time(NULL);
	pstCurTime = localtime(&nCurTime);

	LOG_PRINTF("\n%d-%02d-%02d ", (pstCurTime->tm_year + 1900), (pstCurTime->tm_mon + 1), pstCurTime->tm_mday);
	LOG_PRINTF("%02d:%02d:%02d\n", pstCurTime->tm_hour, pstCurTime->tm_min, pstCurTime->tm_sec);

	return nCurTime;
#else
	return 0;
#endif
}

static INT32 _GetTraceLineCount(char* psWorkloadPath)
{
	FILE* pFP;
	pFP = fopen(psWorkloadPath, "r");
	if (pFP == NULL)
	{
		PRINTF("Fail to open workload \n");
		ASSERT(0);
	}

	INT32 nLineCount = 0;
	char psStr[1024];

	while (!feof(pFP))
	{
		fscanf(pFP, "%s", psStr);
		nLineCount++;
	}

	fclose(pFP);

	return nLineCount;
}

static TRACE_TYPE _ParseMSRTrace(FILE *fp, int *pnStartLPN, int *pnLPNCount)
{
	char psStr[1024];

	UINT64	nTimeStamp;
	char	*psHostName;
	INT32	nDiskNo;
	char	*psType;
	UINT64	nSectorNo;
	INT32	nSectorCount;
	INT32	nResponseTime;

	fscanf(fp, "%s", psStr);

	if (psStr == NULL)
	{
		return TRACE_EOF;
	}

	nTimeStamp = atoll(strtok(psStr, ","));

	psHostName = strtok(NULL, ",");
	if (psHostName == NULL)
	{
		return TRACE_EOF;
	}

	char *psTemp = strtok(NULL, ",");
	if (!psTemp)
	{
		return TRACE_EOF;
	}
	nDiskNo = atoi(psTemp);

	psType = strtok(NULL, ",");
	if (psType == NULL)
	{
		return TRACE_EOF;
	}

	nSectorNo		= atoll(strtok(NULL, ","));
	nSectorCount	= atoi(strtok(NULL, ","));
	nResponseTime	= atoi(strtok(NULL, ","));

	*pnStartLPN = (int)(nSectorNo / LOGICAL_PAGE_SIZE);

	INT32 nEndLPN = (INT32)((nSectorNo + nSectorCount - 1) / LOGICAL_PAGE_SIZE);
	*pnLPNCount = nEndLPN - *pnStartLPN + 1;

	if (strstr(psType, "W"))
	{
		return TRACE_WRITE;
	}
	else if (strstr(psType, "R"))
	{
		return TRACE_READ;
	}
	else
	{
		return TRACE_EOF;
	}
}
#endif	// end of #ifdef WIN32

//////////////////////////////////////////////////////////////////
//
//	Logging
//
//////////////////////////////////////////////////////////////////
#ifdef WIN32
static char* _GetFileName(char* psPath)
{
	char*	psFileName;

	(psFileName = strrchr(psPath, '\\')) ? ++psFileName : (psFileName = psPath);

	return psFileName;
}

static VOID _OpenTestLogFile(char* pstTraceFileName)
{
#ifdef STREAM_FTL
	char	strTestLogOutput[1024];
	char	*psExt = ".txt";
	char	*psMidName = "_testlog";

	strcpy(strTestLogOutput, _GetFileName(pstTraceFileName));
	strtok(strTestLogOutput, ".");
	strcat(strTestLogOutput, psMidName);
	strcat(strTestLogOutput, psExt);

	g_fpTestLog = fopen(strTestLogOutput, "a+");
	ASSERT(g_fpTestLog != NULL);

	LOG_PRINTF("\n\n\n");
	LOG_PRINTF("////////////////////////////////////////////////////////////////////////////////////////////\n");
#endif
	return;
}

static VOID _CloseTestLogFile(VOID)
{
#ifdef STREAM_FTL
	fclose(g_fpTestLog);
#endif
}

static FILE* _OpenAnalysisLogFile(char* pstTraceFileName)
{
	char	strTestLogOutput[1024];
	char	*psExt = ".csv";
	char	*psMidName = "_analysis";

	strcpy(strTestLogOutput, _GetFileName(pstTraceFileName));
	strtok(strTestLogOutput, ".");
	strcat(strTestLogOutput, psMidName);
	strcat(strTestLogOutput, psExt);

	FILE* g_fpAnalysistLog = fopen(strTestLogOutput, "w");
	ASSERT(g_fpAnalysistLog != NULL);

	return g_fpAnalysistLog;
}
#endif


///////////////////////////////////////////////////
//
//	Precondition
//
///////////////////////////////////////////////////
static void _SeqFill(void)
{
#ifdef STREAM_FTL
	INT32	nLPNCount;

	nLPNCount = (INT32)(g_stGlobal.nLPNCount * g_stTestGlobal.fSeqRate);

	for (int i = 0; i < nLPNCount; i++)
	{
		_WriteLPN(i, 1);
		g_stTestGlobal.nAgingIOCount++;

		_PrintTestProgress(i * 100 / nLPNCount);
	}
#endif
}

/*
@brief	print current time in second resulution
*/
static void _CheckTimer(int nDurationSec)
{
	// check timer
	unsigned long long nCurTimeUS = OSAL_GetCurrentTickUS();
	unsigned long long nCurTimeSec;
	unsigned long long nPrevTimeSec = 0;
	do
	{
		nCurTimeUS = OSAL_GetCurrentTickUS();
		nCurTimeSec = nCurTimeUS / 1000000;

		if (nCurTimeSec != nPrevTimeSec)
		{
			PRINTF("TIME: %d sec \r\n", (int)nCurTimeSec);
			nPrevTimeSec = nCurTimeSec;
			nDurationSec--;
			if (nDurationSec <= 0)
			{
				// stop
				return;
			}
		}
	} while (1);
}

