#include "PriAsm.h"

#include "../../BitMap/BitMap.h"
#include "../../Utilities.h"

#include "../../Utilities/Profiling.h"

#include <stdio.h>
#include <intrin.h>
#include <iostream>


namespace SYSR
{
	PrimitiveAssembly::~PrimitiveAssembly()
	{
		m_isAllTaskFinished = true;
		m_notified = true;
		m_conVar.notify_all();
		for (auto &t : threads){
			t.join();
		}
		//threads.clear();
	}

	void PrimitiveAssembly::SetDrawContents(IndexBuffer *i_pIndexBuffer, PixelShader *i_pCurrentPixelShader, BitMap *i_pFrameBuffer, VSOutput *i_output)
	{
		m_pIndexBuffer = i_pIndexBuffer;
		m_pPixelShader = i_pCurrentPixelShader;
		m_pFrameBuffer = i_pFrameBuffer;
		m_vsOutputs = i_output;
		m_currentBatchIndex = 0;
		m_pCurrentBatch = nullptr;
		m_isAllTaskFinished = false;
		m_notified = false;
		m_nextMemoToFree = 0;
	}

	//http://stackoverflow.com/questions/13790208/triangle-square-intersection-test-in-2d
	//bool PrimitiveAssembly::FakeClipping(VSOutput &v1, VSOutput &v2, VSOutput &v3)

