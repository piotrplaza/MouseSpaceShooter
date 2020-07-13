#pragma once

#include <GL/glew.h>

namespace Components
{
	struct Texture
	{
		Texture()
		{
			glGenTextures(1, &textureObject);
		}

		~Texture()
		{
			glDeleteTextures(1, &textureObject);
		}

		unsigned textureUnit = 0;
		unsigned textureObject = 0;

		int width = 0;
		int height = 0;
		int bitDepth = 0;
	};
}
