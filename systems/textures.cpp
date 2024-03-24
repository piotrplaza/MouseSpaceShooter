#include "textures.hpp"

#include <components/texture.hpp>
#include <components/framebuffers.hpp>

#include <globals/components.hpp>

#include <ogl/oglProxy.hpp>

#include <stb_image/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <stdexcept>

namespace Systems
{
	Textures::Textures()
	{
		stbi_set_flip_vertically_on_load(true);
		stbi_ldr_to_hdr_gamma(1.0f);

		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(0.0f)));

		createTextureFramebuffers();

		staticTexturesOffset = Globals::Components().staticTextures().size();
	}

	void Textures::postInit()
	{
		for (auto& texture: Globals::Components().staticTextures())
		{
			if (texture.state == ComponentState::Changed)
			{
				texture.loaded.textureUnit = textureUnits.acquire();
				loadAndConfigureTexture(texture);
				texture.state = ComponentState::Ongoing;
			}
		}

		staticTexturesOffset = Globals::Components().staticTextures().size();
	}

	void Textures::step()
	{
		updateDynamicTextures();
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture)
	{
		class DataSourceVisitor
		{
		public:
			DataSourceVisitor(std::unordered_map<std::string, TextureCache>& pathsToTexturesCache, Components::Texture& texture):
				pathsToTexturesCache(pathsToTexturesCache),
				texture(texture)
			{
			}

			void operator()(const std::string& path)
			{
				const int prevTextureSize = texture.loaded.size.x * texture.loaded.size.y * texture.loaded.numOfChannels;
				auto& textureCache = pathsToTexturesCache[path];
				if (!textureCache.data || textureCache.premultipliedAlpha != texture.premultipliedAlpha || textureCache.darkToTransparent != texture.darkToTransparent)
				{
					textureCache.data.reset(stbi_loadf(path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.numOfChannels, 0));
					if (!textureCache.data)
					{
						assert(!"unable to load image");
						throw std::runtime_error("Unable to load image \"" + path + "\".");
					}

					optionalAlphaProcessing(textureCache.data.get(), textureCache.size, textureCache.numOfChannels);
				}

				texture.loaded.size.x = textureCache.size.x;
				texture.loaded.size.y = textureCache.size.y;
				texture.loaded.numOfChannels = textureCache.numOfChannels;

				if (prevTextureSize != texture.loaded.size.x * texture.loaded.size.y * texture.loaded.numOfChannels)
					glTexImage2D(GL_TEXTURE_2D, 0, texture.loaded.getFormat(), texture.loaded.size.x, texture.loaded.size.y, 0, texture.loaded.getFormat(), GL_FLOAT, textureCache.data.get());
				else
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.loaded.size.x, texture.loaded.size.y, texture.loaded.getFormat(), GL_FLOAT, textureCache.data.get());
			}

			void operator()(const Components::Texture::TextureData& data)
			{
			}

		private:
			void optionalAlphaProcessing(float* data, glm::vec2 size, int numOfChannels) const
			{
				if (numOfChannels == 4 && (texture.premultipliedAlpha || texture.darkToTransparent))
					for (int i = 0; i < size.x * size.y * 4; i += 4)
					{
						data[i + 3] = (1 - texture.darkToTransparent) * data[i + 3] + texture.darkToTransparent * std::min(1.0f, data[i] + data[i + 1] + data[i + 2]);

						const float premultipliedAlphaFactor = (1 - texture.premultipliedAlpha) + texture.premultipliedAlpha * data[i + 3];
						data[i] *= premultipliedAlphaFactor;
						data[i + 1] *= premultipliedAlphaFactor;
						data[i + 2] *= premultipliedAlphaFactor;
					}
			}

			std::unordered_map<std::string, TextureCache>& pathsToTexturesCache;
			Components::Texture& texture;
		};

		glActiveTexture(texture.loaded.textureUnit);
		glGenTextures(1, &texture.loaded.textureObject);
		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		std::visit(DataSourceVisitor{ pathsToTexturesCache, texture }, texture.dataSource);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (texture.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			texture.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			texture.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			texture.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Textures::createTextureFramebuffers()
	{
		assert(Globals::Components().staticTextures().empty());

		auto createTextureFramebuffer = [this](Components::Framebuffers::SubBuffers& subBuffers,
			GLint textureMagFilter)
		{
			const unsigned textureUnit = textureUnits.acquire();
			glActiveTexture(textureUnit);
			unsigned textureObject;
			glGenTextures(1, &textureObject);
			glBindTexture(GL_TEXTURE_2D, textureObject);

			const auto& texture = Globals::Components().staticTextures().emplace(textureUnit, textureObject, GL_CLAMP_TO_EDGE, GL_NEAREST, textureMagFilter);
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

	void Textures::updateDynamicTextures()
	{
		for (auto& texture: Globals::Components().dynamicTextures())
		{
			if (texture.state == ComponentState::Ongoing)
				continue;
			else if (texture.state == ComponentState::Changed)
			{
				if (texture.loaded.textureUnit == 0)
					texture.loaded.textureUnit = textureUnits.acquire();
				loadAndConfigureTexture(texture);
				texture.state = ComponentState::Ongoing;
			}
			else if (texture.state == ComponentState::Outdated)
				deleteTexture(texture);
			else
				assert(!"unsupported texture state");
		}
	}

	void Textures::deleteTexture(Components::Texture& texture)
	{
		textureUnits.release(texture.loaded.textureUnit);
		texture.loaded.textureUnit = 0;
		glDeleteTextures(1, &texture.loaded.textureObject);
		texture.loaded.textureObject = 0;
	}
}
