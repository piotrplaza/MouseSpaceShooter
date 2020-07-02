#include "textures.hpp"

#include <cassert>
#include <stdexcept>

#include <GL/glew.h>

#include <stb_image/stb_image.h>

#include <globals.hpp>

#include <components/texture.hpp>

namespace Systems
{
	Textures::Textures()
	{
		using namespace Globals::Components;
		using namespace Globals::Constants;

		static_assert(maxTextureObjects <= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
		assert(textures.size() <= maxTextureObjects);
		unsigned textureObjectsBuffer[maxTextureObjects];

		glGenTextures(textures.size(), textureObjectsBuffer);
		for (unsigned i = 0; i < textures.size(); ++i)
		{
			textures[i].textureUnit = GL_TEXTURE0 + i;
			textures[i].textureObject = textureObjectsBuffer[i];
			loadAndConfigureTexture(textures[i]);
		}
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture) const
	{
		glActiveTexture(texture.textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture.textureObject);

		if (nullptr == (texture.bytes = stbi_load(texture.path.c_str(), &texture.width, &texture.height, &texture.bitDepth, 0)))
		{
			assert(!"Unable to load image.");
			throw std::runtime_error("Unable to load image \"" + texture.path + "\".");
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, texture.bitDepth == 4 ? GL_RGBA : texture.bitDepth == 3 ? GL_RGB : GL_ALPHA, texture.width, texture.height, 0,
			texture.bitDepth == 4 ? GL_RGBA : texture.bitDepth == 3 ? GL_RGB : GL_ALPHA, GL_UNSIGNED_BYTE, texture.bytes);

		if (texture.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			texture.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			texture.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			texture.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}