	void PrimitiveAssembly::FakeClipping(VSOutput &i_v0, VSOutput &i_v1, VSOutput &i_v2)
	{


#define CLIP_CODE_MORE_Z   0x0001    // z > z_max
#define CLIP_CODE_LESS_Z   0x0002    // z < z_min
#define CLIP_CODE_IN_Z   0x0004    // z_min < z < z_max

#define CLIP_CODE_MORE_X   0x0001    // x > x_max
#define CLIP_CODE_LESS_X   0x0002    // x < x_min
#define CLIP_CODE_IN_X   0x0004    // x_min < x < x_max

#define CLIP_CODE_MORE_Y   0x0001    // y > y_max
#define CLIP_CODE_LESS_Y   0x0002    // y < y_min
#define CLIP_CODE_IN_Y   0x0004    // y_min < y < y_max

#define CLIP_CODE_NULL 0x0000

		int vertex_ccodes[3]; // 三角形三个顶点的判断码存储
		int num_verts_in;     // 在视锥内的顶点个数
		int v0, v1, v2;       // 顶点点序
		float zNear = 1.0f, zFar = 1000.0f;

		//////////////////////////////////////////////////////////////////////////
		// 裁剪X轴
		// vertex 0
		if (i_v0.position.x > i_v0.position.w)
			vertex_ccodes[0] = CLIP_CODE_MORE_X;
		else if (i_v0.position.x < -i_v0.position.w)
			vertex_ccodes[0] = CLIP_CODE_LESS_X;
		else
			vertex_ccodes[0] = CLIP_CODE_IN_X;
		// vertex 1
		if (i_v1.position.x > i_v1.position.w)
			vertex_ccodes[1] = CLIP_CODE_MORE_X;
		else if (i_v1.position.x < -i_v1.position.w)
			vertex_ccodes[1] = CLIP_CODE_LESS_X;
		else
			vertex_ccodes[1] = CLIP_CODE_IN_X;
		// vertex 2
		if (i_v2.position.x > i_v2.position.w)
			vertex_ccodes[2] = CLIP_CODE_MORE_X;
		else if (i_v2.position.x < -i_v2.position.w)
			vertex_ccodes[2] = CLIP_CODE_LESS_X;
		else
			vertex_ccodes[2] = CLIP_CODE_IN_X;
		// 测试：当三角形完全在X轴向上位于视锥外部是，抛弃。
		// 否则不裁剪直接进入光栅化
		if (((vertex_ccodes[0] == CLIP_CODE_MORE_X) &&
			(vertex_ccodes[1] == CLIP_CODE_MORE_X) &&
			(vertex_ccodes[2] == CLIP_CODE_MORE_X)) ||

			((vertex_ccodes[0] == CLIP_CODE_LESS_X) &&
			(vertex_ccodes[1] == CLIP_CODE_LESS_X) &&
			(vertex_ccodes[2] == CLIP_CODE_LESS_X)))

		{
			return;
		}
		// 裁剪X轴
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// 裁剪Y轴
		// vertex 0
		if (i_v0.position.y > i_v0.position.w)
			vertex_ccodes[0] = CLIP_CODE_MORE_Y;
		else if (i_v0.position.y < -i_v0.position.w)
			vertex_ccodes[0] = CLIP_CODE_LESS_Y;
		else
			vertex_ccodes[0] = CLIP_CODE_IN_Y;
		// vertex 1
		if (i_v1.position.y > i_v1.position.w)
			vertex_ccodes[1] = CLIP_CODE_MORE_Y;
		else if (i_v1.position.y < -i_v1.position.w)
			vertex_ccodes[1] = CLIP_CODE_LESS_Y;
		else
			vertex_ccodes[1] = CLIP_CODE_IN_Y;
		// vertex 2
		if (i_v2.position.y > i_v2.position.w)
			vertex_ccodes[2] = CLIP_CODE_MORE_Y;
		else if (i_v2.position.y < -i_v2.position.w)
			vertex_ccodes[2] = CLIP_CODE_LESS_Y;
		else
			vertex_ccodes[2] = CLIP_CODE_IN_Y;
		// 测试：当三角形完全在Y轴向上位于视锥外部是，抛弃。
		// 否则不裁剪直接进入光栅化
		if (((vertex_ccodes[0] == CLIP_CODE_MORE_Y) &&
			(vertex_ccodes[1] == CLIP_CODE_MORE_Y) &&
			(vertex_ccodes[2] == CLIP_CODE_MORE_Y)) ||

			((vertex_ccodes[0] == CLIP_CODE_LESS_Y) &&
			(vertex_ccodes[1] == CLIP_CODE_LESS_Y) &&
			(vertex_ccodes[2] == CLIP_CODE_LESS_Y)))

		{
			return;
		}
		// 裁剪Y轴
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// 裁剪Z轴

		// 裁剪Z轴
		//////////////////////////////////////////////////////////////////////////
		// 首先判断三个顶点与ZNEAR平面的状态

		// 在视锥体内部的顶点数量置0
		num_verts_in = 0;

		// vertex 0
		if (i_v0.position.w > zFar)
		{
			vertex_ccodes[0] = CLIP_CODE_MORE_Z;
		}
		else if (i_v0.position.w < zNear)
		{
			vertex_ccodes[0] = CLIP_CODE_LESS_Z;
		}
		else
		{
			vertex_ccodes[0] = CLIP_CODE_IN_Z;
			num_verts_in++;
		}

		// vertex 1
		if (i_v1.position.w > zFar)
		{
			vertex_ccodes[1] = CLIP_CODE_MORE_Z;
		}
		else if (i_v1.position.w < zNear)
		{
			vertex_ccodes[1] = CLIP_CODE_LESS_Z;
		}
		else
		{
			vertex_ccodes[1] = CLIP_CODE_IN_Z;
			num_verts_in++;
		}

		// vertex 2
		if (i_v2.position.w > zFar)
		{
			vertex_ccodes[2] = CLIP_CODE_MORE_Z;
		}
		else if (i_v2.position.w < zNear)
		{
			vertex_ccodes[2] = CLIP_CODE_LESS_Z;
		}
		else
		{
			vertex_ccodes[2] = CLIP_CODE_IN_Z;
			num_verts_in++;
		}


		// Vertices being drawn must be copies.
		VSOutput tri[3] = { i_v0, i_v1, i_v2 };

		//////////////////////////////////////////////////////////////////////////
		// 当所有顶点，均位于Z范围内，直接提交
		if ((vertex_ccodes[0] == CLIP_CODE_IN_Z) &&
			(vertex_ccodes[1] == CLIP_CODE_IN_Z) &&
			(vertex_ccodes[2] == CLIP_CODE_IN_Z))
		{
			PtoScreen(tri[0]); PtoScreen(tri[1]); PtoScreen(tri[2]);
			AddTask(tri[0], tri[1], tri[2]);
			return;
		}

		// 这时，判断，是否有点位于近平面以内？
		if (((vertex_ccodes[0] | vertex_ccodes[1] | vertex_ccodes[2]) & CLIP_CODE_LESS_Z))
		{
			if (num_verts_in == 1)
			{
				// 情况A:
				// 当一个顶点在视锥内
				// 最简单，直接截取
				//     /\
				//    /  \
				//   /____\
				//  /      \

				// 1.先简单排序，把在视锥里面的顶点排到第一个
				if (vertex_ccodes[0] == CLIP_CODE_IN_Z)
				{
					v0 = 0; v1 = 1; v2 = 2;
				}
				else if (vertex_ccodes[1] == CLIP_CODE_IN_Z)
				{
					v0 = 1; v1 = 2; v2 = 0;
				}
				else
				{
					v0 = 2; v1 = 0; v2 = 1;
				}

				VSOutput &vertex0 = tri[v0];
				VSOutput &vertex1 = tri[v1];
				VSOutput &vertex2 = tri[v2];

				float ratio1 = (vertex0.position.w - zNear) / (vertex0.position.w - vertex1.position.w);
				float ratio2 = (vertex0.position.w - zNear) / (vertex0.position.w - vertex2.position.w);

				VSOutput vertex1_clipped;
				VSOutput vertex2_clipped;

				VertexLerp(vertex0, vertex1, ratio1, vertex1_clipped);
				VertexLerp(vertex0, vertex2, ratio2, vertex2_clipped);

				PtoScreen(vertex0); PtoScreen(vertex1_clipped); PtoScreen(vertex2_clipped);
				AddTask(vertex0, vertex1_clipped, vertex2_clipped);

			}
			else if (num_verts_in == 2)
			{
				// num_verts = 2

				// 情况B：
				// 有两个顶点在视锥内
				// 稍微复杂，裁剪完后是一个四边形，需要分割

				//1          2
				//3__________4
				// \        /
				//  \/_____/
				//   \    /


				//     p0

				// 1. 找出不在视锥体内的顶点
				if (vertex_ccodes[0] == CLIP_CODE_LESS_Z)
				{
					v0 = 0; v1 = 1; v2 = 2;
				}
				else if (vertex_ccodes[1] == CLIP_CODE_LESS_Z)
				{
					v0 = 1; v1 = 2; v2 = 0;
				}
				else
				{
					v0 = 2; v1 = 0; v2 = 1;
				}


				VSOutput &vertex0 = tri[v0];
				VSOutput &vertex1 = tri[v1];
				VSOutput &vertex2 = tri[v2];

				VSOutput vertex3;
				VSOutput vertex4;

				float ratio1 = (zNear - vertex0.position.w) / (vertex1.position.w - vertex0.position.w);
				float ratio2 = (zNear - vertex0.position.w) / (vertex2.position.w - vertex0.position.w);

				VertexLerp(vertex0, vertex1, ratio1, vertex3);
				VertexLerp(vertex0, vertex2, ratio2, vertex4);

				PtoScreen(vertex1);
				PtoScreen(vertex2);
				PtoScreen(vertex3); 
				PtoScreen(vertex4);

				AddTask(vertex3, vertex1, vertex2);
				AddTask(vertex4, vertex3, vertex2);
			}
		}

		
	}

