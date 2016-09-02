#pragma once
#include "glm.hpp"
namespace SYSR
{
	struct Triangle_Index
	{
		int indices[3];
		bool isVisible;// Used for back culling

		Triangle_Index(){ isVisible = true; };
		Triangle_Index(unsigned int i_index0, unsigned int i_index1, unsigned int i_index2, bool i_isVisible)
			:isVisible(i_isVisible)
		{
			indices[0] = i_index0;
			indices[1] = i_index1;
			indices[2] = i_index2;
		}
	};

	__declspec(align(16)) struct VSOutput
	{
		glm::vec4 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 UV;

		//blah.. blah
		VSOutput(glm::vec4 &i_position, glm::vec3& i_color, glm::vec3& i_normal, glm::vec2& i_UV)
			: position(i_position), color(i_color), normal(i_normal), UV(i_UV)
		{
		}
		VSOutput(){};
	};//we need registers to store the varyings 



	__declspec(align(16)) struct VSInput
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 UV;

		VSInput(glm::vec3& i_position, glm::vec3& i_color, glm::vec3& i_normal, glm::vec2& i_UV)
			: position(i_position), color(i_color), normal(i_normal), UV(i_UV)
		{
		}
		VSInput(){};


	};




}