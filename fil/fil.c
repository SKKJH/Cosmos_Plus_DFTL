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
* ESLab: http://nyx.skku.ac.kr
*
*******************************************************/
#include "assert.h"
#include "xil_types.h"
#include "debug.h"

#include "cosmos_plus_system.h"
#include "cosmos_plus_memory_map.h"

#ifdef GREEDY_FTL
#include "request_schedule.h"
#endif

#include "cosmos_plus_system.h"

#include "fil.h"
#include "fil_nand.h"
#include "../Target/osal.h"
#include "fil_request.h"

static void _AddToWaitQ(FTL_REQUEST_TYPE nReqType, FTL_REQUEST_ID stReqID, NAND_ADDR stPhyAddr, void* pMainBuf, void* pSpareBuf);
static BOOL anBadPBN[USER_CHANNELS][USER_WAYS][TOTAL_BLOCKS_PER_DIE / 8];

void set_badPBN(UINT32 channel, UINT32 way, UINT32 pblk)
{
	UINT32 offset = pblk >> 3;
	UINT32 offset_in_byte = pblk % 8;

	anBadPBN[channel][way][offset] |= 1 << offset_in_byte;

}
void FIL_Initialize(void)
{
	NAND_Initialize();
	FIL_InitRequest();
	memset(&anBadPBN[0][0][0], 0, USER_CHANNELS * USER_WAYS * TOTAL_BLOCKS_PER_DIE / 8);

	/*set_badPBN(1, 1, 256);
	set_badPBN(1, 1, 320);
	set_badPBN(1, 1, 330);


	set_badPBN(2, 3, 415);
	set_badPBN(2, 3, 481);
	set_badPBN(2, 3, 505);
	set_badPBN(2, 3, 565);
	set_badPBN(2, 3, 585);

	set_badPBN(3, 3, 517);
	set_badPBN(3, 3, 527);
	set_badPBN(3, 3, 579);*/

#ifndef WIN32
	for (UINT32 channel = 0; channel < USER_CHANNELS; channel++) {
		for (UINT32 way = 0; way < USER_WAYS; way++)
		{
			set_badPBN(channel, way, 256);
			set_badPBN(channel, way, 320);
			set_badPBN(channel, way, 330);
			set_badPBN(channel, way, 415);
			set_badPBN(channel, way, 424);
			set_badPBN(channel, way, 429);
			set_badPBN(channel, way, 481);
			set_badPBN(channel, way, 505);
			set_badPBN(channel, way, 513);
			set_badPBN(channel, way, 517);
			set_badPBN(channel, way, 521);
			set_badPBN(channel, way, 527);
			set_badPBN(channel, way, 531);
			set_badPBN(channel, way, 543);
			set_badPBN(channel, way, 549);
			set_badPBN(channel, way, 556);
			set_badPBN(channel, way, 561);
			set_badPBN(channel, way, 562);
			set_badPBN(channel, way, 563);
			set_badPBN(channel, way, 564);
			set_badPBN(channel, way, 565);
			set_badPBN(channel, way, 566);
			set_badPBN(channel, way, 571);
			set_badPBN(channel, way, 579);
			set_badPBN(channel, way, 585);
			set_badPBN(channel, way, 594);
			set_badPBN(channel, way, 609);
			set_badPBN(channel, way, 612);
			set_badPBN(channel, way, 618);
			set_badPBN(channel, way, 629);
			set_badPBN(channel, way, 640);
			set_badPBN(channel, way, 648);
			set_badPBN(channel, way, 651);
			set_badPBN(channel, way, 653);
			set_badPBN(channel, way, 661);
			set_badPBN(channel, way, 662);
			set_badPBN(channel, way, 683);
			set_badPBN(channel, way, 685);
			set_badPBN(channel, way, 699);
			set_badPBN(channel, way, 703);
			set_badPBN(channel, way, 712);
			set_badPBN(channel, way, 714);
			set_badPBN(channel, way, 723);
			set_badPBN(channel, way, 725);
			set_badPBN(channel, way, 735);
			set_badPBN(channel, way, 741);
			set_badPBN(channel, way, 746);
			set_badPBN(channel, way, 765);
			set_badPBN(channel, way, 770);
			set_badPBN(channel, way, 776);
			set_badPBN(channel, way, 777);
			set_badPBN(channel, way, 791);
			set_badPBN(channel, way, 792);
			set_badPBN(channel, way, 795);
			set_badPBN(channel, way, 799);
			set_badPBN(channel, way, 801);
			set_badPBN(channel, way, 803);
			set_badPBN(channel, way, 809);
			set_badPBN(channel, way, 811);
			set_badPBN(channel, way, 812);
			set_badPBN(channel, way, 818);
			set_badPBN(channel, way, 819);
			set_badPBN(channel, way, 827);
			set_badPBN(channel, way, 828);
			set_badPBN(channel, way, 836);
			set_badPBN(channel, way, 844);
			set_badPBN(channel, way, 849);
			set_badPBN(channel, way, 862);
			set_badPBN(channel, way, 870);
			//set_badPBN(channel, way, 867);
			set_badPBN(channel, way, 886);
			set_badPBN(channel, way, 896);
			set_badPBN(channel, way, 909);
			set_badPBN(channel, way, 919);
			set_badPBN(channel, way, 927);
			set_badPBN(channel, way, 928);
			set_badPBN(channel, way, 935);
			set_badPBN(channel, way, 941);
			set_badPBN(channel, way, 949);
			set_badPBN(channel, way, 952);
			set_badPBN(channel, way, 953);
			set_badPBN(channel, way, 968);
			set_badPBN(channel, way, 974);
			set_badPBN(channel, way, 983);
			set_badPBN(channel, way, 988);
			set_badPBN(channel, way, 989);
			set_badPBN(channel, way, 991);
			set_badPBN(channel, way, 995);
			set_badPBN(channel, way, 996);
			set_badPBN(channel, way, 997);
			set_badPBN(channel, way, 999);
			set_badPBN(channel, way, 1002);
			set_badPBN(channel, way, 1017);
			set_badPBN(channel, way, 1019);
			set_badPBN(channel, way, 1020);
			set_badPBN(channel, way, 1033);
			set_badPBN(channel, way, 1037);
			set_badPBN(channel, way, 1046);
			set_badPBN(channel, way, 1047);
			set_badPBN(channel, way, 1049);
			set_badPBN(channel, way, 1054);
			set_badPBN(channel, way, 1064);
			set_badPBN(channel, way, 1072);
			set_badPBN(channel, way, 1089);
			set_badPBN(channel, way, 1095);
			set_badPBN(channel, way, 1098);
			set_badPBN(channel, way, 1099);
			set_badPBN(channel, way, 1100);
			set_badPBN(channel, way, 1101);
			set_badPBN(channel, way, 1102);
			set_badPBN(channel, way, 1108);
			set_badPBN(channel, way, 1113);
			set_badPBN(channel, way, 1124);
			set_badPBN(channel, way, 1132);
			set_badPBN(channel, way, 1133);
			set_badPBN(channel, way, 1147);
			set_badPBN(channel, way, 1156);
			set_badPBN(channel, way, 1157);
			set_badPBN(channel, way, 1161);
			set_badPBN(channel, way, 1171);
			set_badPBN(channel, way, 1177);
			set_badPBN(channel, way, 1178);
			set_badPBN(channel, way, 1183);
			set_badPBN(channel, way, 1185);
			set_badPBN(channel, way, 1190);
			set_badPBN(channel, way, 1195);
			set_badPBN(channel, way, 1204);
			set_badPBN(channel, way, 1206);
			set_badPBN(channel, way, 1209);
			set_badPBN(channel, way, 1211);
			set_badPBN(channel, way, 1213);
			set_badPBN(channel, way, 1221);
			set_badPBN(channel, way, 1223);
			set_badPBN(channel, way, 1225);
			set_badPBN(channel, way, 1229);
			set_badPBN(channel, way, 1231);
			set_badPBN(channel, way, 1232);
			set_badPBN(channel, way, 1241);
			set_badPBN(channel, way, 1242);
			set_badPBN(channel, way, 1250);
			set_badPBN(channel, way, 1254);
			set_badPBN(channel, way, 1255);
			set_badPBN(channel, way, 1257);
			set_badPBN(channel, way, 1259);
			set_badPBN(channel, way, 1262);
			set_badPBN(channel, way, 1263);
			set_badPBN(channel, way, 1267);
			set_badPBN(channel, way, 1273);
			set_badPBN(channel, way, 1277);
			set_badPBN(channel, way, 1280);
			set_badPBN(channel, way, 1283);
			set_badPBN(channel, way, 1287);
			set_badPBN(channel, way, 1299);
			set_badPBN(channel, way, 1301);
			set_badPBN(channel, way, 1302);
			set_badPBN(channel, way, 1310);
			set_badPBN(channel, way, 1311);
			set_badPBN(channel, way, 1316);
			set_badPBN(channel, way, 1317);
			set_badPBN(channel, way, 1319);
			set_badPBN(channel, way, 1330);
			set_badPBN(channel, way, 1332);
			set_badPBN(channel, way, 1334);
			set_badPBN(channel, way, 1335);
			set_badPBN(channel, way, 1336);
			set_badPBN(channel, way, 1343);
			set_badPBN(channel, way, 1344);
			set_badPBN(channel, way, 1348);
			set_badPBN(channel, way, 1354);
			set_badPBN(channel, way, 1355);
			set_badPBN(channel, way, 1357);
			set_badPBN(channel, way, 1358);
			set_badPBN(channel, way, 1363);
			set_badPBN(channel, way, 1364);
			set_badPBN(channel, way, 1369);
			set_badPBN(channel, way, 1370);
			set_badPBN(channel, way, 1373);
			set_badPBN(channel, way, 1374);
			set_badPBN(channel, way, 1380);
			set_badPBN(channel, way, 1386);
			set_badPBN(channel, way, 1388);
			set_badPBN(channel, way, 1389);
			set_badPBN(channel, way, 1391);
			set_badPBN(channel, way, 1394);
			set_badPBN(channel, way, 1395);
			set_badPBN(channel, way, 1400);
			set_badPBN(channel, way, 1407);
			set_badPBN(channel, way, 1423);
			set_badPBN(channel, way, 1424);
			set_badPBN(channel, way, 1441);
			set_badPBN(channel, way, 1443);
			set_badPBN(channel, way, 1449);
			set_badPBN(channel, way, 1453);
			set_badPBN(channel, way, 1457);
			set_badPBN(channel, way, 1467);
			set_badPBN(channel, way, 1473);
			set_badPBN(channel, way, 1481);
			set_badPBN(channel, way, 1482);
			set_badPBN(channel, way, 1489);
			set_badPBN(channel, way, 1500);
			set_badPBN(channel, way, 1501);
			set_badPBN(channel, way, 1516);
			set_badPBN(channel, way, 1519);
			set_badPBN(channel, way, 1529);
			set_badPBN(channel, way, 1532);
			set_badPBN(channel, way, 1534);
			set_badPBN(channel, way, 1539);
			set_badPBN(channel, way, 1540);
			set_badPBN(channel, way, 1549);
			set_badPBN(channel, way, 1552);
			set_badPBN(channel, way, 1555);
			set_badPBN(channel, way, 1565);
			set_badPBN(channel, way, 1566);
			set_badPBN(channel, way, 1569);
			set_badPBN(channel, way, 1580);
			set_badPBN(channel, way, 1596);
			set_badPBN(channel, way, 1597);
			set_badPBN(channel, way, 1602);
			set_badPBN(channel, way, 1606);
			set_badPBN(channel, way, 1614);
			set_badPBN(channel, way, 1639);
			set_badPBN(channel, way, 1651);
			set_badPBN(channel, way, 1658);
			set_badPBN(channel, way, 1659);
			set_badPBN(channel, way, 1664);
			set_badPBN(channel, way, 1670);
			set_badPBN(channel, way, 1674);
			set_badPBN(channel, way, 1675);
			set_badPBN(channel, way, 1679);
			set_badPBN(channel, way, 1694);
			set_badPBN(channel, way, 1696);
			set_badPBN(channel, way, 1710);
			set_badPBN(channel, way, 1714);
			set_badPBN(channel, way, 1716);
			set_badPBN(channel, way, 1717);
			set_badPBN(channel, way, 1720);
			set_badPBN(channel, way, 1754);
			set_badPBN(channel, way, 1762);
			set_badPBN(channel, way, 1792);
			set_badPBN(channel, way, 1813);
			set_badPBN(channel, way, 1815);
			set_badPBN(channel, way, 1822);
			set_badPBN(channel, way, 1828);
			set_badPBN(channel, way, 1842);
			set_badPBN(channel, way, 1845);
			set_badPBN(channel, way, 1852);
			set_badPBN(channel, way, 1862);
			set_badPBN(channel, way, 1897);
			set_badPBN(channel, way, 1898);
			set_badPBN(channel, way, 1899);
			set_badPBN(channel, way, 1925);
			set_badPBN(channel, way, 1933);
			set_badPBN(channel, way, 1934);
			set_badPBN(channel, way, 1936);
			set_badPBN(channel, way, 1945);
			set_badPBN(channel, way, 1946);
			set_badPBN(channel, way, 1950);
			set_badPBN(channel, way, 1968);
			set_badPBN(channel, way, 1990);
			set_badPBN(channel, way, 1991);
			set_badPBN(channel, way, 2027);
			set_badPBN(channel, way, 2032);
			set_badPBN(channel, way, 2038);
			set_badPBN(channel, way, 2047);
			set_badPBN(channel, way, 2060);
			set_badPBN(channel, way, 2072);
			set_badPBN(channel, way, 2080);
			set_badPBN(channel, way, 2094);
			set_badPBN(channel, way, 2099);
			set_badPBN(channel, way, 2102);
			set_badPBN(channel, way, 2116);
			set_badPBN(channel, way, 2120);
			set_badPBN(channel, way, 2121);
			set_badPBN(channel, way, 2123);
			set_badPBN(channel, way, 2169);
			set_badPBN(channel, way, 2177);
			set_badPBN(channel, way, 2184);
			set_badPBN(channel, way, 2185);
			set_badPBN(channel, way, 2187);
			set_badPBN(channel, way, 2203);
			set_badPBN(channel, way, 2209);
			set_badPBN(channel, way, 2212);
			set_badPBN(channel, way, 2211);
			set_badPBN(channel, way, 2221);
			set_badPBN(channel, way, 2229);
			set_badPBN(channel, way, 2238);
			set_badPBN(channel, way, 2250);
			set_badPBN(channel, way, 2255);
			set_badPBN(channel, way, 2265);
			set_badPBN(channel, way, 2307);
			set_badPBN(channel, way, 2309);
			set_badPBN(channel, way, 2314);
			set_badPBN(channel, way, 2319);
			set_badPBN(channel, way, 2350);
			set_badPBN(channel, way, 2359);
			set_badPBN(channel, way, 2384);
			set_badPBN(channel, way, 2388);
			set_badPBN(channel, way, 2389);
			set_badPBN(channel, way, 2397);
			set_badPBN(channel, way, 2496);
			set_badPBN(channel, way, 2586);
			set_badPBN(channel, way, 849);
			set_badPBN(channel, way, 1263);
			set_badPBN(channel, way, 2094);
			set_badPBN(channel, way, 1263);
			set_badPBN(channel, way, 2094);
			set_badPBN(channel, way, 949);
			set_badPBN(channel, way, 1330);
			set_badPBN(channel, way, 1580);
			set_badPBN(channel, way, 1670);
			set_badPBN(channel, way, 935);
			set_badPBN(channel, way, 1064);
			set_badPBN(channel, way, 1549);
			set_badPBN(channel, way, 1259);
			set_badPBN(channel, way, 1335);
			set_badPBN(channel, way, 1206);
			set_badPBN(channel, way, 1221);
			set_badPBN(channel, way, 1223);
			set_badPBN(channel, way, 1229);
			set_badPBN(channel, way, 1255);
			set_badPBN(channel, way, 1257);
			set_badPBN(channel, way, 1273);
			set_badPBN(channel, way, 1310);
			set_badPBN(channel, way, 1316);
			set_badPBN(channel, way, 1317);
			set_badPBN(channel, way, 1319);
			set_badPBN(channel, way, 1336);
			set_badPBN(channel, way, 1358);
			set_badPBN(channel, way, 1364);
			set_badPBN(channel, way, 1369);
			set_badPBN(channel, way, 1389);
			set_badPBN(channel, way, 1394);
			set_badPBN(channel, way, 1424);
			set_badPBN(channel, way, 1449);
			set_badPBN(channel, way, 1453);
			set_badPBN(channel, way, 1455);
			set_badPBN(channel, way, 1467);
			set_badPBN(channel, way, 1482);
			set_badPBN(channel, way, 1489);
			set_badPBN(channel, way, 1500);
			set_badPBN(channel, way, 1501);
			set_badPBN(channel, way, 1532);
			set_badPBN(channel, way, 1534);
			set_badPBN(channel, way, 1552);
			set_badPBN(channel, way, 1565);
			set_badPBN(channel, way, 1590);
			set_badPBN(channel, way, 1608);
			set_badPBN(channel, way, 1610);
			set_badPBN(channel, way, 1638);
			set_badPBN(channel, way, 1642);
			set_badPBN(channel, way, 1666);
			set_badPBN(channel, way, 1698);
			set_badPBN(channel, way, 1702);
			set_badPBN(channel, way, 1752);
			set_badPBN(channel, way, 1768);
			set_badPBN(channel, way, 1776);
			set_badPBN(channel, way, 1795);
			set_badPBN(channel, way, 1810);
			set_badPBN(channel, way, 1820);
			set_badPBN(channel, way, 1824);
			set_badPBN(channel, way, 1824);
			set_badPBN(channel, way, 1848);
			set_badPBN(channel, way, 1856);
			set_badPBN(channel, way, 1866);
			set_badPBN(channel, way, 1870);
			set_badPBN(channel, way, 1880);
			set_badPBN(channel, way, 1885);
			set_badPBN(channel, way, 1892);
			set_badPBN(channel, way, 1894);
			set_badPBN(channel, way, 1900);
			set_badPBN(channel, way, 1902);
			set_badPBN(channel, way, 1926);
			set_badPBN(channel, way, 1940);
			set_badPBN(channel, way, 1942);
			set_badPBN(channel, way, 1961);
			set_badPBN(channel, way, 1972);
			set_badPBN(channel, way, 1996);
			set_badPBN(channel, way, 2004);
			set_badPBN(channel, way, 2012);
			set_badPBN(channel, way, 2029);
			set_badPBN(channel, way, 2035);
			set_badPBN(channel, way, 2054);
			set_badPBN(channel, way, 2058);
			set_badPBN(channel, way, 2067);
			set_badPBN(channel, way, 2077);
			set_badPBN(channel, way, 2086);
			set_badPBN(channel, way, 2110);
			set_badPBN(channel, way, 2114);
			set_badPBN(channel, way, 2118);
			set_badPBN(channel, way, 2136);
			set_badPBN(channel, way, 2138);
			set_badPBN(channel, way, 2152);
			set_badPBN(channel, way, 2155);
			set_badPBN(channel, way, 2164);
			set_badPBN(channel, way, 2172);
			set_badPBN(channel, way, 2173);
			set_badPBN(channel, way, 2179);
			set_badPBN(channel, way, 1825);
			set_badPBN(channel, way, 2220);
			set_badPBN(channel, way, 2386);
			set_badPBN(channel, way, 2405);
			set_badPBN(channel, way, 2407);
			set_badPBN(channel, way, 2408);
			set_badPBN(channel, way, 2438);
			set_badPBN(channel, way, 2446);
			set_badPBN(channel, way, 2455);
			set_badPBN(channel, way, 2456);
			set_badPBN(channel, way, 2457);
			set_badPBN(channel, way, 2459);
			set_badPBN(channel, way, 2468);
			set_badPBN(channel, way, 2484);
			set_badPBN(channel, way, 2491);
			set_badPBN(channel, way, 2490);
			set_badPBN(channel, way, 2500);
			set_badPBN(channel, way, 2509);
			set_badPBN(channel, way, 2532);
			set_badPBN(channel, way, 2535);
			set_badPBN(channel, way, 2540);
			set_badPBN(channel, way, 2543);
			set_badPBN(channel, way, 2551);
			set_badPBN(channel, way, 2550);
			set_badPBN(channel, way, 2557);
			set_badPBN(channel, way, 2600);
			set_badPBN(channel, way, 2603);
			set_badPBN(channel, way, 2604);
			set_badPBN(channel, way, 2616);
			set_badPBN(channel, way, 2617);
			set_badPBN(channel, way, 2645);
			set_badPBN(channel, way, 2674);
			set_badPBN(channel, way, 2701);
			set_badPBN(channel, way, 2714);
			set_badPBN(channel, way, 2727);
			set_badPBN(channel, way, 2733);
			set_badPBN(channel, way, 2738);
			set_badPBN(channel, way, 2741);
			set_badPBN(channel, way, 2747);
			set_badPBN(channel, way, 2762);
			set_badPBN(channel, way, 2769);
			set_badPBN(channel, way, 2781);
			set_badPBN(channel, way, 2795);
			set_badPBN(channel, way, 2802);
			set_badPBN(channel, way, 2806);
			set_badPBN(channel, way, 2445);
			set_badPBN(channel, way, 2780);
			set_badPBN(channel, way, 2837);
			set_badPBN(channel, way, 2845);
			set_badPBN(channel, way, 2854);
			set_badPBN(channel, way, 2856);
			set_badPBN(channel, way, 2876);
			set_badPBN(channel, way, 2877);
			set_badPBN(channel, way, 2882);
			set_badPBN(channel, way, 2888);
			set_badPBN(channel, way, 2895);
			set_badPBN(channel, way, 2902);
			set_badPBN(channel, way, 2920);
			set_badPBN(channel, way, 2926);
			set_badPBN(channel, way, 2927);
			set_badPBN(channel, way, 2928);
			set_badPBN(channel, way, 2951);
			set_badPBN(channel, way, 2967);
			set_badPBN(channel, way, 2966);
			set_badPBN(channel, way, 2970);
			set_badPBN(channel, way, 2979);
			set_badPBN(channel, way, 2978);
			set_badPBN(channel, way, 2989);
			set_badPBN(channel, way, 2994);
			set_badPBN(channel, way, 3004);
			set_badPBN(channel, way, 3008);
			set_badPBN(channel, way, 3035);
			set_badPBN(channel, way, 3060);
			set_badPBN(channel, way, 3061);
			set_badPBN(channel, way, 3072);
			set_badPBN(channel, way, 3078);
			set_badPBN(channel, way, 3079);
			set_badPBN(channel, way, 3078);
			set_badPBN(channel, way, 3100);
			set_badPBN(channel, way, 3116);
			set_badPBN(channel, way, 3138);
			set_badPBN(channel, way, 3144);
			set_badPBN(channel, way, 3159);
			set_badPBN(channel, way, 3168);
			set_badPBN(channel, way, 3175);
			set_badPBN(channel, way, 3185);
			set_badPBN(channel, way, 3188);
			set_badPBN(channel, way, 3189);
			set_badPBN(channel, way, 3187);
			set_badPBN(channel, way, 3192);
			set_badPBN(channel, way, 3201);
			set_badPBN(channel, way, 3214);
			set_badPBN(channel, way, 3219);
			set_badPBN(channel, way, 3231);
			set_badPBN(channel, way, 3248);
			set_badPBN(channel, way, 3259);
			set_badPBN(channel, way, 3178);
			set_badPBN(channel, way, 3284);
			set_badPBN(channel, way, 3285);
			set_badPBN(channel, way, 3287);
			set_badPBN(channel, way, 3295);
			set_badPBN(channel, way, 3304);
			set_badPBN(channel, way, 3315);
			set_badPBN(channel, way, 3319);
			set_badPBN(channel, way, 3325);
			set_badPBN(channel, way, 3327);
			set_badPBN(channel, way, 3344);
			set_badPBN(channel, way, 3342);
			set_badPBN(channel, way, 3343);
			set_badPBN(channel, way, 3360);
			set_badPBN(channel, way, 3367);
			set_badPBN(channel, way, 3383);
			set_badPBN(channel, way, 3382);
			set_badPBN(channel, way, 3397);
			set_badPBN(channel, way, 3400);
			set_badPBN(channel, way, 3402);
			set_badPBN(channel, way, 3407);
			set_badPBN(channel, way, 3402);
			set_badPBN(channel, way, 3416);
			set_badPBN(channel, way, 3417);
			set_badPBN(channel, way, 3429);
			set_badPBN(channel, way, 3431);
			set_badPBN(channel, way, 3433);
			set_badPBN(channel, way, 3459);
			set_badPBN(channel, way, 3457);
			set_badPBN(channel, way, 3460);
			set_badPBN(channel, way, 3471);
			set_badPBN(channel, way, 3496);
			set_badPBN(channel, way, 3497);
			set_badPBN(channel, way, 3503);
			set_badPBN(channel, way, 3501);
			set_badPBN(channel, way, 3509);
			set_badPBN(channel, way, 3511);
			set_badPBN(channel, way, 3520);
			set_badPBN(channel, way, 3525);
			set_badPBN(channel, way, 3524);
			set_badPBN(channel, way, 3541);
			set_badPBN(channel, way, 3540);
			set_badPBN(channel, way, 3573);
			set_badPBN(channel, way, 3566);
			set_badPBN(channel, way, 3572);
			set_badPBN(channel, way, 3574);
			set_badPBN(channel, way, 3575);
			set_badPBN(channel, way, 3580);
			set_badPBN(channel, way, 3582);
			set_badPBN(channel, way, 3588);
			set_badPBN(channel, way, 3590);
			set_badPBN(channel, way, 3596);
			set_badPBN(channel, way, 3598);
			set_badPBN(channel, way, 3594);
			set_badPBN(channel, way, 3593);
			set_badPBN(channel, way, 3598);
			set_badPBN(channel, way, 3604);
			set_badPBN(channel, way, 3606);
			set_badPBN(channel, way, 3608);
			set_badPBN(channel, way, 3612);
			set_badPBN(channel, way, 3609);
			set_badPBN(channel, way, 3641);
			set_badPBN(channel, way, 3627);
			set_badPBN(channel, way, 3624);
			set_badPBN(channel, way, 3626);
			set_badPBN(channel, way, 3665);
			set_badPBN(channel, way, 3666);
			set_badPBN(channel, way, 3679);
			set_badPBN(channel, way, 3695);
			set_badPBN(channel, way, 3701);
			set_badPBN(channel, way, 3693);
			set_badPBN(channel, way, 3703);
			set_badPBN(channel, way, 3707);
			set_badPBN(channel, way, 3708);
			set_badPBN(channel, way, 3714);
			set_badPBN(channel, way, 3740);
			set_badPBN(channel, way, 3746);
			set_badPBN(channel, way, 3762);
			set_badPBN(channel, way, 3768);
			set_badPBN(channel, way, 3782);
			set_badPBN(channel, way, 3783);
			set_badPBN(channel, way, 3804);
			set_badPBN(channel, way, 3810);
			set_badPBN(channel, way, 3829);
			set_badPBN(channel, way, 3828);
			set_badPBN(channel, way, 3831);
			set_badPBN(channel, way, 3840);
			set_badPBN(channel, way, 3850);
			set_badPBN(channel, way, 3857);
			set_badPBN(channel, way, 3858);
			set_badPBN(channel, way, 3870);
			set_badPBN(channel, way, 3858);
			set_badPBN(channel, way, 3870);
			set_badPBN(channel, way, 3858);
			set_badPBN(channel, way, 3866);
			set_badPBN(channel, way, 3864);
			set_badPBN(channel, way, 3871);
			set_badPBN(channel, way, 3869);
			set_badPBN(channel, way, 3875);
			set_badPBN(channel, way, 3877);
			set_badPBN(channel, way, 3882);
			set_badPBN(channel, way, 3886);
			set_badPBN(channel, way, 3888);
			set_badPBN(channel, way, 3885);
			set_badPBN(channel, way, 3895);
			set_badPBN(channel, way, 3896);
			set_badPBN(channel, way, 3900);
			set_badPBN(channel, way, 3906);
			set_badPBN(channel, way, 3913);
			set_badPBN(channel, way, 3914);
			set_badPBN(channel, way, 3922);
			set_badPBN(channel, way, 3929);
			set_badPBN(channel, way, 3930);
			set_badPBN(channel, way, 3938);
			set_badPBN(channel, way, 3939);
			set_badPBN(channel, way, 3945);
			set_badPBN(channel, way, 3946);
			set_badPBN(channel, way, 3956);
			set_badPBN(channel, way, 3957);
			set_badPBN(channel, way, 3961);
			set_badPBN(channel, way, 3960);
			set_badPBN(channel, way, 3965);
			set_badPBN(channel, way, 3972);
			set_badPBN(channel, way, 3973);
			set_badPBN(channel, way, 3969);
			set_badPBN(channel, way, 3977);
			set_badPBN(channel, way, 3978);
			set_badPBN(channel, way, 3985);
			set_badPBN(channel, way, 3986);
			set_badPBN(channel, way, 3993);
			set_badPBN(channel, way, 3990);
			set_badPBN(channel, way, 3992);
			set_badPBN(channel, way, 3994);
			set_badPBN(channel, way, 4001);
			set_badPBN(channel, way, 4005);
			set_badPBN(channel, way, 4009);
			set_badPBN(channel, way, 4011);
			set_badPBN(channel, way, 4010);
			set_badPBN(channel, way, 4015);
			set_badPBN(channel, way, 4016);
			set_badPBN(channel, way, 4017);
			set_badPBN(channel, way, 4031);
			set_badPBN(channel, way, 4024);
			set_badPBN(channel, way, 4018);
			set_badPBN(channel, way, 4032);
			set_badPBN(channel, way, 4030);
			set_badPBN(channel, way, 4026);
			set_badPBN(channel, way, 4033);
			set_badPBN(channel, way, 4032);
			set_badPBN(channel, way, 4033);
			set_badPBN(channel, way, 4028);
			set_badPBN(channel, way, 4029);
			set_badPBN(channel, way, 4037);
			set_badPBN(channel, way, 4034);
			set_badPBN(channel, way, 4036);
			set_badPBN(channel, way, 4034);
			set_badPBN(channel, way, 4041);
			set_badPBN(channel, way, 4040);
			set_badPBN(channel, way, 4042);
			set_badPBN(channel, way, 4049);
			set_badPBN(channel, way, 4051);
			set_badPBN(channel, way, 4057);
			set_badPBN(channel, way, 4050);
			set_badPBN(channel, way, 4065);
			set_badPBN(channel, way, 4064);
			set_badPBN(channel, way, 4058);
			set_badPBN(channel, way, 4067);
			set_badPBN(channel, way, 4064);
			set_badPBN(channel, way, 4065);
			set_badPBN(channel, way, 4070);
			set_badPBN(channel, way, 4066);
			set_badPBN(channel, way, 4067);
			set_badPBN(channel, way, 4073);
			set_badPBN(channel, way, 4067);
			set_badPBN(channel, way, 4065);
			set_badPBN(channel, way, 4070);
			set_badPBN(channel, way, 4066);
			set_badPBN(channel, way, 4067);
			set_badPBN(channel, way, 4073);
			set_badPBN(channel, way, 4065);
			set_badPBN(channel, way, 4075);
			set_badPBN(channel, way, 4085);
			set_badPBN(channel, way, 4073);
			set_badPBN(channel, way, 4084);
			set_badPBN(channel, way, 4081);
			set_badPBN(channel, way, 4075);
			set_badPBN(channel, way, 4076);
			set_badPBN(channel, way, 4074);
			set_badPBN(channel, way, 4083);
			set_badPBN(channel, way, 4081);
			set_badPBN(channel, way, 4089);
			set_badPBN(channel, way, 4088);
			set_badPBN(channel, way, 4083);
			set_badPBN(channel, way, 4091);
			set_badPBN(channel, way, 4089);
			set_badPBN(channel, way, 4090);
			set_badPBN(channel, way, 4093);
			set_badPBN(channel, way, 4089);
			set_badPBN(channel, way, 4092);
			set_badPBN(channel, way, 4090);
			set_badPBN(channel, way, 4091);
			set_badPBN(channel, way, 4108);
			set_badPBN(channel, way, 4109);
			set_badPBN(channel, way, 4106);
			set_badPBN(channel, way, 4114);
			set_badPBN(channel, way, 4117);
			set_badPBN(channel, way, 4119);
			set_badPBN(channel, way, 4121);
			set_badPBN(channel, way, 4135);
			set_badPBN(channel, way, 4131);
			set_badPBN(channel, way, 4130);
			set_badPBN(channel, way, 4145);
			set_badPBN(channel, way, 4145);
			set_badPBN(channel, way, 4154);
			set_badPBN(channel, way, 3614);
			set_badPBN(channel, way, 4082);
			set_badPBN(channel, way, 4184);
			set_badPBN(channel, way, 4185);
			set_badPBN(channel, way, 4186);
			set_badPBN(channel, way, 4187);
			set_badPBN(channel, way, 4188);
			set_badPBN(channel, way, 4189);
			set_badPBN(channel, way, 4190);
			set_badPBN(channel, way, 4191);
			set_badPBN(channel, way, 4192);
			set_badPBN(channel, way, 4193);
			set_badPBN(channel, way, 4194);
			set_badPBN(channel, way, 4195);
			set_badPBN(channel, way, 4196);
			set_badPBN(channel, way, 4197);
			set_badPBN(channel, way, 4198);
			set_badPBN(channel, way, 4199);
			set_badPBN(channel, way, 4200);
			set_badPBN(channel, way, 4201);
			set_badPBN(channel, way, 4202);
			set_badPBN(channel, way, 4203);
			set_badPBN(channel, way, 4204);
			set_badPBN(channel, way, 4205);
			set_badPBN(channel, way, 4206);
			set_badPBN(channel, way, 4207);
			set_badPBN(channel, way, 4208);
			set_badPBN(channel, way, 4209);
			set_badPBN(channel, way, 4210);
			set_badPBN(channel, way, 4211);
			set_badPBN(channel, way, 4212);
			set_badPBN(channel, way, 4213);
			set_badPBN(channel, way, 4214);
			set_badPBN(channel, way, 4215);
			set_badPBN(channel, way, 4216);
			set_badPBN(channel, way, 4217);
			set_badPBN(channel, way, 4218);
			set_badPBN(channel, way, 4219);
			set_badPBN(channel, way, 4221);
			set_badPBN(channel, way, 4222);
			set_badPBN(channel, way, 4223);
			set_badPBN(channel, way, 4220);
			set_badPBN(channel, way, 4224);
			set_badPBN(channel, way, 4225);
			set_badPBN(channel, way, 4226);
			set_badPBN(channel, way, 4227);
			set_badPBN(channel, way, 4228);
			set_badPBN(channel, way, 4229);
			set_badPBN(channel, way, 4230);
			set_badPBN(channel, way, 4231);
			set_badPBN(channel, way, 4232);
			set_badPBN(channel, way, 4233);
			set_badPBN(channel, way, 4234);
			set_badPBN(channel, way, 4235);
			set_badPBN(channel, way, 4236);
			set_badPBN(channel, way, 4237);
			set_badPBN(channel, way, 4238);
			set_badPBN(channel, way, 4239);
			set_badPBN(channel, way, 4240);
			set_badPBN(channel, way, 4241);
			set_badPBN(channel, way, 4242);
			set_badPBN(channel, way, 4243);
			set_badPBN(channel, way, 4244);
			set_badPBN(channel, way, 4245);
			set_badPBN(channel, way, 4246);
			set_badPBN(channel, way, 4247);
			set_badPBN(channel, way, 4248);
			set_badPBN(channel, way, 4249);
			set_badPBN(channel, way, 4250);
			set_badPBN(channel, way, 4251);
			set_badPBN(channel, way, 4252);
			set_badPBN(channel, way, 4253);
			set_badPBN(channel, way, 4254);
			set_badPBN(channel, way, 4255);
			set_badPBN(channel, way, 4256);
			set_badPBN(channel, way, 4257);
			set_badPBN(channel, way, 4258);
			set_badPBN(channel, way, 4259);
			set_badPBN(channel, way, 4260);
			set_badPBN(channel, way, 4261);
			set_badPBN(channel, way, 4262);
			set_badPBN(channel, way, 4263);
			set_badPBN(channel, way, 4264);
			set_badPBN(channel, way, 4265);
			set_badPBN(channel, way, 4266);
			set_badPBN(channel, way, 4267);
			set_badPBN(channel, way, 4268);
			set_badPBN(channel, way, 4269);
			set_badPBN(channel, way, 4270);
			set_badPBN(channel, way, 4271);
			set_badPBN(channel, way, 4272);
			set_badPBN(channel, way, 4273);
			set_badPBN(channel, way, 4274);
			set_badPBN(channel, way, 4275);
			set_badPBN(channel, way, 4276);
			set_badPBN(channel, way, 4277);
			set_badPBN(channel, way, 4278);
			set_badPBN(channel, way, 4279);
			set_badPBN(channel, way, 4280);
			set_badPBN(channel, way, 4281);
			set_badPBN(channel, way, 4282);
			set_badPBN(channel, way, 4283);
			set_badPBN(channel, way, 4284);
			set_badPBN(channel, way, 4285);
			set_badPBN(channel, way, 4286);
			set_badPBN(channel, way, 4287);
			set_badPBN(channel, way, 4288);
			set_badPBN(channel, way, 4289);
			set_badPBN(channel, way, 4290);
			set_badPBN(channel, way, 4291);
			set_badPBN(channel, way, 4292);
			set_badPBN(channel, way, 4293);
			set_badPBN(channel, way, 4294);
			set_badPBN(channel, way, 4295);
			set_badPBN(channel, way, 4296);
			set_badPBN(channel, way, 4297);
			set_badPBN(channel, way, 4298);
			set_badPBN(channel, way, 4299);
			set_badPBN(channel, way, 4300);
			set_badPBN(channel, way, 4301);
			set_badPBN(channel, way, 4302);
			set_badPBN(channel, way, 4303);
			set_badPBN(channel, way, 4304);
			set_badPBN(channel, way, 4305);
			set_badPBN(channel, way, 4306);
			set_badPBN(channel, way, 4307);
			set_badPBN(channel, way, 4308);
			set_badPBN(channel, way, 4309);
			set_badPBN(channel, way, 4310);
			set_badPBN(channel, way, 4311);
			set_badPBN(channel, way, 4312);
			set_badPBN(channel, way, 4313);
			set_badPBN(channel, way, 4314);
			set_badPBN(channel, way, 4315);
			set_badPBN(channel, way, 4316);
			set_badPBN(channel, way, 4317);
			set_badPBN(channel, way, 4318);
			set_badPBN(channel, way, 4319);
			set_badPBN(channel, way, 4320);
			set_badPBN(channel, way, 4321);
			set_badPBN(channel, way, 4322);
			set_badPBN(channel, way, 4323);
			set_badPBN(channel, way, 4324);
			set_badPBN(channel, way, 4325);
			set_badPBN(channel, way, 4326);
			set_badPBN(channel, way, 4327);
			set_badPBN(channel, way, 4328);
			set_badPBN(channel, way, 4329);
			set_badPBN(channel, way, 4330);
			set_badPBN(channel, way, 4331);
			set_badPBN(channel, way, 4332);
			set_badPBN(channel, way, 4333);
			set_badPBN(channel, way, 4334);
			set_badPBN(channel, way, 4335);
			set_badPBN(channel, way, 4336);
			set_badPBN(channel, way, 4337);
			set_badPBN(channel, way, 4338);
			set_badPBN(channel, way, 4339);
			set_badPBN(channel, way, 4340);
			set_badPBN(channel, way, 4341);
			set_badPBN(channel, way, 4342);
			set_badPBN(channel, way, 4343);
			set_badPBN(channel, way, 4344);
			set_badPBN(channel, way, 4345);
			set_badPBN(channel, way, 4346);
			set_badPBN(channel, way, 4347);
			set_badPBN(channel, way, 4348);
			set_badPBN(channel, way, 4349);
			set_badPBN(channel, way, 4350);
			set_badPBN(channel, way, 4351);
			set_badPBN(channel, way, 4352);
			set_badPBN(channel, way, 4353);
			set_badPBN(channel, way, 4354);
			set_badPBN(channel, way, 4355);
			set_badPBN(channel, way, 4356);
			set_badPBN(channel, way, 4357);
			set_badPBN(channel, way, 4358);
			set_badPBN(channel, way, 4359);
			set_badPBN(channel, way, 4360);
			set_badPBN(channel, way, 4361);
			set_badPBN(channel, way, 4362);
			set_badPBN(channel, way, 4363);
			set_badPBN(channel, way, 4364);
			set_badPBN(channel, way, 4365);
			set_badPBN(channel, way, 4366);
			set_badPBN(channel, way, 4367);
			set_badPBN(channel, way, 4368);
			set_badPBN(channel, way, 4369);
			set_badPBN(channel, way, 4370);
			set_badPBN(channel, way, 4371);
			set_badPBN(channel, way, 4372);
			set_badPBN(channel, way, 4373);
			set_badPBN(channel, way, 4374);
			set_badPBN(channel, way, 4375);
			set_badPBN(channel, way, 4376);
			set_badPBN(channel, way, 4377);
			set_badPBN(channel, way, 4378);
			set_badPBN(channel, way, 4379);
			set_badPBN(channel, way, 4380);
			set_badPBN(channel, way, 4381);
			set_badPBN(channel, way, 4382);
			set_badPBN(channel, way, 4383);
			set_badPBN(channel, way, 4384);
			set_badPBN(channel, way, 4385);
			set_badPBN(channel, way, 4386);
			set_badPBN(channel, way, 4387);
			set_badPBN(channel, way, 4388);
			set_badPBN(channel, way, 4389);
			set_badPBN(channel, way, 4390);
			set_badPBN(channel, way, 4391);
			set_badPBN(channel, way, 4392);
			set_badPBN(channel, way, 4393);
			set_badPBN(channel, way, 4394);
			set_badPBN(channel, way, 4395);
			set_badPBN(channel, way, 4396);
			set_badPBN(channel, way, 4397);
			set_badPBN(channel, way, 4398);
			set_badPBN(channel, way, 4399);
			set_badPBN(channel, way, 4400);
			set_badPBN(channel, way, 4401);
			set_badPBN(channel, way, 4402);
			set_badPBN(channel, way, 4403);
			set_badPBN(channel, way, 4404);
			set_badPBN(channel, way, 4405);
			set_badPBN(channel, way, 4406);
			set_badPBN(channel, way, 4407);
			set_badPBN(channel, way, 4408);
			set_badPBN(channel, way, 4409);
			set_badPBN(channel, way, 4410);
			set_badPBN(channel, way, 4411);
			set_badPBN(channel, way, 4412);
			set_badPBN(channel, way, 4413);
			set_badPBN(channel, way, 4414);
			set_badPBN(channel, way, 4415);
			set_badPBN(channel, way, 4416);
			set_badPBN(channel, way, 4417);
			set_badPBN(channel, way, 4418);
			set_badPBN(channel, way, 4419);
			set_badPBN(channel, way, 4420);
			set_badPBN(channel, way, 4421);
			set_badPBN(channel, way, 4422);
			set_badPBN(channel, way, 4423);
			set_badPBN(channel, way, 4424);
			set_badPBN(channel, way, 4425);
			set_badPBN(channel, way, 4426);
			set_badPBN(channel, way, 4427);
			set_badPBN(channel, way, 4428);
			set_badPBN(channel, way, 4429);
			set_badPBN(channel, way, 4430);
			set_badPBN(channel, way, 4431);
			set_badPBN(channel, way, 4432);
			set_badPBN(channel, way, 4433);
			set_badPBN(channel, way, 4434);
			set_badPBN(channel, way, 4435);
			set_badPBN(channel, way, 4436);
			set_badPBN(channel, way, 4437);
			set_badPBN(channel, way, 4438);
			set_badPBN(channel, way, 4439);
			set_badPBN(channel, way, 4440);
			set_badPBN(channel, way, 4441);
			set_badPBN(channel, way, 4442);
			set_badPBN(channel, way, 4443);
			set_badPBN(channel, way, 4444);
			set_badPBN(channel, way, 4445);
			set_badPBN(channel, way, 4446);
			set_badPBN(channel, way, 4447);
			set_badPBN(channel, way, 4167);
			set_badPBN(channel, way, 4178);
			set_badPBN(channel, way, 4180);
			set_badPBN(channel, way, 4177);
			set_badPBN(channel, way, 4179);
		}
	}
#endif
}

