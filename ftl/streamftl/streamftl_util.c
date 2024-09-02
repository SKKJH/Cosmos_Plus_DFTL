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

#include "debug.h"

#include "StreamFTL.h"
#include "error.h"
#include "streamftl_map.h"
#include "streamftl_types.h"
#include "dump.h"
#include "streamftl_vnand.h"
#include "util.h"
#include "osal.h"
#include "list.h"

#include "streamftl_util.h"

void* UTIL_MemAlloc(INT32 nSize, DUMP_TYPE eDumpType)
{
	void *pPtr;

	pPtr = OSAL_MemAlloc(MEM_TYPE_FW_DATA, nSize, OSAL_MEMALLOC_FW_ALIGNMENT);
	ASSERT(pPtr);

	if (eDumpType != DUMP_TYPE_NO_DUMP)
	{
		DUMP_Add(eDumpType, nSize, pPtr);
	}

	return pPtr;
}

///////////////////////////////////////////////////////////////////////////////
//
//	static function
//
///////////////////////////////////////////////////////////////////////////////

