#include "PixelShader.h"
#include "../Graphic.h"
#include "../Objects/Texture.h"
namespace SYSR
{

	glm::vec3 PixelShader::main(VSOutput &i_Input)
	{
		using namespace glm;

		//return vec3(255,255,255);
		//vec3 pixelToLight = normalize(light_pos - i_Input.position);
		//float dotValue = dot(pixelToLight, i_Input.normal);
		//pBindedTexture->Sample(i_Input.UV);
		;
		//return vec3(i_Input.color.x, i_Input.color.y, i_Input.color.z);
		return pBindedTexture->Sample(i_Input.UV) * (i_Input.color.z / 255.0f);



	}



}