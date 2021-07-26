#include "textures.hpp"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>

#include <stb_image/stb_image.h>

#include <globals.hpp>

#include <components/textureDef.hpp>
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
		assert(Globals::Components().texturesDef().size() <= maxTextureObjects);
		
		Globals::Components().textures() = std::vector<Components::Texture>(Globals::Components().texturesDef().size() + Globals::Components().framebuffers().instances);
		for (unsigned i = 0; i < Globals::Components().texturesDef().size(); ++i)
		{
			auto& textureDef = Globals::Components().texturesDef()[i];
			auto& texture = Globals::Components().textures()[i];
			texture.textureUnit = GL_TEXTURE0 + i;
			loadAndConfigureTexture(textureDef, texture);
		}

		createLowResFramebuffersTextures();
	}

	void Textures::loadAndConfigureTexture(const Components::TextureDef& textureDef, Components::Texture& texture)
	{
		glActiveTexture(texture.textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture.textureObject);

		auto& textureCache = pathsToTexturesCache[textureDef.path];
		if (!textureCache.bytes)
		{
			textureCache.bytes.reset(stbi_load(textureDef.path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.bitDepth, 0));
			if (!textureCache.bytes)
			{
				assert(!"Unable to load image.");
				throw std::runtime_error("Unable to load image \"" + textureDef.path + "\".");
			}
		}
		texture.size.x = textureCache.size.x;
		texture.size.y = textureCache.size.y;
		texture.bitDepth = textureCache.bitDepth;

		const GLint format = texture.bitDepth == 4 ? GL_RGBA : texture.bitDepth == 3 ? GL_RGB : texture.bitDepth == 2 ? GL_RG : GL_RED;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureDef.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureDef.wrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureDef.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureDef.magFilter);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture.size.x, texture.size.y, 0, format, GL_UNSIGNED_BYTE, textureCache.bytes.get());

		if (textureDef.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			textureDef.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			textureDef.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			textureDef.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Textures::createLowResFramebuffersTextures() const
	{
		auto& framebuffers = Globals::Components().framebuffers();

		auto createLowResFramebufferTexture = [counter = 1](Components::Framebuffers::SubBuffers& subBuffers,
			GLint textureMagFilter) mutable
		{
			subBuffers.textureUnit = GL_TEXTURE0 + Globals::Components().textures().size() - counter;
			std::prev(Globals::Components().textures().end(), counter)->textureUnit = subBuffers.textureUnit;
			glActiveTexture(subBuffers.textureUnit);
			glGenTextures(1, &subBuffers.textureObject);
			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter);
			++counter;
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
