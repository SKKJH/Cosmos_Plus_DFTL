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

#ifndef __ERROR_H__
#define __ERROR_H__

#include "streamftl_types.h"

VOID check_LPN_MAP(INT32 nLPN, INT32 nVPPN);
VOID check_OOB_MAP(INT32 nLPN, INT32 nVPPN);

VOID check_total_MAP(VOID);

VOID _CheckStreamValidity(INT32 partition);
VOID CheckStreamValidity(VOID);
VOID __check_block_validity(INT32 nPBN);
VOID CheckBlockValidity(VOID);

VOID CheckHotColdMgmt(VOID);

#endif