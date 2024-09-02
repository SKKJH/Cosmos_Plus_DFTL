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

#include "dftl_internal.h"


VOID DFTL_Profile_Initialize() {
	for (int iter = 0; iter < Prof_Total_Num; iter++) {
		DFTL_profile[iter] = 0;
	}
}

VOID DFTL_IncreaseProfile(DFTL_PROFILE offset) {
	DFTL_profile[(UINT32)offset]++;
}

VOID DFTL_IncreaseProfile(DFTL_PROFILE offset, UINT32 count) {
	DFTL_profile[(UINT32)offset] += count;
}

UINT32 DFTL_GetProfile(DFTL_PROFILE offset) {
	return DFTL_profile[(UINT32)offset];
}

VOID DFTL_PrintProfile() {
	for (int iter = 0; iter < Prof_Total_Num; iter++) {
		xil_printf("DFTL_PrintProfile: %u\r\n", DFTL_profile[(UINT32)iter]);
	}
}
