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

#include "dftl_internal.h"

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

#if (SUPPORT_META_DEMAND_LOADING == 1)
	m_stMetaRequestInfo.Initialize();
#endif
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

#if (SUPPORT_META_DEMAND_LOADING == 1)
	_ProcessMetaRequestDoneQ();
#endif
}


VOID
REQUEST_MGR::_ProcessWaitQ(VOID)
{
#if (SUPPORT_META_DEMAND_LOADING == 1)
	if (DFTL_GLOBAL::GetInstance()->isMetaGCing() == FALSE) {
		_ProcessMetaRequestWaitQ();
	}
#endif
	_ProcessGCRequestWaitQ();
	if (DFTL_GLOBAL::GetInstance()->isMetaGCing() == FALSE) {
		_ProcessHILRequestWaitQ();
	}
}

VOID
REQUEST_MGR::_ProcessHILRequestWaitQ(VOID)
{
	HIL_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;
	static int way_counter = 0;
	int way, channel;
	int process_count = 0;
	int real_process_count = 0;
	static int total_pu = 0;

	if (total_pu == 0)
		total_pu = USER_WAYS * USER_CHANNELS;

restart:

	way = way_counter >> CHANNEL_BITS;
	channel = way_counter % USER_CHANNELS;


	pstRequest = m_stHILRequestInfo.GetWaitRequest_per_way(channel, way);
	if (pstRequest != NULL)
	{
		bSuccess = pstRequest->Run();
		real_process_count++;
	}

	way_counter = (way_counter + 1) % (1 << (NUM_BIT_WAY + CHANNEL_BITS));
	if (++process_count < total_pu && real_process_count < DFTL_REQUEST_PER_LOOP)
		goto restart;


	//do {
	pstRequest = m_stHILRequestInfo.GetWaitRequest();
	if (pstRequest == NULL)
	{
		return;
	}


	bSuccess = pstRequest->Run();
	//} while (bSuccess == TRUE);



	return;

/*	HIL_REQUEST*	pstRequest;
	BOOL bSuccess = FALSE;
	static int way_counter = 0;
	int way, channel;
	int process_count = 0;
	int real_process = 0;

	for(way = 0; way < USER_WAYS; way++)
	{
		for(channel = 0; channel < USER_CHANNELS; channel++)
		{
			pstRequest = m_stHILRequestInfo.GetWaitRequest_per_way(channel, way);
			if (pstRequest != NULL)
			{
				bSuccess = pstRequest->Run();
			}
		}
	}

	do {
	pstRequest = m_stHILRequestInfo.GetWaitRequest();
	if (pstRequest == NULL)
	{
		return;
	}
	//	if (m_stHILRequestInfo.TooBusy())
	//	return;

	bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

	return;*/
}

VOID
REQUEST_MGR::_ProcessHILRequestDoneQ(VOID)
{
	HIL_REQUEST*	pstRequest;
	BOOL bSuccess;

	do
	{
		pstRequest = m_stHILRequestInfo.GetDoneRequest_per_way();
		if (pstRequest == NULL)
		{
			break;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);
}

VOID
REQUEST_MGR::_ProcessGCRequestWaitQ(VOID)
{
	GC_REQUEST*	pstRequest;
	BOOL bSuccess;

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
REQUEST_MGR::_ProcessHDMARequestIssuedQ(VOID)
{
	HIL_REQUEST*	pstRequest;
	BOOL bSuccess;

	do
	{
		pstRequest = m_stHILRequestInfo.GetHDMARequest();
		if (pstRequest == NULL)
		{
			// Nothing to do
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);
}

VOID
REQUEST_MGR::_ProcessGCRequestDoneQ(VOID)
{
	GC_REQUEST*	pstRequest;
	BOOL bSuccess;

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
	BOOL bSuccess;
#if (META_PER_WAY==1)
	static UINT32 way = 0;
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {

		pstRequest = m_stMetaRequestInfo.GetWaitRequest(channel, way);
		if (pstRequest == NULL)
		{
			continue;
		}

		bSuccess = pstRequest->Run();

	}
	way = (way + 1) % USER_WAYS;
#else
	do
	{
		pstRequest = m_stMetaRequestInfo.GetWaitRequest();
		if (pstRequest == NULL)
		{
			return;
		}

		bSuccess = pstRequest->Run();
	} while (bSuccess == TRUE);

#endif
	return;
}

VOID
REQUEST_MGR::_ProcessMetaRequestDoneQ(VOID)
{
	META_REQUEST*	pstRequest;
	BOOL bSuccess;

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

