#include"cache.h"
#include"shell.h"
#include<stdlib.h>
#include<time.h>
#include<stdio.h>

//初始化 
void initDCache(dCache *cache)
{
	cache->setNum=64;
	cache->lineNum=8;
	cache->sets=(dSet *)malloc(cache->setNum*sizeof(dSet));
	int i,j;
	
	for(i=0;i<cache->setNum;i++)
	{
		cache->sets[i].line=(dLine *)malloc(cache->lineNum*sizeof(dLine));
		
		for(j=0;j<cache->lineNum;j++)
		{
			cache->sets[i].line[j].valid=0;
			cache->sets[i].line[j].lru=0;
			cache->sets[i].line[j].dirty=0;
		}
	}
}

void initICache(iCache *cache)
{
	cache->setNum=64;
	cache->lineNum=4;
	cache->sets=(iSet *)malloc(cache->setNum*sizeof(iSet));
	int i,j;
	
	for(i=0;i<cache->setNum;i++)
	{
		cache->sets[i].line=(iLine *)malloc(cache->lineNum*sizeof(iLine));
		
		for(j=0;j<cache->lineNum;j++)
		{
			cache->sets[i].line[j].valid=0;
			cache->sets[i].line[j].lru=0;
		}
	}
}

//更新lru 
void dUpdateLru(dCache *cache,int setBits,int hitIndex)
{
	cache->sets[setBits].line[hitIndex].lru=0;//最近使用过的lru值置为最小 
	int i;
	
	//未使用过的lru值+1 
	for(i=0;i<cache->lineNum;i++)
	{
		if(i!=hitIndex)
		{
			cache->sets[setBits].line[i].lru++;
		}
	}
}

void iUpdateLru(iCache *cache,int setBits,int hitIndex)
{
	cache->sets[setBits].line[hitIndex].lru=0;//最近使用过的lru值置为最小 
	int i;
	
	//未使用过的lru值+1 
	for(i=0;i<cache->lineNum;i++)
	{
		if(i!=hitIndex)
		{
			cache->sets[setBits].line[i].lru++;
		}
	}
}

//找最大的lru值，作为可使用行 
int dFindMaxLru(dCache *cache,int setBits)
{
	int maxLru=0,maxIndex=0,i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].lru>maxLru)
		{
			maxLru=cache->sets[setBits].line[i].lru;
			maxIndex=i;
		}
	}
	return maxIndex;//返回行号 
}

int iFindMaxLru(iCache *cache,int setBits)
{
	int maxLru=0,maxIndex=0,i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].lru>maxLru)
		{
			maxLru=cache->sets[setBits].line[i].lru;
			maxIndex=i;
		}
	}
	return maxIndex;//返回行号 
}

int randomEliminate(int lineSize)
{
	srand((unsigned)time(NULL));
	int index = rand() % lineSize;
	return index;
}

//获取标记 
int getTag(uint32_t address,int s,int b)//s:组索引，b:块内地址 
{
	int x=s+b;
	return address>>x;
}

//获取cache组号 
int getSet(uint32_t address,int s,int b)
{
	int setBits=address>>b;
	int x=(1<<s)-1;
	return setBits&x;
}

//更新cache 
void updateDCache(dCache *cache,uint32_t address,int s,int b) 
{
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int eliminateIndex=dFindMaxLru(cache,setBits);
	//int eliminateIndex=randomEliminate(cache->lineNum);
	int i;
	
	//若该行为替换行且数据被改变，则要改变主存中相应的数据 
	if(cache->sets[setBits].line[eliminateIndex].valid==1&&cache->sets[setBits].line[eliminateIndex].dirty==1)
	{
		//获取主存地址 
		int tag=cache->sets[setBits].line[eliminateIndex].tag;
		uint32_t add=(tag<<s)|(setBits);
		add=add<<b;
		
		for(i=0;i<8;i++)
		{
			mem_write_32(add, cache->sets[setBits].line[eliminateIndex].block[i]);
			add+=4;//一个指令4字节 
		}
	}
	
	cache->sets[setBits].line[eliminateIndex].valid=1;
	cache->sets[setBits].line[eliminateIndex].tag=tag;
	address=(address>>5)<<5;//计算首地址，块内地址为5位 
	
	//把主存块内容移至cache 
	for(i=0;i<8;i++)
	{
		cache->sets[setBits].line[eliminateIndex].block[i]=mem_read_32(address);
		address+=4;
	}
	dUpdateLru(cache,setBits,eliminateIndex);
}

void updateICache(iCache *cache,uint32_t address,int s,int b) 
{
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int eliminateIndex=iFindMaxLru(cache,setBits);
	//int eliminateIndex=randomEliminate(cache->lineNum);
	int i;
	
	cache->sets[setBits].line[eliminateIndex].valid=1;
	cache->sets[setBits].line[eliminateIndex].tag=tag;
	address=(address>>5)<<5;//计算首地址，块内地址为5位 
	
	//把主存块内容移至cache 
	for(i=0;i<8;i++)
	{
		cache->sets[setBits].line[eliminateIndex].block[i]=mem_read_32(address);
		address+=4;
	}
	iUpdateLru(cache,setBits,eliminateIndex);
}

//判断是否命中 
int dIsMiss(dCache *cache,int setBits,int tag)
{
	uint8_t isMiss=1;
	int i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].valid==1&&cache->sets[setBits].line[i].tag==tag)
		{
			return i;//返回行号 
		}
	}
	return -1;//未命中 
}

int iIsMiss(iCache *cache,int setBits,int tag)
{
	uint8_t isMiss=1;
	int i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].valid==1&&cache->sets[setBits].line[i].tag==tag)
		{
			return i;//返回行号 
		}
	}
	return -1;//未命中 
}

//从cache中取数据 
uint32_t dLoadData(dCache *cache,uint32_t address)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=dIsMiss(cache,setBits,tag);
	
	//命中 
	if(index!=-1)
	{
		int offset=address&0x0000001f;//取块内地址 
		offset=offset>>2;//数组索引0~8 
		
		dUpdateLru(cache,setBits,index);
		return cache->sets[setBits].line[index].block[offset];
	}
	
	//未命中
	uint32_t val = mem_read_32(address);//从主存中读取 
	
	delay1=50;
	updateDCache(cache,address,s,b);//把主存的相应块移至cache中 
	return val;
}

uint32_t iLoadData(iCache *cache,uint32_t address)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=iIsMiss(cache,setBits,tag);
	
	//命中 
	if(index!=-1)
	{
		int offset=address&0x0000001f;//取块内地址 
		offset=offset>>2;//数组索引0~8 
		
		iUpdateLru(cache,setBits,index);
		return cache->sets[setBits].line[index].block[offset];
	}
	
	//未命中
	uint32_t val = mem_read_32(address);//从主存中读取 
	
	delay2=50;
	updateICache(cache,address,s,b);//把主存的相应块移至cache中 
	return val;
}

//修改cache中的数据 
void storeData(dCache *cache,uint32_t address,uint32_t val)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=dIsMiss(cache,setBits,tag);
	
	//命中 
	if(index!=-1)
	{
		int offset=address&0x0000001f;
		offset=offset>>2;
		
		dUpdateLru(cache,setBits,index);
		cache->sets[setBits].line[index].block[offset]=val; 
		cache->sets[setBits].line[index].dirty=1;//数据被改变 
		return;
	}
	
	//未命中 
	mem_write_32(address, val);//修改主存中的数据 
	updateDCache(cache,address,s,b);
}
