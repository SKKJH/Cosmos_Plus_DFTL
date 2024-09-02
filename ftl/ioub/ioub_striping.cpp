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
* Author: Kyuhwa Han (hgh6877@gmail.com)
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/

#include "ioub_internal.h"


///////////////////////////////////////////////////////////////////////////////
//
//	SEGMENT_LIST
//
///////////////////////////////////////////////////////////////////////////////
#if (IOUB_STRIPING == 1)
VOID SEGMENT_LIST::Initialize(UINT32 vsn, UINT32 lsn)
{
	m_vsn = vsn;
	m_lsn = lsn;

	INIT_LIST_HEAD(&m_dlList);
	return;
}

VOID SEGMENT_LIST::Update_mapping(UINT32 vsn, UINT32 lsn)
{
	m_vsn = vsn;
	m_lsn = lsn;
	return;
}

VOID SEGMENT_LIST::Release()
{
	UINT32 channel, way;
	channel = get_channel_from_lsn(m_lsn);
	way = get_way_from_lsn(m_lsn);

	list_del_init(&m_dlList);
	list_add_tail(&m_dlList, &IOUB_GLOBAL::GetStrinpingMgr()->m_unallocated_seg[channel][way]);
	return VOID();
}



///////////////////////////////////////////////////////////////////////////////
//
//	STRIPING_INFO
//
///////////////////////////////////////////////////////////////////////////////

VOID STRIPING_INFO::Initialize()
{
	for (int channel = 0; channel < USER_CHANNELS; channel++) {
		for (int way = 0; way < USER_WAYS; way++) {
			INIT_LIST_HEAD(&m_unallocated_seg[channel][way]);

			for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {
				UINT32 lsn, vsn, off;
				off = (way << CHANNEL_BITS) + channel;
				vsn = vsn_from_ch_wy_blk(channel, way, block);
				lsn = lsn_from_ch_wy_blk_sec_off(channel, way, block, 16-1, off);
				segment_struct[channel][way][block].Initialize(vsn, lsn);
				segment_struct[channel][way][block].Release();

				IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, 0xffffffff);
			}
		}
	}
	//Synthetic Test
	UINT32 test_lsn = 0;
	
		
	for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {
		int channel = 0;
		for (int way = 0; way < USER_WAYS; way++) {
			UINT32 lsn, vsn, off;
			off = (way << CHANNEL_BITS) + channel;
			vsn = test_lsn++;
			lsn = ((block << (CHANNEL_BITS + NUM_BIT_WAY))  + (way << CHANNEL_BITS) + channel) << NUM_BIT_SEC_SIZE;
			lsn = (lsn + 2 - 1) << NUM_BIT_STRIPING_OFFSET;
			lsn += way;

			segment_struct[channel][way][block].Update_mapping(vsn, lsn);

			IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, lsn);
		}
	}

	
		
	for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {
		int channel = 1;
		for (int way = 0; way < USER_WAYS; way++) {
			UINT32 lsn, vsn, off;
			off = (way << CHANNEL_BITS) + channel;
			vsn = test_lsn++;
			lsn = ((block << (CHANNEL_BITS + NUM_BIT_WAY)) +  (way << CHANNEL_BITS) + channel) << NUM_BIT_SEC_SIZE;
			lsn = (lsn + 2 - 1) << NUM_BIT_STRIPING_OFFSET;
			lsn += way;

			segment_struct[channel][way][block].Update_mapping(vsn, lsn);

			IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, lsn);
		}
	}
	for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {
		for (int way = 0; way < USER_WAYS; way++) {
			for (int channel = 2; channel < USER_CHANNELS; channel++) {

				UINT32 lsn, vsn, off;
				off = (way << CHANNEL_BITS) + channel;
				vsn = test_lsn++;
				lsn = ((block << (CHANNEL_BITS + NUM_BIT_WAY)) + (way << CHANNEL_BITS) + channel) << NUM_BIT_SEC_SIZE;
				lsn = (lsn + 12 - 1) << NUM_BIT_STRIPING_OFFSET;
				lsn += (channel - 2) + way * 6;

				segment_struct[channel][way][block].Update_mapping(vsn, lsn);

				IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, lsn);
			}
		}
	}

	/*for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {
		for (int way = 0; way < USER_WAYS; way++) {
			for (int channel = 2; channel < 4; channel++) {

				UINT32 lsn, vsn, off;
				off = (way << CHANNEL_BITS) + channel;
				vsn = test_lsn++;
				lsn = ((block << (CHANNEL_BITS + NUM_BIT_WAY)) +  (way << CHANNEL_BITS) + channel) << NUM_BIT_SEC_SIZE;
				lsn = (lsn + 4 - 1) << NUM_BIT_STRIPING_OFFSET;
				lsn += (channel - 2) + way * 2;

				segment_struct[channel][way][block].Update_mapping(vsn, lsn);

				IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, lsn);
			}
		}
	}


	for (int block = 0; block < MAIN_BLOCKS_PER_DIE; block++) {

		for (int way = 0; way < USER_WAYS; way++) {
			for (int channel = 4; channel < USER_CHANNELS; channel++) {

				UINT32 lsn, vsn, off;
				off = (way << CHANNEL_BITS) + channel;
				vsn = test_lsn++;
				lsn = ((block << (CHANNEL_BITS + NUM_BIT_WAY)) +  (way << CHANNEL_BITS) + channel) << NUM_BIT_SEC_SIZE;
				lsn = (lsn + 8 - 1) << NUM_BIT_STRIPING_OFFSET;
				lsn += (channel - 4) + way * 4;

				segment_struct[channel][way][block].Update_mapping(vsn, lsn);

				IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(vsn, lsn);
			}
		}
	}*/

	////////////////////////////// Page mapping
	return VOID();
}

