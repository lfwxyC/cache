#ifndef _CACHE_H
#define _CACHE_H

#include<stdint.h>

typedef struct dLine
{
	uint8_t valid;//��Чλ 
	int tag;//���λ
	uint32_t block[8];//������ 
	int lru;//ֵԽ���ʾ����ʹ�� 
	uint8_t dirty;//��λ�����ݱ��ı� 
}dLine;

typedef struct dSet
{
	dLine *line;//8·������ӳ�䣬ÿ��8�� 
}dSet;

typedef struct dCache
{
	int setNum;//���� 
	int lineNum;//���� 
	dSet *sets;//cache��256�� 
}dCache;

typedef struct iLine
{
	uint8_t valid;//��Чλ 
	int tag;//���λ
	uint32_t block[8];//������ 
	int lru;//ֵԽ���ʾ����ʹ�� 
}iLine;

typedef struct iSet
{
	iLine *line;
}iSet;

typedef struct iCache
{
	int setNum;//���� 
	int lineNum;//���� 
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
