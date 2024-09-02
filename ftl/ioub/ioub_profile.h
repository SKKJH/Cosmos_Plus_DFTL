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

#ifndef __IOUB_PROFILE_H__
#define __IOUB_PROFILE_H__

typedef enum {
	//Host
	Prof_Host_read = 0,
	Prof_Host_write,

	Prof_SC_valid,
	Prof_SC_real_valid,
	Prof_SC_may_read,
	Prof_Padding_read,

	Prof_Block_padding,
	Prof_Block_padding_cpbk,
	Prof_Block_merge_copy,

	Prof_IM_count,
	Prof_IM_paddings,
	Prof_IM_data_cpbk,
	Prof_IM_node_cpbk,
	Prof_SC_count,
	Prof_SC_IM_count,

	Prof_BG_padding,

	Prof_GC_read,
	Prof_GC_write,
	Prof_GC_count,	//2MB block

	Prof_NAND_read_host,
	Prof_NAND_read,		//16KB
	Prof_NAND_write,	//16KB
	Prof_NAND_erase,

	Prof_Total_Num,
} IOUB_PROFILE;

static UINT32 IOUB_profile[Prof_Total_Num];


VOID IOUB_Profile_Initialize(VOID);
VOID IOUB_IncreaseProfile(IOUB_PROFILE offset);
VOID IOUB_IncreaseProfile(IOUB_PROFILE offset, UINT32 count);
UINT32 IOUB_GetProfile(IOUB_PROFILE offset);
VOID IOUB_PrintProfile(VOID);

#endif