UINT32 STRIPING_INFO::get_striped_lpn(UINT32 vpn)
{
	UINT32 channel, way, block;
	UINT32 vsn;
	UINT32 lsn;
	UINT32 striping_offset;
	UINT32 segs_in_striping_groups;
	UINT32 lpn_in_striping_groups;
	UINT32 start_vsn;

	UINT32 target_lpn;


	vsn = vpn >> NUM_BIT_VPAGE;
	lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(vsn);

	if (lsn == 0xffffffff)
		return vpn;

	striping_offset = lsn & STRIPING_OFFSET_MASK;

	segs_in_striping_groups = get_secsize_from_lsn(lsn) + 1;
	start_vsn = vsn - striping_offset;

	//calculates lpn_in_striping_groups  (Physical page offset)
	UINT32 PageNo, page_offset;
	page_offset = vpn % LPN_PER_PHYSICAL_PAGE;
	PageNo = (vpn >> (NUM_BIT_LPN_PER_PAGE)) & NAND_ADDR_PPAGE_MASK;

	lpn_in_striping_groups = striping_offset << (NUM_BIT_PPAGE);
	lpn_in_striping_groups += PageNo;

	//calculate target segs and offset
	UINT32 target_segoff = lpn_in_striping_groups % segs_in_striping_groups;
	UINT32 target_page_offset = (lpn_in_striping_groups / segs_in_striping_groups);

	lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(start_vsn + target_segoff);

	channel = get_channel_from_lsn(lsn);
	way = get_way_from_lsn(lsn);
	block = get_lbn_from_lsn(lsn);

	target_lpn = GET_LPN_FROM_VPN_VBN(channel, way, target_page_offset, block) + page_offset;

	/*UINT32 test_lpn = get_original_lpn(target_lpn);
	DEBUG_ASSERT(vpn == test_lpn);*/

	return target_lpn;
}

UINT32 STRIPING_INFO::get_original_lpn(UINT32 striped_lpn)
{
	UINT32 channel;
	UINT32 way;
	UINT32 blk;
	UINT32 page;
	SEGMENT_LIST* segment;
	UINT32 page_offset = striped_lpn % LPN_PER_PHYSICAL_PAGE;
	UINT32 segs_in_striping_groups;
	UINT32 start_vsn;

	channel = get_channel_from_lpn(striped_lpn);
	way = get_way_from_lpn(striped_lpn);
	blk = get_lbn_from_lpn(striped_lpn);
	page = (striped_lpn >> (NUM_BIT_WAY + CHANNEL_BITS + NUM_BIT_LPN_PER_PAGE)) & NAND_ADDR_PPAGE_MASK;

	segment = &IOUB_GLOBAL::GetStrinpingMgr()->segment_struct[channel][way][blk];

	UINT32 vsn = segment->m_vsn;
	UINT32 lsn = segment->m_lsn;
	UINT32 lsn_offset = lsn & STRIPING_OFFSET_MASK;
	
	segs_in_striping_groups = get_secsize_from_lsn(lsn) + 1;
	start_vsn = vsn - lsn_offset;

	UINT32 page_offset_in_section = (segs_in_striping_groups * page + lsn_offset) << NUM_BIT_LPN_PER_PAGE;
	page_offset_in_section += page_offset;

	UINT32 new_lpn = start_vsn << (NUM_BIT_VPAGE);
	new_lpn += page_offset_in_section;

	return new_lpn;
}

