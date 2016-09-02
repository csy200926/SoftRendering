#pragma once
#include <glm.hpp>
#include <intrin.h>
namespace SYSR
{


	using namespace glm;

	int signum(int i);
	void swap(int &x, int &y);
	void SwapVec2(vec2 &v1, vec2 &v2);

	int signum(int i)
	{
		return (i > 0) - (i < 0);
	}
	void swap(int &x, int &y)
	{
		int temp = x;
		x = y;
		y = temp;
	}
	void SwapVec2(vec2 &v1, vec2 &v2)
	{
		vec2 temp = v1;
		v1 = v2;
		v2 = temp;
	}



	void BarycentricCoord(float &a,float &b, float &c, vec2 &v1,vec2 &v2,vec2 &v3,int x,int y)
	{
		a = ( (v2.y-v3.y)*(x-v3.x)+(v3.x-v2.x)*(y-v3.y) ) / ( (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y) );
		b = ( (v3.y-v1.y)*(x-v3.x)+(v1.x-v3.x)*(y-v3.y) ) / ( (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y) );
		c = 1.0f - a - b;
	}

	bool PointRectTest(vec2 &min,vec2 &max,vec2 &point)
	{
		if (point.x >= min.x
			&& point.x <= max.x
			&& point.y >= min.y
			&& point.y <= max.y)
			return true;
		return false;
	}
	bool PointTriangleTest(vec2 &v1, vec2 &v2, vec2 &v3, vec2 &point)
	{
		float a, b, c;
		BarycentricCoord(a,b,c,v1,v2,v3,point.x,point.y);

		return 0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1;
	}

	void V4LinearLerp(float value1,float value2, float* addr1, float *addr2, float *out)
	{

		__m128 registerV1 = _mm_load_ps(addr1);
		__m128 registerV2 = _mm_load_ps(addr2);
		__m128 scalarA = _mm_set1_ps(value1);
		__m128 scalarB = _mm_set1_ps(value2);

		__m128 resultPos = _mm_add_ps(_mm_mul_ps(registerV1, scalarA), _mm_mul_ps(registerV2, scalarB));
		_mm_storeu_ps(out, resultPos);
	}
}