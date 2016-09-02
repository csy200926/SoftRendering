#include "Material.h"
#include "../FakeShaders/PixelShader.h"
#include "../FakeShaders/VertexShader.h"
#include <stdio.h>
namespace SYSR
{
	Material::Material()
	{
		m_PixelShader = new PixelShader();
		m_VertexShader = new VertexShader();
	}
	Material::~Material()
	{
		delete m_PixelShader;
		delete m_VertexShader;
	}



}