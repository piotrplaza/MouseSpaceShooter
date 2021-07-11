#pragma once

#include <GL/glew.h>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

namespace Components
{
	struct Texture : ComponentBase
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

		glm::ivec2 size = {0, 0};
		int bitDepth = 0;
	};
}
