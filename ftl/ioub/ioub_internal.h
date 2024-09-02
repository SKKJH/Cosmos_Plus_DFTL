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

#ifndef __IOUB_INTERNAL_H__
#define __IOUB_INTERNAL_H__

// only headers for ioub internal operation

#include <new>

// target header
#include "cosmos_types.h"
#include "cosmos_plus_system.h"

#include "statistics.h"

// hil headers
#include "hil_config.h"

// fil headers
#include "fil.h"

// common headers
#include "osal.h"
#include "util.h"
#include "debug.h"
#include "list.h"

// ftl headers
#include "ftl.h"


#include "ioub_config.h"
#include "ioub_types.h"
#include "ioub_hdma.h"
#include "ioub_vnand.h"
#include "ioub_meta.h"
#include "ioub_block.h"
#include "ioub_bufferpool.h"
#include "ioub_activeblock.h"
#include "ioub_garbagecollector.h"
#include "ioub_request_meta.h"
#include "ioub_request_gc.h"
#include "ioub_request_hil.h"
#include "ioub_request.h"

//Striping Policy
#if (IOUB_STRIPING==1)
#include "ioub_striping.h"
#endif

#include "ioub_global.h"

//Profile
#include "ioub_profile.h"



#endif		// end of #ifndef __IOUB_INTERNAL_H__