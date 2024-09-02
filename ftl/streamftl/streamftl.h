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

#ifndef __STREAMFTL_H__
#define __STREAMFTL_H__

#include "list.h"
#include "ftl_config.h"
#include "streamftl_types.h"
#include "streamftl_request.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define L2P_CACHE_RATIO			(0.008)			// cache cover ratio of density, (1: 100% == all caching)

#define INVALID_PARTITION		(INVALID_INDEX)
#define INVALID_STREAM			(INVALID_INDEX)
#define INVALID_PPN				(INVALID_INDEX)
#define INVALID_LPN				(INVALID_INDEX)
#define INVALID_PBN				(INVALID_INDEX)
#define INVALID_EC				-1				// erase count

#define NOT_CACHED_PPN			-2				// Does not cached LPN

// Refer to ftl_config.h for the PPN layout 
#define VBN_FROM_VPPN(_nVPPN)				(((_nVPPN) >> NAND_ADDR_BLOCK_SHIFT) & NAND_ADDR_BLOCK_MASK)
#define VPAGE_FROM_VPPN(_nVPPN)				(_nVPPN & NAND_ADDR_VPAGE_MASK)
#define VPAGE_FROM_VBN_VPN(_nVBN, _nVPage)	(_nVPage + (_nVBN << NUM_BIT_VPAGE))

#define CHANNEL_FROM_VPPN(_nVPPN)			((_nVPPN >> NAND_ADDR_CHANNEL_SHIFT) & NAND_ADDR_CHANNEL_MASK)
#define WAY_FROM_VPPN(_nVPPN)				((_nVPPN >> NAND_ADDR_WAY_SHIFT) & NAND_ADDR_WAY_MASK)
#define PBN_FROM_VPPN(_nVPPN)				(VBN_FROM_VPPN(_nVPPN))
#define PPAGE_FROM_VPPN(_nVPPN)				((_nVPPN >> (NAND_ADDR_PPAGE_SHIFT)) & NAND_ADDR_PPAGE_MASK)
#define LPN_OFFSET_FROM_VPPN(_nVPPN)		((_nVPPN >> (NAND_ADDR_LPN_OFFSET_SHIFT)) & NAND_ADDR_LPN_PER_PAGE_MASK)

#define IS_VBLOCK_FULL(_VPPN)				((((_VPPN + 1) & g_stGlobal.nLPagesPerVBlockMask) == 0) ? TRUE : FALSE)

#define GET_PARTITION(_nPartNo)				(&g_pstStreamFTL->stPartitionMgr.pstPartitions[_nPartNo])
#define GET_STREAM(_nStreamNo)				(&g_pstStreamFTL->stStreamMgr.pstStreams[_nStreamNo])
#define GET_ACTIVE_BLOCK(_nActiveBlkIndex)	(&g_pstStreamFTL->stActiveBlockMgr.pstActiveBlock[_nActiveBlkIndex])
#define GET_BLOCK_INFO(_nPBN)				(&g_pstStreamFTL->stBlockMgr.pstBIT[_nPBN])
#define GET_L2P_CACHE_ENTRY(_nIndex)		(&g_pstStreamFTL->stL2PCache.m_pstL2PCacheEntries[_nIndex])
#define GET_PBN_FROM_PPN(_nPPN)				(_nPPN / PAGE_PER_BLOCK)

#define GET_VPPN_FROM_VPN_VBN(_nVPN, _nVBN)	(_nVPN + (_nVBN << NAND_ADDR_VBN_SHIFT))

#define	INDEX_OF(_pBase, _pPtr, _Type)		((_Type*)(_pPtr) - (_Type*)_pBase)
#define	INDEX_OF_L2P_CACHE_ENTRY(_pPtr)		(INDEX_OF(g_pstStreamFTL->stL2PCache.m_pstL2PCacheEntries, _pPtr, L2P_CACHE_ENTRY))