	void PrimitiveAssembly::Assembly()
	{
		using namespace std;

		// Task list init
		{
			m_isAllTaskFinished = false;
			lock_guard<mutex> lk(m_taskMutex2);
			m_pCurrentBatch = (SRTriangle*)m_pMemoryPool->alloc();//new SRTriangle[BATCH_MIN_SIZE];// (SRTriangle*)_aligned_malloc(sizeof(SRTriangle)*BATCH_MIN_SIZE, 16);
			m_currentBatchIndex = 0;
		}

		{
			if (threads.size() == 0)
				for (int i = 0; i < 8; i++)
				{
					threads.push_back(std::thread(&PrimitiveAssembly::DataProcessing, this));
				}
		}

		// Calculate what to draw
		int triCount = 0;
		int count = m_pIndexBuffer->GetCount();
		for (unsigned int i = 0; i <count; i++)
		{

			// Back culling
			Triangle_Index &triangle = (*m_pIndexBuffer)[i];
			if (triangle.isVisible == false)
				continue;

			FakeClipping(m_vsOutputs[triangle.indices[0]], m_vsOutputs[triangle.indices[1]], m_vsOutputs[triangle.indices[2]]);
			
		}

		// Run last one
		{

			int length = m_currentBatchIndex;
			//if (m_batchList.size() > 0)
			{
				RunBatch(m_pCurrentBatch, length);
			}
		}

	}




