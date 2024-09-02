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

#ifndef __IOUB_BLOCK_H__
#define __IOUB_BLOCK_H__

class IOUB_GLOBAL;

typedef enum
{
	BLOCKTYPE_FREE,
	BLOCKTYPE_USER,
	BLOCKTYPE_GC,
	BLOCKTYPE_META,
} BLOCK_TYPE;

class VBINFO
{
public:
	VOID SetFree(VOID)		{ m_bFree = TRUE;	}
	VOID ClearFree(VOID)	{ m_bFree = FALSE;	}
	VOID SetBad(VOID)		{ m_bBad = TRUE;	}	
	VOID SetUser(VOID)		{ m_bUser = TRUE;	}
	VOID ClearUser(VOID)	{ m_bUser = FALSE;	}
	VOID SetGC(VOID)		{ m_bGC = TRUE;	}
	VOID ClearGC(VOID)		{ m_bGC = FALSE;	}
	VOID SetMeta(VOID)		{ m_bMeta = TRUE;	}
	VOID ClearMeta(VOID)	{ m_bMeta = FALSE;	}
	VOID SetActive(VOID)	{ m_bActive = TRUE;	}
	VOID ClearActive(VOID)	{ m_bActive = FALSE;	}
	BOOL IsActive(VOID)		{ return m_bActive;	}
	BOOL IsMeta(VOID)		{ return m_bMeta;	}
	BOOL IsFree(VOID)		{ return m_bFree;	}

	VOID IncreaseInvalidate(VOID);
	BOOL IsFullInvalid(VOID);
	VOID SetFullInvalid(VOID);
	VOID SetInvalidPageCount(UINT32 nCount) { m_nInvalidLPN = nCount; }
	UINT32 GetInvalidLPNCount(VOID) { return m_nInvalidLPN; }

	UINT32				m_nVBN;				// Virtual Block Number
	struct list_head	m_dlList;			// list for connecting free block list

private:
	unsigned int		m_bFree : 1;		// free blcok
	unsigned int		m_bBad	: 1;		// bad block
	unsigned int		m_bUser : 1;		// user block
	unsigned int		m_bGC	: 1;		// gc block
	unsigned int		m_bMeta : 1;		// meta block
	unsigned int		m_bActive : 1;		// active block

	UINT32				m_nInvalidLPN;
};

/*
	@brief Virtual Block Information
*/
class VBINFO_MGR
{
public:
	VOID	Initialize(VOID);
	VOID	Format(VOID);

	VBINFO*	GetVBInfo(UINT32 nVBN);
	UINT32	GetVBSize(VOID);

private:
	VBINFO*	m_pastVBInfo;		// VBInfo array
};

typedef enum
{
	META_BLOCK_MGR,
	USER_BLOCK_MGR,
} BLOCK_MGR_TYPE;

class BLOCK_MGR
{
public:
	VOID	Initialize(BLOCK_MGR_TYPE eType);
	VOID	Format(VOID);
	UINT32	Allocate(UINT32 channel, UINT32 way, BOOL bUser, BOOL bGC, BOOL bMeta, IOTYPE mtype);				// get a free Virtual Block
	VOID	Release(UINT32 channel, UINT32 way, UINT32 nVBN, METATYPE mtype);

	UINT32	GetFreeBlockCount(UINT32 channel, UINT32 way) { return m_nFreeBlocks[channel][way]; }
	VOID	Invalidate(UINT32 nLPN, UINT32 nVPPN);

	VOID	CheckVPC(UINT32 channel, UINT32 way, UINT32 nVBN);

	struct list_head	m_dlUsedBlocks[USER_CHANNELS][USER_WAYS][METATYPE_COUNT];		// allocated block list, 
											// Newly allocated block will be addred to tail
											// victim search start from the head
	METATYPE			m_mType;
private:
	VOID	_FormatUser(VOID);
	VOID	_FormatMeta(VOID);

	BLOCK_MGR_TYPE		m_eType;
	
	struct list_head	m_dlFreeBlocks[USER_CHANNELS][USER_WAYS];		// list for connecting free block list
	UINT32				m_nFreeBlocks[USER_CHANNELS][USER_WAYS];		// free block count
	UINT32				m_nUsedBlocks[USER_CHANNELS][USER_WAYS][METATYPE_COUNT];		// used block count

	unsigned int		m_bFormatted : 1;	// format status
};

#endif