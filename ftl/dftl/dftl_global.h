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

#ifndef __DFTL_GLOBAL_H__
#define __DFTL_GLOBAL_H__


#define get_channel_from_lpn(lpn)	(((lpn) >> NUM_BIT_LPN_PER_PAGE) % USER_CHANNELS)
#ifdef CH_WY_FLAX
#define get_way_from_lpn(lpn)		((((lpn) >> NUM_BIT_LPN_PER_PAGE) / USER_CHANNELS) % USER_WAYS)
#define get_page_from_lpn(lpn)		((((lpn) >> NUM_BIT_LPN_PER_PAGE) / (USER_CHANNELS * USER_WAYS)) & NAND_ADDR_PPAGE_MASK)
#define get_lbn_from_lpn(lpn)		(((((lpn) >> NUM_BIT_LPN_PER_PAGE) / (USER_CHANNELS * USER_WAYS)) >> (NUM_BIT_PPAGE)) & NAND_ADDR_BLOCK_MASK)
#else
#define get_way_from_lpn(lpn)		((((lpn) >> NUM_BIT_LPN_PER_PAGE) >> (CHANNEL_BITS)) % USER_WAYS)
#define get_page_from_lpn(lpn)		((((lpn) >> NUM_BIT_LPN_PER_PAGE) >> (CHANNEL_BITS + NUM_BIT_WAY)) & NAND_ADDR_PPAGE_MASK)
#define get_lbn_from_lpn(lpn)		((((lpn) >> NUM_BIT_LPN_PER_PAGE) >> (CHANNEL_BITS + NUM_BIT_WAY + NUM_BIT_PPAGE)) & NAND_ADDR_BLOCK_MASK)
#endif

#define BLOCKS_PER_MAP_PAGES		(L2V_PER_META_PAGE >> NUM_BIT_VPAGE))		//4096 div 512
#define BLOCKS_PER_MAP_BITS			(3)
#define BLOCKS_PER_MAP_MASKS		((1 << BLOCKS_PER_MAP_BITS) - 1)

#ifdef CH_WY_FLAX
//Channel -> Way -> Block -> Page -> Offset
#define get_mod_lpn_from_lpn_lbn(nCh, nWy, nBlock, nPage) ((nPage << NUM_BIT_LPN_PER_PAGE)  \
											+ ((nBlock & BLOCKS_PER_MAP_MASKS) << (NUM_BIT_VPAGE)) \
											+ (nWy << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS)) \
											+ ((nCh << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS)) * USER_WAYS) \
											+ (((nBlock >> BLOCKS_PER_MAP_BITS) << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS)) * USER_CHANNELS * USER_WAYS))
#else
#define get_mod_lpn_from_lpn_lbn(nCh, nWy, nBlock, nPage) ((nPage << NUM_BIT_LPN_PER_PAGE)  \
											+ ((nBlock & BLOCKS_PER_MAP_MASKS) << (NUM_BIT_VPAGE)) \
											+ (nWy << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS)) \
											+ (nCh << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS + NUM_BIT_WAY)) \
											+ ((nBlock >> BLOCKS_PER_MAP_BITS) << (NUM_BIT_VPAGE + BLOCKS_PER_MAP_BITS + CHANNEL_BITS + NUM_BIT_WAY)))
#endif

#define ADDR_PRINT_PROFILE		(8288)

#define READ_CAHCE_PER_WAY_BIT		(3)
#define READ_CACHE_PER_WAY			(8)
#define MAX_READ_CACHE_ENTRY		(USER_CHANNELS * USER_WAYS * READ_CACHE_PER_WAY)
class Read_Cache
{
public:
	VOID	Initialize();
	BUFFER_ENTRY *change_next_buffer(UINT32 src_lpn, BUFFER_ENTRY* input_buf);
	BUFFER_ENTRY *get_buffer_by_VPPN(UINT32 nVPPN_input);
	VOID free_buffer_by_VPPN(UINT32 nVPPN_input);
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
	DFTL_STATUS_NONE		= 0x00,
	DFTL_STATUS_META_IO		= 0x01,			// meta loading / storing
	DFTL_STATUS_FORMATTING	= 0x02,			// now formatting
} DFTL_STATUS;

class DFTL_GLOBAL : public FTL_INTERFACE
{
public:
	//DFTL_GLOBAL() {};

