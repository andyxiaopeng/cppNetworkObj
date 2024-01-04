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
		// ��һ��λ��
		NodeHeader* pNext;
		// �ڴ����
		int nID;
		// ���ô���
		int nRef;
		// �Ƿ����ڴ����
		bool nPool;
	private:
		// �������ڴ����
		char c1;
		char c2;
	};

private:
	// ������ڴ滺������ַ
	char* _pBuf;
	// ����Ԫ�ڵ�ͷָ��
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

	// ��������ڴ�
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;

		if (nullptr == _pHeader)
		{
			// ��������˻����������
			// �����ڳ��������µĶ����ڴ�ռ�

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

	// �ͷŶ����ڴ�
	void freeObjMemory(void* pMem)
	{
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));

		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->nPool)
		{
			// ���ڶ���
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			// �������
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}
private:
	// ��ʼ�������
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
		{
			return;
		}

		// �������صĴ�С
		size_t realSize = sizeof(NodeHeader) + sizeof(Type); // ���� ����Ԫ��ͷ����Ϣ + ����ʵ����Ϣ���Ĵ洢��С
		size_t n = nPoolSize * realSize;

		// �������ص��ڴ�ռ�
		_pBuf = new char[n];

		// ------ ��ʼ�������
		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->nRef = 0;
		_pHeader->nID = 0;
		_pHeader->nPool = true;
		_pHeader->pNext = nullptr;
		//ѭ����������ظ�����Ԫ�����г�ʼ��
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
{ // ����Ļ��࣬�ṩ����Ƕ�����صķ���������Ҫ�޸�ԭ������
public:

	// new �� delete �ķ�������
	void* operator new (size_t nSize){
		return objectPool().allocObjMemory(nSize);
	}
	void operator delete(void* pMem){
		objectPool().freeObjMemory(pMem);
	}

	// ����
	template<typename ...Args>
	static Type* createObject(Args... args)
	{
		// �������� �ɱ����
		Type* obj = new Type(args...);
		return obj;
	}
	// ����
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
