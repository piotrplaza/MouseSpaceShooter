#include "textures.hpp"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>

#include <stb_image/stb_image.h>

#include <globals.hpp>

#include <components/textureDef.hpp>
#include <components/texture.hpp>

namespace Systems
{
	Textures::Textures()
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		stbi_set_flip_vertically_on_load(true);

		static_assert(maxTextureObjects <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		assert(texturesDef.size() <= maxTextureObjects);
		
		textures = std::vector<::Components::Texture>(texturesDef.size());
		for (unsigned i = 0; i < texturesDef.size(); ++i)
		{
			auto& textureDef = texturesDef[i];
			auto& texture = textures[i];
			texture.textureUnit = GL_TEXTURE0 + i;
			loadAndConfigureTexture(textureDef, texture);
		}
	}

	void Textures::loadAndConfigureTexture(const Components::TextureDef& textureDef, Components::Texture& texture)
	{
		glActiveTexture(texture.textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture.textureObject);

		auto& textureCache = pathsToTexturesCache[textureDef.path];
		if (!textureCache.bytes)
		{
			textureCache.bytes.reset(stbi_load(textureDef.path.c_str(), &textureCache.width, &textureCache.height, &textureCache.bitDepth, 0));
			if (!textureCache.bytes)
			{
				assert(!"Unable to load image.");
				throw std::runtime_error("Unable to load image \"" + textureDef.path + "\".");
			}
		}
		texture.width = textureCache.width;
		texture.height = textureCache.height;
		texture.bitDepth = textureCache.bitDepth;

		const GLint format = texture.bitDepth == 4 ? GL_RGBA : texture.bitDepth == 3 ? GL_RGB : texture.bitDepth == 2 ? GL_RG : GL_RED;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureDef.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureDef.wrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureDef.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureDef.magFilter);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, texture.width, texture.height, 0, format, GL_UNSIGNED_BYTE, textureCache.bytes.get());

		if (textureDef.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			textureDef.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			textureDef.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			textureDef.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}
