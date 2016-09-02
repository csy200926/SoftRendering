#include "MemoryManager.h"
#include "MemoryPool.h"
#include "stdio.h"
#include "malloc.h"
#include "assert.h"
namespace SYSR
{
	MemoryManager::~MemoryManager()
	{
		delete m_pBlockDescriptors;
	}


	MemoryManager::MemoryManager(void * i_pMemoryBlock, const size_t i_MemoryBolckSize, const unsigned int i_NumberOfDescriptors)
	{
		m_pBlockDescriptors = MemoryPool::create(i_pMemoryBlock, sizeof(BlockDescriptor), i_NumberOfDescriptors);
		m_pFreeList = reinterpret_cast<BlockDescriptor*>(m_pBlockDescriptors->alloc());


		m_pFreeList->m_pBlock = reinterpret_cast<char*>(i_pMemoryBlock)+i_NumberOfDescriptors * sizeof(BlockDescriptor);
		m_pFreeList->m_size = i_MemoryBolckSize - i_NumberOfDescriptors * sizeof(BlockDescriptor);
		m_pFreeList->m_pNext = NULL;

		m_pUsedList = NULL;

	}

	void* MemoryManager::alloc(const size_t i_size)
	{
		BlockDescriptor *pDescriptor = reinterpret_cast<BlockDescriptor*>(m_pBlockDescriptors->alloc());

		if (pDescriptor == NULL)
		{
			//assert(pDescriptor);
			return NULL;
		}
		BlockDescriptor *pBlock = FindBlockFits(i_size);
		if (pBlock == NULL)
		{
			m_pBlockDescriptors->free(pDescriptor);
			//assert(pBlock);
			return NULL;
		}

		pDescriptor->m_pBlock = pBlock->m_pBlock;
		pDescriptor->m_size = i_size;
		pDescriptor->m_pNext = m_pUsedList;

		pBlock->m_pBlock = reinterpret_cast<char*>(pBlock->m_pBlock) + i_size;
		pBlock->m_size -= i_size;

		// Stick it on the top of UsedList
		m_pUsedList = pDescriptor;


		return pDescriptor->m_pBlock;
	}

	void MemoryManager::free(void *i_ptr)
	{
		BlockDescriptor * pDescriptor = m_pUsedList;

		while (pDescriptor != NULL)
		{
			if (pDescriptor->m_pBlock == i_ptr)
			{

				RemoveFromUsedList(pDescriptor);

				// add to freeList
				pDescriptor->m_pNext = m_pFreeList;
				m_pFreeList = pDescriptor;

				collectFreeBlocks();
				return;
			}

			pDescriptor = pDescriptor->m_pNext;

		}
		//"You can't free a block that doesn't exist!"
		assert(false);
	}

	void MemoryManager::RemoveFromUsedList(BlockDescriptor* i_pBlockDescriptor)
	{

		BlockDescriptor* previousBD = m_pUsedList;

		// If it is the only used block
		if (previousBD == i_pBlockDescriptor)
		{
			m_pUsedList = i_pBlockDescriptor->m_pNext;
			return;
		}

		while (previousBD->m_pNext != i_pBlockDescriptor)
		{
			previousBD = previousBD->m_pNext;
		}

		previousBD->m_pNext = i_pBlockDescriptor->m_pNext;
	}

	void MemoryManager::RemoveFromFreeList(BlockDescriptor* i_pBlockDescriptor)
	{
		BlockDescriptor* previousBD = m_pFreeList;

		// If it is the only free block
		if (previousBD == i_pBlockDescriptor)
		{
			m_pFreeList = i_pBlockDescriptor->m_pNext;
			return;
		}
		while (previousBD->m_pNext != i_pBlockDescriptor)
		{
			previousBD = previousBD->m_pNext;
		}

		previousBD->m_pNext = i_pBlockDescriptor->m_pNext;
	}


	BlockDescriptor *MemoryManager::FindBlockFits(const size_t i_size)
	{
		BlockDescriptor *pFreeBD = m_pFreeList;

		if (pFreeBD->m_size >= i_size)
		{
			return pFreeBD;
		}

		for (pFreeBD = pFreeBD->m_pNext; pFreeBD != NULL; pFreeBD = pFreeBD->m_pNext)
		{
			if (pFreeBD->m_size >= i_size)
				return pFreeBD;
		}
		return NULL;
	}


	void MemoryManager::collectFreeBlocks()
	{
		BlockDescriptor *i_pBlockDescriptor_1 = m_pFreeList;
		BlockDescriptor *i_pBlockDescriptor_2 = NULL;

		while (i_pBlockDescriptor_1)
		{
			i_pBlockDescriptor_2 = m_pFreeList;

			while (i_pBlockDescriptor_2)
			{
				if (isContinuous(i_pBlockDescriptor_1, i_pBlockDescriptor_2))
				{
					i_pBlockDescriptor_1->m_size += i_pBlockDescriptor_2->m_size;
					RemoveFromFreeList(i_pBlockDescriptor_2);
					m_pBlockDescriptors->free(i_pBlockDescriptor_2);
					break;
				}
				i_pBlockDescriptor_2 = i_pBlockDescriptor_2->m_pNext;
			}
			i_pBlockDescriptor_1 = i_pBlockDescriptor_1->m_pNext;
		}

	}

	size_t MemoryManager::getTotalFreeMemory()
	{
		BlockDescriptor *pWalker = m_pFreeList;

		if (!pWalker)
		{
			return 0;
		}
		size_t result = pWalker->m_size;

		while (pWalker = pWalker->m_pNext)
			result += pWalker->m_size;

		return result;
	}

	size_t MemoryManager::getLargestFreeMemory()
	{
		BlockDescriptor *pWalker = m_pFreeList;
		size_t result = 0;

		if (!pWalker)
			return result;

		result = pWalker->m_size >= result ? pWalker->m_size : result;

		while (pWalker = pWalker->m_pNext)
			result = pWalker->m_size >= result ? pWalker->m_size : result;

		return result;
	}

}