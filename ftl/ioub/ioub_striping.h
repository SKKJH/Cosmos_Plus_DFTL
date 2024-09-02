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

#ifndef __IOUB_STRIPING_H__
#define __IOUB_STRIPING_H__

#define NUM_BIT_STRIPING_OFFSET		(4)
#define STRIPING_OFFSET_MASK		((1 <<(NUM_BIT_STRIPING_OFFSET)) - 1)

#define NUM_BIT_SEC_SIZE		(4)
#define SEC_SIZE_MASK			((1 <<(NUM_BIT_STRIPING_OFFSET)) - 1)

#define get_channel_from_vsn(vsn)	((vsn) & NAND_ADDR_CHANNEL_MASK)
#define get_way_from_vsn(vsn)		((vsn >> (CHANNEL_BITS)) & NAND_ADDR_WAY_MASK)
#define get_lbn_from_vsn(vsn)		((vsn >> (CHANNEL_BITS+NUM_BIT_WAY)) & NAND_ADDR_BLOCK_MASK)


#define get_segoff_from_lsn(lsn)	(lsn & STRIPING_OFFSET_MASK)
#define get_secsize_from_lsn(lsn)	((lsn >> NUM_BIT_STRIPING_OFFSET) & SEC_SIZE_MASK)
#define get_channel_from_lsn(lsn)	((lsn >> NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE) & NAND_ADDR_CHANNEL_MASK)
#define get_way_from_lsn(lsn)		((lsn >> (NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE+CHANNEL_BITS)) & NAND_ADDR_WAY_MASK)
#define get_lbn_from_lsn(lsn)		((lsn >> (NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE+CHANNEL_BITS+NUM_BIT_WAY)) & NAND_ADDR_BLOCK_MASK)

#define vsn_from_ch_wy_blk(ch,wy,blk)	(ch + (wy << (CHANNEL_BITS)) + (blk <<(CHANNEL_BITS+NUM_BIT_WAY)))
#define lsn_from_ch_wy_blk_sec_off(ch,wy,blk,secsize,off)	(off + (secsize << NUM_BIT_STRIPING_OFFSET) + (ch << (NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE)) + (wy << (CHANNEL_BITS+NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE)) + (blk <<(CHANNEL_BITS+NUM_BIT_WAY+NUM_BIT_STRIPING_OFFSET+NUM_BIT_SEC_SIZE)))


class SEGMENT_LIST
{
public:
	VOID Initialize(UINT32 vsn, UINT32 lsn);

	VOID Update_mapping(UINT32 vsn, UINT32 lsn);

	VOID Release();

	UINT32 m_vsn;
	UINT32 m_lsn;		//Logical SegNo + striping_offset (4bit) + segs_in_group (4bit)
	struct list_head		m_dlList;		// host DMA issued list
private:
};


class STRIPING_INFO
{
public:
	VOID Initialize();
	UINT32 get_striped_lpn(UINT32 original_lpn);
	UINT32 get_original_lpn(UINT32 striped_lpn);
	UINT32 alloc_and_get_segment(UINT32 channel, UINT32 way);
	VOID release_vsn(UINT32 channel, UINT32 way, UINT32 block);
	VOID allocate_vsn(UINT32 channel, UINT32 way, UINT32 block);
	VOID update_striping_region(UINT32 start_segment, UINT32 count, UINT32 striping_policy_count, UINT32* striping_policy);
	SEGMENT_LIST* get_one_segment(UINT32 channel, UINT32 way);
	BOOL isalloced(UINT32 LPN);

	SEGMENT_LIST			segment_struct[USER_CHANNELS][USER_WAYS][MAIN_BLOCKS_PER_DIE]; //logical segment NO

	struct list_head		m_unallocated_seg[USER_CHANNELS][USER_WAYS];		//
private:
};

#endif