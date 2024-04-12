#pragma once

#include "_componentBase.hpp"

#include "details/textureData.hpp"

#include <ogl/oglProxy.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <vector>
#include <variant>

namespace Components
{
	struct Texture : ComponentBase
	{
		Texture(std::variant<std::string, TextureData> dataSource, GLint wrapMode = GL_CLAMP_TO_BORDER, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLint magFilter = GL_LINEAR):
			dataSource(std::move(dataSource)),
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter),
			sourceWithPremultipliedAlpha(std::holds_alternative<TextureData>(this->dataSource))
		{
		}

		Texture(unsigned textureUnit, unsigned textureObject, GLint wrapMode, GLint minFilter, GLint magFilter):
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter),
			sourceWithPremultipliedAlpha(true)
		{
			loaded.textureUnit = textureUnit;
			loaded.textureObject = textureObject;

			state = ComponentState::Ongoing;
		}

		std::variant<std::string, TextureData> dataSource;
		GLint wrapMode = GL_CLAMP_TO_BORDER;
		GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
		GLint magFilter = GL_LINEAR;

		glm::vec2 translate{ 0.0f };
		glm::vec2 scale{ 1.0f };

		bool sourceWithPremultipliedAlpha;
		bool convertDarkToTransparent = false;

		struct
		{
			unsigned textureUnit = 0;
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
			int numOfChannels = 0;

			GLint getFormat() const
			{
				switch (numOfChannels)
				{
				case 1: return GL_RED;
				case 2: return GL_RG;
				case 3: return GL_RGB;
				case 4: return GL_RGBA;
				default: assert(!"unsupported number of channels"); return 0;
				}
			}
		} loaded;
	};
}