void FIL_Run(void)
{
	if (FIL_IsIdle() == TRUE)
	{
		return;
	}

	// not implemented yet!
	FIL_ProcessDoneQ();
	FIL_ProcessIssuedQ();
	FIL_ProcessWaitQ();
}

/*
	@brief check a block is bad or not.
			channel and way does not considered in current implementation.
			the reason is FTL uses VBlock, which is super block.
*/
BOOL FIL_IsBad(INT32 channel, INT32 way, INT32 nPBN)
{
	// Bad block of cosmos+
	/*static int anBadPBN2[] = 
	{
		2300, 2312, 2327, 2336, 2368, 2379, 2386, 2399,
		2135, 2136, 2138, 2147, 2162, 2172, 2255, 2259, 2270, 2271, 2275, 2283, 2285,
		1920, 1927, 1937, 1964, 1990, 2008, 2009, 2037, 2051, 2095, 2111, 2113,
		1728, 1739, 1747, 1765, 1790, 1798, 1814, 1841, 1859, 1881, 1895,
		1626, 1629, 1650, 1663, 1685, 1695, 1704, 1708, 1711, 1715,
		1332, 1344, 1348, 1355, 1380, 1388, 1391, 1395, 1481, 1529, 1539, 1566, 1613, 1625,
		1161, 1204, 1205, 1209, 1211, 1225, 1231, 1241, 1242, 1277, 1283, 1311,
		424, 429, 521, 543, 594, 629, 640, 909, 1095, 1098, 1101, 1113,
		928, 566, 564, 562, 256, 320, 330, 415, 481, 505, 517, 527 ,565, 579, 585, 685, 699,
		712, 714, 723, 735, 770, 776, 777, 801, 811, 818, 844 ,849, 886, 919, 927, 935, 949, 
		952, 974, 983, 997, 1017, 1019, 1033, 1049, 1054, 1064, 1099, 1147, 1157, 1183, 1190, 
		1195, 1232, 1250, 1263, 1287, 1330, 1334, 13335, 1343, 1354, 1357, 1363, 1370, 1374, 
		1386, 1400, 1407, 1423, 1443, 1457, 1540, 1569, 1580, 1596, 1597, 1602, 1614, 1651,
		1658, 1659, 1664, 1670, 1674, 1675, 1679, 1694, 1710, 1714, 1754, 1762, 1813, 1815,
		1822, 1828, 1842, 1845, 1852, 1862, 1897, 1899, 1925, 1933, 1936, 1946, 1950, 1991, 
		2027, 2032, 2038, 2047, 2060, 2072, 2080, 2094, 2116, 2120, 2121, 2123, 2169, 2177, 2185, 
		2187, 2203, 2209, 2212, 2211, 2221, 2229, 2238, 2265, 2307, 2309, 2314, 2319, 2350, 2359, 2384,
		2388, 2389, 2397, 849, 1263, 2094, 1263, 2094, 949, 1330, 1580, 1670, 935, 1064, 1549, 1259,
		1335, 1206, 1221, 1223, 1229, 1255, 1257, 1273, 1310, 1316, 1317, 1319, 1336, 1358, 1364, 
		1369, 1389, 1394, 1424, 1449, 1453, 1455, 1467, 1482, 1489, 1500, 1501, 1532, 1534, 1552, 
		1565, 1590, 1608, 1610, 1638, 1642, 1666, 1698, 1702, 1752, 1768, 1776, 1795, 1810, 1820, 
		1824, 1824, 1848, 1856, 1866, 1870, 1880, 1885, 1892, 1894, 1900, 1902, 1926, 1940, 1942, 1961, 
		1972, 1996, 2004, 2012, 2029, 2035, 2054, 2058, 2067, 2077, 2086, 2110, 2114, 2118, 2152, 
		2155, 2164, 2173, 2179, 1825, 2220, 2405, 2407, 2408, 2438, 2446, 2455, 2456, 2457, 2459, 2468, 
		2484, 2491, 2490, 2500, 2509, 2532, 2535, 2540, 2543, 2551, 2550, 2557, 2600, 2603, 2604, 2616, 2617, 
		2645, 2674, 2701, 2714, 2727, 2733, 2738, 2741, 2747, 2762, 2769, 2781, 2795, 2802, 2806, 2445, 2780,
		2837, 2845, 2854, 2856, 2876, 2877, 2882, 2888, 2895, 2902, 2920, 2926, 2927, 2928, 2951, 2967, 2966, 
		2970, 2979, 2978, 2989, 2994, 3004, 3008, 3035, 3060, 3061, 3072, 3078, 3079, 3078, 3100, 3116, 3138, 
		3144, 3159, 3168, 3175, 3185, 3188, 3189, 3187, 3192, 3201, 3214, 3219, 3231, 3248, 3259, 3178, 3284, 3285, 
		3287, 3295, 3304, 3315, 3319,3325,3327,3344,3342,3343,3360,3367,3383,3382,3397,3400,3402,3407,3402,3416,
		3417,3429,3431,3433,3459,3457,3460,3471,3496,3497,3503,3501,3509,3511,3520,3525,3524,3541,3540,3573,3566,
		3572,3574,3575,3580,3582,3588,3590,3596,3598,3594,3593,3598,3604,3606,3608,3612,3609,3641,3627,3624,3626,3665,
		3666,3679,3695,3701,3693,3703,3707,3708,3714,3740,3746,3762,3768,3782,3783,3804,3810,3829,3828,3831,3840,3850,3857,3858,
		3870,3858,3870,3858,3866,3864,3871,3869,3875,3877,3882,3886,3888,3885,3895,3896,3900,3906,3913,3914,3922,3929,3930,3938,3939,
		3945,3946,3956,3957,3961,3960,3965,3972,3973,3969,3977,3978,3985,3986,3993,3990,3992,3994,4001,4005,4009,4011,4010,4015,4016,4017,
		4031,4024,4018,4032,4030,4026,4033,4032,4033,4028,4029,4037,4034,4036,4034,4041,4040,4042,4049,4051,4057,4050,4065,4064,4058,4067,4064,4065,
		4070,4066,4067,4073,4067,4065,4070,4066,4067,4073,4065,4075,4085,4073,4084,4081,4075,4076,4074,4083,4081,4089,4088,4083,4091,
		4089,4090,4093,4089,4092,4090,4091,4108,4109,4106,4114,4117,4119,4121,4135,4131,4130,4145,4145,4154, 3614, 4082,
		4184, 4185, 4186, 4187, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4197, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209,
		4210, 4211, 4212, 4213, 4214, 4215, 4216, 4217, 4218, 4219, 4221, 4222, 4223, 4220, 4224, 4225, 4226, 4227, 4228, 4229, 4230, 4231, 4232, 4233, 4234, 4235,
		4236, 4237, 4238, 4239, 4240, 4241, 4242, 4243, 4244, 4245, 4246, 4247, 4248, 4249, 4250, 4251, 4252, 4253, 4254, 4255, 4256, 4257, 4258, 4259, 4260, 4261, 4262,
		4263, 4264, 4265, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4278, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4289,
		4290, 4291, 4292, 4293, 4294, 4295, 4296, 4297, 4298, 4299, 4300, 4301, 4302, 4303, 4304, 4305, 4306, 4307, 4308, 4309, 4310, 4311, 4312, 4313, 4314, 4315,  4316,
		4317, 4318, 4319, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327, 4328, 4329, 4330, 4331, 4332, 4333, 4334, 4335, 4336, 4337, 4338, 4339, 4340, 4341, 4342, 4343, 4344,
		4345, 4346, 4347, 4348, 4349, 4350, 4351, 4352, 4353, 4354, 4355, 4356, 4357, 4358, 4359, 4360, 4361, 4362, 4363, 4364, 4365, 4366, 4367, 4368, 4369, 4370, 4371, 4372, 4373,
		4374, 4375, 4376, 4377, 4378, 4379, 4380, 4381, 4382, 4383, 4384, 4385, 4386, 4387, 4388, 4389, 4390, 4391, 4392, 4393, 4394, 4395, 4396, 4397, 4398, 4399, 4400, 4401, 4402,
		4403, 4404, 4405, 4406, 4407, 4408, 4409, 4410, 4411, 4412, 4413, 4414, 4415, 4416, 4417, 4418, 4419, 4420, 4421, 4422, 4423, 4424, 4425, 4426, 4427, 4428, 4429, 4430, 4431,
		4432, 4433, 4434, 4435, 4436, 4437, 4438, 4439, 4440, 4441, 4442, 4443, 4444, 4445, 4446, 4447, 4167, 4178, 4180, 4177, 4179,
	};
	for (int i = 0; i < sizeof(anBadPBN2) / sizeof(int); i++)
	{
		if (anBadPBN2[i] == nPBN)
		{
			return TRUE;
		}
	}*/
	static int anBadPBN2[] =
	{
		2300, 2312, 2327, 2336, 2368, 2379, 2386, 2399,
		2135, 2136, 2138, 2147, 2162, 2172, 2255, 2259, 2270, 2271, 2275, 2283, 2285,
		1920, 1927, 1937, 1964, 1990, 2008, 2009, 2037, 2051, 2095, 2111, 2113,
		1728, 1739, 1747, 1765, 1790, 1798, 1814, 1841, 1859, 1881, 1895,
		1626, 1629, 1650, 1663, 1685, 1695, 1704, 1708, 1711, 1715,
		1332, 1344, 1348, 1355, 1380, 1388, 1391, 1395, 1481, 1529, 1539, 1566, 1613, 1625, 1990,
		1161, 1204, 1205, 1209, 1211, 1225, 1231, 1241, 1242, 1277, 1283, 1311,
		424, 429, 521, 543, 594, 629, 640, 909, 1095, 1098, 1101, 1113,
		928, 566, 564, 562, 256, 320, 330, 415, 481, 505, 517, 527 ,565, 579, 585, 685, 699,
		712, 714, 723, 735, 770, 776, 777, 801, 811, 818, 844 ,849, 886, 919, 927, 935, 949,
		952, 974, 983, 997, 1017, 1019, 1033, 1049, 1054, 1064, 1099, 1147, 1157, 1183, 1190,
		1195, 1232, 1250, 1263, 1287, 1330, 1334, 13335, 1343, 1354, 1357, 1363, 1370, 1374,
		1386, 1400, 1407, 1423, 1443, 1457, 1540, 1569, 1580, 1596, 1597, 1602, 1614, 1651,
		1658, 1659, 1664, 1670, 1674, 1675, 1679, 1694, 1710, 1714, 1754, 1762, 1813, 1815,
		1822, 1828, 1842, 1845, 1852, 1862, 1897, 1899, 1925, 1933, 1936, 1946, 1950, 1991,
		2027, 2032, 2038, 2047, 2060, 2072, 2080, 2094, 2116, 2120, 2121, 2123, 2169, 2177, 2185,
		2187, 2203, 2209, 2212, 2211, 2221, 2229, 2238, 2265, 2307, 2309, 2314, 2319, 2350, 2359, 2384,
		2388, 2389, 2397, 849, 1263, 2094, 1263, 2094, 949, 1330, 1580, 1670, 935, 1064, 1549, 1259,
		1335, 1206, 1221, 1223, 1229, 1255, 1257, 1273, 1310, 1316, 1317, 1319, 1336, 1358, 1364,
		1369, 1389, 1394, 1424, 1449, 1453, 1455, 1467, 1482, 1489, 1500, 1501, 1532, 1534, 1552,
		1565, 1590, 1608, 1610, 1638, 1642, 1666, 1698, 1702, 1752, 1768, 1776, 1795, 1810, 1820,
		1824, 1824, 1848, 1856, 1866, 1870, 1880, 1885, 1892, 1894, 1900, 1902, 1926, 1940, 1942, 1961,
		1972, 1996, 2004, 2012, 2029, 2035, 2054, 2058, 2067, 2077, 2086, 2110, 2114, 2118, 2152,
		2155, 2164, 2173, 2179, 1825, 2220, 2405, 2407, 2408, 2438, 2446, 2455, 2456, 2457, 2459, 2468,
		2484, 2491, 2490, 2500, 2509, 2532, 2535, 2540, 2543, 2551, 2550, 2557, 2600, 2603, 2604, 2616, 2617,
	};
	for (int i = 0; i < sizeof(anBadPBN2) / sizeof(int); i++)
	{
		if (anBadPBN2[i] == nPBN)
		{
			break;
		}
	}

	UINT32 offset = nPBN >> 3;
	UINT32 offset_in_byte = nPBN % 8;
	if (anBadPBN[channel][way][offset] & (1 << offset_in_byte))
	{
		return TRUE;
	}
	

	return FALSE;
}

