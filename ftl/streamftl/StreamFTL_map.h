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

#ifndef __STREAMFTL_MAP_H__
#define __STREAMFTL_MAP_H__

#define STREAM_FTL_CHANNEL_MASK
#define STREAM_FTL_WAY_MASK
#define STREAM_FTL_BLOCK_MASK
#define STREAM_FTL_PAGE_MASK

VOID	RemoveStreamFromPartition(INT32 partition);

VOID	FreeFullInvalidStream(INT32 partition);

VOID	LinkStreamToBIT(INT32 nStream, INT32 nBlock);
void	UnlinkStreamFromBIT(int nStream);

void	insert_bitmap(int nStream, int nOffset);
int		bitCount_u32_HammingWeight(unsigned int n);
int		get_offset_fast(unsigned int *a, int n);

INT32	StreamFTL_AllocateBlock(VOID);
VOID	StreamFTL_ReleaseBlock(INT32 nVBN);

int		AllocateStream(IOTYPE eIOType);
void	InsertStreamIntoPartition(int nPartition, int nStream);

INT32	LPN2PPN(int LPN, int *partition, IOTYPE eIOType);

INT32	LPN2Stream(INT32 nLPN);
VOID	CloseActiveStream(INT32 nActiveBlockIndex);

void	InsertPartitionToVictimList(int nPartition);

INT32	SelectVictimBlock(VOID);
INT32	SelectVictimPartition(VOID);

// L2P Cache
VOID	AddToL2PCache(INT32 nLPN, INT32 nVPPN, INT32 nStream);


#endif	// end of #ifndef _STREAMFTL_MAP_H_