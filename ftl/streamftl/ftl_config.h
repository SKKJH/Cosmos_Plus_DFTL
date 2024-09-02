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
		1. (Done)16K page�� �°� 4K 4�� ������
			_Write()���� buffering �ϰ� �ؾ���. stream���� ���� buffering �ؾ��Ѵ�. �� active block��ŭ buffering �Ǿ�� ��..
			read interleaving�� ����Ͽ� ��� �Ѹ��� �ȵ�. ��ƾ��� �ؾ���.
		2. (Done) buffer cache ���� buffering ���ΰͿ� �ִ��� Ȯ���ϱ�
		3. (Done) block GC ���� �ʿ�
		4. (Done)sMerge ���� �ʿ�
		5. SUPPORT_LIMIT_STREAM_PER_PARTITION enable �ϱ�
		6. PhysicalFlashSize, LogicalFlashSize ���� KB ������ ��ȯ�ϱ�. 
		7. Bad block manager

	Optimization
		1. Block GC Read�� 16K page �ѹ��� ���� �� LPage ���� write �ϱ�.

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
#define SUPPORT_LIMIT_STREAM_PER_PARTITION	(1)		// Partition�� stream�� ���� ����
#define MAX_STREAM_PER_PARTITION_DEFAULT	(16)	// PARTITION�� �ִ� STREAM�� ����.

#define SUPPORT_PERCONDITION_DUMP			(1)		// Dump precondition result to binary and re-load it.

#define SUPPORT_L2P_CACHE					(1)

#define STREAM_MERGE_SKIP_STREAM_COUNT		(0)		// STREAM merge �� skip �� stream�� ����.
// Partition�� ���õ� �� ������ n���� stream�� merge ��󿡼� �����Ѵ�.
// ������ latest stream�� invalidation���� merge cost�� �پ� �� �� �ֱ� �����̴�.
//	0: no skip
//	n: n�� skip, ������ �ּ� 2���� stream�� merge �����Ѵ�. (stream Ȯ���� ����)
// ����:  STREAM merge�� partition�� �Ҵ�� stream �߿��� ���������� �� ���� ����.
//	��� stream�� write �ؾ���.  latest written LPN�� latest stream�� �ֱ� ������ 
//	old merge�Ͽ� latest stream�� �Ǹ� old�� new�� �Ǿ� mis-compare �߻���.

#define STREAM_FTL_SMERGE_VICTIM_POLICY_DEFAULT		SMERGE_POLICY_GREEDY

#define SUPPORT_HOTCOLD						(1)

#define STREAM_FTL_HOTCOLD_VICTIM_ACTIVE_BLOCK_LOOKUP_RANGE_DEFAULT		(0.1)
// user block LRU list���� victim active block ���� ����.

#define STREAM_FTL_HOT_PARTITION_RATIO_DEFAULT		(0.1)
// Hot partition���� ���õǴ� ����. 0.1 = 10%�� valid lpn�� overwrite�� ��� (GC�� GC ����)

#define STREAM_FTL_MAX_HOT_RATIO_INDEX				(16)		// Max index of overwrite ratio
// 0: 0.1 , 1:0.2, 2:0.4 ...  increased by power of two * 0.1

#define STREAM_GC_THRESHOLD_DEFAULT			20
#define NUMBER_ACTIVE_BLOCK_DEFAULT			256				// maximum count of active block

#define DEFAULT_LPN_PER_STREAM				(DEFAULT_STREAM_SIZE / LOGICAL_PAGE_SIZE)

#define ACTIVE_BLOCK_BUFFERING_COUNT		(USER_CHANNELS * USER_WAYS * 3)				// 2 is enough, must be a small value for nCurProgramBuffering

#endif // #end of __FTL_CONFIG_H__
