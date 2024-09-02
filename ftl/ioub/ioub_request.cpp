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

#include "ioub_internal.h"

///////////////////////////////////////////////////////////////////////////////
//
//	REQUEST MANAGER
//
///////////////////////////////////////////////////////////////////////////////

VOID
REQUEST_MGR::Initialize(VOID)
{
	m_stHILRequestInfo.Initialize();
	m_stGCRequestInfo.Initialize();
}

VOID
REQUEST_MGR::Run(VOID)
{
	_ProcessDoneQ();
	_ProcessWaitQ();
}

VOID
REQUEST_MGR::_ProcessDoneQ(VOID)
{
	_ProcessHILRequestDoneQ();
	_ProcessGCRequestDoneQ();
}

VOID
REQUEST_MGR::_ProcessWaitQ(VOID)
{
	_ProcessGCRequestWaitQ();

	_ProcessHILRequestWaitQ();

}

VOID
REQUEST_MGR::_ProcessHILRequestWaitQ(VOID)
{
	HIL_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;
	static int way_counter = 0;
	static int test_counter = 0;
	int way, channel;
	int process_count = 0;
	int searched_ways = 0;
	int run_ways = 0;
	int total_ways = USER_CHANNELS * USER_WAYS;
	

	do {
		pstRequest = m_stHILRequestInfo.GetWaitRequest();
		if (pstRequest == NULL)
		{
			break;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

restart:
	

	way = way_counter >> CHANNEL_BITS;
	channel = way_counter % USER_CHANNELS;

	pstRequest = m_stHILRequestInfo.GetWaitRequest_per_way(channel, way);
	if (pstRequest != NULL)
	{
		bSuccess = pstRequest->Run();
		run_ways++;
		
		if (bSuccess == FALSE)
		{ //for NAND paralellism
			test_counter++;
		//	IOUB_GLOBAL::GetActiveBlockMgr(channel, way)->Background_work();
		}
	}
	else
	{
		//IOUB_GLOBAL::GetActiveBlockMgr(channel, way)->Background_work();
		bSuccess = FALSE;
	}
	if (FIL_IsEmpty_way(channel, way))
	{
		//IOUB_GLOBAL::GetActiveBlockMgr(channel, way)->Background_work();
	}
	searched_ways++;


	way_counter = (way_counter + 1) % (1 << (NUM_BIT_WAY + CHANNEL_BITS));
	if (++process_count < IOUB_REQUEST_PER_LOOP)
	{
			goto restart;
	}
	else
	{
		process_count = 0;
		if (run_ways == 0 && searched_ways < total_ways)
			goto restart;
	}

	for(channel = 0; channel < USER_CHANNELS; channel++) {
		for(way = 0; way < USER_WAYS; way++) {
			if(FIL_IsEmpty_way(channel, way)) {
				IOUB_GLOBAL::GetActiveBlockMgr(channel, way)->Background_work();
			}
		}
	}

	/*static int way = 0;


	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		pstRequest = m_stHILRequestInfo.GetWaitRequest_per_way(channel, way);
		if (pstRequest == NULL)
		{
			continue;
		}

		bSuccess = pstRequest->Run();

	}

	way = (way + 1) % USER_WAYS;
	*/


	return;
}

VOID
REQUEST_MGR::_ProcessHILRequestDoneQ(VOID)
{
	HIL_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;

	do
	{
		pstRequest = m_stHILRequestInfo.GetDoneRequest_per_way();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);


}

VOID
REQUEST_MGR::_ProcessGCRequestWaitQ(VOID)
{
	GC_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;

	do
	{
		pstRequest = m_stGCRequestInfo.GetWaitRequest();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

	return;
}

VOID
REQUEST_MGR::_ProcessGCRequestDoneQ(VOID)
{
	GC_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;

	do
	{
		pstRequest = m_stGCRequestInfo.GetDoneRequest();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

	return;
}

VOID
REQUEST_MGR::_ProcessMetaRequestWaitQ(VOID)
{
	META_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;

	do
	{
		pstRequest = m_stMetaRequestInfo.GetWaitRequest();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

	return;
}

VOID
REQUEST_MGR::_ProcessMetaRequestDoneQ(VOID)
{
	META_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;

	do
	{
		pstRequest = m_stMetaRequestInfo.GetDoneRequest();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

	return;
}

