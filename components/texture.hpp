#pragma once

#include "_componentBase.hpp"

#include <ogl/oglProxy.hpp>

#include <glm/vec2.hpp>

#include <string>

namespace Components
{
	struct Texture : ComponentBase
	{
		Texture(std::string path, GLint wrapMode = GL_CLAMP_TO_BORDER, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLint magFilter = GL_LINEAR):
			path(std::move(path)),
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter)
		{
		}

		Texture(unsigned textureUnit, unsigned textureObject, GLint wrapMode, GLint minFilter, GLint magFilter):
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter)
		{
			loaded.textureUnit = textureUnit;
			loaded.textureObject = textureObject;

			state = ComponentState::Ongoing;
		}

		std::string path;
		GLint wrapMode = GL_CLAMP_TO_BORDER;
		GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
		GLint magFilter = GL_LINEAR;

		glm::vec2 translate{ 0.0f };
		glm::vec2 scale{ 1.0f };

		bool premultipliedAlpha = true;
		bool darkToTransparent = false;

		struct
		{
			unsigned textureUnit = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
			int bitDepth = 0;
		} loaded;
	};
}
