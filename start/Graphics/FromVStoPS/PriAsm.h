#pragma once
#include "../IndexBuffer.h"
#include "../FakeShaders/VertexShader.h"
#include "../FakeShaders/PixelShader.h"
#include "../CommonStructs.h"
#include "../../BitMap/BitMap.h"
#include "../SRDef.h"

#include "../../Utilities/Memory/MemoryPool.h"

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace SYSR
{
	class IndexBuffer;
	class PixelShader;


	__declspec(align(16)) struct SRTriangle
	{
		VSOutput vertex0;
		VSOutput vertex1;
		VSOutput vertex2;
		void Set(VSOutput &v0,VSOutput &v1,VSOutput &v2)
		{
			vertex0 = v0;
			vertex1 = v1;
			vertex2 = v2;
		}

		SRTriangle()
		{}
	};

#define BATCH_MIN_SIZE 500
#define BATCH_MAX_COUNT 1000
//#define WIREFRAMEMODE
	class PrimitiveAssembly
	{
	public:
		PrimitiveAssembly()
		{

			m_pMemoryPool = MemoryPool::create(_aligned_malloc(sizeof(SRTriangle) * BATCH_MIN_SIZE * BATCH_MAX_COUNT, 16), sizeof(SRTriangle) * BATCH_MIN_SIZE, BATCH_MAX_COUNT);

			m_screenMin.x = 0; m_screenMin.y = 0;
			m_screenMax.x = SCREENWIDTH_INDEX;
			m_screenMax.y = SCREENHEIGHT_INDEX;
			CleanZbuffer();

			for (int i = 0; i < BATCH_MAX_COUNT; i++)
			{
				//m_batchList.push_back((SRTriangle*)m_pMemoryPool->alloc());
			}
		};

		~PrimitiveAssembly();

		inline void CleanZbuffer()
		{

			memset(m_zBuffer, 99, SCREENHEIGHT * SCREENWIDTH * sizeof(float));
		}

		void SetDrawContents(IndexBuffer *i_pIndexBuffer,PixelShader *i_pCurrentPixelShader,BitMap *i_pFrameBuffer, VSOutput *i_output );
		void Assembly();
		void Rasterize(VSOutput &v1,VSOutput &v2,VSOutput &v3 );//(Triangle &i_Triangle);

		void StartTasks();
	private:
		std::vector<std::thread> threads;
		MemoryPool *m_pMemoryPool;

		int m_currentBatchIndex;
		SRTriangle* m_pCurrentBatch;
		std::vector<SRTriangle*> m_batchList;

		int m_nextMemoToFree;
		std::mutex m_taskMutex; std::mutex m_taskMutex2;
		std::condition_variable m_conVar;
		std::queue<SRTriangle*> m_taskQueue;
		bool m_isAllTaskFinished;
		bool m_notified;
		bool m_lastQuestAccepted;

		PixelShader *m_pPixelShader;
		VSOutput    *m_vsOutputs;


		IndexBuffer *m_pIndexBuffer;
		BitMap      *m_pFrameBuffer;
		glm::vec2 m_screenMin,m_screenMax;

		float m_zBuffer[SCREENHEIGHT][SCREENWIDTH];

		VSOutput Interpolate(int x, int y, VSOutput &v1, VSOutput &v2, VSOutput &v3);

		inline bool IsQueueEmpty(){ return m_taskQueue.empty(); };
		void AddTask(VSOutput &v1, VSOutput &v2, VSOutput &v3);
		void DataProcessing();

		void DrawTriangle_new(VSOutput &v1, VSOutput &v2, VSOutput &v3);
		void DrawFlatTopTriangle(glm::vec2& v1, glm::vec2& v2, glm::vec2& v3, VSOutput **vertices, int *indices);
		void DrawFlatBtmTriangle(glm::vec2& v1, glm::vec2& v2, glm::vec2& v3, VSOutput **vertices, int *indices);
		void DrawLineNew(int start, int end, int y, VSOutput &vertex0, VSOutput &vertex1);
	
		void DrawLineBresenham(int x1, int y1, int x2, int y2, VSOutput &vertex1, VSOutput &vertex2, VSOutput &vertex3);
		inline void DrawTriangleWireFrame(glm::vec2& v1, glm::vec2& v2, glm::vec2& v3, VSOutput &vertex1, VSOutput &vertex2, VSOutput &vertex3);

		void FakeClipping(VSOutput &i_v1, VSOutput &i_v2,VSOutput &i_v3);

		void RunBatch(SRTriangle* pBatch,int count);


		//bool FakeClipping_New(glm::vec4 &point1, glm::vec4 &point2, glm::vec4 &point3);
		inline void VertexLerp(VSOutput &i_v1, VSOutput &i_v2, float alphaValue, VSOutput &o_v);
		void VertexLerp_Perspective(VSOutput &i_v1, VSOutput &i_v2, float alphaValue, VSOutput &o_v);
		
		inline void PtoScreen(VSOutput &v)
		{
			//
			v.position.w = 1.0 / v.position.w;
			if (v.position.w < 0.0f)
				v.position.w = 0.1f;// -v.position.w;
			// View coordinate is -1 - +1 after divided by w value
			// Here change it to 0 - 1
			// And match it to screen resolution
			v.position.x = (int)((v.position.x * v.position.w + 1.0f) * 0.5f * SCREENWIDTH_INDEX + 0.5f);
			v.position.y = (int)((v.position.y * v.position.w + 1.0f) * 0.5f * SCREENHEIGHT_INDEX + 0.5f);
		}

	};


	void PrimitiveAssembly::DrawTriangleWireFrame(glm::vec2& v1, glm::vec2& v2, glm::vec2& v3, VSOutput &vertex1, VSOutput &vertex2, VSOutput &vertex3)
	{
		DrawLineBresenham(v1.x, v1.y, v2.x, v2.y, vertex1, vertex2, vertex3);
		//DrawLineBresenham(v2.x, v2.y, v3.x, v3.y, vertex1, vertex2, vertex3);
		DrawLineBresenham(v1.x, v1.y, v3.x, v3.y, vertex1, vertex2, vertex3);
	}

	// No perspective corrections
	void PrimitiveAssembly::VertexLerp(VSOutput &i_v1, VSOutput &i_v2, float alphaValue, VSOutput &o_v)
	{
		float oneMinusAlpha = 1.0f - alphaValue;

		o_v.position = i_v1.position * oneMinusAlpha + i_v2.position * alphaValue;
		o_v.color = i_v1.color * oneMinusAlpha + i_v2.color  * alphaValue;
		o_v.normal = i_v1.normal  * oneMinusAlpha + i_v2.normal  * alphaValue;
		o_v.UV = i_v1.UV  * oneMinusAlpha + i_v2.UV  * alphaValue;

	}



}