BOOL FIL_IsEmpty_way(INT32 channel, INT32 way)
{
	return _FIL_IsEmptyWay(channel, way);
}

/*
	@brief	COSMOS+ does not support partial page read 
*/
void FIL_ReadPage(FTL_REQUEST_ID stReqID, NAND_ADDR stPhyAddr, void* pMainBuf, void* pSpareBuf)
{
#ifdef SUPPORT_DATA_VERIFICATION
	#ifdef WIN32
		OSAL_MEMSET(pMainBuf, 0xFF, (sizeof(INT32) * 4/*LPN_PER_PHYSICAL_PAGE*/));
	#endif
		OSAL_MEMSET(pSpareBuf, 0xFF, (sizeof(INT32) * 4 /*LPN_PER_PHYSICAL_PAGE*/));
#endif
	
	_AddToWaitQ(FTL_REQUEST_READ, stReqID, stPhyAddr, pMainBuf, pSpareBuf);
}

void FIL_ProgramPage(FTL_REQUEST_ID	stReqID, NAND_ADDR stPhyAddr, void* pMainBuf, void* pSpareBuf)
{
	_AddToWaitQ(FTL_REQUEST_PROGRAM, stReqID, stPhyAddr, pMainBuf, pSpareBuf);
}

void FIL_EraseBlock(FTL_REQUEST_ID	stReqID, NAND_ADDR stPhyAddr)
{
	_AddToWaitQ(FTL_REQUEST_ERASE, stReqID, stPhyAddr, NULL, NULL);
}