	void PrimitiveAssembly::AddTask(VSOutput &v0, VSOutput &v1, VSOutput &v2)
	{
		using namespace std;

		m_pCurrentBatch[m_currentBatchIndex].Set(v0,v1,v2);
		m_currentBatchIndex++;

		if (m_currentBatchIndex == BATCH_MIN_SIZE)
		{
			{
				std::lock_guard<std::mutex> lk(m_taskMutex);
				m_taskQueue.push(m_pCurrentBatch);
				m_conVar.notify_one();
			}


			m_currentBatchIndex = 0;
			lock_guard<mutex> lk(m_taskMutex2);
			m_pCurrentBatch = (SRTriangle*)m_pMemoryPool->alloc();//(SRTriangle*)_aligned_malloc(sizeof(SRTriangle)*BATCH_MIN_SIZE, 16);


		}

	}

	void PrimitiveAssembly::DataProcessing()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lk(m_taskMutex);

			m_conVar.wait(lk, [this](){return !m_taskQueue.empty(); });

			SRTriangle *pBatch = m_taskQueue.front();
			m_taskQueue.pop();
				
			lk.unlock();
			RunBatch(pBatch, BATCH_MIN_SIZE);
			
			if (m_isAllTaskFinished)
			{
				break;
			}
		
		}
	}
	void PrimitiveAssembly::RunBatch(SRTriangle* pBatch, int count)
	{
		for (int i = 0; i < count; i++)
		{
			DrawTriangle_new(pBatch[i].vertex0,
				pBatch[i].vertex1,
				pBatch[i].vertex2);
		}
		using namespace std;
		lock_guard<mutex> lk(m_taskMutex2);
		m_pMemoryPool->free(pBatch);
		//if (pBatch != nullptr)
			//_aligned_free(pBatch);
	}

	void PrimitiveAssembly::DrawTriangle_new(VSOutput &vertex1, VSOutput &vertex2, VSOutput &vertex3)
	{
	
		VSOutput* VS[3];
		VS[0] = &vertex1;
		VS[1] = &vertex2;
		VS[2] = &vertex3;

		int indices[3] = { 0, 1, 2 };

		vec2 v1(vertex1.position.x, vertex1.position.y);
		vec2 v2(vertex2.position.x, vertex2.position.y);
		vec2 v3(vertex3.position.x, vertex3.position.y);

		//Sort three vec2
		if (v1[1] < v3[1])
		{
			SwapVec2(v1, v3);
			swap(indices[0],indices[2]);
		}
		if (v1[1] < v2[1])
		{
			SwapVec2(v1, v2);
			swap(indices[0], indices[1]);
		}
		if (v2[1] < v3[1])
		{
			SwapVec2(v2, v3);
			swap(indices[1],indices[2]);
		}
			
//#define WIREFRAMEMODE
#ifdef WIREFRAMEMODE
		if (v2.y == v3.y)
			DrawTriangleWireFrame(v1, v2, v3, vertex1, vertex2, vertex3);
		else if (v1.y == v2.y)
			DrawTriangleWireFrame(v3, v1, v2, vertex1, vertex2, vertex3);
		else
		{
			vec2 v4(v1.x + (v2.y - v1.y) / (v3.y - v1.y) * (v3.x - v1.x), v2.y);
			DrawTriangleWireFrame(v1, v2, v4, vertex1, vertex2, vertex3);
			DrawTriangleWireFrame(v3, v2, v4, vertex1, vertex2, vertex3);
		}
#else
		if (abs(v2.y - v3.y) < 0.01f)
		{
			if (v3.x < v2.x)
			{
				vec2 temp = v3;
				v3 = v2;
				v2 = temp;
				swap(indices[1], indices[2]);
			}
			DrawFlatBtmTriangle(v1, v2, v3, VS, indices);
		}
		else if (abs(v1.y - v2.y) < 0.01f)
		{
			if (v2.x < v1.x)
			{
				vec2 temp = v1;
				v1 = v2;
				v2 = temp;
				swap(indices[0], indices[1]);
			}
			DrawFlatTopTriangle(v3, v1, v2, VS, indices);
		}
		else
		{
			float ratio = (v2.y - v1.y) / (v3.y - v1.y);
			vec2 v4(v1.x + ratio * (v3.x - v1.x), v2.y);
			VSOutput vertex4;

			VertexLerp_Perspective(*VS[indices[0]], *VS[indices[2]], ratio, vertex4);

			//Draw FlatTop Triangle
			DrawTriangle_new(*VS[indices[2]], *VS[indices[1]], vertex4);

			//Draw FlatBtm Triangle
			DrawTriangle_new(*VS[indices[0]], *VS[indices[1]], vertex4);

		}
#endif
		
	}

	void PrimitiveAssembly::DrawFlatBtmTriangle(glm::vec2& v0, glm::vec2& v1, glm::vec2& v2, VSOutput **vertices, int *indices)
	{
		using namespace glm;


		float height = v0.y - v1.y;
		float dx_left = (v1.x - v0.x) / height;
		float dx_right = (v2.x - v0.x) / height;

		float xs = v0.x;
		float xe = v0.x;

		int endY = v1.y + 0.5f;

		for (int y = v0.y; y >= endY; y--)
		{
			float ratio = (float)(v0.y - y) / (float)(v0.y - endY);
			
			VSOutput vertexS, vertexE;
			VertexLerp_Perspective(*vertices[indices[0]], *vertices[indices[1]], ratio, vertexS);
			VertexLerp_Perspective(*vertices[indices[0]], *vertices[indices[2]], ratio, vertexE);

			//Screen space clipping
			if (y < 0 || y > SCREENHEIGHT_INDEX)
			{
				xs += dx_left;
				xe += dx_right;
				continue;
			}
				
				
			DrawLineNew(int(xs + 0.5f), int(xe + 0.5f), y, vertexS, vertexE);
			xs += dx_left;
			xe += dx_right;
		}
	}
	void PrimitiveAssembly::DrawFlatTopTriangle(glm::vec2& v0, glm::vec2& v1, glm::vec2& v2, VSOutput **vertices, int *indices)
	{
		using namespace glm;


		// VSOutput *vertices,int *indices
		float height = v1.y - v0.y;// +0.5f;
		float dx_left = (v1.x - v0.x) / height;
		float dx_right = (v2.x - v0.x) / height;

		float xs = v0.x;
		float xe = v0.x;

		int endY = v1.y + 0.5f;

		for (int y = v0.y; y <= endY; y++)
		{
			float ratio = (float)(y - v0.y) / (float)(endY - v0.y);

			VSOutput vertexS, vertexE;
			VertexLerp_Perspective(*vertices[indices[2]], *vertices[indices[0]], ratio, vertexS);
			VertexLerp_Perspective(*vertices[indices[2]], *vertices[indices[1]], ratio, vertexE);

			//Screen space clipping
			if (y < 0 || y > SCREENHEIGHT_INDEX)
			{
				xs += dx_left;
				xe += dx_right;
				continue;
			}

			DrawLineNew(int(xs + 0.5f), int(xe + 0.5f), y, vertexS, vertexE);
			xs += dx_left;
			xe += dx_right;
		}

	}

	void PrimitiveAssembly::DrawLineNew(int start, int end, int y, VSOutput &vertexStart, VSOutput &vertexEnd)
	{
		VSOutput& input_start = vertexStart;
		VSOutput& input_end = vertexEnd;

		float total = (end) - start;

		if ((total - 0.0001f) < 0.0f)
			return;

		for (int x = start; x <= end; x++)
		{
			// Screen space clipping x
			if (y < 0 || y > SCREENHEIGHT_INDEX|| x < 0 || x > SCREENWIDTH_INDEX)
				continue;

			float alphaValue = (float)(x - start) / total;
			float oneMinusAlpha = 1.0f - alphaValue;

			float zValue = input_start.position.z * oneMinusAlpha + input_end.position.z * alphaValue;

			if (m_zBuffer[y][x] <= zValue)
				continue;

			VSOutput input;
			VertexLerp_Perspective(input_start, input_end, alphaValue, input);

			vec3 outputColor = m_pPixelShader->main(input);

			//Bigger closer  // ztest 可以再光栅化之前作，这时可以预判z值，提前kill掉，加速????
			m_zBuffer[y][x] = zValue;

			if (outputColor.x < 0.0f)outputColor.x = 0.0f;
			if (outputColor.y < 0.0f)outputColor.y = 0.0f;
			if (outputColor.z < 0.0f)outputColor.z = 0.0f;
	
			m_pFrameBuffer->DrawPixel(x, y, outputColor.x, outputColor.y, outputColor.z, 255);
			
		}
	}

	void PrimitiveAssembly::DrawLineBresenham(int x1, int y1, int x2, int y2, VSOutput &vertex1, VSOutput &vertex2, VSOutput &vertex3)
	{
		bool changed = false;

		int x = x1;
		int y = y1;

		int dx = abs(x2 - x1);
		int dy = abs(y2 - y1);

		int signx = signum(x2 - x1);
		int signy = signum(y2 - y1);

		if (dy > dx)//判断以哪个为基准增长
		{
			swap(dx, dy);
			changed = true;
		}

		float e = 2 * (float)dy - (float)dx;
		for (int i = 1; i <= dx; i++)
		{
			// z is divided by w. so z become the depth value after this operation
			float w;
			//PSInput input = Interpolate(x, y, vertex1, vertex2, vertex3, w);
			//vec3 outputColor = m_pPixelShader->main(input);

			//Bigger closer
			//if (m_zBuffer[y][x] > input.position.z)
			{
				//printf("%f",input.position.z);
				//m_zBuffer[y][x] = input.position.z;
				if (y < 0 || x < 0 || y > SCREENHEIGHT_INDEX || x > SCREENWIDTH_INDEX)
					continue;
				m_pFrameBuffer->DrawPixel(x, y,0,0,0, 255);
			}


			while (e >= 0)
			{
				if (changed)
					x = x + signx;
				else
					y = y + signy;
				e = e - 2 * dx;
			}
			if (changed)
				y += signy;
			else
				x += signx;
			e = e + 2 * dy;
		}


	}
