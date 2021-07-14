#include "textures.hpp"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>

#include <stb_image/stb_image.h>

#include <globals.hpp>

#include <components/textureDef.hpp>
#include <components/texture.hpp>
#include <components/lowResBuffers.hpp>

namespace
{
	constexpr int maxTextureObjects = 100;
}

namespace Systems
{
	Textures::Textures()
	{
		using namespace Globals::Components;

		stbi_set_flip_vertically_on_load(true);

		static_assert(maxTextureObjects <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		assert(texturesDef.size() <= maxTextureObjects);
		
		textures = std::vector<Components::Texture>(texturesDef.size() + Globals::Components::lowResBuffers.instances);
		for (unsigned i = 0; i < texturesDef.size(); ++i)
		{
			auto& textureDef = texturesDef[i];
			auto& texture = textures[i];
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
		using namespace Globals::Components;

		auto createLowResFramebufferTexture = [counter = 1](Components::LowResBuffers::SubBuffers& subBuffers,
			GLint textureMagFilter) mutable
		{
			subBuffers.textureUnit = GL_TEXTURE0 + textures.size() - counter;
			std::prev(textures.end(), counter)->textureUnit = subBuffers.textureUnit;
			glActiveTexture(subBuffers.textureUnit);
			glGenTextures(1, &subBuffers.textureObject);
			glBindTexture(GL_TEXTURE_2D, subBuffers.textureObject);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMagFilter);
			++counter;
		};

		createLowResFramebufferTexture(lowResBuffers.lowerLinear, GL_LINEAR);
		createLowResFramebufferTexture(lowResBuffers.lowestLinear, GL_LINEAR);
		createLowResFramebufferTexture(lowResBuffers.pixelArt, GL_NEAREST);
	}
}
