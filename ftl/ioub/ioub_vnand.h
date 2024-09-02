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
* Author: DongYoung Seo (dongyoung.seo@gmail.com)
		  Kyuhwa Han (hgh6877@gmail.com)
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#ifndef __IOUB_VNAND_H__
#define __IOUB_VNAND_H__

// Refer to ftl_config.h for the PPN layout 
#define VBN_FROM_VPPN(_nVPPN)				(((_nVPPN) >> NAND_ADDR_BLOCK_SHIFT) & NAND_ADDR_BLOCK_MASK)
#define VPN_FROM_VPPN(_nVPPN)				(_nVPPN & NAND_ADDR_VPAGE_MASK)
#define VPPN_FROM_VBN_VPN(_nChannel, _nWay, _nVBN, _nVPN)		(_nVPN + (_nVBN << NUM_BIT_VPAGE) + (_nWay << NUM_BIT_PAGE_PER_WAY) + (_nChannel << NUM_BIT_PAGE_PER_CHANNEL))
#define CHANNEL_VPPN(_nVPPN)				(_nVPPN & NAND_ADDR_PAGE_PER_WAY)


#define CHANNEL_FROM_VPPN(_nVPPN)			((_nVPPN >> NAND_ADDR_CHANNEL_SHIFT) & NAND_ADDR_CHANNEL_MASK)
#define WAY_FROM_VPPN(_nVPPN)				((_nVPPN >> NAND_ADDR_WAY_SHIFT) & NAND_ADDR_WAY_MASK)
#define PBN_FROM_VPPN(_nVPPN)				(VBN_FROM_VPPN(_nVPPN))
#define PPAGE_FROM_VPPN(_nVPPN)				((_nVPPN >> (NAND_ADDR_PPAGE_SHIFT)) & NAND_ADDR_PPAGE_MASK)
#define LPN_OFFSET_FROM_VPPN(_nVPPN)		((_nVPPN >> (NAND_ADDR_LPN_OFFSET_SHIFT)) & NAND_ADDR_LPN_PER_PAGE_MASK)

#define GET_VPPN_FROM_VPN_VBN(_nChannel, _nWay, _nVPN, _nVBN)	(_nVPN + (_nVBN << NAND_ADDR_VBN_SHIFT) + (_nWay << NUM_BIT_PAGE_PER_WAY) + (_nChannel << NUM_BIT_PAGE_PER_CHANNEL))
#define GET_LPN_FROM_VPN_VBN(_nChannel, _nWay, _nLPN, _nLBN)	((_nChannel + (_nWay << CHANNEL_BITS) + (_nLPN << (CHANNEL_BITS + NUM_BIT_WAY)) + (_nLBN << (CHANNEL_BITS + NUM_BIT_WAY + PAGES_PER_BLOCK_BITS))) << NUM_BIT_LPN_PER_PAGE)

//IOUB
#define PAGE_OFFSET_FROM_LPN(_nLPN)			((_nLPN >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_WAY + CHANNEL_BITS)) % PAGES_PER_BLOCK)
#define FIRST_PAGE_OFFSET_OF_LPN(_nLPN)		((_nLPN >> (NUM_BIT_LPN_PER_PAGE)) << NUM_BIT_LPN_PER_PAGE)

// physical block
class VIRTUAL_BLOCK
{
public:
	VIRTUAL_BLOCK() :
		m_nEC(0) {}

	UINT32*	m_pnV2L;			// store PPN 2 LPN
	UINT8*	m_pbValid;			// Logical Page Valid Bitmap
	INT32	m_nEC;				// Erase Count of block

};

class PROGRAM_UNIT;

/*
	Virtual NAND 
*/
class VNAND
{
public:
	VOID	Initialize(VOID);

	UINT32	GetPPagesPerVBlock(VOID);
	UINT32	GetVPagesPerVBlock(VOID);
	UINT32	GetVBlockCount(void);
	UINT32	GetV2L(UINT32 nVPPN);
	VOID	Invalidate(UINT32 nVPPN);

	BOOL	IsValid(UINT32 channel, UINT32 way, UINT32 nVBN, UINT32 nVPageNo);

	BOOL	IsBadBlock(UINT32 channel, UINT32 way, UINT32 nVBN) { return FIL_IsBad(channel, way, nVBN); }

	BOOL	ReadPage(FTL_REQUEST_ID stReqID, UINT32 nVPPN, void * pMainBuf, void * pSpareBuf);
	VOID	ProgramPage(FTL_REQUEST_ID stReqID, PROGRAM_UNIT *pstProgram);
	VOID	ReadPageSimul(UINT32 nVPPN, void * pMainBuf);		// for metadata read (only for demand loading)
	VOID	ProgramPageSimul(UINT32 nLPN, UINT32 nVPPN);
	VOID	EraseSimul(UINT32 nLPN, UINT32 nVPPN);

	UINT32	GetValidVPNCount(UINT32 channel, UINT32 way, UINT32 nVBN);

private:
	NAND_ADDR	_GetNandAddrFromVPPN(UINT32 nVPPN);
	VOID		_EraseSimul(INT32 channel, INT32 way, INT32 nVBN);

	VIRTUAL_BLOCK*		m_pstVB[USER_CHANNELS][USER_WAYS];
};

#endif		// end of #ifndef __IOUB_VNAND_H__