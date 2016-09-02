#pragma once
#include <glm.hpp>
#include "CommonStructs.h"
namespace SYSR
{


	class VertexBuffer
	{
	public:
		void Set(int i_vertexCount,VSInput *i_pVSInput);
		void ShutDown();

		VSInput& operator[](std::size_t idx){return m_pVSInput[idx];}
		VSInput *GetData(){ return m_pVSInput; };
		unsigned int GetVertexCount() const{ return m_vsCount; };
	protected:
	private:
		VSInput *m_pVSInput;
		unsigned int m_vsCount;

	};



}