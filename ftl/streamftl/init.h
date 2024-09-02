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

#ifndef _INIT_H
#define _INIT_H

#include <stdio.h>
#include "list.h"

VOID StreamFTL_Initialize();

VOID InitStream(STREAM* pstStream);

VOID InitializeL2PCacheEntry(INT32 nIndex);

#endif