UINT32 STRIPING_INFO::alloc_and_get_segment(UINT32 channel, UINT32 way)
{
	SEGMENT_LIST* segment;
	segment = list_first_entry(&m_unallocated_seg[channel][way], SEGMENT_LIST, m_dlList);
	list_del_init(&segment->m_dlList);

	return segment->m_lsn;
}

VOID STRIPING_INFO::allocate_vsn(UINT32 channel, UINT32 way, UINT32 block)
{
	list_del_init(&segment_struct[channel][way][block].m_dlList);
	return;
}

VOID STRIPING_INFO::release_vsn(UINT32 channel, UINT32 way, UINT32 block)
{
	segment_struct[channel][way][block].Release();
	return;
}

SEGMENT_LIST * STRIPING_INFO::get_one_segment(UINT32 channel, UINT32 way)
{
	SEGMENT_LIST* ret_val;
	ret_val = list_first_entry(&m_unallocated_seg[channel][way], SEGMENT_LIST, m_dlList);
	return ret_val;
}

BOOL STRIPING_INFO::isalloced(UINT32 LPN)
{
	UINT32 lsn, vsn;
	vsn = LPN >> NUM_BIT_VPAGE;
	lsn = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(vsn);

	if (lsn == 0xffffffff)
		return FALSE;
	return TRUE;
}

VOID STRIPING_INFO::update_striping_region(UINT32 start_segment, UINT32 count, UINT32 striping_policy_count, UINT32 * striping_policy)
{
	for (int iter = 0; iter < count; iter++) {
		UINT32 src_VSN, target_VSN;
		UINT32 src_LSN, target_LSN;
		UINT32 src_ch_vsn, src_wy_vsn, src_blk_vsn;
		UINT32 src_ch_lsn, src_wy_lsn, src_blk_lsn;
		UINT32 target_ch_vsn, target_wy_vsn, target_blk_vsn;
		UINT32 target_ch_lsn, target_wy_lsn, target_blk_lsn;
		UINT32 offset = iter % striping_policy_count;
		//allocate one segment
		SEGMENT_LIST* src_segment;
		SEGMENT_LIST* target_segment;

		src_VSN = start_segment + iter;
		src_ch_vsn = get_channel_from_vsn(src_VSN);
		src_wy_vsn = get_way_from_vsn(src_VSN);
		src_blk_vsn = get_lbn_from_vsn(src_VSN);

		src_LSN = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(src_VSN);
		src_ch_lsn = get_channel_from_lsn(src_LSN);
		src_wy_lsn = get_way_from_lsn(src_LSN);
		src_blk_lsn = get_lbn_from_lsn(src_LSN);

		src_segment = &segment_struct[src_ch_lsn][src_wy_lsn][src_blk_lsn];
		allocate_vsn(src_ch_lsn, src_wy_lsn, src_blk_lsn);
		

		target_wy_vsn = striping_policy[offset];
		target_ch_vsn = target_wy_vsn % striping_policy_count;
		target_wy_vsn = target_wy_vsn >> CHANNEL_BITS;

		target_segment = get_one_segment(target_ch_vsn, target_wy_vsn);
		target_VSN = target_segment->m_vsn;
		target_LSN = target_segment->m_lsn;
		target_blk_vsn = get_lbn_from_vsn(target_VSN);

		target_LSN = IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->GetL2V(target_VSN);
		target_ch_lsn = get_channel_from_lsn(target_LSN);
		target_wy_lsn = get_way_from_lsn(target_LSN);
		target_blk_lsn = get_lbn_from_lsn(target_LSN);

		allocate_vsn(target_ch_lsn, target_wy_lsn, target_blk_lsn);

		//segment mapping update
		target_segment->m_lsn = src_LSN;
		src_segment->m_lsn = target_LSN;

		IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(target_VSN, src_LSN);
		IOUB_GLOBAL::GetMetaMgr(METATYPE_STRIPING)->SetL2V(src_VSN, target_LSN);

		//mapping table update



		//Data swap
	}

	return VOID();
}
#endif

