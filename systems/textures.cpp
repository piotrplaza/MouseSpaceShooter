#include "textures.hpp"

#include <components/texture.hpp>
#include <components/framebuffers.hpp>

#include <globals/components.hpp>

#include <stb_image/stb_image.h>

#include <GL/glew.h>

#include <cassert>
#include <stdexcept>

namespace
{
	constexpr int maxTextureObjects = 100;
}

namespace Systems
{
	Textures::Textures()
	{
		static_assert(maxTextureObjects <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		stbi_set_flip_vertically_on_load(true);
		stbi_ldr_to_hdr_gamma(1.0f);
	}

	void Textures::postInit()
	{
		assert(Globals::Components().textures().size() <= maxTextureObjects);

		for (unsigned i = 0; i < Globals::Components().textures().size(); ++i)
		{
			auto& texture = Globals::Components().textures()[i];

			if (texture.state == ComponentState::Changed)
			{
				texture.loaded.textureUnit = GL_TEXTURE0 + i;
				loadAndConfigureTexture(texture);
				texture.state = ComponentState::Ongoing;
			}
		}

		createTextureFramebuffers();
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture)
	{
		glActiveTexture(texture.loaded.textureUnit);
		glGenTextures(1, &texture.loaded.textureObject);
		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		auto& textureCache = pathsToTexturesCache[texture.path];
		if (!textureCache.channels || textureCache.premultipliedAlpha != texture.premultipliedAlpha || textureCache.darkToTransparent != texture.darkToTransparent)
		{
			textureCache.channels.reset(stbi_loadf(texture.path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.bitDepth, 0));
			if (!textureCache.channels)
			{
				assert(!"unable to load image");
				throw std::runtime_error("Unable to load image \"" + texture.path + "\".");
			}

			if (textureCache.bitDepth == 4 && (texture.premultipliedAlpha || texture.darkToTransparent))
			{
				for (int i = 0; i < textureCache.size.x * textureCache.size.y * textureCache.bitDepth; i += 4)
				{
					textureCache.channels[i + 3] = (1 - texture.darkToTransparent) * textureCache.channels[i + 3] + texture.darkToTransparent *
						std::min(1.0f, textureCache.channels[i] + textureCache.channels[i + 1] + textureCache.channels[i + 2]);

					const float premultipliedAlphaFactor = (1 - texture.premultipliedAlpha) + texture.premultipliedAlpha * textureCache.channels[i + 3];
					textureCache.channels[i] *= premultipliedAlphaFactor;
					textureCache.channels[i + 1] *= premultipliedAlphaFactor;
					textureCache.channels[i + 2] *= premultipliedAlphaFactor;
				}
			}
		}
		texture.loaded.size.x = textureCache.size.x;
		texture.loaded.size.y = textureCache.size.y;
		texture.loaded.bitDepth = textureCache.bitDepth;

		const GLint format = texture.loaded.bitDepth == 4 ? GL_RGBA : texture.loaded.bitDepth == 3 ? GL_RGB : texture.loaded.bitDepth == 2 ? GL_RG : GL_RED;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture.loaded.size.x, texture.loaded.size.y, 0, format, GL_FLOAT, textureCache.channels.get());

		if (texture.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			texture.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			texture.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			texture.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Textures::createTextureFramebuffers() const
	{
		auto createTextureFramebuffer = [](Components::Framebuffers::SubBuffers& subBuffers,
			GLint textureMagFilter)
		{
			const unsigned textureUnit = GL_TEXTURE0 + Globals::Components().textures().size();
			glActiveTexture(textureUnit);
			unsigned textureObject;
			glGenTextures(1, &textureObject);
			glBindTexture(GL_TEXTURE_2D, textureObject);

			const auto& texture = Globals::Components().textures().emplace(textureUnit, textureObject, GL_CLAMP_TO_EDGE, GL_NEAREST, textureMagFilter);
			subBuffers.textureUnit = texture.loaded.textureUnit;
			subBuffers.textureObject = texture.loaded.textureObject;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);

			glGenFramebuffers(1, &subBuffers.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, subBuffers.fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, subBuffers.textureObject, 0);

			glGenRenderbuffers(1, &subBuffers.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, subBuffers.depthBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, subBuffers.depthBuffer);
		};

		auto& framebuffers = Globals::Components().framebuffers();

		createTextureFramebuffer(framebuffers.main, GL_LINEAR);
		createTextureFramebuffer(framebuffers.lowerLinearBlend0, GL_LINEAR);
		createTextureFramebuffer(framebuffers.lowerLinearBlend1, GL_LINEAR);
		createTextureFramebuffer(framebuffers.lowestLinearBlend0, GL_LINEAR);
		createTextureFramebuffer(framebuffers.lowestLinearBlend1, GL_LINEAR);
		createTextureFramebuffer(framebuffers.pixelArtBlend0, GL_NEAREST);
		createTextureFramebuffer(framebuffers.pixelArtBlend1, GL_NEAREST);
		createTextureFramebuffer(framebuffers.lowPixelArtBlend0, GL_NEAREST);
		createTextureFramebuffer(framebuffers.lowPixelArtBlend1, GL_NEAREST);
	}
}
