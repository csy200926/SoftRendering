#pragma once
#include <glm.hpp>
#include <set>
#include "CommonStructs.h"
namespace SYSR
{
	class IndexBuffer
	{
	public:
		IndexBuffer();
		void ShutDown();


		unsigned int m_triangleCount;
		
		void Set(unsigned int i_triangleCount, Triangle_Index *i_pTriangleList);

		inline Triangle_Index*GetData(){ return m_pTriangleList; };
		inline unsigned int GetCount(){ return m_triangleCount; };
		Triangle_Index& operator[](unsigned int index){ return m_pTriangleList[index]; };



	protected:
	private:

		Triangle_Index *m_pTriangleList;
	};



}