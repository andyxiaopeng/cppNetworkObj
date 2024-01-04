#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_

#include <stdlib.h> // malloc �� free �Ȳ���
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
// �ڴ��    ���ڴ���������С��λ��
class MemoryBlock
{
public:
	//�������ڴ�飨�أ�
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	// Ԥ��  Ϊ���ڴ����
	char c1;
	char c2;
	char c3;
};

// �ڴ��
class MemoryAlloc
{
protected:
	// �ڴ�ص�ַ
	char* _pBuf;
	// ���п��������ͷ��ָ��
	MemoryBlock* _pHeader;
	// ���е�Ԫ��Ĵ�С
	size_t _nSize;
	// ���е�Ԫ�������
	size_t _nBlockNum;
	// ��
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

	// �����ڴ�
	void* allocMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		if (!_pBuf) // ��û�г�ʼ�������
		{
			initMemory();
		}

		MemoryBlock* pReturn = nullptr;
		if (nullptr == _pHeader) // ����������е�Ԫ�ľ�
		{
			// �ӳ��� ���������ڴ�ռ䣻
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));

			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else // ���л��пɷ��䵥Ԫ�����
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}

		//xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	// �ͷ��ڴ�
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)( (char*)pMem - sizeof(MemoryBlock) );

		// ��ȷ���õ�Ԫ����ʹ��һ�� 
		assert(1 == pBlock->nRef);

		if (pBlock->bPool)// ���ڳ��е�Ԫ   (����Ҫ�ͷŸ��ڴ�)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else // ���ⵥԪ	��ֱ���ͷŸ��ڴ棩
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	// ��ʼ��
	void initMemory()
	{
		// ȷ���ó�û�г�ʼ����
		assert(nullptr == _pBuf); 
		if (_pBuf)
		{
			return;
		}

		// �����ڴ�ش�С
		size_t realSize = _nSize + sizeof(MemoryBlock); //��Ԫ���С
		size_t bufSize = _nBlockNum * realSize; // �ش�С

		// ��ϵͳ����ص��ڴ�
		_pBuf = (char*)malloc(bufSize);

		//��ʼ���ڴ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->pNext = nullptr;
		_pHeader->pAlloc = this;
		_pHeader->nRef = 0;
		_pHeader->nID = 0;
		
		MemoryBlock* pTemp1 = _pHeader;
		//�����ڴ�鵥Ԫ���г�ʼ��
		for (size_t n = 1;n < _nBlockNum;++n)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * realSize));
			pTemp2->pNext = nullptr;
			pTemp2->nRef = 0;
			pTemp2->nID = n;
			pTemp2->bPool = true;
			pTemp2->pAlloc = this;

			// �����ָ���ƶ�
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}

	}
};

// �������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
template<size_t nSize,size_t nBlockNum>
class MemoryAlloctor : public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		const size_t n = sizeof(void*); // ��ͬ����ϵͳ����ָ���Ȳ�ͬ��64λ��32λ���� ָ���ռ��λ���ǲ�ͬ��

		_nSize = (nSize / n)*n + (nSize % n ? n : 0);
		_nBlockNum = nBlockNum;
	}
};

// �ڴ�ع�������
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
	//��ʼ���ڴ��ӳ������
	void init_szAlloc(int nBegin,int nEnd,MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
public:

	// ����
	static MemoryMgr& Instance()
	{ // ����ģʽ ��̬
		static MemoryMgr mgr;
		return mgr;
	}

	//�����ڴ�
	void* allocMem(size_t nSize)
	{
		if (nSize <= MAX_MEMORY_SZIE) // ������ڴ��С �� �����ڴ�ش�С��Χ��
		{
			return _szAlloc[nSize]->allocMemory(nSize); // ���ö�� �����е�ĳһ�����ӵ������ڴ溯��
		}
		else // ������ڴ��С ���� �����ڴ�ش�С��Χ�ڣ���Ҫ��������ڴ�ռ�
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
	//�ͷ��ڴ�
	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{ // ���е�Ԫ
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
	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
};
#endif
