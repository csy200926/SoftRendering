#pragma once

namespace SYSR
{
	class PixelShader;
	class FrameBuffer;
	class Rasterizer
	{

	public:
		PixelShader *m_pPixelShader;
		void Initilize(PixelShader *i_pCurrentPixelShader ,FrameBuffer *i_pFrameBuffer);
		void Rasterize();
	};



}