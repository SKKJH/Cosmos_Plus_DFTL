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

#ifndef __IOUB_ACTIVEBLOCK_H__
#define __IOUB_ACTIVEBLOCK_H__

class HIL_REQUEST;
class GC_REQUEST;

class PROGRAM_UNIT
{
public:
	PROGRAM_UNIT(VOID);
	BOOL Add(VOID* pstRequest, UINT32 nVPPN, IOTYPE eBlockType, IOTYPE eRequestType, BOOL* add_page, BOOL page_validity);
	VOID Add_full_page(UINT32 nLPN, UINT32 nVPPN);
	UINT32 Partial_read_page(UINT32 nLPN, UINT32 nVPPN);
	VOID pages_IM_copy(BUFFER_ENTRY *buf, UINT32 srcLPN, UINT32 nLPN, UINT32 nVPPN, UINT32 count);

	VOID padding_init(UINT32 VPPN);

	BOOL IsFull(VOID)
	{
		return (m_nLPNCount == LPN_PER_PHYSICAL_PAGE) ? TRUE : FALSE;
	}

	UINT32	GetVPPN(VOID) {return m_nVPPN;}
	BUFFER_ENTRY*	GetBufferEntry(VOID) {return m_pstBufferEntry;}
	VOID SetBufferEntry(BUFFER_ENTRY* pstEntry) {m_pstBufferEntry = pstEntry;}
	UINT32	GetLPN(INT32 nIndex) {return m_anLPN[nIndex];}
	BOOL isBufferedOffset(UINT32 nIndex) { return m_anLPN[nIndex] != INVALID_LPN; }


	unsigned int		m_bFree : 1;			// this entry is free
	unsigned int		m_bProgramming : 1;		// on programming
	unsigned int		m_bPadding : 1;		// on programming
	unsigned int		m_bReadDone : 1;
	unsigned int		m_nLPNCount : 3;		// max 4 LPNs
	unsigned int		m_nDMAReqTail : 8;		// DMA tail request, to check HDMA done, Host Write only

	UINT32				m_nDMAOverFlowCount;		// count of DMA index overflow, to check HDMA done

	UINT32				isInIMBuffer;

private:
	FTL_REQUEST_ID		m_astRequestID[LPN_PER_PHYSICAL_PAGE];	// request ID for current request = Request ID for each LPN
	INT32				m_anLPN[LPN_PER_PHYSICAL_PAGE];			// Buffered LPN

	UINT32				m_nVPPN;					// Program VPPN, 1st page of requests
	BUFFER_ENTRY*		m_pstBufferEntry;			// NAND IO Buffer
} ;

/*
	ActiveBlock
*/
#define MAX_VALID (0xf)
class ACTIVE_BLOCK
{
public:
	ACTIVE_BLOCK(VOID);
	VOID Initialize(UINT32 channel, UINT32 way, UINT32 VBN, UINT32 LBN);
	BOOL Write(VOID* pstRequest, IOTYPE eRequestType);

	VOID IncreaseVPPN(VOID);
	VOID SetIOType(IOTYPE eIOType) { m_eBlockType = eIOType; }
	VOID ProgramDone(INT32 nBufferingIndex);
	VOID PaddingReadDone(UINT32 bufferIndex);
	VOID PartialReadDone(UINT32 bufferIndex);
	VOID IMReadDone(UINT32 bufIdx);

	BOOL ReqIssuable(VOID);

	BOOL CheckAllProgramUnitIsFree(VOID);			// debugging
	BOOL CompactOnePage(VOID);
	BOOL ProgramPages(UINT32 count);

	BOOL IM_page_copies(UINT32 src_lpn, UINT32 dst_lpn, UINT32 , BUFFER_ENTRY*	IMBuffer);

	UINT32 ReadBufferingLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry);
	BOOL isBufferedOffset(UINT32 nIndex) { 
		if (m_astBuffering[m_nCurProgramBuffering].m_nLPNCount != 0)
			return m_astBuffering[m_nCurProgramBuffering].isBufferedOffset(nIndex);
		else
			return FALSE;
	}

	UINT32 GetFirstInvalid(VOID);
	UINT32 GetFirstReadInvalid(VOID);
	BOOL BackgroundWork();

	VOID Relase();
	struct list_head	m_dlList;			// list for active block

	UINT32			m_nVBN;
	UINT32			m_nCurVPPN;				// PPN recently written
	UINT32			m_nLBN;
	UINT32			m_nOffset;

	UINT32			Index;
	UINT32			m_Channel;
	UINT32			m_Way;
	UINT32			doPadding;
	UINT32			OpenBySC;

	UINT8			page_validity[PAGES_PER_BLOCK];
	UINT32			sec_type;
