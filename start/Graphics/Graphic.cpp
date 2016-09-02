#include "Graphic.h"
#include <glew.h> // include GLEW and new version of GL on Windows
#include <glfw3.h> // GLFW helper library
#include <stdio.h>
#include <glm.hpp>
#include "../BitMap/BitMap.h"
#include "detail/func_common.hpp"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CommonStructs.h"
#include "FakeShaders/VertexShader.h"
#include "FromVStoPS/FrameBuffer.h"
#include "FromVStoPS/PriAsm.h"
#include "Camera.h"

#include "SRDef.h"
#include "Objects\Texture.h"

#include "../Utilities/Memory/MemoryPool.h"
namespace SYSR
{
	PrimitiveAssembly *pPrimitiveAssembly;
	IndexBuffer *pIndexBuffer;
	VertexBuffer *pVertexBuffer;
	PixelShader *pPixelShader;
	VertexShader *pVertexShader;
	Camera *pCamera;

	BitMap *bitmap = NULL;
	Texture *pBindedTexture = NULL;

	MemoryPool *pMemoryManager = MemoryPool::create(sizeof(VSOutput) * 146317,1 );

	void Graphics_Initilize()
	{
		using namespace glm;
		using namespace SYSR;

		bitmap = new SYSR::BitMap(SCREENWIDTH, SCREENHEIGHT); bitmap->Clear(0, 0, 0, 255);

		// System initialize
		pPrimitiveAssembly = new PrimitiveAssembly();

		pIndexBuffer = new IndexBuffer();
		pVertexBuffer = new VertexBuffer();

		pPixelShader = new PixelShader();
		pVertexShader = new VertexShader();

		pCamera = new Camera();

		//pCamera->SetLookAt(vec3(0, 0, 30), vec3(0, 0, 0), vec3(0, 1, 0));
		pCamera->SetPerspective(70, 4.0f / 3.0f, 0.1f, 100.0f);

	}

	void Graphics_Draw(VertexBuffer *i_pVertexBuffer, IndexBuffer *i_pIndexBuffer, VertexShader *i_pVertexShader, PixelShader * i_pPixelShader, Texture *i_pTexture)
	{
		
		pBindedTexture = i_pTexture;

		//TODO: The size is hardcoded!!
		VSOutput *vsOutputs = (VSOutput *)pMemoryManager->alloc();// (sizeof(VSOutput) * 146317);
		//new VSOutput[i_pVertexBuffer->GetVertexCount()];
		int count = i_pVertexBuffer->GetVertexCount();
		// VertexShaders //TODO: do not go over all vertexBuffer data???
		for (std::size_t i = 0; i < count; i++)
		{
			pVertexShader->main((*i_pVertexBuffer)[i], vsOutputs[i]);
		}//printf("\n");

		// Rasterization
		// And
		// PixelShaders
		pPrimitiveAssembly->SetDrawContents(i_pIndexBuffer, pPixelShader, bitmap, vsOutputs);
		pPrimitiveAssembly->Assembly();

		pMemoryManager->free(vsOutputs);
		
	}

	void CleanZBuffer()
	{
		pPrimitiveAssembly->CleanZbuffer();
	}

#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;
	void Graphic_ShutDown()
	{
		SAFE_DELETE(bitmap);

		SAFE_DELETE(pPrimitiveAssembly);

		SAFE_DELETE(pIndexBuffer);
		SAFE_DELETE(pVertexBuffer);

		SAFE_DELETE(pPixelShader);
		SAFE_DELETE(pVertexShader);

		SAFE_DELETE(pCamera);
	}


}
