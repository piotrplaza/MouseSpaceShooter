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
#include <functional>
#include <optional>

namespace Components
{
	struct Texture : ComponentBase
	{
		Texture() = default;

		Texture(TextureSourceVariant source, GLint wrapMode = GL_CLAMP_TO_BORDER, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLint magFilter = GL_LINEAR) :
			source(std::move(source)),
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter)
		{
		}

		TextureSourceVariant source;

		GLint wrapMode = GL_CLAMP_TO_BORDER;
		GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
		GLint magFilter = GL_LINEAR;

		glm::vec2 translate{ 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale{ 1.0f };
		bool preserveAspectRatio = false;
		glm::vec4 borderColor{ 0.0f };

		std::function<std::pair<glm::ivec2, glm::ivec2>(glm::ivec2)> sourceFragmentCornerAndSizeF;
		std::function<TextureSubData()> subImagesF;

		struct
		{
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

			float getAspectRatio() const
			{
				return (float)size.x / size.y;
			}
		} loaded;
	};
}
