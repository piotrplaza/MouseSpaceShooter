#include "textures.hpp"

#include <components/texture.hpp>
#include <components/framebuffers.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>

#include <ogl/oglProxy.hpp>

#include <stb_image/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <stdexcept>
#include <execution>

namespace
{
	constexpr bool parallelProcessing = true;

	template<typename ColorType>
	inline float* getFirstFloatPtr(std::vector<ColorType>& data)
	{
		assert(!data.empty());

		if (data.empty())
			return nullptr;
		if constexpr (std::is_same_v<ColorType, float>)
			return data.data();
		else
			return &data[0].r;
	};

	inline float* getFirstFloatPtr(std::pair<std::vector<float>, int>& data)
	{
		return data.first.data();
	}

	template<typename Variant>
	inline float* getFirstFloatPtrFromVariant(Variant& variant)
	{
		return std::visit([&](auto& data) { return getFirstFloatPtr(data); }, variant);
	}

	template<typename Variant>
	inline int getNumOfChannelsFromVariant(Variant& variant)
	{
		if (std::holds_alternative<std::pair<std::vector<float>, int>>(variant))
			return std::get<std::pair<std::vector<float>, int>>(variant).second;
		else if (std::holds_alternative<std::vector<glm::vec2>>(variant))
			return 2;
		else if (std::holds_alternative<std::vector<glm::vec3>>(variant))
			return 3;
		else if (std::holds_alternative<std::vector<glm::vec4>>(variant))
			return 4;

		assert(!"unsupported variant type");
		throw std::runtime_error("Unsupported variant type.");
	}
}

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
		updateStaticTextures();
		updateDynamicTextures();
	}

	void Textures::step()
	{
		updateDynamicTextures();
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture, bool initial)
	{
		class DataSourceVisitor
		{
		public:
			DataSourceVisitor(std::unordered_map<std::string, TextureCache>& keysToTexturesCache, std::vector<float>& sourceFragmentBuffer, Components::Texture& texture):
				keysToTexturesCache(keysToTexturesCache),
				sourceFragmentBuffer(sourceFragmentBuffer),
				texture(texture)
			{
			}

			void operator()(const std::string& path)
			{
				const glm::ivec2 prevSize = texture.loaded.size;
				const int prevNumOfChannels = texture.loaded.numOfChannels;
				auto& textureCache = fileLoading(path, texture.convertToPremultipliedAlpha, texture.additionalConversion);

				texture.loaded.size = textureCache.size;
				texture.loaded.numOfChannels = textureCache.numOfChannels;

				texImage2D(textureCache.data.get(), prevSize, prevNumOfChannels);
			}

			void operator()(TextureData& textureData)
			{
				const glm::ivec2 prevSize = texture.loaded.size;
				const int prevNumOfChannels = texture.loaded.numOfChannels;

				if (textureData.toBeLoaded.path.empty())
				{
					texture.loaded.numOfChannels = getNumOfChannelsFromVariant(textureData.loaded.data);
					optionalAlphaProcessing(getFirstFloatPtrFromVariant(textureData.loaded.data), textureData.loaded.size, texture.loaded.numOfChannels,
						texture.convertToPremultipliedAlpha, texture.additionalConversion);
				}
				else
				{
					auto& textureCache = fileLoading(textureData.toBeLoaded.path, textureData.toBeLoaded.convertToPremultipliedAlpha, textureData.toBeLoaded.additionalConversion);
					textureData.loaded.size = textureCache.size;
					texture.loaded.numOfChannels = textureCache.numOfChannels;

					switch (textureCache.numOfChannels)
					{
					case 1:
						textureData.loaded.data = std::make_pair(std::vector<float>(textureCache.size.x * textureCache.size.y), 1);
						break;
					case 2:
						textureData.loaded.data = std::vector<glm::vec2>(textureCache.size.x * textureCache.size.y);
						break;
					case 3:
						textureData.loaded.data = std::vector<glm::vec3>(textureCache.size.x * textureCache.size.y);
						break;
					case 4:
						textureData.loaded.data = std::vector<glm::vec4>(textureCache.size.x * textureCache.size.y);
						break;
					default:
						assert(!"unsupported number of channels");
						throw std::runtime_error("Unsupported number of channels in texture \"" + textureData.toBeLoaded.path + "\".");
					}

					std::copy(textureCache.data.get(), textureCache.data.get() + textureCache.numOfChannels * textureCache.size.x * textureCache.size.y,
						getFirstFloatPtrFromVariant(textureData.loaded.data));
					textureData.toBeLoaded = {};
				}

				texture.loaded.size = textureData.loaded.size;

				texImage2D(getFirstFloatPtrFromVariant(textureData.loaded.data), prevSize, prevNumOfChannels);
			}

		private:
			TextureCache& fileLoading(const std::string& path, bool convertToPremultipliedAlpha, TextureData::AdditionalConversion additionalConversion)
			{
				const std::string cacheKey = path + std::to_string(texture.desiredFileChannels) + std::to_string((int)convertToPremultipliedAlpha) + std::to_string((int)additionalConversion);
				auto& textureCache = keysToTexturesCache[cacheKey];

				if (!textureCache.data)
				{
					textureCache.data.reset(stbi_loadf(path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.numOfChannels, texture.desiredFileChannels));
					if (!textureCache.data)
					{
						assert(!"unable to load image");
						throw std::runtime_error("Unable to load image \"" + path + "\".");
					}
					if (texture.desiredFileChannels)
						textureCache.numOfChannels = texture.desiredFileChannels;

					optionalAlphaProcessing(textureCache.data.get(), textureCache.size, textureCache.numOfChannels,
						convertToPremultipliedAlpha, additionalConversion);
				}

				return textureCache;
			}

			void optionalAlphaProcessing(float* data, glm::vec2 size, int numOfChannels, bool convertToPremultipliedAlpha, TextureData::AdditionalConversion additionalConversion) const
			{
				const bool convertDarkToTransparent = additionalConversion == TextureData::AdditionalConversion::DarkToTransparent;
				const bool convertTransparentToDark = additionalConversion == TextureData::AdditionalConversion::TrasparentToDark;

				if (numOfChannels == 4 && (convertToPremultipliedAlpha || (bool)additionalConversion))
				{
					for (int i = 0; i < size.x * size.y * 4; i += 4)
					{
						float& alpha = data[i + 3];
						alpha = (1 - convertDarkToTransparent) * alpha + convertDarkToTransparent * std::min(1.0f, data[i] + data[i + 1] + data[i + 2]);
						const float premultipliedAlphaFactor = (1 - convertToPremultipliedAlpha) + convertToPremultipliedAlpha * alpha;
						for (int j = 0; j < 3; ++j)
						{
							float& color = data[i + j];
							color *= premultipliedAlphaFactor * (1 - convertTransparentToDark * color);
						}
					}
				}
			}

			void texImage2D(const float* data, glm::ivec2 prevSize, int prevNumOfChannels) const
			{
				const auto& [finalData, finalSize] = !texture.sourceFragmentCornerAndSizeF
					? std::make_pair(data, texture.loaded.size)
					: [&]() {
						const auto [fragmentCorner, fragmentSize] = texture.sourceFragmentCornerAndSizeF(texture.loaded.size);
						const size_t rowSize = fragmentSize.x * texture.loaded.numOfChannels * sizeof(float);
						const size_t totalSize = fragmentSize.y * fragmentSize.x * texture.loaded.numOfChannels;

						sourceFragmentBuffer.resize(totalSize);

						auto copyRow = [&](const auto y) {
							const float* rowStart = data + ((fragmentCorner.y + y) * texture.loaded.size.x + fragmentCorner.x) * texture.loaded.numOfChannels;
							float* destStart = sourceFragmentBuffer.data() + y * fragmentSize.x * texture.loaded.numOfChannels;
							std::memcpy(destStart, rowStart, rowSize);
						};

						if constexpr (parallelProcessing)
						{
							Tools::ItToId itToId(fragmentSize.y);
							std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), copyRow);
						}
						else
							for (int y = 0; y < fragmentSize.y; ++y)
								copyRow(y);

						texture.loaded.size = fragmentSize;

						return std::make_pair(sourceFragmentBuffer.data(), fragmentSize);
					}();
				if (prevSize != texture.loaded.size || prevNumOfChannels != texture.loaded.numOfChannels)
					glTexImage2D(GL_TEXTURE_2D, 0, texture.loaded.getFormat(), finalSize.x, finalSize.y, 0, texture.loaded.getFormat(), GL_FLOAT, finalData);
				else
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, finalSize.x, finalSize.y, texture.loaded.getFormat(), GL_FLOAT, finalData);
			}

			std::unordered_map<std::string, TextureCache>& keysToTexturesCache;
			std::vector<float>& sourceFragmentBuffer;
			Components::Texture& texture;
		};

		glActiveTexture(texture.loaded.textureUnit);
		if (initial)
			glGenTextures(1, &texture.loaded.textureObject);
		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		std::visit(DataSourceVisitor{ keysToTexturesCache, sourceFragmentBuffer, texture }, texture.dataSource);

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

		auto createTextureFramebuffer = [this](Components::Framebuffers::SubBuffers& subBuffers, GLint textureMagFilter) {
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

	void Textures::updateStaticTextures()
	{
		for (auto& texture : Globals::Components().staticTextures())
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

	void Textures::updateDynamicTextures()
	{
		for (auto& texture: Globals::Components().dynamicTextures())
		{
			if (texture.state == ComponentState::Ongoing)
				continue;
			else if (texture.state == ComponentState::Changed)
			{
				if (texture.loaded.textureUnit == 0)
				{
					texture.loaded.textureUnit = textureUnits.acquire();
					loadAndConfigureTexture(texture);
				}
				else
					loadAndConfigureTexture(texture, false);
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
