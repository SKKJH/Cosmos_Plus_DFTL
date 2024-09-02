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

#ifndef __IOUB_GLOBAL_H__
#define __IOUB_GLOBAL_H__


#define get_channel_from_lpn(lpn)	(((lpn) >> NUM_BIT_LPN_PER_PAGE) % USER_CHANNELS)
#define get_way_from_lpn(lpn)		(((lpn) >> (NUM_BIT_LPN_PER_PAGE + CHANNEL_BITS)) % USER_WAYS)
#define get_lbn_from_lpn(lpn)		((lpn) >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_WAY + CHANNEL_BITS + NUM_BIT_PPAGE))
#define get_lpn_from_lbn(lbn)		((lbn) << (NUM_BIT_LPN_PER_PAGE + NUM_BIT_WAY + CHANNEL_BITS + NUM_BIT_PPAGE))

//IOUB ADDRESS
#define IOUB_START_ADDR 8224 // excpet partition table
#define IOUB_ADDR_CNT   (16 * 1024 * 2 - IOUB_START_ADDR) // use first 16MB space

#define CMD_TRANSFER_UNIT       32
#define ADDR_PRINT_PROFILE		(IOUB_START_ADDR + CMD_TRANSFER_UNIT * 2) // 8288
#define ADDR_CLOSE_SECTION      (IOUB_START_ADDR + CMD_TRANSFER_UNIT * 4) // 8352
#define ADDR_INTERNAL_MERGE     (IOUB_START_ADDR + CMD_TRANSFER_UNIT * 6) // 8416

#if (IOUB_STRIPING == 1)
#define ADDR_SET_STRIPING		(IOUB_START_ADDR + CMD_TRANSFER_UNIT * 8) // 8480
#define ADDR_FREE_STRIPING		(IOUB_START_ADDR + CMD_TRANSFER_UNIT * 10) // 8544
#endif

static UINT32 IOUB_partition_start = 0;
//static UINT32 IOUB_partition_PMsize[IOUB_PARTITION_NO] = {114688};
//#define IOUB_BLOCK_START_LBN		(114688 >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_WAY + CHANNEL_BITS + NUM_BIT_PPAGE))
static UINT32 IOUB_partition_PMsize = 49152;
#define IOUB_BLOCK_START_LBN		(49152 >> (NUM_BIT_LPN_PER_PAGE + NUM_BIT_WAY + CHANNEL_BITS + NUM_BIT_PPAGE))

#define IOUB_MAX_IM_CNT			(512)

#if (IOUB_STRIPING==1)
#define SEGS_PER_SECTION					(4)
#define NUM_BIT_SEGS_PER_SECTION			(2)
#endif


#define READ_CAHCE_PER_WAY_BIT		(4)
#define READ_CACHE_PER_WAY			(16)
#define MAX_READ_CACHE_ENTRY		(USER_CHANNELS * USER_WAYS * READ_CACHE_PER_WAY)
class Read_Cache
{
public:
	VOID	Initialize();
	UINT32 get_next_buffer(UINT32 src_lpn);
	BUFFER_ENTRY *change_next_buffer(UINT32 src_lpn, BUFFER_ENTRY* input_buf);
	UINT32 get_buffer(UINT32 src_lpn);
	BUFFER_ENTRY *get_buffer_by_idx(UINT32 bufIdx);
	VOID	set_buffer_readDone(UINT32 bufIdx);
	BUFFER_ENTRY *get_buffer_by_VPPN(UINT32 nVPPN_input);
	VOID free_buffer_by_VPPN(UINT32 nVPPN_input);
	VOID free_buffer_by_Idx(UINT32 bufIdx);
	BOOL has_free_buffer(UINT32 src_lpn);
private:
	UINT32 source_lpn[MAX_READ_CACHE_ENTRY];
	UINT32 nVPPN[MAX_READ_CACHE_ENTRY];
	BUFFER_ENTRY* Buf[MAX_READ_CACHE_ENTRY];
};


class FTL_INTERFACE;
class VNAND;
class META_MGR;

class PROFILE
{
public:
	VOID Initialize(VOID)
	{
		for (INT32 i = 0; i < PROFILE_COUNT; i++)
		{
			m_astStatistics[i].nType = static_cast<PROFILE_TYPE>(i);
			m_astStatistics[i].nCount = 0;
		}
	}

	VOID IncreaseCount(PROFILE_TYPE eType)
	{
		IncreaseCount(eType, 1);
	}

	VOID IncreaseCount(PROFILE_TYPE eType, UINT32 nCount)
	{
		m_astStatistics[eType].nCount += nCount;
	}

	UINT32 GetCount(PROFILE_TYPE eType)
	{
		return m_astStatistics[eType].nCount;
	}

	VOID Print(VOID)
	{
		static const char *apsProfileString[] =
		{
			FOREACH_PROFILE(GENERATE_STRING)
		};

		for (int i = 0; i < PROFILE_COUNT; i++)
		{
			PRINTF("%s, %d\n\r", apsProfileString[i], GetCount((PROFILE_TYPE)i));
		}
	}


private:
	PROFILE_ENTRY	m_astStatistics[PROFILE_COUNT];
};

typedef enum
{
	IOUB_STATUS_NONE		= 0x00,
	IOUB_STATUS_META_IO		= 0x01,			// meta loading / storing
	IOUB_STATUS_FORMATTING	= 0x02,			// now formatting
} IOUB_STATUS;

class IOUB_GLOBAL : public FTL_INTERFACE
{
public:
	//IOUB_GLOBAL() {};

