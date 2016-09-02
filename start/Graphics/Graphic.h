#pragma once

namespace SYSR
{
	class VertexBuffer;
	class IndexBuffer;
	class VertexShader;
	class PixelShader;
	class Camera;
	class Texture;
	class MemoryPool;

	class BitMap;

	extern BitMap *bitmap;
	extern Camera *pCamera;
	extern Texture *pBindedTexture;
	extern MemoryPool *pMemoryManager;

	void Graphics_Initilize();
	void Graphics_Draw(VertexBuffer *pVertexBuffer, IndexBuffer *pIndexBuffer, VertexShader *i_pVertexShader, PixelShader * i_pPixelShader,Texture *i_pTexture);
	void Graphic_ShutDown();
	void CleanZBuffer();


}