int FIL_GetPagesPerBlock(void)
{
	int nPagesPerBlock;

	if (BITS_PER_CELL == 1)
	{
		nPagesPerBlock = PAGES_PER_SLC_BLOCK;
	}
	else if (BITS_PER_CELL == 2)
	{
		nPagesPerBlock = PAGES_PER_MLC_BLOCK;
	}
	else
	{
		ASSERT(0);	// not supported
		nPagesPerBlock = 0;
	}

	return nPagesPerBlock;
}

static void _AddToWaitQ(FTL_REQUEST_TYPE nReqType, FTL_REQUEST_ID stReqID, NAND_ADDR stPhyAddr, void* pMainBuf, void* pSpareBuf)
{
#if defined(_DEBUG) && defined(STREAM_FTL)
	DEBUG_ASSERT(stReqID.stCommon.nType < FTL_REQUEST_ID_TYPE_COUNT);
	switch (stReqID.stCommon.nType)
	{
		case FTL_REQUEST_ID_TYPE_PROGRAM:
			// In this context FIL does not know what sub-type of write, (sub type must be one of HIL Write, GC Write, StreamMerge Write)
			break;

		case FTL_REQUEST_ID_TYPE_HIL_READ:
		case FTL_REQUEST_ID_TYPE_STREAM_MERGE_READ:
		case FTL_REQUEST_ID_TYPE_BLOCK_GC_READ:
			break;

		default:
			ASSERT(0);		// Invalid type for NAND IO
			break;
	}

	DEBUG_ASSERT(pMainBuf);
	DEBUG_ASSERT(pSpareBuf);
#endif

	// Allocate Request
	FTL_REQUEST*	pstRequest;
	pstRequest = FIL_AllocateRequest();

	DEBUG_ASSERT(pstRequest->nStatus == FTL_REQUEST_FREE);

	pstRequest->stRequestID = stReqID;
	pstRequest->nType = nReqType;
	pstRequest->stAddr = stPhyAddr;
	pstRequest->stBuf.pMainBuf = pMainBuf;
	pstRequest->stBuf.pSpareBuf = pSpareBuf;

	switch (nReqType)
	{
	case FTL_REQUEST_READ:
		pstRequest->nStatus = FTL_REQUEST_READ_WAIT;
		break;

	case FTL_REQUEST_PROGRAM:
		if (stPhyAddr.nPPage == 0)
		{
			pstRequest->nStatus = FTL_REQUEST_ERASE_FOR_PROGRAM_WAIT;
		}
		else
		{
			pstRequest->nStatus = FTL_REQUEST_PROGRAM_WAIT;
		}
		break;

	case FTL_REQUEST_ERASE:
		pstRequest->nStatus = FTL_REQUEST_ERASE_WAIT;
		break;
	}

	// Add to waitQ
	FIL_AddToRequestWaitQ(pstRequest, FALSE);

}
