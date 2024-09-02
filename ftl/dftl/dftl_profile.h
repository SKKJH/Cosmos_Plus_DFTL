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
* Author: Kyuhwa Han (hgh6877@gmail.com)
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#ifndef __DFTL_PROFILE_H__
#define __DFTL_PROFILE_H__

typedef enum {
	//Host
	Prof_Host_read = 0,
	Prof_Host_write,

	Prof_CMT_read,
	Prof_CMT_write,

	Prof_CMTGC_read,
	Prof_CMTGC_write,
	Prof_CMTGC_count,

	Prof_GC_read,
	Prof_GC_write,
	Prof_GC_count,	//2MB block

	Prof_NAND_read,		//16KB
	Prof_NAND_write,	//16KB
	Prof_NAND_erase,

	Prof_Total_Num,
} DFTL_PROFILE;

static UINT32 DFTL_profile[Prof_Total_Num];


VOID DFTL_Profile_Initialize(VOID);
VOID DFTL_IncreaseProfile(DFTL_PROFILE offset);
VOID DFTL_IncreaseProfile(DFTL_PROFILE offset, UINT32 count);
UINT32 DFTL_GetProfile(DFTL_PROFILE offset);
VOID DFTL_PrintProfile(VOID);
#endif