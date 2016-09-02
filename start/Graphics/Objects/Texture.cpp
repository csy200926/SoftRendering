#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb_image.h"

namespace SYSR
{
	Texture::Texture(const char *i_path) :channels(4)
	{
		
		int n;
		int force_channels = 4;
		image_data = stbi_load(i_path, &width, &height, &n, force_channels);

		if (image_data == nullptr)
		{
			printf("worng image path!");
			return;
		}
		
		int width_in_bytes = width * 4;
		unsigned char *top = NULL;
		unsigned char *bottom = NULL;
		unsigned char temp = 0;
		int half_height = height / 2;

		for (int row = 0; row < half_height; row++) {
			top = image_data + row * width_in_bytes;
			bottom = image_data + (height - row - 1) * width_in_bytes;
			for (int col = 0; col < width_in_bytes; col++) {
				temp = *top;
				*top = *bottom;
				*bottom = temp;
				top++;
				bottom++;
			}
		}

	}

}