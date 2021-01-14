#include"cache.h"
#include"shell.h"
#include<stdlib.h>
#include<time.h>
#include<stdio.h>

//��ʼ�� 
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

//����lru 
void dUpdateLru(dCache *cache,int setBits,int hitIndex)
{
	cache->sets[setBits].line[hitIndex].lru=0;//���ʹ�ù���lruֵ��Ϊ��С 
	int i;
	
	//δʹ�ù���lruֵ+1 
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
	cache->sets[setBits].line[hitIndex].lru=0;//���ʹ�ù���lruֵ��Ϊ��С 
	int i;
	
	//δʹ�ù���lruֵ+1 
	for(i=0;i<cache->lineNum;i++)
	{
		if(i!=hitIndex)
		{
			cache->sets[setBits].line[i].lru++;
		}
	}
}

//������lruֵ����Ϊ��ʹ���� 
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
	return maxIndex;//�����к� 
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
	return maxIndex;//�����к� 
}

int randomEliminate(int lineSize)
{
	srand((unsigned)time(NULL));
	int index = rand() % lineSize;
	return index;
}

//��ȡ��� 
int getTag(uint32_t address,int s,int b)//s:��������b:���ڵ�ַ 
{
	int x=s+b;
	return address>>x;
}

//��ȡcache��� 
int getSet(uint32_t address,int s,int b)
{
	int setBits=address>>b;
	int x=(1<<s)-1;
	return setBits&x;
}

//����cache 
void updateDCache(dCache *cache,uint32_t address,int s,int b) 
{
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int eliminateIndex=dFindMaxLru(cache,setBits);
	//int eliminateIndex=randomEliminate(cache->lineNum);
	int i;
	
	//������Ϊ�滻�������ݱ��ı䣬��Ҫ�ı���������Ӧ������ 
	if(cache->sets[setBits].line[eliminateIndex].valid==1&&cache->sets[setBits].line[eliminateIndex].dirty==1)
	{
		//��ȡ�����ַ 
		int tag=cache->sets[setBits].line[eliminateIndex].tag;
		uint32_t add=(tag<<s)|(setBits);
		add=add<<b;
		
		for(i=0;i<8;i++)
		{
			mem_write_32(add, cache->sets[setBits].line[eliminateIndex].block[i]);
			add+=4;//һ��ָ��4�ֽ� 
		}
	}
	
	cache->sets[setBits].line[eliminateIndex].valid=1;
	cache->sets[setBits].line[eliminateIndex].tag=tag;
	address=(address>>5)<<5;//�����׵�ַ�����ڵ�ַΪ5λ 
	
	//���������������cache 
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
	address=(address>>5)<<5;//�����׵�ַ�����ڵ�ַΪ5λ 
	
	//���������������cache 
	for(i=0;i<8;i++)
	{
		cache->sets[setBits].line[eliminateIndex].block[i]=mem_read_32(address);
		address+=4;
	}
	iUpdateLru(cache,setBits,eliminateIndex);
}

//�ж��Ƿ����� 
int dIsMiss(dCache *cache,int setBits,int tag)
{
	uint8_t isMiss=1;
	int i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].valid==1&&cache->sets[setBits].line[i].tag==tag)
		{
			return i;//�����к� 
		}
	}
	return -1;//δ���� 
}

int iIsMiss(iCache *cache,int setBits,int tag)
{
	uint8_t isMiss=1;
	int i;
	
	for(i=0;i<cache->lineNum;i++)
	{
		if(cache->sets[setBits].line[i].valid==1&&cache->sets[setBits].line[i].tag==tag)
		{
			return i;//�����к� 
		}
	}
	return -1;//δ���� 
}

//��cache��ȡ���� 
uint32_t dLoadData(dCache *cache,uint32_t address)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=dIsMiss(cache,setBits,tag);
	
	//���� 
	if(index!=-1)
	{
		int offset=address&0x0000001f;//ȡ���ڵ�ַ 
		offset=offset>>2;//��������0~8 
		
		dUpdateLru(cache,setBits,index);
		return cache->sets[setBits].line[index].block[offset];
	}
	
	//δ����
	uint32_t val = mem_read_32(address);//�������ж�ȡ 
	
	delay1=50;
	updateDCache(cache,address,s,b);//���������Ӧ������cache�� 
	return val;
}

uint32_t iLoadData(iCache *cache,uint32_t address)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=iIsMiss(cache,setBits,tag);
	
	//���� 
	if(index!=-1)
	{
		int offset=address&0x0000001f;//ȡ���ڵ�ַ 
		offset=offset>>2;//��������0~8 
		
		iUpdateLru(cache,setBits,index);
		return cache->sets[setBits].line[index].block[offset];
	}
	
	//δ����
	uint32_t val = mem_read_32(address);//�������ж�ȡ 
	
	delay2=50;
	updateICache(cache,address,s,b);//���������Ӧ������cache�� 
	return val;
}

//�޸�cache�е����� 
void storeData(dCache *cache,uint32_t address,uint32_t val)
{
	int s=6,b=5;
	int tag=getTag(address,s,b);
	int setBits=getSet(address,s,b);
	int index=dIsMiss(cache,setBits,tag);
	
	//���� 
	if(index!=-1)
	{
		int offset=address&0x0000001f;
		offset=offset>>2;
		
		dUpdateLru(cache,setBits,index);
		cache->sets[setBits].line[index].block[offset]=val; 
		cache->sets[setBits].line[index].dirty=1;//���ݱ��ı� 
		return;
	}
	
	//δ���� 
	mem_write_32(address, val);//�޸������е����� 
	updateDCache(cache,address,s,b);
}
