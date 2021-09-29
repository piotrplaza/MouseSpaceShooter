#include "textures.hpp"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>

#include <stb_image/stb_image.h>

#include <globals.hpp>

#include <components/texture.hpp>
#include <components/framebuffers.hpp>

namespace
{
	constexpr int maxTextureObjects = 100;
}

namespace Systems
{
	Textures::Textures()
	{
		stbi_set_flip_vertically_on_load(true);

		static_assert(maxTextureObjects <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		assert(Globals::Components().textures().size() <= maxTextureObjects);

		for (unsigned i = 0; i < Globals::Components().textures().size(); ++i)
		{
			auto& texture = Globals::Components().textures()[i];
			texture.loaded.textureUnit = GL_TEXTURE0 + i;
			loadAndConfigureTexture(texture);
		}

		createLowResFramebuffersTextures();
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture)
	{
		glActiveTexture(texture.loaded.textureUnit);
		glGenTextures(1, &texture.loaded.textureObject);
		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		auto& textureCache = pathsToTexturesCache[texture.path];
		if (!textureCache.bytes)
		{
			textureCache.bytes.reset(stbi_load(texture.path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.bitDepth, 0));
			if (!textureCache.bytes)
			{
				assert(!"Unable to load image.");
				throw std::runtime_error("Unable to load image \"" + texture.path + "\".");
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
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture.loaded.size.x, texture.loaded.size.y, 0, format, GL_UNSIGNED_BYTE, textureCache.bytes.get());

		if (texture.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			texture.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			texture.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			texture.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Textures::createLowResFramebuffersTextures() const
	{
		auto& framebuffers = Globals::Components().framebuffers();

		auto createLowResFramebufferTexture = [](Components::Framebuffers::SubBuffers& subBuffers,
			GLint textureMagFilter)
		{
			const unsigned textureUnit = GL_TEXTURE0 + Globals::Components().textures().size();
			glActiveTexture(textureUnit);
			unsigned textureObject;
			glGenTextures(1, &textureObject);
			glBindTexture(GL_TEXTURE_2D, textureObject);

			const auto& texture = Globals::Components().textures().emplace_back(textureUnit, textureObject, GL_CLAMP_TO_EDGE, GL_NEAREST, textureMagFilter);
			subBuffers.textureUnit = texture.loaded.textureUnit;
			subBuffers.textureObject = texture.loaded.textureObject;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);
		};

		createLowResFramebufferTexture(framebuffers.main, GL_LINEAR);
		createLowResFramebufferTexture(framebuffers.lowerLinearBlend0, GL_LINEAR);
		createLowResFramebufferTexture(framebuffers.lowerLinearBlend1, GL_LINEAR);
		createLowResFramebufferTexture(framebuffers.lowestLinearBlend0, GL_LINEAR);
		createLowResFramebufferTexture(framebuffers.lowestLinearBlend1, GL_LINEAR);
		createLowResFramebufferTexture(framebuffers.pixelArtBlend0, GL_NEAREST);
		createLowResFramebufferTexture(framebuffers.pixelArtBlend1, GL_NEAREST);
		createLowResFramebufferTexture(framebuffers.lowPixelArtBlend0, GL_NEAREST);
		createLowResFramebufferTexture(framebuffers.lowPixelArtBlend1, GL_NEAREST);
	}
}
