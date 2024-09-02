//////////////////////////////////////////////////////////////////////////////////
// ftl_config.h for Cosmos+ OpenSSD
// Copyright (c) 2017 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: Flash Translation Layer Configuration Manager
// File Name: ftl_config.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of flash translation layer configuration manager
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_CONFIG_H_
#define FTL_CONFIG_H_

#include "cosmos_plus_global_config.h"
#include "cosmos_plus_system.h"

//------------------
//FTL configurations
//------------------

#define	SLC_MODE				1
#define	MLC_MODE				2

//************************************************************************
#define	BITS_PER_FLASH_CELL		SLC_MODE	//user configurable factor
#define	USER_BLOCKS_PER_LUN		MAIN_BLOCKS_PER_LUN		//user configurable factor
//#define	USER_CHANNELS			(NUMBER_OF_CONNECTED_CHANNEL)		//user configurable factor,		dy, = 8
//#define	USER_WAYS				8			//user configurable factor
//************************************************************************

#define	BYTES_PER_DATA_REGION_OF_SLICE		16384		//slice is a mapping unit of FTL
#define	BYTES_PER_SPARE_REGION_OF_SLICE		256			// dy, last 8 byte used by ECC engine (CRC function), refer to BYTES_PER_SPARE_REGION_OF_PAGE

#define SLICES_PER_PAGE				(BYTES_PER_DATA_REGION_OF_PAGE / BYTES_PER_DATA_REGION_OF_SLICE)	//a slice directs a page, full page mapping
#define NVME_BLOCKS_PER_SLICE		(BYTES_PER_DATA_REGION_OF_SLICE / BYTES_PER_NVME_BLOCK)				// dy 16K / 4K = 4

#define	USER_DIES					(USER_CHANNELS * USER_WAYS)							// dy, 8 channel * 8 way = 64

#define	USER_PAGES_PER_BLOCK		(PAGES_PER_SLC_BLOCK * BITS_PER_FLASH_CELL)
#define	USER_PAGES_PER_LUN			(USER_PAGES_PER_BLOCK * USER_BLOCKS_PER_LUN)
#define	USER_PAGES_PER_DIE			(USER_PAGES_PER_LUN * LUNS_PER_DIE)
#define	USER_PAGES_PER_CHANNEL		(USER_PAGES_PER_DIE * USER_WAYS)
#define	USER_PAGES_PER_SSD			(USER_PAGES_PER_CHANNEL * USER_CHANNELS)

#define	SLICES_PER_BLOCK			(USER_PAGES_PER_BLOCK * SLICES_PER_PAGE)
#define	SLICES_PER_LUN				(USER_PAGES_PER_LUN * SLICES_PER_PAGE)
#define	SLICES_PER_DIE				(USER_PAGES_PER_DIE * SLICES_PER_PAGE)
#define	SLICES_PER_CHANNEL			(USER_PAGES_PER_CHANNEL * SLICES_PER_PAGE)
#define	SLICES_PER_SSD				(USER_PAGES_PER_SSD * SLICES_PER_PAGE)

#define	USER_BLOCKS_PER_DIE			(USER_BLOCKS_PER_LUN * LUNS_PER_DIE)
#define	USER_BLOCKS_PER_CHANNEL		(USER_BLOCKS_PER_DIE * USER_WAYS)
#define	USER_BLOCKS_PER_SSD			(USER_BLOCKS_PER_CHANNEL * USER_CHANNELS)

#define	MB_PER_BLOCK						((BYTES_PER_DATA_REGION_OF_SLICE * SLICES_PER_BLOCK) / (1024 * 1024))
#define MB_PER_SSD							(USER_BLOCKS_PER_SSD * MB_PER_BLOCK)
#define MB_PER_MIN_FREE_BLOCK_SPACE			(USER_DIES * MB_PER_BLOCK)
#define MB_PER_METADATA_BLOCK_SPACE			(USER_DIES * MB_PER_BLOCK)
#define MB_PER_OVER_PROVISION_BLOCK_SPACE	((USER_BLOCKS_PER_SSD / 10) * MB_PER_BLOCK)


void InitFTL();
void InitChCtlReg();
void InitNandArray();
void CheckConfigRestriction();

extern unsigned int storageCapacity_L;
extern V2FMCRegisters* chCtlReg[USER_CHANNELS];

#endif /* FTL_CONFIG_H_ */
