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

#ifndef __DFTL_REQUEST_HIL_H__
#define __DFTL_REQUEST_HIL_H__

typedef enum
{
	HIL_REQUEST_FREE,

	HIL_REQUEST_READ_WAIT,			// At the wait list
	HIL_REQUEST_READ_NAND_ISSUED,
	HIL_REQUEST_READ_DONE,

	HIL_REQUEST_WRITE,
	HIL_REQUEST_WRITE_WAIT,
} HIL_REQUEST_STATUS;

#define REQUEST_LPN_COUNT_MAX		256			// 1MB

class HIL_REQUEST
{
public:
	VOID Initialize(HIL_REQUEST_STATUS nStatus, NVME_CMD_OPCODE nCmd, UINT32 nLPN,
		UINT32 nHostCmdSlotTag, INT32 nLPNCount);
	BOOL Run(VOID);

	VOID SetStatus(HIL_REQUEST_STATUS nStatus)	{ m_nStatus = nStatus; }
	HIL_REQUEST_STATUS GetStatus(VOID) { return m_nStatus; }
	VOID SetCmd(NVME_CMD_OPCODE nCmd) { m_nCmd = nCmd; }
	VOID SetLPN(UINT32 nLPN) { m_nLPN = nLPN; }
	UINT32 GetStartLPN(VOID) { return m_nLPN; }
	UINT32 GetCurLPN(VOID) { return (m_nLPN + m_nIssued_Count); }
	VOID SetHostCmdSlotTag(UINT32 nTag) { m_nHostCmdSlotTag = nTag; }
	VOID SetLPNCount(INT32 nCount) { m_nLPNCount = nCount; }
	INT32 GetLPNCount(VOID) { return m_nLPNCount; }
	VOID SetRequestIndex(INT32 nIndex) { m_nRequestIndex = nIndex; }
	VOID GoToNextStatus(VOID) { m_nStatus = (HIL_REQUEST_STATUS)(m_nStatus + 1); }
	UINT32 GetDoneCount(VOID) { return m_nDoneLPNCount; }
	VOID IncreaseDoneCount(VOID) { m_nDoneLPNCount++; }

	UINT32 GetHostCmdSlotTag(VOID) {return m_nHostCmdSlotTag;}
	

	VOID IncIssued_count(VOID) { m_nIssued_Count++; }
	INT32 GetIssued_count() { return m_nIssued_Count; }
	


	BOOL _CheckAndLoadMeta(VOID);
	BOOL _CheckMetaWritable(VOID);

	BOOL _ProcessRead(VOID);
	virtual BOOL _ProcessRead_Wait(VOID);
	virtual BOOL _ProcessRead_Issued(VOID);
	virtual BOOL _ProcessRead_Done(VOID);



	BOOL _IsWritable(UINT32 channel, UINT32 way);
	BOOL _ProcessWrite(VOID);
	virtual BOOL _ProcessWrite_Wait(VOID);

	FTL_REQUEST_ID	_GetRquestID(VOID);

	VOID SetHDMAIssueInfo(UINT8 nDMAReqTail, UINT32 nOverFlowCount)
	{
		m_nDMAReqTail = nDMAReqTail;
		m_nDMAOverFlowCount = nOverFlowCount;
	}

public:
	struct list_head	m_dlList;				// list for queueing
												// public for macro operation
	struct list_head	m_dlBuffer;				// read buffer list

protected:
	// HDMA information (only for last issued)
	unsigned int		m_nDMAReqTail : 8;		// DMA tail request, to check HDMA done
	unsigned int		m_nDMAOverFlowCount;	// count of DMA index overflow, to check HDMA done

private:
	HIL_REQUEST_STATUS	m_nStatus;
	NVME_CMD_OPCODE		m_nCmd;					// bool 0:Read, 1:Write, assign 1 more  bit for signed enum type
												// Read & Write

	INT32				m_nLPN;					// LPN
	UINT16				m_nLPNCount;			// count of read LPN
	
	INT32				m_nIssued_Count;
	
	UINT32				m_nHostCmdSlotTag;		// SQ Tag from HIL, This is used for HDMA
	UINT16				m_nDoneLPNCount;		// LPN Count for (read done) or (write buffered)
												// this is HDMA offset

	unsigned int		m_nRequestIndex:FTL_REQUEST_ID_REQUEST_INDEX_BITS;
												// index of HIL_REQUEST_MGR::m_astRequestPool
	
};

class HIL_REQUEST_PER_WAY : public HIL_REQUEST {
public:
	VOID Initialize_per_way(HIL_REQUEST_STATUS nStatus, NVME_CMD_OPCODE nCmd, UINT32 nLPN,
		UINT32 nHostCmdSlotTag, INT32 nLPNCount, HIL_REQUEST *parent);
	VOID SetChannel(INT32 channel) { m_channel = channel; }
	INT32 GetChannel() { return m_channel; }

	UINT16 GetHDMAOffset(VOID) { return m_nDMA_offset; }

	VOID SetWay(INT32 channel) { m_way = channel; }
	INT32 GetWay() { return m_way; }

