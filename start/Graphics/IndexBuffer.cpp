#include "IndexBuffer.h"


namespace SYSR
{
	IndexBuffer::IndexBuffer() :m_pTriangleList(nullptr)
	{

	}

	void IndexBuffer::Set(unsigned int i_triangleCount, Triangle_Index *i_pTriangleList)
	{
		m_pTriangleList = new Triangle_Index[i_triangleCount];//i_pTriangleList;
		m_triangleCount = i_triangleCount;

		memcpy(m_pTriangleList, i_pTriangleList, sizeof(Triangle_Index) * i_triangleCount);

	}
	void IndexBuffer::ShutDown()
	{
		delete[]m_pTriangleList;
	}



}