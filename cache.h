#ifndef _CACHE_H
#define _CACHE_H

#include<stdint.h>

typedef struct dLine
{
	uint8_t valid;//有效位 
	int tag;//标记位
	uint32_t block[8];//块数据 
	int lru;//值越大表示可以使用 
	uint8_t dirty;//脏位，数据被改变 
}dLine;

typedef struct dSet
{
	dLine *line;//8路组相联映射，每组8行 
}dSet;

typedef struct dCache
{
	int setNum;//组数 
	int lineNum;//行数 
	dSet *sets;//cache共256组 
}dCache;

typedef struct iLine
{
	uint8_t valid;//有效位 
	int tag;//标记位
	uint32_t block[8];//块数据 
	int lru;//值越大表示可以使用 
}iLine;

typedef struct iSet
{
	iLine *line;
}iSet;

typedef struct iCache
{
	int setNum;//组数 
	int lineNum;//行数 
	iSet *sets;
}iCache;

extern int delay1,delay2;

void initDCache(dCache *cache);

void initICache(iCache *cache);

void dUpdateLru(dCache *cache,int setBits,int hitIndex);

void iUpdateLru(iCache *cache,int setBits,int hitIndex);

int dFindMaxLru(dCache *cache,int setBits);

int iFindMaxLru(iCache *cache,int setBits);

void updateDCache(dCache *cache,uint32_t address,int s,int b);

void updateICache(iCache *cache,uint32_t address,int s,int b);

int dIsMiss(dCache *cache,int setBits,int tag);

int iIsMiss(iCache *cache,int setBits,int tag);

uint32_t dLoadData(dCache *cache,uint32_t address);

uint32_t iLoadData(iCache *cache,uint32_t address);

void storeData(dCache *cache,uint32_t address,uint32_t val);
#endif
