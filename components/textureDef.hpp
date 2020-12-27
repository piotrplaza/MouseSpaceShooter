#pragma once

#include <string>

#include <GL/glew.h>

#include <glm/vec2.hpp>

#include <componentBase.hpp>

namespace Components
{
	struct TextureDef : ComponentBase
	{
		TextureDef(std::string path, GLenum wrapMode = GL_CLAMP_TO_EDGE, GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum magFilter = GL_LINEAR_MIPMAP_LINEAR):
			path(std::move(path)),
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter)
		{
		}

		std::string path;
		GLenum wrapMode;
		GLenum minFilter;
		GLenum magFilter;

		glm::vec2 translate{ 0.0f };
		glm::vec2 scale{ 1.0f };
	};
}