#define	MALLOC			UTIL_MemAlloc

typedef enum
{
	RETURN_SUCCESS = 0,
	RETURN_FAIL,
	RETURN_FAIL_MAX_STREAMS_PER_PARTITION,
	RETURN_NOT_ENOUGH_STREAM,
	RETURN_NOT_ENOUGH_BUFFER_POOL,
	RETURN_GC_RUNNING,
} RETURN;

#define PARTITION_ID_FROM_LPN(LPN)			((LPN) / g_stGlobal.nLPagePerPartition)
#define LPN_TO_PARTITION					PARTITION_ID_FROM_LPN
#define PARTITION_START_LPN(_nPartition)	(_nPartition * g_stGlobal.nLPagePerPartition);
#define STREAM_START_LPN(_nLPN)				ROUND_DOWN(_nLPN, g_stGlobal.nLPagePerStream)

#define	NO_REFERENCE					(0)	// 사용되지 않는 CODE 표시용.

/*
* Mapping Structure
*/
#define SET_FLAG(_nFlagVar, _nFlag)			(_nFlagVar |= _nFlag);
#define CLEAR_FLAG(_nFlagVar, _nFlag)		(_nFlagVar &= (~_nFlag));
#define CHECK_FLAG(_nFlagVar, _nFlag)		((_nFlagVar & _nFlag) ? TRUE : FALSE)

/*
	g_pstStreams : map for bitmap. Page Valid Bitmap
	STREAM Information table, 
	STREAM의 정보 관리
*/
typedef struct
{
	UINT32*			paBitmap;				// bitmap for logical page written. Logical Valid Bitmap, LSB: lower LPN
	INT32			nStartLPN;				// start logical page number
	INT32			nStartVPPN;				// start physical page number
	INT32			nEndVPPN;				// end physical page number
	INT32*			pnVBN;				// allocated block number, CHECKME: INT16가능.
	INT16			nVPC;					// number of valid page
	INT8			nAllocatedBlocks;		// # of blocks allocated in this partition
	UINT8			bFree	: 1;			// free
	UINT8			bActive	: 1;			// active
	UINT8			bGC		: 1;			// GC Stream
	UINT8			bSMerge : 1;			// SMerge Stream
	UINT8			bRSVD	: 4;

	struct list_head dlList;				// list for FreeStreamPool or Partition
	INT32			nStreamIndex;			// index of current stream
	INT8*			pnEC;					// erase count of Physical Block
											// GC 중 block의 validity를 확인하기 위해 필요함.
											// i.e. Partition-P를 위해 STREAM-N 에 PBN 100, 200이 할당됨.
											//		PBN 100의 VPC가 0이 되어 freeblock이 됨.
											//		PBN 100이 다시 Partition-P를 위해 STREAM-M 에 할당..
											//		이후 GC수행되면 STREAM-N.pBlock[0]의 valid page 정보로 copy 되고
											//		다시 STREAM-M.pBlock[0]의 valid page 정보로 copy 될 수 있음.
											//	이러한 현상을 막기위해 EC로 PBN의 validity 정보를 확인함.
											// INT32로 full EC를 저장하면 좋지만 차이만 확인하면 되므로 memory 사용량 개선을 위해 INT8로 설정
} STREAM;

