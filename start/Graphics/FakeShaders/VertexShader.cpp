#include "VertexShader.h"
#include "../Camera.h"
#include <stdio.h>

namespace SYSR
{
	glm::vec3 light_pos(5, 40, 1);
	void VertexShader::main(const VSInput &i_Input, VSOutput &i_OutPut)
	{
		using namespace glm;
		i_OutPut.position = 
			  Camera::Projective_Matrix 
			* Camera::WorldToView_Matrix 
			* Camera::ModelToWorld_Matrix
			* glm::vec4(i_Input.position.x,i_Input.position.y,i_Input.position.z,1);
//		printf("pos%f,%f,%f,%f\n", i_OutPut.position.x, i_OutPut.position.y, i_OutPut.position.z, i_OutPut.position.w);
		vec3 pixelToLight = normalize(light_pos - i_Input.position);
		float dotValue = dot(pixelToLight, i_Input.normal);
		i_OutPut.color = vec3(255, 255, 255) * dotValue; //i_Input.color;

		i_OutPut.normal = i_Input.normal;
		i_OutPut.UV = i_Input.UV;


	}



}