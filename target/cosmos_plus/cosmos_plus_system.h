
#ifndef __COSMOS_PLUS_SYSTEM_H__
#define __COSMOS_PLUS_SYSTEM_H__

#include "cosmos_plus_global_config.h"

#include "../fil/nsc_driver.h"
#include "xparameters.h"
#include "nvme.h"

#define LUN_0_BASE_ADDR		0x00000000
#define LUN_1_BASE_ADDR		0x00200000

//checks NSC connection, initializes base address
#ifdef	XPAR_TIGER4NSC_7_BASEADDR
#define NSC_7_CONNECTED	1
#define NSC_7_BASEADDR	XPAR_TIGER4NSC_7_BASEADDR
#else
#define NSC_7_CONNECTED	0
#define NSC_7_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_6_BASEADDR
#define NSC_6_CONNECTED	1
#define NSC_6_BASEADDR	XPAR_TIGER4NSC_6_BASEADDR
#else
#define NSC_6_CONNECTED	0
#define NSC_6_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_5_BASEADDR
#define NSC_5_CONNECTED	1
#define NSC_5_BASEADDR	XPAR_TIGER4NSC_5_BASEADDR
#else
#define NSC_5_CONNECTED	0
#define NSC_5_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_4_BASEADDR
#define NSC_4_CONNECTED	1
#define NSC_4_BASEADDR	XPAR_TIGER4NSC_4_BASEADDR
#else
#define NSC_4_CONNECTED	0
#define NSC_4_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_3_BASEADDR
#define NSC_3_CONNECTED	1
#define NSC_3_BASEADDR	XPAR_TIGER4NSC_3_BASEADDR
#else
#define NSC_3_CONNECTED	0
#define NSC_3_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_2_BASEADDR
#define NSC_2_CONNECTED	1
#define NSC_2_BASEADDR	XPAR_TIGER4NSC_2_BASEADDR
#else
#define NSC_2_CONNECTED	0
#define NSC_2_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_1_BASEADDR
#define NSC_1_CONNECTED	1
#define NSC_1_BASEADDR	XPAR_TIGER4NSC_1_BASEADDR
#else
#define NSC_1_CONNECTED	0
#define NSC_1_BASEADDR	0
#endif
#ifdef	XPAR_TIGER4NSC_0_BASEADDR
#define NSC_0_CONNECTED	1
#define NSC_0_BASEADDR	XPAR_TIGER4NSC_0_BASEADDR
#else
#define NSC_0_CONNECTED	0
#define NSC_0_BASEADDR	0
#endif

//number of connected (=AXI mapped) NSC
#define NUMBER_OF_CONNECTED_CHANNEL (NSC_7_CONNECTED + NSC_6_CONNECTED + NSC_5_CONNECTED + NSC_4_CONNECTED + NSC_3_CONNECTED + NSC_2_CONNECTED + NSC_1_CONNECTED + NSC_0_CONNECTED)

//--------------------------------
//NAND flash memory specifications
//--------------------------------

#define BITS_PER_CELL				(1)			// 1: SLC, 2: MLC

#define	BYTES_PER_DATA_REGION_OF_NAND_ROW		16384
#define	BYTES_PER_SPARE_REGION_OF_NAND_ROW		1664
#define BYTES_PER_NAND_ROW						(BYTES_PER_DATA_REGION_OF_NAND_ROW + BYTES_PER_SPARE_REGION_OF_NAND_ROW)

#define	ROWS_PER_SLC_BLOCK			128
#define	ROWS_PER_MLC_BLOCK			256
#define ROTS_PER_MLC_BLOCK_BITS		8

#if (BITS_PER_CELL == 1)
	#define PAGES_PER_BLOCK				ROWS_PER_SLC_BLOCK
	#define PAGES_PER_BLOCK_BITS		7
#elif (BITS_PER_CELL == 2)
	#define PAGES_PER_BLOCK				ROWS_PER_MLC_BLOCK
	#define PAGES_PER_BLOCK_BITS		8
#else
	#error		"check configuration"
#endif

#ifdef GREEDY_FTL
	//#define	MAIN_BLOCKS_PER_LUN		4096		// (original)This is physical NAND blocks.
	#define	MAIN_BLOCKS_PER_LUN		512				// for test