/*
	(old)CLUSTER : map for cluster management. 
			Partition 용어로 Date논문에서 사용됨.
			초기 연구에서는 STREAM에 복수개의 Partition 할당 가능하였으나 이후 고정함.

	전체 LOGICAL SPACE를 PARTITION SIZE로 나눈 개념. LPN / g_stGlobal.nPartitionSize로 PARTITION ID를 구한다.
	Partition에 stream들이 달려 있음.
*/
typedef struct
{
	INT32				nPartitionNo;		// current partition No, FIXME, remove this member
	INT16				nVPC;				// # of valid page in partition

	INT8				nNumStream;			// # of stream in cur partition,
#if (SUPPORT_HOTCOLD == 1)
	INT8				m_nOverwriteRatioIndexPrev;	// HotCold의 m_anPartitionCount[]에 저장된 index, 
													// new index로 옮길때 prev를 -- 해주어야 함.
#endif

	INT32				nWriteCountAfterGC;		// partition write count after GC, to check hot/cold

	struct list_head	dlStream;			// stream list head to link allocated stream
											// p_list is LRU list for the partition, active partition will be MRU partition
											// latest stream is located at head

	struct list_head	dlVictimPartition;	// list for partition

#if (SUPPORT_L2P_CACHE == 1)
	INT32				m_nL2PCacheIndex;	// index of L2P Cache
#endif
} PARTITION;

typedef struct
{
	struct list_head	dlList;		// to link free list
	void*				pMainBuf;
	void*				pSpareBuf;
} BUFFER_ENTRY;

typedef struct
{
	BUFFER_ENTRY*		pastEntry;
	struct list_head	dlFree;
	INT32				nFreeCount;					// free buffer count
	INT32				nTotalCount;				// total buffer count
} BUFFER_POOL;

typedef struct
{
	unsigned int		bFree : 1;				// this entry is free
	unsigned int		bProgramming : 1;		// on programming
	unsigned int		nLPNCount : 3;			// max 4 LPNs
	unsigned int		nDMAReqTail : 8;		// DMA tail request, to check HDMA done, Host Write only
	UINT32				nDMAOverFlowCount;		// count of DMA index overflow, to check HDMA done

	FTL_REQUEST_ID		astRequestID[LPN_PER_PHYSICAL_PAGE];	// request ID for current request = Request ID for each LPN
	INT32				anLPN[LPN_PER_PHYSICAL_PAGE];			// Buffered LPN

	UINT32				nVPPN;					// Program VPPN, 1st page of requests
	BUFFER_ENTRY*		pstBufferEntry;			// NAND IO Buffer
} PROGRAM_UNIT;

/*
	ActiveBlock
*/
typedef struct
{
	struct list_head	dlUsedOrFree;				// dlist for free/used link
	unsigned int		nIndex				:16;	// active stream index, FIXME, remove this member 
	unsigned int		nBufferingLPN		: 4;	// current buffering LPN Count
	unsigned int		nCurProgramBuffering : ACTIVE_BLOCK_BUFFERING_INDEX_BITS;
													// current index of astBuffering 
	INT32				nStreamNo;					// STREAM recently written, ActiveStreamNo
	INT32				nRecentLPN;					// LPN recently written
	INT32				nRecentVPPN;				// PPN recently written

	// TODO, let's add double buffering feature
	PROGRAM_UNIT		astBuffering[ACTIVE_BLOCK_BUFFERING_COUNT];		// program buffering information
} ACTIVE_BLOCK;

#define BLOCK_INFO_SET_FLAG(_nPBN, _nFlag)		SET_FLAG(g_pstStreamFTL->g_pstBIT[_nPBN].nFlag, _nFlag)
#define BLOCK_INFO_CLEAR_FLAG(_nPBN, _nFlag)	CLEAR_FLAG(g_pstStreamFTL->g_pstBIT[_nPBN].nFlag, ~_nFlag)
#define BLOCK_INFO_CHECK_FLAG(_nPBN, _nFlag)	CHECK_FLAG(g_pstStreamFTL->g_pstBIT[_nPBN].nFlag, _nFlag)

