#include "MemoryPool.h"
#include "assert.h"
#include "windows.h"
#include "BitArray.h"
#include "Extension.h"
namespace SYSR
{
	MemoryPool::MemoryPool(char * i_pPoolMemory, BitArray *i_pInUseFlags, const unsigned int i_itemSize, const unsigned int i_itemCount) :
		m_pMemoryPool(i_pPoolMemory),
		m_pIsInUse(i_pInUseFlags),
		m_itemSize(i_itemSize),
		m_itemCount(i_itemCount)
	{
		assert(i_pPoolMemory != NULL);
		assert(i_pInUseFlags != NULL);

		for (unsigned int i = 0; i < i_itemCount; i++)
			m_pIsInUse->clearBit(i);
	}


	MemoryPool::~MemoryPool(void)
	{
		delete[] m_pMemoryPool;
		delete m_pIsInUse;
	}


	MemoryPool *MemoryPool::create(const unsigned int i_itemSize, const unsigned int i_itemCount)
	{
		char *pPoolMemory = new char[i_itemSize * i_itemCount];// Operation new might not be safe,so we place it out
		BitArray *pInUseFlags = BitArray::create(i_itemCount);

		if (pPoolMemory == NULL)
		{
			if (pPoolMemory)
				delete[] pPoolMemory;
			if (pInUseFlags)
				delete pInUseFlags;
			return NULL;
		}
		else
			return new MemoryPool(pPoolMemory, pInUseFlags, i_itemSize, i_itemCount);
	}

	MemoryPool *MemoryPool::create(void * i_pPoolMemory, const unsigned int i_itemSize, const unsigned int i_itemCount)
	{
		BitArray *pInUseFlags = BitArray::create(i_itemCount);

		if (i_pPoolMemory == NULL)
		{
			if (pInUseFlags)
				delete pInUseFlags;
			return NULL;
		}
		else
			return new MemoryPool(reinterpret_cast<char*>(i_pPoolMemory), pInUseFlags, i_itemSize, i_itemCount);
	}



	void *MemoryPool::alloc()
	{
		unsigned int i_firstFreeBlock = m_pIsInUse->getFirstFreeBit();

		if (i_firstFreeBlock == Error_Full)
		{
			//DEBUG_PRINT("-----------------********Memory pool is full!!! Take care!-*******--------------------");
			return NULL;
		}

		if (i_firstFreeBlock >= m_itemCount)
			return NULL;

		m_pIsInUse->setBit(i_firstFreeBlock);

		return m_pMemoryPool + (i_firstFreeBlock * m_itemSize);
	}

	void MemoryPool::free(void *i_pPtr)
	{
		unsigned int i_firstFreeBlock = 0;

		for (; i_firstFreeBlock < m_itemCount; i_firstFreeBlock++)// Find out the flag
		{
			if ((m_pMemoryPool + (i_firstFreeBlock * m_itemSize)) == i_pPtr)
				break;
		}

		m_pIsInUse->clearBit(i_firstFreeBlock);

		i_pPtr = NULL;
	}
}