#else
#ifdef WIN32
	#define	MAIN_BLOCKS_PER_LUN		512		// This is physical NAND blocks. 32GB
#else
	#define MAIN_BLOCKS_PER_LUN		2048		//16GB
#endif
#endif

#ifdef WIN32
#define EXTENDED_BLOCKS_PER_LUN		88
#else
#define EXTENDED_BLOCKS_PER_LUN		176
#endif

#define TOTAL_BLOCKS_PER_LUN		(MAIN_BLOCKS_PER_LUN + EXTENDED_BLOCKS_PER_LUN)			// dy, Total block count (spare block included )

#define	MAIN_ROWS_PER_SLC_LUN		(ROWS_PER_SLC_BLOCK * MAIN_BLOCKS_PER_LUN)
#define	MAIN_ROWS_PER_MLC_LUN		(ROWS_PER_MLC_BLOCK * MAIN_BLOCKS_PER_LUN)

#define	LUNS_PER_DIE				2

#define	MAIN_BLOCKS_PER_DIE			(MAIN_BLOCKS_PER_LUN * LUNS_PER_DIE)
#define TOTAL_BLOCKS_PER_DIE		(TOTAL_BLOCKS_PER_LUN * LUNS_PER_DIE)

#define BAD_BLOCK_MARK_PAGE0		0										//first row of a block
#define BAD_BLOCK_MARK_PAGE1		(ROWS_PER_MLC_BLOCK - 1)				//last row of a block
#define BAD_BLOCK_MARK_BYTE0 		0										//first byte of data region of the row
#define BAD_BLOCK_MARK_BYTE1 		(BYTES_PER_DATA_REGION_OF_NAND_ROW)		//first byte of spare region of the row

//------------------------------------
//NAND storage controller specifications
//------------------------------------

//supported maximum channel/way structure

#define	NSC_MAX_CHANNELS			(NUMBER_OF_CONNECTED_CHANNEL)
#define	NSC_MAX_WAYS				8			// physical way: 8

//#define CH_WY_FLAX
//off:  channel and way can use only # of 2^x ch/way

#define USER_CHANNELS					(4)
#define USER_WAYS						(4)

//row -> page
#define	BYTES_PER_DATA_REGION_OF_PAGE	16384
#define BYTES_PER_SPARE_REGION_OF_PAGE	256		//last 8 byte used by ECC engine (CRC function)
												// (BYTES_PER_SPARE_REGION_OF_NAND_ROW - BYTES_PER_SPARE_REGION_OF_PAGE) bytes are used by ECC engine (Parity data)

#define PHYSICAL_PAGE_SIZE				BYTES_PER_DATA_REGION_OF_PAGE
#define LOGICAL_PAGE_SIZE				HOST_BLOCK_SIZE				// size of mapping unit, byte
#define LOGICAL_PAGE_SIZE_KB			(LOGICAL_PAGE_SIZE / KB)
#define LPN_PER_PHYSICAL_PAGE			(PHYSICAL_PAGE_SIZE / LOGICAL_PAGE_SIZE)

#define	PAGES_PER_SLC_BLOCK				(ROWS_PER_SLC_BLOCK)
#define	PAGES_PER_MLC_BLOCK				(ROWS_PER_MLC_BLOCK)

//ECC encoder/decoder specification
#define ECC_CHUNKS_PER_PAGE				32
#define BIT_ERROR_THRESHOLD_PER_CHUNK	20				// dy, CheckEccErrorInfo()���� error�� report�� bit error count
#define ERROR_INFO_WORD_COUNT 			11

//------------------------------
//NVMe Controller Specifications
//------------------------------
#define	BYTES_PER_NVME_BLOCK			4096
#define NVME_BLOCKS_PER_PAGE			(BYTES_PER_DATA_REGION_OF_PAGE / BYTES_PER_NVME_BLOCK)

#if defined(__cplusplus) && defined(WIN32)
extern "C" {
#endif
	void SYSTEM_initialize(void);
	void SYSTEM_Run(void);
#if defined(__cplusplus) && defined(WIN32)
}
#endif

#endif	// end of #ifndef __COSMOS+_SYSTEM_H__
