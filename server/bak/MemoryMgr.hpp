#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_

#include <stdlib.h> // malloc 和 free 等操作
#include <assert.h>
#include <mutex>

#ifdef _DEBUG
	#include <stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif

#define MAX_MEMORY_SZIE 128

class MemoryAlloc;
// 内存块    （内存池里面的最小单位）
class MemoryBlock
{
public:
	//所属大内存块（池）
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//是否在内存池中
	bool bPool;
private:
	// 预留  为了内存对齐
	char c1;
	char c2;
	char c3;
};

// 内存池
class MemoryAlloc
{
protected:
	// 内存池地址
	char* _pBuf;
	// 池中可用区域的头部指针
	MemoryBlock* _pHeader;
	// 池中单元块的大小
	size_t _nSize;
	// 池中单元块的数量
	size_t _nBlockNum;
	// 锁
	std::mutex _mutex;

public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockNum = 0;
		xPrintf("MemoryAlloc\n");
	}
	~MemoryAlloc()
	{
		if (_pBuf)
		{
			free(_pBuf);
		}
	}

	// 申请内存
	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf) // 池没有初始化的情况
		{
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;
		if (nullptr == _pHeader) // 池里面的所有单元耗尽
		{
			// 从池外 申请额外的内存空间；
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));

			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else // 池中还有可分配单元的情况
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	// 释放内存
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)( (char*)pMem - sizeof(MemoryBlock) );

		// 先确保该单元仅被使用一次 
		assert(1 == pBlock->nRef);

		if (pBlock->bPool)// 属于池中单元   (不需要释放该内存)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else // 池外单元	（直接释放该内存）
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	// 初始化
	void initMemory()
	{
		// 确保该池没有初始化过
		assert(nullptr == _pBuf); 
		if (_pBuf)
		{
			return;
		}

		// 计算内存池大小
		size_t realSize = _nSize + sizeof(MemoryBlock); //单元格大小
		size_t bufSize = _nBlockNum * realSize; // 池大小

		// 向系统申请池的内存
		_pBuf = (char*)malloc(bufSize);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->pNext = nullptr;
		_pHeader->pAlloc = this;
		_pHeader->nRef = 0;
		_pHeader->nID = 0;
		
		MemoryBlock* pTemp1 = _pHeader;
		//遍历内存块单元进行初始化
		for (size_t n = 1;n < _nBlockNum;++n)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * realSize));
			pTemp2->pNext = nullptr;
			pTemp2->nRef = 0;
			pTemp2->nID = n;
			pTemp2->bPool = true;
			pTemp2->pAlloc = this;

			// 链表的指针移动
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}

	}
};

// 便于在声明类成员变量时初始化MemoryAlloc的成员数据
template<size_t nSize,size_t nBlockNum>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		const size_t n = sizeof(void*); // 不同操作系统或者指令宽度不同（64位、32位）的 指针的占用位数是不同的

		_nSize = (nSize / n)*n + (nSize % n ? n : 0);
		_nBlockNum = nBlockNum;
	}
};

// 内存池管理工具类
class MemoryMgr
{
private:
	MemoryAlloctor<64, 100000> _mem64;
	MemoryAlloctor<128, 100000> _mem128;
	// MemoryAlloctor<256, 100000> _mem256;
	// MemoryAlloctor<512, 100000> _mem512;
	// MemoryAlloctor<1024, 100000> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		//init_szAlloc(129, 256, &_mem256);
		//init_szAlloc(257, 512, &_mem512);
		//init_szAlloc(513, 1024, &_mem1024);
	}
	~MemoryMgr()
	{
		
	}
	//初始化内存池映射数组
	void init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
public:

	// 单例
	static MemoryMgr& Instance()
	{ // 单例模式 静态
		static MemoryMgr mgr;
		return mgr;
	}

	//申请内存
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE) // 申请的内存大小 在 最大的内存池大小范围内
		{
			return _szAlloc[nSize]->allocMemory(nSize); // 调用多个 池子中的某一个池子的申请内存函数
		}
		else // 申请的内存大小 不在 最大的内存池大小范围内，需要申请池外内存空间
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->pNext = nullptr;
			pReturn->pAlloc = nullptr;
			pReturn->nRef = 1;
			pReturn->nID = -1;

			//xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}
	//释放内存
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{ // 池中单元
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
			{
				free(pBlock);
			}
		}
	}
	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
};
#endif
