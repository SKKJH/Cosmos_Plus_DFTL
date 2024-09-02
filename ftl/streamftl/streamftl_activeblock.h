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

#ifndef __STREAMFTL_ACTIVEBLOCK_H__
#define __STREAMFTL_ACTIVEBLOCK_H__

#include "streamftl_internal.h"

VOID	ActiveBlock_Initialize(VOID);

VOID	ActiveBlock_AddBufferingLPN(ACTIVE_BLOCK* pstActiveBlock, FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, UINT32 nHostCmdSlotTag, INT16 nDMAIndex);
VOID	ActiveBlock_RemoveBufferingLPNHash(INT32 nLPN);
BOOL	ActiveBlock_ReadFromBuffering(INT32 nLPN, INT32 nVPPN, INT32* pMainBuf, void* pSpareBuf);

INT32	ActiveBlock_Check(INT32 nLPN);
INT32	ActiveBlock_Select(INT32 nLPN, IOTYPE eIOType, BOOL* pbFreeActiveBlock);
VOID	ActiveBlock_MoveToFreeList(INT32 nIndex);

ACTIVE_BLOCK *ActiveBlock_GetActiveBlockForPartition(INT32 nPartition);

#endif	// end of #ifndef __STREAMFTL_ACTIVEBLOCK_H__