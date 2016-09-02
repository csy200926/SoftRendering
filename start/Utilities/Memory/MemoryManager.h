#pragma once
namespace SYSR
{


	class MemoryPool;

	struct BlockDescriptor 
	{
		char* m_pBlock;
		size_t m_size;

		BlockDescriptor * m_pNext;

	};


	class MemoryManager
	{
	public:

		~MemoryManager(void);

		MemoryManager( void * i_pMemoryBlock,const size_t i_MemoryBolckSize ,const unsigned int i_NumberOfDescriptors);
	
		void* alloc(const size_t i_size);
		void free( void *i_ptr );
	
		size_t getTotalFreeMemory();
		size_t getLargestFreeMemory();

	private:

		BlockDescriptor *m_pFreeList;
		BlockDescriptor *m_pUsedList;
		MemoryPool		*m_pBlockDescriptors;

		BlockDescriptor *FindBlockFits( const size_t i_size );

		void collectFreeBlocks();

		void RemoveFromUsedList( BlockDescriptor* i_pBlockDescriptor );
		void RemoveFromFreeList(  BlockDescriptor* i_pBlockDescriptor );

		inline bool isContinuous( BlockDescriptor *i_pBlockDescriptor_1 , BlockDescriptor *i_pBlockDescriptor_2 );
	
	};

	bool MemoryManager::isContinuous( BlockDescriptor *i_pBlockDescriptor_1 , BlockDescriptor *i_pBlockDescriptor_2 )
	{
		if ( (i_pBlockDescriptor_1->m_pBlock + i_pBlockDescriptor_1->m_size) == i_pBlockDescriptor_2->m_pBlock )
			return true;
		return false;
		
	}
}