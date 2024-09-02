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

#include "ioub_internal.h"

VOID IOUB_Profile_Initialize() {
	for (int iter = 0; iter < Prof_Total_Num; iter++) {
		IOUB_profile[iter] = 0;
	}
}

VOID IOUB_IncreaseProfile(IOUB_PROFILE offset) {
	IOUB_profile[(UINT32)offset]++;
}

VOID IOUB_IncreaseProfile(IOUB_PROFILE offset, UINT32 count) {
	IOUB_profile[(UINT32)offset] += count;
}

UINT32 IOUB_GetProfile(IOUB_PROFILE offset) {
	return IOUB_profile[(UINT32)offset];
}

VOID IOUB_PrintProfile() {
	for (int iter = 0; iter < Prof_Total_Num; iter++) {
		xil_printf("%u\r\n", IOUB_profile[(UINT32)iter]);
		IOUB_profile[iter] = 0;
	}
	xil_printf("\n");
}