	VIRTUAL VOID Initialize(VOID);
	VIRTUAL BOOL Format(VOID);
	VIRTUAL VOID Run(VOID);
	VIRTUAL VOID ReadPage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VIRTUAL VOID WritePage(UINT32 nCmdSlotTag, UINT32 nLPN, UINT32 nCount);
	VIRTUAL VOID CallBack(FTL_REQUEST_ID stReqID);
	VIRTUAL VOID IOCtl(IOCTL_TYPE eType);

	static DFTL_GLOBAL*		GetInstance(VOID) { return m_pstInstance; }
	static VNAND*			GetVNandMgr(VOID) { return &m_pstInstance->m_stVNand; }
	static BLOCK_MGR*		GetUserBlockMgr(VOID) { return &m_pstInstance->m_stUserBlockMgr; }
#if (SUPPORT_META_DEMAND_LOADING == 1)
	static BLOCK_MGR*		GetMetaBlockMgr(VOID) { return &m_pstInstance->m_stMetaBlockMgr; }
#endif
	static VBINFO_MGR*		GetVBInfoMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stVBInfoMgr[channel][way]; }
	static REQUEST_MGR*		GetRequestMgr(VOID)	{return &m_pstInstance->m_stRequestMgr;}
	static GC_MGR*			GetGCMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stGCMgr[channel][way]; }
#if (SUPPORT_META_DEMAND_LOADING == 1)
	static GC_MGR*			GetMetaGCMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stMetaGCMgr[channel][way]; }
#endif
	static ACTIVE_BLOCK_MGR* GetActiveBlockMgr(UINT32 channel, UINT32 way) { return &m_pstInstance->m_stActiveBlockMgr[channel][way]; }
	static BUFFERING_LPN*	GetActiveBlockBufferingLPN(UINT32 channel, UINT32 way) { return m_pstInstance->m_stActiveBlockMgr[channel][way].GetBufferingMgr(); }

	static BUFFERING_LPN* GetMetaActiveBlockBufferingLPN(UINT32 channel, UINT32 way) { return m_pstInstance->m_stActiveBlockMgr[channel][way].GetMetaBufferingMgr(); }

	static META_MGR*		GetMetaMgr(VOID) { return &m_pstInstance->m_stMetaMgr; }
	static META_L2V_MGR*	GetMetaL2VMgr(VOID) { return &m_pstInstance->m_stMetaL2VMgr; }
	static BUFFER_MGR*		GetBufferMgr()	{return &m_pstInstance->m_stBufferMgr;}
	static HDMA*			GetHDMAMgr(VOID)	{return &m_pstInstance->m_stHostDMA;}

	static Read_Cache*		GetReadCacheMgr() { return &m_pstInstance->m_stReadCache; }

	UINT32 GetVPagePerVBlock(VOID) {return m_nVPagesPerVBlock;}

	UINT32 GetLPNCount(VOID) {return m_nLPNCount;}
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

	VOID SetStatus(DFTL_STATUS eStatus);
	BOOL CheckStatus(DFTL_STATUS eStatus);

	VOID SetMetaGCing() { m_MetaGCing = TRUE; }
	VOID ClearMetaGCing() { m_MetaGCing = FALSE; }
	BOOL isMetaGCing() { return m_MetaGCing;}

private:		// fuctions
	VOID _Initialize(VOID);

private:
	VOID _PrintInfo(VOID);

	static DFTL_GLOBAL*	m_pstInstance;

	VNAND				m_stVNand;				// virtual nand module
	META_MGR			m_stMetaMgr;			// meta moudle
	META_L2V_MGR		m_stMetaL2VMgr;			// met data location
	BLOCK_MGR			m_stUserBlockMgr;		// block module

	BLOCK_MGR			m_stMetaBlockMgr;		// block module

	VBINFO_MGR			m_stVBInfoMgr[USER_CHANNELS][USER_WAYS];			// Virtual Information Manager
	REQUEST_MGR			m_stRequestMgr;			// read/write request manager
	GC_MGR				m_stGCMgr[USER_CHANNELS][USER_WAYS];				// garbage collector
#if (SUPPORT_META_DEMAND_LOADING == 1)
	GC_MGR				m_stMetaGCMgr[USER_CHANNELS][USER_WAYS];			// meta garbage collector
#endif
	ACTIVE_BLOCK_MGR	m_stActiveBlockMgr[USER_CHANNELS][USER_WAYS];
	BUFFER_MGR			m_stBufferMgr;
	HDMA				m_stHostDMA;

	DFTL_STATUS	m_eStatus;

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

	Read_Cache			m_stReadCache;

};

#endif		// end of #ifndef __DFTL_H__