/*
	BIT : Block Information Table.
	Physical block당 1개씩 할당
*/
typedef struct
{
	INT32	nBlockNo;						// Physical Block Number, this can be removed
	INT32	nInvalidPages;					// # of invalid page in block
	UINT32	nStreamCount	: 24;			// number of stream in block, 모두 다른 stream이면 partition 개수와 같다.
	UINT32	bActive			: 1;			// if TRUE, then the block is in the active stream
											// Victim selection시 제외하기 위한 목적으로 관린
	UINT32	bFree			: 1;			// if TRUE, then this block is a free block
	UINT32	bMultiStreams	: 1;			// if 1, this physical block has more than 1 stream
	UINT32	bMetaBlock		: 1;			// Flag for block information, this will be used to reduce size of metadata
	UINT32	bUser			: 1;			// user data is written at this block
	UINT32	bGC				: 1;			// GC data is written at this block
	UINT32	bSMerge			: 1;			// SMerge data is written at this block
	UINT32	bBad			: 1;

	struct list_head dlFreeOrAllocated;		// for free or allocated list
} BLOCK_INFO;

typedef struct _LIST_MAP
{
	INT32 value;								// STREAM index
	struct list_head list;
}_LIST_MAP;

typedef struct
{
	INT32				m_nPartition;	// partition id, m_anPPN이 모두 invalid인 경우에는 Partition을 찾을 수 없으므로 필요하다.
	INT32*				m_pnL2P;
	INT32*				m_pnStream;		// just for debugging
	struct list_head	m_dlList;		// for Free or victimLRU list
} L2P_CACHE_ENTRY;

typedef struct
{
	INT32				m_nEntryCount;					// total entry count, ratio: g_stGlobal.fL2PCacheRatio;
	L2P_CACHE_ENTRY*	m_pstL2PCacheEntries;
	INT32*				m_pnL2P;						// L2P Array
	INT32*				m_pnStream;						// STREAM Index array, for debugging

	struct list_head	m_dlFreeEntry;					// Free L2P Cache entries
	struct list_head	m_dlUsedLRU;					// LRU list, head :recently access cache
} L2P_CACHE;

// Partition hot/cold separation
typedef struct
{
	BOOL			m_bEnable;							// HotCold enable or not

#if (SUPPORT_HOTCOLD == 1)
	float			fHotPartitionRatio;				// ratio of hot partition, 0.1 : 10%
														// used partition 중 hot partition의 비율
	float			fVictimActiveBlockVictimRange;	// range(ratio) of victim active block lookup within LRU list
														// used active list에서 victim lookup을 수행할 영역의 비율. 0.1: 10%

	INT32			m_nNumValidParitions;				// total count of partitions at anPartitionCount
	INT32			m_anPartitionCount[STREAM_FTL_MAX_HOT_RATIO_INDEX];
														// array of overwrite ratio 별 partition count
														// log scale

	INT16			m_nCurHotIndex;						// hot index of m_anPartitionCount, performance optimization
														// PartitionCount의 변경이 있으면 INVALID_INDEX로 설정됨.
#endif
} HOTCOLD_MANAGER;

typedef struct
{
	STREAM*				pstStreams;
	INT32*				pstStreamValidBitmap;
	INT32*				pstStreamBlock;					// arrary for stream block
	INT8*				pstStreamEC;						// array for stream block EC
	struct list_head	dlFreeStreamPool;					// poll of free partition, Free STREAM Pool
	INT32				nFreeStreamCount;					// Count of Free STREAM
} STREAM_MANAGER;

#define BUFFERING_LPN_HASH_BUCKET			16384
#define BUFFERING_LPN_HASH_BUCKET_MASK		(BUFFERING_LPN_HASH_BUCKET - 1)
#define GET_BUCKET_INDEX(_nLPN)				(_nLPN & BUFFERING_LPN_HASH_BUCKET_MASK)

typedef struct
{
	UINT16		anBufferingLPNCount[BUFFERING_LPN_HASH_BUCKET];
} BUFFERING_LPN_HASH;