//#define SIMD_LERP
	//  |position |  |color  | | normal | |UV |
	// [p0 p1 p2 p3] [c0 c1 c2 n0] [n1 n2 u0 u1]
	void PrimitiveAssembly::VertexLerp_Perspective(VSOutput &i_v1, VSOutput &i_v2, float alphaValue, VSOutput &o_v)
	{
#ifdef SIMD_LERP

		{
	
			float oneMinusAlpha = 1.0f - alphaValue;
			V4LinearLerp(oneMinusAlpha, alphaValue, &i_v1.position[0], &i_v2.position[0], &o_v.position[0]);

			float reciprocalInterpolatedW = 1.0f / o_v.position.w;

			float factorS = oneMinusAlpha * i_v1.position.w * reciprocalInterpolatedW;
			float factorE = alphaValue	 * i_v2.position.w * reciprocalInterpolatedW;

			V4LinearLerp(factorS, factorE, &i_v1.color[0], &i_v2.color[0], &o_v.color[0]);
			V4LinearLerp(factorS, factorE, &i_v1.normal[1], &i_v2.normal[1], &o_v.normal[1]);

			o_v.position.z = i_v1.position.z * factorS + i_v2.position.z * factorE;
		}
		

#else
		float oneMinusAlpha = 1.0f - alphaValue;

		glm::vec4 position = i_v1.position * oneMinusAlpha + i_v2.position * alphaValue;
		float reciprocalInterpolatedW = 1.0f / position.w;

		float factorS = oneMinusAlpha * i_v1.position.w * reciprocalInterpolatedW;
		float factorE = alphaValue	 * i_v2.position.w * reciprocalInterpolatedW;

		position.z = i_v1.position.z * factorS + i_v2.position.z * factorE;

			o_v = VSOutput(
		position,
		(i_v1.color * factorS + i_v2.color  * factorE),
		(i_v1.normal  * factorS + i_v2.normal  * factorE),
		(i_v1.UV  * factorS + i_v2.UV  * factorE)
		);
#endif
	}



}