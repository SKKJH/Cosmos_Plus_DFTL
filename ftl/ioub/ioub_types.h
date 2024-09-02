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

#ifndef __IOUB_TYPES_H__
#define __IOUB_TYPES_H__

#define VIRTUAL
#define STATIC

#define INVALID_PPN				(INVALID_INDEX)
#define INVALID_PPN				(INVALID_INDEX)
#define INVALID_VPPN			(INVALID_INDEX)
#define INVALID_LPN				(INVALID_INDEX)
#define INVALID_VBN				(INVALID_INDEX)
#define INVALID_EC				-1				// erase count

typedef enum
{
	IOTYPE_PAGE = 0,	// HOST REQUEST
	IOTYPE_BLOCK,
	IOTYPE_GC,			// GC REQUEST
	IOTYPE_META,		// META REQUEST

	IOTYPE_TEST,
	IOTYPE_COUNT,
} IOTYPE;

typedef enum
{
	METATYPE_PAGE = 0,	// HOST REQUEST
	METATYPE_BLOCK,			// GC REQUEST
#if (IOUB_STRIPING == 1)
	METATYPE_STRIPING,
#endif

	METATYPE_TEST,
	METATYPE_COUNT = METATYPE_TEST,
} METATYPE;

typedef enum
{
	LBNTYPE_PAGE = 0,	// HOST REQUEST
	LBNTYPE_BLOCK,			// GC REQUEST

	LBNTYPE_TEST,
	LBNTYPE_COUNT = METATYPE_TEST,
} LBNTYPE;

#ifdef __cplusplus

/*
	// pure virtual class
*/
class FTL_INTERFACE
{
public:
	virtual VOID Initialize(VOID) = 0;
	virtual BOOL Format(VOID) = 0;
	virtual VOID Run(VOID) = 0;
	virtual VOID ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount) = 0;
	virtual VOID WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount) = 0;
	virtual VOID CallBack(FTL_REQUEST_ID stReqID) = 0;
	virtual VOID IOCtl(IOCTL_TYPE eType) = 0;
};

class COMMON_REQUEST
{
public:
	COMMON_REQUEST(VOID) : m_eIOType(IOTYPE_COUNT) {}

protected:
	IOTYPE	m_eIOType;
};

#endif
#endif		// end of #ifndef __IOUB_INIT_H__