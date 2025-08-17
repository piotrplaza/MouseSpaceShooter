#pragma once

#include "_componentBase.hpp"

#include <commonTypes/standardRenderMode.hpp>

#include <optional>

namespace Components
{
	struct RenderTexture : ComponentBase
	{
		RenderTexture(unsigned textureObject, GLint wrapMode, GLint minFilter, GLint magFilter) :
			wrapMode(wrapMode),
			minFilter(minFilter),
			magFilter(magFilter)
		{
			loaded.textureObject = textureObject;

			state = ComponentState::Ongoing;
		}

		GLint wrapMode = GL_CLAMP_TO_BORDER;
		GLint minFilter = GL_LINEAR_MIPMAP_LINEAR;
		GLint magFilter = GL_LINEAR;

		glm::vec2 translate{ 0.0f };
		float rotate = 0.0f;
		glm::vec2 scale{ 1.0f };

		struct
		{
			unsigned textureObject = 0;

			glm::ivec2 size = { 0, 0 };
			int numOfChannels = 0;

			std::optional<StandardRenderMode> standardRenderMode;

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