typedef struct
{
	ACTIVE_BLOCK*		pstActiveBlock;					// Active STREAM Information Table List
	struct list_head	dlUsedActiveBlocks;				// managed as LRU, 사용중인 active streams, Cold ActiveBlockss for hot/cold enabled
	struct list_head	dlFreeActiveBlocks;				// Free Active STREAM Pool
	INT32				nUsedActiveBlocks;

	ACTIVE_BLOCK*		pstActiveBlockGC;					// Active Block for GC
	ACTIVE_BLOCK*		pstActiveBlockSMerge;				// Active Block for StreamMerge

	BUFFERING_LPN_HASH		stBufferingLPN;
} ACTIVE_BLOCK_MANAGER;

typedef struct
{
	BLOCK_INFO*			pstBIT;							// Block Information Table array, Physical block당 1개씩
	struct list_head	dlAllocatedBlocks;				// if the block is not hybrid block, then the block will be in the top of the list
	struct list_head	dlFreeBlocks;
	INT32				nFreeBlockCount;					// Free Block Count
} BLOCK_MANAGER;

typedef struct
{
	PARTITION*			pstPartitions;					// Partition Array, LogicalSpaceSize를 StreamSize로 나눈 개수 [32GB/256KB]
	struct list_head	dlVictimPartitionHead;			// Partition에 할당된 STREAM의 개수에 따라 정렬하여 관리
} PARTITION_MANAGER;

typedef enum
{
	GC_TYPE_NONE,
	GC_TYPE_STREAM_MERGE,
	GC_TYPE_BLOCK_GC,
} GC_TYPE;

typedef struct
{
	/*
		StreamFTL manages read/write status for in-order write (considering stream structure)
	*/
	unsigned int		bValid			: 1;		// Mapped(Written) LPN
	unsigned int		bReadDone		: 1;
	unsigned int		bWriteIssued	: 1;
	unsigned int		bWriteDone		: 1;
	unsigned int		nGCRequestIndex : GC_REQUST_COUNT_BITS;
} SMERGE_COPY;

typedef struct
{
	INT32				nPartition;			// merge partition
	INT32				nStartLPN;
	INT32				nEndLPN;
	INT32				nCurReadLPN;		// LPN to read issue
	SMERGE_COPY			astCopyInfo[DEFAULT_LPN_PER_STREAM];			// Merge status of each LPN
	INT32				nWriteOffset;		// Write index of anReadDone array
											// must be written in a ascending order
											// NAND read request will be issued in parallel,
											// sthe response sequence of read does not guarantee issued order.
	INT32				nVPC;				// VPC for this partition
	INT16				nWriteDoneLPNCount;	// count of write done LPN
} SMERGE_INFO;

typedef struct
{
	/*
		StreamFTL manages read/write status for in-order write (considering stream structure)
	*/
	unsigned int		nGCRequestIndex :GC_REQUST_COUNT_BITS;
	unsigned int		bFree			: 1;
	unsigned int		bReadDone		: 1;
	unsigned int		bWriteIssued	: 1;
	unsigned int		bWriteDone		: 1;
} BLOCK_GC_COPY;

#define BLOCK_GC_COPY_ENTRY_COUNT		((USER_CHANNELS * 4 /*way*/) * LPN_PER_PHYSICAL_PAGE)
#define BLOCK_GC_COPY_ENTRY_COUNT_BITS	8

typedef struct
{
	// A block GC will be done on the below condition
	// 1. when nCurReadOffset offset goes up to the end of page.
	// 2. nReadLPNCount == nWriteLPNCount				<== Write issue done

	INT32				nVictimVBN;				// Victim block
	INT32				nVPC;					// Valid Page count of VBN @ GC start
	INT32				nCurReadLPageOffset;	// Read logical page offset
	INT32				nWriteLPNCount;			// count of write request(on buffering or on programming)
	INT32				nWriteDoneLPNCount;		// count of write done LPN

	// Circular queue 
	INT32				nReadIndex;				// circular queue index (tail),
	INT32				nWriteIndex;			// circular queue index (head)
	BLOCK_GC_COPY		astCopyInfo[BLOCK_GC_COPY_ENTRY_COUNT];
												// Arrary for LPN information, circular queue
} BLOCK_GC_INFO;

