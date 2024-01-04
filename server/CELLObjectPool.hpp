#ifndef _CELLObjectPool_hpp_
#define _CELLObjectPool_hpp_

#include <mutex>
#include <assert.h>

#ifdef _DEBUG
	#ifndef xPrintf
		#include<stdio.h>
		#define xPrintf(...) printf(__VA_ARGS__)
	#endif
#else
	#ifndef xPrintf
		#define xPrintf(...)
	#endif
#endif // _DEBUG


template<class Type, size_t nPoolSize>
class CELLObjectPoll
{
private:
	class NodeHeader
	{
	public:
		// 下一块位置
		NodeHeader* pNext;
		// 内存块编号
		int nID;
		// 引用次数
		int nRef;
		// 是否在内存池中
		bool nPool;
	private:
		// 保留，内存对齐
		char c1;
		char c2;
	};

private:
	// 对象池内存缓存区地址
	char* _pBuf;
	// 对象单元节点头指针
	NodeHeader* _pHeader;
	//
	std::mutex _mutex;

public:
	CELLObjectPoll()
	{
		initPool();
	}
	~CELLObjectPoll()
	{
		if (_pBuf)
		{
			delete[] _pBuf;
		}
	}

	// 申请对象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;

		if (nullptr == _pHeader)
		{
			// 对象池满了或者其他情况
			// 重新在池外申请新的对象内存空间

			pReturn = (NodeHeader*)new char[sizeof(NodeHeader) + sizeof(Type)];
			pReturn->nRef = 1;
			pReturn->nID = -1;
			pReturn->pNext = nullptr;
			pReturn->nPool = false;
		}
		else
		{
			pReturn = (NodeHeader*)_pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			++pReturn->nRef;
		}

		xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}

	// 释放对象内存
	void freeObjMemory(void* pMem)
	{
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));

		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->nPool)
		{
			// 池内对象
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			// 池外对象
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}
private:
	// 初始化对象池
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
		{
			return;
		}

		// 计算对象池的大小
		size_t realSize = sizeof(NodeHeader) + sizeof(Type); // 单个 对象单元（头部信息 + 对象实际信息）的存储大小
		size_t n = nPoolSize * realSize;

		// 申请对象池的内存空间
		_pBuf = new char[n];

		// ------ 初始化对象池
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->nRef = 0;
		_pHeader->nID = 0;
		_pHeader->nPool = true;
		_pHeader->pNext = nullptr;
		//循环遍历对象池各个单元，进行初始化
		NodeHeader* pTemp1 = _pHeader;
		for (size_t n = 1;n < nPoolSize;++n)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n * realSize));
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->nPool = true;
			pTemp2->pNext = nullptr;

			pTemp1->pNext = pTemp2;
			pTemp1->pNext = pTemp2;
		}
	}
};

template<class Type,size_t nPoolSize>
class ObjectPoolBase
{ // 对象的基类，提供了类嵌入对象池的方法，不需要修改原来的类
public:

	// new 和 delete 的方法重载
	void* operator new (size_t nSize){
		return objectPool().allocObjMemory(nSize);
	}
	void operator delete(void* pMem){
		objectPool().freeObjMemory(pMem);
	}

	// 创建
	template<typename ...Args>
	static Type* createObject(Args... args)
	{
		// 不定参数 可变参数
		Type* obj = new Type(args...);
		return obj;
	}
	// 销毁
	static void destroyObject(Type* obj)
	{
		delete obj;
	}
private:

	typedef CELLObjectPoll<Type, nPoolSize> ClassTypePool;
	static ClassTypePool& objectPool()
	{
		static ClassTypePool sPool;
		return sPool;
	}
};
#endif
