
#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#define FOREACH_PROFILE(PROFILE) \
	PROFILE(PROFILE_NAND_WRITE) \
	PROFILE(PROFILE_NAND_READ) \
	PROFILE(PROFILE_NAND_ERASE) \
	PROFILE(PROFILE_HOST_WRITE) \
	PROFILE(PROFILE_HOST_READ) \
	PROFILE(PROFILE_HOST_OVERWRITE) \
	PROFILE(PROFILE_HOST_UNMAP_READ) \
	PROFILE(PROFILE_HOST_WRITE_REQ) \
	PROFILE(PROFILE_HOST_READ_REQ) \
	PROFILE(PROFILE_BGC_WRITE) \
	PROFILE(PROFILE_BGC_READ) \
	PROFILE(PROFILE_BGC_ERASE) \
	PROFILE(PROFILE_BGC) \
	PROFILE(PROFILE_SMERGE_write) \
	PROFILE(PROFILE_SMERGE_read) \
	PROFILE(PROFILE_SMERGE_erase) \
	PROFILE(PROFILE_SMERGE_FULL_INVALID) \
	PROFILE(PROFILE_SMERGE) \
	PROFILE(PROFILE_SMERGE_victim) \
	PROFILE(PROFILE_SMERGE_meet_max_stream) \
	PROFILE(PROFILE_IO_ERASE) \
	PROFILE(PROFILE_GC_ERASE) \
	PROFILE(PROFILE_META_ERASE) \
	PROFILE(PROFILE_L2PCACHE_HIT_HOST) \
	PROFILE(PROFILE_L2PCACHE_MISS_HOST) \
	PROFILE(PROFILE_L2PCache_Hit_SMERGE) \
	PROFILE(PROFILE_L2PCache_Miss_SMERGE) \
	PROFILE(PROFILE_L2PCache_Hit_BLOCKGC) \
	PROFILE(PROFILE_L2PCache_Miss_BLOCKGC) \
	PROFILE(PROFILE_BGC_WITH_SMERGE) \
	PROFILE(PROFILE_SMERAGE_WHILE_BGC) \
	PROFILE(PROFILE_L2PCACHE_HIT) \
	PROFILE(PROFILE_L2PCACHE_MISS) \
	PROFILE(PROFILE_META_READ) \
	PROFILE(PROFILE_META_WRITE) \
	PROFILE(PROFILE_FULL_INVALID_BLOCK) \
	PROFILE(PROFILE_FULL_INVALID_STREAM) \
	PROFILE(PROFILE_MISCOMAPRE) \
	PROFILE(PROFILE_ACTIVEBLOCK_PROGRAM_UNIT_FULL) \
	PROFILE(PROFILE_OUT_OF_BUFFER) \
	PROFILE(PROFILE_COUNT) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum 
{
	FOREACH_PROFILE(GENERATE_ENUM)
} PROFILE_TYPE;

typedef struct
{
	PROFILE_TYPE	nType;
	unsigned int	nCount;
} PROFILE_ENTRY;

#if defined(__cplusplus) && defined(WIN32)
extern "C" {
#endif

	void	STAT_Initialize(VOID);
	void	STAT_IncreaseCount(PROFILE_TYPE eType, unsigned int nCnt);
	unsigned int	STAT_GetCount(PROFILE_TYPE eType);
	void STAT_Print(void);

#if defined(__cplusplus) && defined(WIN32)
}
#endif

#endif	// end of #ifndef __UTIL_H__
