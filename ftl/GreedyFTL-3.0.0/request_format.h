//////////////////////////////////////////////////////////////////////////////////
// request_format.h for Cosmos+ OpenSSD
// Copyright (c) 2017 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: Request Allocator
// File Name: request_format.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure of request
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#ifndef REQUEST_FORMAT_H_
#define REQUEST_FORMAT_H_

#include "nvme.h"

#define REQ_TYPE_SLICE			0x0				// dy, LPN 정보.
#define REQ_TYPE_NAND			0x1				// dy, NAND IO information
#define REQ_TYPE_NVME_DMA		0x2				// dy, DMA information

#define REQ_QUEUE_TYPE_NONE							0x0
#define REQ_QUEUE_TYPE_FREE							0x1
#define REQ_QUEUE_TYPE_SLICE						0x2
#define REQ_QUEUE_TYPE_BLOCKED_BY_BUF_DEP			0x3
#define REQ_QUEUE_TYPE_BLOCKED_BY_ROW_ADDR_DEP		0x4
#define REQ_QUEUE_TYPE_NVME_DMA						0x5
#define REQ_QUEUE_TYPE_NAND							0x6

#define REQ_CODE_WRITE				0x00
#define REQ_CODE_READ				0x08
#define REQ_CODE_READ_TRANSFER		0x09
#define REQ_CODE_ERASE				0x0C
#define REQ_CODE_RESET				0x0D
#define REQ_CODE_SET_FEATURE		0x0E
#define REQ_CODE_FLUSH				0x0F
#define REQ_CODE_RxDMA				0x10				// receive from host (for write)
#define REQ_CODE_TxDMA				0x20

#define REQ_CODE_OCSSD_PHY_TYPE_BASE	0xA0
#define REQ_CODE_OCSSD_PHY_WRITE		0xA0
#define REQ_CODE_OCSSD_PHY_READ			0xA8
#define REQ_CODE_OCSSD_PHY_ERASE		0xAC


#define REQ_OPT_DATA_BUF_ENTRY		0			// dy, buffer data
#define REQ_OPT_DATA_BUF_TEMP_ENTRY	1
#define REQ_OPT_DATA_BUF_ADDR		2
#define REQ_OPT_DATA_BUF_NONE		3

#define REQ_OPT_NAND_ADDR_VSA		0			// dy, ch/way/die
#define REQ_OPT_NAND_ADDR_PHY_ORG	1			// dy, ch/way

#define REQ_OPT_NAND_ECC_OFF		0
#define REQ_OPT_NAND_ECC_ON			1

#define REQ_OPT_NAND_ECC_WARNING_OFF		0
#define REQ_OPT_NAND_ECC_WARNING_ON			1

#define REQ_OPT_WRAPPING_NONE		0
#define REQ_OPT_WRAPPING_REQ		1

#define REQ_OPT_ROW_ADDR_DEPENDENCY_NONE	0
#define REQ_OPT_ROW_ADDR_DEPENDENCY_CHECK 	1

#define REQ_OPT_BLOCK_SPACE_MAIN	0
#define REQ_OPT_BLOCK_SPACE_TOTAL 	1

#define LOGICAL_SLICE_ADDR_NONE 	0xffffffff

typedef struct _DATA_BUF_INFO{
	union {
		unsigned int addr;
		unsigned int entry;						// dy, buffer entry index, dataBufMapPtr->dataBuf[enntry]
	};
} DATA_BUF_INFO, *P_DATA_BUF_INFO;


typedef struct _NVME_DMA_INFO{
	unsigned int startIndex : 16;				// dy, dma start index
	unsigned int nvmeBlockOffset : 16;			// dy, 4KB sector offset within 16KB NAND Page 
	unsigned int numOfNvmeBlock : 16;			// dy, count of 4K blocks
	unsigned int reqTail	: 8;				// dy, DMA count = block count
	unsigned int reserved0 : 8;
	unsigned int overFlowCnt;
} NVME_DMA_INFO, *P_NVME_DMA_INFO;


typedef struct _NAND_INFO{
	union {
		unsigned int virtualSliceAddr;
		struct {
			unsigned int physicalCh : 4;
			unsigned int physicalWay : 4;
			unsigned int physicalBlock : 16;
			unsigned int phyReserved0 : 8;
		};
	};
	union {
		unsigned int programmedPageCnt;			// dy, count of programmed page
		struct {
			unsigned int physicalPage : 16;
			unsigned int phyReserved1 : 16;
		};
	};
} NAND_INFO, *P_NAND_INFO;


typedef struct _REQ_OPTION{
	unsigned int dataBufFormat : 2;				// dy, REQ_OPT_DATA_BUF_ENTRY
	unsigned int nandAddr : 2;					// dy, type of NAND address REQ_OPT_NAND_ADDR_VSA, REQ_OPT_NAND_ADDR_PHY_ORG
	unsigned int nandEcc : 1;					// dy, ECC on/Off, 1:On = REQ_OPT_NAND_ECC_ON
	unsigned int nandEccWarning : 1;			// dy, error report 여부 결정, REQ_OPT_NAND_ECC_WARNING_ON
	unsigned int rowAddrDependencyCheck : 1;	// dy, REQ_OPT_ROW_ADDR_DEPENDENCY_CHECK
	unsigned int blockSpace : 1;				// dy, REQ_OPT_BLOCK_SPACE_MAIN or REQ_OPT_BLOCK_SPACE_TOTAL
	unsigned int reserved0 : 24;
} REQ_OPTION, *P_REQ_OPTION;


typedef struct _SSD_REQ_FORMAT
{
	unsigned int reqType : 4;				// dy, REQ_TYPE_SLICE, REQ_TYPE_NAND, REQ_TYPE_NVME_DMA
	unsigned int reqQueueType : 4;			// dy, request queue의 status를 나타냄, REQ_QUEUE_TYPE_BLOCKED_BY_BUF_DEP
	unsigned int reqCode : 8;				// dy, read/write, REQ_CODE_WRITE/READ
	unsigned int nvmeCmdSlotTag : 16;		// dy, NVMe Command Slot

	unsigned int logicalSliceAddr;			// dy, = LPN, 16K mapping index

	REQ_OPTION reqOpt;						// dy, Option, ECC여부, error report 여부 등. refer to REQ_OPTION 
	DATA_BUF_INFO dataBufInfo;				// dy, data buffer에 존재하는 경우 buffer의 정보를 저장함.
	NVME_DMA_INFO nvmeDmaInfo;				//
	NAND_INFO nandInfo;						// dy, NAND Address

	unsigned int prevReq : 16;				// dy, linked list for (blockedByRowAddrDepReqQ, freeReqQ, sliceQueue, nandReqQ, etc..)
	unsigned int nextReq : 16;				// dy, list의 마지막인 경우에는 REQ_SLOT_TAG_NONE가 설정됨.
	unsigned int prevBlockingReq : 16;		// dy, buffer가 valid 하지 않으면 index를 가진다., valid 하면 REQ_SLOT_TAG_NONE,
	unsigned int nextBlockingReq : 16;

} SSD_REQ_FORMAT, *P_SSD_REQ_FORMAT;

#endif /* REQUEST_FORMAT_H_ */
