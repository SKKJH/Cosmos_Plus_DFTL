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

#ifndef __STREAMFTL_UTIL_H__
#define __STREAMFTL_UTIL_H__

#include "streamftl_types.h"
#include "dump.h"

void*	UTIL_MemAlloc(INT32 nSize, DUMP_TYPE eDumpType);

#endif	// end of __STREAMFTL_UTIL_H__