#define GET_GC_MANAGER()					(&(g_pstStreamFTL->stGCMgr))
#define GET_STREAM_MERGE_INFO(_nInfoIndex)	(&GET_GC_MANAGER()->astMoveInfo[_nInfoIndex].stSMerge)
#define GET_BLOCK_GC_INFO(_nInfoIndex)		(&GET_GC_MANAGER()->astMoveInfo[_nInfoIndex].stBlockGC)
#define GET_STREAM_MERGE_INFO(_nInfoIndex)	(&GET_GC_MANAGER()->astMoveInfo[_nInfoIndex].stSMerge)
#define GET_CUR_STREAM_MERGE()				GET_STREAM_MERGE_INFO(GET_GC_MANAGER()->nCurMoveIndex)
#define GET_CUR_BLOCK_GC()					GET_BLOCK_GC_INFO(GET_GC_MANAGER()->nCurMoveIndex)

typedef struct
{
	unsigned int		bFree : 1;
	unsigned int		bSMerge : 1;		// just for debugging
	unsigned int		bBlockGC : 1;		// just for debugging
	unsigned int		nRsvd : 29;

	union
	{
		SMERGE_INFO		stSMerge;
		BLOCK_GC_INFO	stBlockGC;
	};
} MOVE_INFO;

#define GC_MANAGER_MOVE_INFO_COUNT		(LPN_PER_PHYSICAL_PAGE * 2/*bGC+sMerge*/)

typedef struct
{
	SMERGE_POLICY		eSMergePolicy;			// stream merge victim selection policy
	GC_TYPE				eCurGCType;

	// there are some rest write requests or buffering requests before start next sMerge or BlockGC
	// I reserved space for 4 sets of GC to avoid information corruption 
	// index of information is stored at Request ID
	INT32				nCurMoveIndex;			// index of astMoveInfo
	MOVE_INFO			astMoveInfo[GC_MANAGER_MOVE_INFO_COUNT];
} GC_MANAGER;

typedef struct
{
	STREAM_MANAGER			stStreamMgr;			// Stream information
	PARTITION_MANAGER		stPartitionMgr;			// Partition information
	ACTIVE_BLOCK_MANAGER	stActiveBlockMgr;		// Active Block information
	BLOCK_MANAGER			stBlockMgr;				// Block information
	L2P_CACHE				stL2PCache;				// L2P Cache
	HOTCOLD_MANAGER			stHotCold;				// Hot Cold Management
	GC_MANAGER				stGCMgr;				// GC Management
	BUFFER_POOL				stBufferPool;
	INT32					nVPC;					// Total Valid Page Count
} STREAMFTL_GLOBAL;

STREAMFTL_GLOBAL		*g_pstStreamFTL;

// data structure used for GC
// cuz StreamFTL GC needs sorting to reduce the # of partition
// we also need to examine the actual benefit from sorting
INT32* g_pnLPNsForGC;
INT32* g_pnVPPNsForGC;
INT32* g_pnPartitionForGC;

// function

INT32	StreamFTL_Read(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pMainBuf, void* pSpareBuf, IOTYPE eIOType, BOOL* pbBufferHit);
RETURN	StreamFTL_Write(FTL_REQUEST_ID stReqID, INT32 nLPN, void* pBuf, IOTYPE eIOType, UINT32 nHostCmdSlotTag, INT16 nDMAIndex);
INT32	StreamFTL_ReadVPPN(FTL_REQUEST_ID stReqID, INT32 nVPPN, INT32 * pMainBuf, void* pSpareBuf, IOTYPE eIOType);

VOID	StreamFTL_Format(VOID);
INT32	StreamFTL_GetMetaBlockCount(VOID);

float	StreamFTL_HotCold_GetOverWriteRatio(INT32 nPartition);
VOID	StreamFTL_HotCold_Update(INT32 nPartition, BOOL bOverWrite, IOTYPE eIOType);

#endif