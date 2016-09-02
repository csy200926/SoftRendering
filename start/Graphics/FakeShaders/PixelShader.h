#pragma once
#include <glm.hpp>
#include "VertexShader.h"
#include "../CommonStructs.h"
namespace SYSR
{

	class PixelShader
	{
	public:
		glm::vec3 main(VSOutput &i_Input);
	protected:
	private:

	};



}