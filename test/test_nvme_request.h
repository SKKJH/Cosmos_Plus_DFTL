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

#ifndef __TEST_NVME_REQUEST_H__
#define __TEST_NVME_REQUEST_H__

#include "host_lld.h"
#include "nvme.h"

#include "list.h"

#define NVME_REQUEST_COUNT		128
#define NVME_SQID_ADMIN			(0)
#define NVME_SQID_USER			(1)

#define NVME_CMD_VALID			(1)
#define NVME_CMD_INVALID		(0)

#define COUNT_UINT8				(1 << (sizeof(unsigned char) * 8))
#define MAX_UINT8				(COUNT_UINT8 - 1)

typedef struct
{
	NVME_CMD_FIFO_REG		stFIFOReg;
	NVME_IO_COMMAND			stCmd;
	IO_READ_COMMAND_DW12	stCmdDW12;		// same structure for DW12 both read and write cmd

	unsigned int			nDMACount: 16;		// current on-going DMA Count
	unsigned int			nRestDMACount: 16;	// count of DMA for this request

	struct list_head		dlList;
} NVMeRequest;


typedef struct
{
	NVMeRequest			m_astRequest[NVME_REQUEST_COUNT];
	int					m_anHDMAToRequest_Tx[COUNT_UINT8];
	int					m_anHDMAToRequest_Rx[COUNT_UINT8];

	struct list_head	dlFree;							// free NVMeRequest
	int					m_nFree;			// count of free request
} NVMeRequestPool;

extern NVMeRequestPool		*g_pstNVMeRequestPool;

void NVMeRequest_Initialize(void);
NVMeRequest* NVMeRequest_Allocate(void);
void NVMeRequest_Release(NVMeRequest* pstRequest, int nDMAIndex);
void NVMeRequest_TxDMAIssue(int nDMAIndex, int nCmdSlotTag);
void NVMeRequest_RxDMAIssue(int nDMAIndex, int nCmdSlotTag);
void NVMeRequest_TxDMADone(int nDMAIndex);
void NVMeRequest_RxDMADone(int nDMAIndex);

#endif	// end of #ifndef __TEST_NVME_REQUEST_H__