#pragma once
#include <glm.hpp>
#include <vector>

namespace SYSR
{

	class Texture
	{
	public:

		glm::vec3 Sample(glm::vec2 &UV)
		{
			int x = UV.x * (width - 1) + 0.5f;
			int y = UV.y * (height - 1) + 0.5f;

			x = x % width;
			y = y % height;
			//if (x >= width)
			//	x = width - 1;
			//if (x < 0)
			//	x = 0;
			//if (y >= height)
			//	y = height - 1;
			//if (y < 0)
			//	y = 0;

			unsigned char value1 = image_data[y * channels * width + x * channels];
			unsigned char value2 = image_data[y * channels * width + x * channels + 1];
			unsigned char value3 = image_data[y * channels * width + x * channels + 2];
			return glm::vec3(value1, value2, value3);
		};
		const int channels;
		Texture(const char *i_path);
		int width, height;
		unsigned char * image_data;
	};
}