private:
	BOOL			_IssueProgram(UINT32 bufferOff);

	IOTYPE			m_eBlockType;				// type of active block

	unsigned int	m_nCurProgramBuffering : ACTIVE_BLOCK_BUFFERING_INDEX_BITS;
												// current index of astBuffering 
	PROGRAM_UNIT	m_astBuffering[ACTIVE_BLOCK_BUFFERING_COUNT];		// program buffering information
} ;

/*
	@brief Buffering LPNs for host write
*/

class BUFFERING_LPN
{
public:

	static const UINT32 HASH_BUCKET_COUNT = 4096;
	static const UINT32 HASH_BUCKET_MASK = (HASH_BUCKET_COUNT - 1);

	VOID Initialize(IOTYPE eIOType);
	UINT32 ReadLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry);

	static inline UINT32 GET_BUCKET_INDEX(UINT32 nLPN)
	{
		return nLPN & HASH_BUCKET_MASK;
	}

	VOID Add(UINT32 nLPN);
	VOID Remove(UINT32 nLPN);

private:
	IOTYPE		m_eIOType;
	UINT16		m_anBufferingLPNCount[HASH_BUCKET_COUNT];
};

class ACTIVE_BLOCK_MGR
{
public:
	VOID Initialize(UINT32 channel, UINT32 way);
	ACTIVE_BLOCK* GetActiveBlock(IOTYPE eIOType);
	ACTIVE_BLOCK* GetActiveBlock(INT32 nIndex, IOTYPE eIOType);
	ACTIVE_BLOCK* GetActiveBlockptr(UINT32 nLBN, IOTYPE eIOType);
	ACTIVE_BLOCK* GetfreeActiveBlockptr(IOTYPE eIOType);
	ACTIVE_BLOCK* GetActiveBlockType(IOTYPE eIOType, UINT32 type);

	BUFFERING_LPN* GetBufferingMgr(IOTYPE type) { return &m_stBufferingMgr[type]; }

	BUFFERING_LPN* GetMetaBufferingMgr(VOID) { return &m_stMetaBufferingMgr; }

	UINT32 ReadBufferingLPN(UINT32 nLPN, BUFFER_ENTRY* pstBufferEntry, IOTYPE eIOType);

	UINT32 GetActiveBlockIndex(IOTYPE eIOType);

	BOOL	IsCompactionRunning(VOID);
	VOID	StartCompaction(UINT32 input_index);
	BOOL	ProcessCompaction(VOID);
	UINT32	GetCompactBlock(VOID);

	VOID printActiveBlock();

	VOID Background_work();

	
	struct list_head	HostBlock;
private:
	BUFFERING_LPN	m_stBufferingMgr[LBNTYPE_COUNT];

	BUFFERING_LPN	m_stMetaBufferingMgr;

	ACTIVE_BLOCK*	_GetCurActiveBlock(IOTYPE eIOType);
	ACTIVE_BLOCK*	_GoToNextActiveBlock(IOTYPE eIOType);

	INT8			m_nCurActiveBlockPage;
	INT8			m_nCurActiveBlockBlock;
	INT8			m_nCurActiveBlockGC;
	INT8			m_nCurActiveBlockMeta;

	ACTIVE_BLOCK*	m_pstCurActiveBlockPage;
	ACTIVE_BLOCK*	m_pstCurActiveBlockBlock;
	ACTIVE_BLOCK*	m_pstCurActiveBlockGC;
	ACTIVE_BLOCK*	m_pstCurActiveBlockMeta;


	UINT32			Comp_Block_Index;
	
	ACTIVE_BLOCK	m_astActiveBlockHost[LBNTYPE_COUNT][ACTIVE_BLOCK_COUNT_PER_STREAM];
	ACTIVE_BLOCK	m_astActiveBlockGC[ACTIVE_BLOCK_COUNT_PER_STREAM];
	ACTIVE_BLOCK	m_astActiveBlockMeta[ACTIVE_BLOCK_COUNT_PER_STREAM];


	UINT32	m_Channel;
	UINT32	m_Way;
};

#endif
