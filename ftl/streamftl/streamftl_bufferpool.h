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

#ifndef __STREAMFTL_BUFFERPOOL_H__
#define __STREAMFTL_BUFFERPOOL_H__

#include "streamftl_types.h"

VOID			BufferPool_Initialize(VOID);
BUFFER_ENTRY*	BufferPool_Allocate(VOID);
VOID			BufferPool_Release(BUFFER_ENTRY* pstEntry);
BOOL			BufferPool_IsEmpty(VOID);
INT32			BufferPool_GetFreeCount(VOID);

#endif	// end of __STREAMFTL_BUFFERPOOL_H__