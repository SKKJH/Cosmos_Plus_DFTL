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

/*
	TODO
		1. (Done)16K page에 맞게 4K 4개 모으기
			_Write()에서 buffering 하게 해야함. stream별로 각각 buffering 해야한다. 즉 active block만큼 buffering 되어야 함..
			read interleaving을 고려하여 흩어 뿌리면 안됨. 모아쓰기 해야함.
		2. (Done) buffer cache 만들어서 buffering 중인것에 있는지 확인하기
		3. (Done) block GC 개발 필요
		4. (Done)sMerge 개발 필요
		5. SUPPORT_LIMIT_STREAM_PER_PARTITION enable 하기
		6. PhysicalFlashSize, LogicalFlashSize 등을 KB 단위로 변환하기. 
		7. Bad block manager

	Optimization
		1. Block GC Read시 16K page 한번에 읽은 후 LPage 별로 write 하기.

	refactoring
		1. (Done) remove Active Block Manager::bSeparatedGCBlock, always TRUE
		2. GC victim selection optimization, why not use array list according to the VPC
*/

#ifndef __FTL_CONFIG_H__
#define __FTL_CONFIG_H__

#include "cosmos_plus_global_config.h"

#include "nsc_driver.h"
#include "xparameters.h"
#include "nvme.h"

#include "cosmos_types.h"

#define DEFAULT_STREAM_SIZE			(128 * KB)
#define DEFAULT_PARTITION_SIZE		(128 * KB)
#define DEFAULT_STREAM_RATIO		(1.5)

#define BLOCK_PER_STREAM			(2)

#ifdef _DEBUG
#define ERROR_CHECKING_ON		(1)
#else
#define ERROR_CHECKING_ON		(0)
#endif

#define STREAM_FTL_VERSION					0x18101500		// 20{18/08/20/00}
															// this must be increased if you change memory layout

// StreamFTL OPTION
#define SUPPORT_LIMIT_STREAM_PER_PARTITION	(1)		// Partition당 stream의 개수 제한
#define MAX_STREAM_PER_PARTITION_DEFAULT	(16)	// PARTITION당 최대 STREAM의 개수.

#define SUPPORT_PERCONDITION_DUMP			(1)		// Dump precondition result to binary and re-load it.

#define SUPPORT_L2P_CACHE					(1)

#define STREAM_MERGE_SKIP_STREAM_COUNT		(0)		// STREAM merge 중 skip 할 stream의 개수.
// Partition이 선택된 후 설정된 n개의 stream은 merge 대상에서 제외한다.
// 이유는 latest stream은 invalidation으로 merge cost가 줄어 들 수 있기 때문이다.
//	0: no skip
//	n: n개 skip, 하지만 최소 2개의 stream은 merge 수행한다. (stream 확보를 위해)
// 주의:  STREAM merge는 partition에 할당된 stream 중에서 선택적으로 할 수는 없음.
//	모든 stream을 write 해야함.  latest written LPN이 latest stream에 있기 때문에 
//	old merge하여 latest stream이 되면 old가 new가 되어 mis-compare 발생함.

#define STREAM_FTL_SMERGE_VICTIM_POLICY_DEFAULT		SMERGE_POLICY_GREEDY

#define SUPPORT_HOTCOLD						(1)

#define STREAM_FTL_HOTCOLD_VICTIM_ACTIVE_BLOCK_LOOKUP_RANGE_DEFAULT		(0.1)
// user block LRU list에서 victim active block 선택 영역.

#define STREAM_FTL_HOT_PARTITION_RATIO_DEFAULT		(0.1)
// Hot partition으로 선택되는 조건. 0.1 = 10%의 valid lpn이 overwrite된 경우 (GC와 GC 사이)

#define STREAM_FTL_MAX_HOT_RATIO_INDEX				(16)		// Max index of overwrite ratio
// 0: 0.1 , 1:0.2, 2:0.4 ...  increased by power of two * 0.1

#define STREAM_GC_THRESHOLD_DEFAULT			20
#define NUMBER_ACTIVE_BLOCK_DEFAULT			256				// maximum count of active block

#define DEFAULT_LPN_PER_STREAM				(DEFAULT_STREAM_SIZE / LOGICAL_PAGE_SIZE)

#define ACTIVE_BLOCK_BUFFERING_COUNT		(USER_CHANNELS * USER_WAYS * 3)				// 2 is enough, must be a small value for nCurProgramBuffering

#endif // #end of __FTL_CONFIG_H__
