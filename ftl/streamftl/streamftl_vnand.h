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

#ifndef __STREAMFTL_VNAND_H__
#define __STREAMFTL_VNAND_H__

#include "streamftl_types.h"
#include "streamftl.h"

// physical block
typedef struct
{
	INT32	*panPPN2LPN;		// store PPN 2 LPN
	UINT8	*pabValid;			// page's validity bitmap
	INT32	nEC;				// Erase Count
} _PB;

_PB			*g_pstPB;				// Physical Block
BOOL		*g_pstPBValidBitmap;		// memory start point of Physical block valid bitmap
INT32		*g_pstPBPPN2LPN;

void	VNAND_Initialize();

INT32	VNAND_GetPBlocksPerVBlock(VOID);
INT32	VNAND_GetPPagesPerVBlock(VOID);
INT32	VNAND_GetLPagesPerVBlock(VOID);
INT32	VNAND_GetVBlockCount(VOID);
INT32	VNAND_GetLPN(INT32 nVPPN);

INT32	VNAND_ReadPage(FTL_REQUEST_ID stReq, UINT32 nVPPN, void *pMainBuf, void *pSpareBuf);
void	VNAND_ProgramPage(FTL_REQUEST_ID stReqID, PROGRAM_UNIT *pstProgram);
void	VNAND_ProgramPageSimul(ACTIVE_BLOCK* pstActiveBlock, int nBufferedLPNIndex);
void	VNAND_Invalidate(INT32 nVPN);
INT32	VNAND_IsValid(INT32 nVBN, INT32 nVPage);
VOID	VNAND_Erase(INT32 nVBN);
INT32	VNAND_GetEC(INT32 nVBN);
BOOL	VNAND_IsBad(INT32 nVBN);
#endif