	virtual BOOL _ProcessRead_Wait(VOID);
	BOOL	HDMAIssue(VOID);
	virtual BOOL _ProcessRead_Done(VOID);

	virtual BOOL _ProcessWrite_Wait(VOID);


	BUFFER_ENTRY	*pBufEntry[LPN_PER_PHYSICAL_PAGE];
private:
	INT32 m_channel;
	INT32 m_way;
	HIL_REQUEST *m_parent_req;
	UINT16 m_nDMA_offset;

	UINT32 read_VPPN[LPN_PER_PHYSICAL_PAGE];
	//for read
	//BUFFER_ENTRY	*pBufEntry;
	//INT8			*BufOffset;
	
	INT8			BufOffset[LPN_PER_PHYSICAL_PAGE];
};


#define HIL_REQUEST_COUNT			(HOST_REQUEST_BUF_COUNT)
#define GC_REQUEST_COUNT			(NSC_MAX_CHANNELS * USER_WAYS * LPN_PER_PHYSICAL_PAGE * 2 /* Meta & GC */ * 2 /* double buffering */)

class HIL_REQUEST_INFO
{
public:
	VOID Initialize(VOID);
	HIL_REQUEST* AllocateRequest(VOID);
	HIL_REQUEST_PER_WAY* AllocateRequest_per_way();
	VOID ReleaseRequest(HIL_REQUEST* pstRequest);
	VOID ReleaseRequest_per_way(HIL_REQUEST_PER_WAY* pstRequest);

	VOID AddToWaitQ(HIL_REQUEST* pstRequest);
	VOID RemoveFromWaitQ(HIL_REQUEST* pstRequest);

	VOID AddToIssuedQ(HIL_REQUEST* pstRequest);
	VOID RemoveFromIssuedQ(HIL_REQUEST* pstRequest);

	VOID AddToDoneQ(HIL_REQUEST* pstRequest);
	VOID RemoveFromDoneQ(HIL_REQUEST* pstRequest);

	VOID AddToHDMAIssuedQ(HIL_REQUEST* pstRequest);
	VOID RemoveFromHDMAIssuedQ(HIL_REQUEST* pstRequest);


	VOID AddToWaitQ_per_way(HIL_REQUEST_PER_WAY* pstRequest, INT32 channel, INT32 way);
	VOID RemoveFromWaitQ_per_way(HIL_REQUEST_PER_WAY* pstRequest, INT32 channel, INT32 way);
	HIL_REQUEST* GetWaitRequest_per_way(INT32 channel, INT32 way);

	VOID AddToIssuedQ_per_way(HIL_REQUEST_PER_WAY* pstRequest);
	VOID RemoveFromIssuedQ_per_way(HIL_REQUEST_PER_WAY* pstRequest);
	HIL_REQUEST* GetIssuedRequest_per_way(VOID);


	VOID AddToDoneQ_per_way(HIL_REQUEST_PER_WAY* pstRequest);
	VOID RemoveFromDoneQ_per_way(HIL_REQUEST_PER_WAY* pstRequest);
	HIL_REQUEST* GetDoneRequest_per_way(VOID);

	HIL_REQUEST* GetWaitRequest(VOID);
	HIL_REQUEST* GetIssuedRequest(VOID);
	HIL_REQUEST* GetDoneRequest(VOID);
	HIL_REQUEST* GetHDMARequest(VOID);

	HIL_REQUEST* GetRequest(INT32 nIndex) { return &m_astRequestPool[nIndex]; }
	HIL_REQUEST_PER_WAY* GetRequest_per_way(INT32 nIndex) { return &m_astRequestPerWayPool[nIndex]; }

private:
	INT32 _GetRequestIndex(HIL_REQUEST* pstRequest);
	INT32 _GetRequestIndex_per_way(HIL_REQUEST_PER_WAY* pstRequest);

private:
	HIL_REQUEST				m_astRequestPool[HIL_REQUEST_COUNT];
	HIL_REQUEST_PER_WAY		m_astRequestPerWayPool[HIL_REQUEST_COUNT];

	struct list_head		m_dlFree;				// free NVMeRequest
	INT32					m_nFreeCount;			// count of free request

	struct list_head		m_dlWait;				// NAND issue wait queue
	INT32					m_nWaitCount;

	struct list_head		m_dlIssued;			// issued queue
	INT32					m_nIssuedCount;

	struct list_head		m_dlHDMA;				// host DMA issued list
	INT32					m_nHDMACount;

	struct list_head		m_dlDone;				// Read done or Program done
	INT32					m_nDoneCount;



	struct list_head	m_dlFree_per_way;
	INT32				m_dlFreeCount_per_way;

	struct list_head	m_dlWait_per_way[USER_CHANNELS][USER_WAYS];
	INT32				m_nWaitCount_per_way[USER_CHANNELS][USER_WAYS];


	struct list_head	m_dlIssued_per_way;
	INT32				m_nIssuedCount_per_way;

	struct list_head	m_dlDone_per_way;
	INT32				m_nDoneCount_per_way;

};

#endif
