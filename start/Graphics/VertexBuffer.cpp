#include "VertexBuffer.h"


namespace SYSR
{
	void VertexBuffer::Set(int i_vertexCount,VSInput *i_pVSInput)
	{
		// ģ���� cpu ������ gpu���������ﲻ��

		m_pVSInput = new VSInput[i_vertexCount];//i_pVSInput;
		m_vsCount = i_vertexCount;

		memcpy(m_pVSInput,i_pVSInput,sizeof(VSInput) * i_vertexCount);
	}
	void VertexBuffer::ShutDown()
	{
		delete[]m_pVSInput;
	}



}