	VIRTUAL VOID Initialize(VOID);
	VIRTUAL BOOL Format(VOID);
	VIRTUAL VOID Run(VOID);
	VIRTUAL VOID ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VIRTUAL VOID WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VIRTUAL VOID CallBack(FTL_REQUEST_ID stReqID);
	VIRTUAL VOID IOCtl(IOCTL_TYPE eType);

	static IOUB_GLOBAL*		GetInstance(VOID) { return m_pstInstance; }
	static VNAND*			GetVNandMgr(VOID) { return &m_pstInstance->m_stVNand; }
	static BLOCK_MGR*		GetUserBlockMgr(VOID) { return &m_pstInstance->m_stUserBlockMgr; }

	static VBINFO_MGR*		GetVBInfoMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stVBInfoMgr[channel][way]; }
	static REQUEST_MGR*		GetRequestMgr(VOID)	{return &m_pstInstance->m_stRequestMgr;}
	static GC_MGR*			GetGCMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stGCMgr[channel][way]; }

	static ACTIVE_BLOCK_MGR* GetActiveBlockMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stActiveBlockMgr[channel][way]; }
	static BUFFERING_LPN*	GetActiveBlockBufferingLPN(UINT32 channel, UINT32 way, IOTYPE type) { return m_pstInstance->m_stActiveBlockMgr[channel][way].GetBufferingMgr(type); }

	static BUFFERING_LPN* GetMetaActiveBlockBufferingLPN(UINT32 channel, UINT32 way) { return m_pstInstance->m_stActiveBlockMgr[channel][way].GetMetaBufferingMgr(); }

	static META_MGR*		GetMetaMgr(METATYPE type) { return &m_pstInstance->m_stMetaMgr[type]; }
	//static META_L2V_MGR*	GetMetaL2VMgr(METATYPE type) { return &m_pstInstance->m_stMetaL2VMgr[type]; }
	static BUFFER_MGR*		GetBufferMgr()	{return &m_pstInstance->m_stBufferMgr;}
	static HDMA*			GetHDMAMgr(VOID)	{return &m_pstInstance->m_stHostDMA;}

	static Read_Cache*		GetIMBufferMgr() { return &m_pstInstance->m_stIMbuffer; }
	static Read_Cache*		GetReadCacheMgr() {	return &m_pstInstance->m_stReadCache;}

#if (IOUB_STRIPING)
	static STRIPING_INFO*	GetStrinpingMgr(VOID) {return &m_pstInstance->m_Striping_Info;}
#endif

	UINT32 GetVPagePerVBlock(VOID) {return m_nVPagesPerVBlock;}

	UINT32 GetLPNCount(VOID) {return m_nLPNCount;}
	static BOOL	isBlockMappingLPN(UINT32 nLPN);
	BOOL	IsValidLPN(UINT32 nLPN)
	{
		return (nLPN < m_nLPNCount) ? TRUE : FALSE;
	}

	VOID IncreaseProfileCount(PROFILE_TYPE eType)
	{
		m_stProfile.IncreaseCount(eType);
	}

	VOID IncreaseProfileCount(PROFILE_TYPE eType, UINT32 nCount)
	{
		m_stProfile.IncreaseCount(eType, nCount);
	}

	UINT32 GetProfileCount(PROFILE_TYPE eType)
	{
		return m_stProfile.GetCount(eType);
	}

	UINT32 GetGCTh(VOID) { return m_nGCTh; }

	VOID SetStatus(IOUB_STATUS eStatus);
	BOOL CheckStatus(IOUB_STATUS eStatus);

	BOOL	IM_start;
	BOOL	Initialized_for_test;
private:		// fuctions
	VOID _Initialize(VOID);

private:
	VOID _PrintInfo(VOID);

	static IOUB_GLOBAL*	m_pstInstance;

	VNAND				m_stVNand;				// virtual nand module
	META_MGR			m_stMetaMgr[METATYPE_COUNT];			// meta moudle
	META_L2V_MGR		m_stMetaL2VMgr[METATYPE_COUNT];			// met data location
	BLOCK_MGR			m_stUserBlockMgr;		// block module

	VBINFO_MGR			m_stVBInfoMgr[USER_CHANNELS][USER_WAYS];			// Virtual Information Manager
	REQUEST_MGR			m_stRequestMgr;			// read/write request manager
	GC_MGR				m_stGCMgr[USER_CHANNELS][USER_WAYS];				// garbage collector

	ACTIVE_BLOCK_MGR	m_stActiveBlockMgr[USER_CHANNELS][USER_WAYS];
	BUFFER_MGR			m_stBufferMgr;
	HDMA				m_stHostDMA;

	Read_Cache			m_stIMbuffer;
	Read_Cache			m_stReadCache;

#if (IOUB_STRIPING==1)
	STRIPING_INFO		m_Striping_Info;
#endif


	IOUB_STATUS	m_eStatus;

	PROFILE		m_stProfile;

	UINT32		m_nPhysicalFlashSizeKB;
	UINT32		m_nVBlockSizeKB;
	UINT32		m_nVPagesPerVBlock;
	UINT32		m_nLPagesPerVBlockBits;
	UINT32		m_nLPagesPerVBlockMask;

	float		m_fOverProvisionRatio;
	UINT32		m_nOverprovisionSizeKB;
	UINT32		m_nLogicalFlashSizeKB;

	UINT32		m_nLPNCount;
	UINT32		m_nVBlockCount;

	UINT32		m_nGCTh;				// free block count for GC trigger

	BOOL		m_bEnableMetaBlock;

	BOOL		m_MetaGCing;


};

#endif		// end of #ifndef __IOUB_H__
