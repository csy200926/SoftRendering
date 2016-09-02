#pragma once
#include "../Graphics/SRDef.h"
#include  <memory.h>
#include "../Utilities/Profiling.h"

#include <intrin.h>

namespace SYSR
{
	class BitMap
	{
	private:
		unsigned int m_width, m_height;
		unsigned int m_imageSize;

		
	public:
		unsigned char * m_data;
		BitMap(unsigned int i_width, unsigned int i_height)
		{
			m_height = i_height;
			m_width = i_width;

			m_imageSize = m_width * m_height * 4;
			m_data = (unsigned char*)_aligned_malloc(m_imageSize * sizeof(unsigned char), 16);//new unsigned char [m_imageSize]; 
		}
		~BitMap()
		{
			_aligned_free(m_data);//delete []m_data;
		}

		inline unsigned char * GetData();

		inline void ImageProcess();

		inline void Clear_White();
		inline void Clear(unsigned char r,unsigned char g,unsigned char b,unsigned char a);
		inline void DrawPixel(int x, int y,unsigned char r,unsigned char g,unsigned char b,unsigned char a);

	};

	unsigned char * BitMap::GetData()
	{
		return m_data;
	}

	void BitMap::DrawPixel(int x, int y,unsigned char r,unsigned char g,unsigned char b,unsigned char a)
	{
		int index = (x + y * m_width) * 4;
		m_data[index] = r;
		m_data[index + 1] = g;
		m_data[index + 2] = b;
		m_data[index + 3] = a;
	}

	void BitMap::ImageProcess()
	{
		//PROFILE_SCOPE_BEGIN("Image processing")

		__m128 rFactor = _mm_set_ps(0.393f, 0.349f, 0.272f, 0.0f);
		__m128 gFactor = _mm_set_ps(0.769f, 0.686f, 0.534f, 0.0f);
		__m128 bFactor = _mm_set_ps(0.189f, 0.168f, 0.131f, 0.0f);

		int length = m_width * m_height * 4;
		for (int index = 0; index < length; index+=4)
		{
			unsigned char r = m_data[index];
			unsigned char g = m_data[index + 1];
			unsigned char b = m_data[index + 2];
			//unsigned char a = m_data[index + 3];

			// expand as scalar
			const __m128 rScalar = _mm_set1_ps(r);
			const __m128 gScalar = _mm_set1_ps(g);
			const __m128 bScalar = _mm_set1_ps(b);
			//const __m128 aScalar = _mm_set1_ps(a);

			// accmulate mul
			__m128 result = _mm_mul_ps(rScalar, rFactor);
			result = _mm_add_ps(result, _mm_mul_ps(gScalar, gFactor));
			result = _mm_add_ps(result, _mm_mul_ps(bScalar, bFactor));

			// inverse
			__m128i resultInt = _mm_cvtps_epi32(result);
			resultInt = _mm_shuffle_epi32(resultInt, _MM_SHUFFLE(0, 1, 2, 3));

			// shrink to 4 x 8 bits
			resultInt = _mm_packus_epi32(resultInt, resultInt);
			resultInt = _mm_packus_epi16(resultInt, resultInt);

			// put back to array memory 
			*(int*)(&m_data[index]) = _mm_cvtsi128_si32(resultInt);
		}
		




		// No float point version, 
		// Not faster than float point version

		//__m128i rFactor = _mm_set_epi32(402, 357, 279, 0);
		//__m128i gFactor = _mm_set_epi32(787, 702, 547, 0);
		//__m128i bFactor = _mm_set_epi32(194, 172, 134, 0);

		//int length = m_width * m_height * 4;
		//for (int index = 0; index < length; index+=4)
		//{
		//	unsigned char r = m_data[index];
		//	unsigned char g = m_data[index + 1];
		//	unsigned char b = m_data[index + 2];
		//	//unsigned char a = m_data[index + 3];

		//	// expand as scalar
		//	const __m128i rScalar = _mm_set1_epi32(r);
		//	const __m128i gScalar = _mm_set1_epi32(g);
		//	const __m128i bScalar = _mm_set1_epi32(b);
		//	//const __m128 aScalar = _mm_set1_ps(a);

		//	// accmulate mul
		//	__m128i result = _mm_mullo_epi32(rScalar, rFactor);
		//	result = _mm_add_epi32(result, _mm_mullo_epi32(gScalar, gFactor));
		//	result = _mm_add_epi32(result, _mm_mullo_epi32(bScalar, bFactor));

		//	// divided by 1024
		//	result = _mm_srli_epi32(result, 10);

		//	// inverse
		//	result = _mm_shuffle_epi32(result, _MM_SHUFFLE(0, 1, 2, 3));

		//	// shrink to 4 x 8 bits
		//	result = _mm_packus_epi32(result, result);
		//	result = _mm_packus_epi16(result, result);

		//	// put back to array memory 
		//	*(int*)(&m_data[index]) = _mm_cvtsi128_si32(result);
		//}


		// No SIME

		//for (unsigned int x = 0; x < m_width; x++)
		//{
		//	for (unsigned int y = 0; y < m_height; y++)
		//	{
		//		int index = (x + y * m_width) * 4;
		//		unsigned char r = m_data[index];
		//		unsigned char g = m_data[index + 1];
		//		unsigned char b = m_data[index + 2];
		//		unsigned char a = m_data[index + 3];

		//		int or = (r *402 + g * 787 + b * 194) >> 10;
		//		int og = (r *357 + g * 702 + b * 172) >> 10;
		//		int ob = (r *279 + g * 547 + b * 134) >> 10;
		//		int oa = a;

		//		m_data[index] = or > 255 ? 255 : or;
		//		m_data[index + 1] = og > 255 ? 255 : og;
		//		m_data[index + 2] = ob > 255 ? 255 : ob;
		//		m_data[index + 3] = oa > 255 ? 255 : oa;

		//	}
		//}
		//PROFILE_SCOPE_END
	}

	void BitMap::Clear(unsigned char r,unsigned char g,unsigned char b,unsigned char a)
	{
		for (unsigned int x = 0 ; x < m_width ; x++)
		{
			for (unsigned int y = 0 ; y < m_height ; y++)
			{
				DrawPixel(x,y,r,g,b,a);
			}
		}
	}
	void BitMap::Clear_White()
	{
		memset(m_data, 255, sizeof(char) * SCREENHEIGHT * SCREENWIDTH * 4);
	}

}

