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

#ifndef __IOUB_BUFFERPOOL_H__
#define __IOUB_BUFFERPOOL_H__
#define BUFFER_FOR_WRITE_REQ	(USER_CHANNELS * USER_WAYS)

class BUFFER_ENTRY
{
public:
	struct list_head	m_dlList;		// to link free list
	void*				m_pMainBuf;
	void*				m_pSpareBuf;

	UINT8				refCount;
	UINT32				nVPPN;
	UINT8				readtype;
	BOOL				readDone;
};

class BUFFER_MGR
{
public:
	VOID			Initialize(VOID);
	BUFFER_ENTRY*	GetEntry(INT32 nIndex);
	BUFFER_ENTRY*	Allocate(VOID);
	VOID			Release(BUFFER_ENTRY* pstEntry);
	BOOL			IsEmpty(VOID) { return GetFreeCount() == 0 ? TRUE : FALSE; }

	INT32			GetFreeCount(VOID)
	{
		return m_nFreeCount;
	}

private:
	INT32				m_nTotalCount;
	BUFFER_ENTRY*		m_pastEntry;

	struct list_head	m_dlFree;		// to link free list
	INT32				m_nFreeCount;
};

#endif