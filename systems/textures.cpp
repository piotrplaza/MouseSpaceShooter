#include "textures.hpp"

#include <components/texture.hpp>
#include <components/framebuffers.hpp>

#include <globals/components.hpp>

#include <tools/utility.hpp>
#include <tools/buffersHelpers.hpp>

#include <ogl/oglProxy.hpp>
#include <ogl/oglHelpers.hpp>

#include <stb_image/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <stdexcept>
#include <execution>
#include <ranges>

namespace
{
	constexpr bool parallelProcessing = true;

	void optionalAlphaProcessing(float* data, glm::ivec2 size, int numOfChannels, bool convertToPremultipliedAlpha, TextureFile::AdditionalConversion additionalConversion)
	{
		if (numOfChannels < 4 || (!convertToPremultipliedAlpha && !(bool)additionalConversion))
			return;

		const bool convertDarkToTransparent = additionalConversion == TextureFile::AdditionalConversion::DarkToTransparent;
		const bool convertTransparentToDark = additionalConversion == TextureFile::AdditionalConversion::TransparentToDark;

		auto processRow = [&](const auto y) {
			for (int x = 0; x < size.x; ++x)
			{
				const int i = (y * size.x + x) * 4;
				float& alpha = data[i + 3];
				alpha = (1 - convertDarkToTransparent) * alpha + convertDarkToTransparent * std::min(1.0f, data[i] + data[i + 1] + data[i + 2]);
				const float premultipliedAlphaFactor = (1 - convertToPremultipliedAlpha) + convertToPremultipliedAlpha * alpha;
				for (int j = 0; j < 3; ++j)
				{
					float& color = data[i + j];
					color *= premultipliedAlphaFactor * (1 - convertTransparentToDark * (1 - alpha));
				}
			}
		};

		if constexpr (parallelProcessing && 1)
		{
			Tools::ItToId itToId(size.y);
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
		}
		else
			for (int y = 0; y < size.y; ++y)
				processRow(y);
	}

	void changeNumOfChannels(Systems::Textures::TextureCache& textureCache, int newNumOfChannels)
	{
		if (textureCache.numOfChannels == newNumOfChannels)
			return;

		const glm::vec4 defaultColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		const int width = textureCache.size.x;
		const int height = textureCache.size.y;
		const int minChannels = std::min(textureCache.numOfChannels, newNumOfChannels);
		auto newData = std::make_unique<float[]>(width * height * newNumOfChannels);

		auto processRow = [&](const auto y) {
			for (int x = 0; x < width; ++x)
			{
				const int oldIndex = (y * width + x) * textureCache.numOfChannels;
				const int newIndex = (y * width + x) * newNumOfChannels;
				for (int c = 0; c < minChannels; ++c)
					newData[newIndex + c] = textureCache.data[oldIndex + c];
			}
		};

		if constexpr (parallelProcessing && 1)
		{
			Tools::ItToId itToId(height);
			std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
		}
		else
			for (int y = 0; y < height; ++y)
				processRow(y);

		if (newNumOfChannels > textureCache.numOfChannels)
		{
			auto processRow = [&](const auto y) {
				for (int x = 0; x < width; ++x)
				{
					const int newIndex = (y * width + x) * newNumOfChannels;
					for (int c = minChannels; c < newNumOfChannels; ++c)
						newData[newIndex + c] = defaultColor[c];
				}
			};

			if constexpr (parallelProcessing && 1)
			{
				Tools::ItToId itToId(height);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
			}
			else
				for (int y = 0; y < height; ++y)
					processRow(y);
		}

		textureCache.data = std::move(newData);
		textureCache.numOfChannels = newNumOfChannels;
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

	const Textures::TextureCache& Textures::loadFile(const TextureFile& file)
	{
		auto& textureCache = keysToTexturesCache[file.path + std::to_string(file.desiredChannels) + std::to_string((int)file.convertToPremultipliedAlpha) + std::to_string((int)file.additionalConversion)];

		if (!textureCache.data)
		{
			textureCache.data.reset(stbi_loadf(file.path.c_str(), &textureCache.size.x, &textureCache.size.y, &textureCache.numOfChannels, 0));
			if (!textureCache.data)
			{
				assert(!"unable to load image");
				throw std::runtime_error("Unable to load image \"" + file.path + "\".");
			}

			if (file.desiredChannels)
				changeNumOfChannels(textureCache, file.desiredChannels);

			if (file.customConversionF)
				file.customConversionF(textureCache.data.get(), textureCache.size, textureCache.numOfChannels);

			optionalAlphaProcessing(textureCache.data.get(), textureCache.size, textureCache.numOfChannels,
				file.convertToPremultipliedAlpha, file.additionalConversion);
		}

		return textureCache;
	}

	const Textures::TextureCache& Textures::textureDataFromFile(TextureData& textureData)
	{
		const auto& textureCache = loadFile(textureData.file);
		textureData.loaded.size = textureCache.size;

		if (std::holds_alternative<std::pair<float*, int>>(textureData.loaded.data))
			textureData.loaded.data = std::make_pair(textureCache.data.get(), textureCache.numOfChannels);
		else
		{
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
				throw std::runtime_error("Unsupported number of channels in texture \"" + textureData.file.path + "\".");
			}

			std::copy(textureCache.data.get(), textureCache.data.get() + textureCache.numOfChannels * textureCache.size.x * textureCache.size.y,
				textureData.getRawData());
		}
		textureData.file = {};

		return textureCache;
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture, bool initial)
	{
		class DataSourceVisitor
		{
		public:
			DataSourceVisitor(Textures& textures, Components::Texture& texture):
				textures(textures),
				texture(texture)
			{
			}

			void operator()(const TextureFile& file)
			{
				const glm::ivec2 prevSize = texture.loaded.size;
				const int prevNumOfChannels = texture.loaded.numOfChannels;
				auto& textureCache = textures.loadFile(file);

				texture.loaded.size = textureCache.size;
				texture.loaded.numOfChannels = textureCache.numOfChannels;

				applyTexture(textureCache.data.get(), prevSize, prevNumOfChannels);
			}

			void operator()(TextureData& textureData)
			{
				const glm::ivec2 prevSize = texture.loaded.size;
				const int prevNumOfChannels = texture.loaded.numOfChannels;

				if (textureData.file.path.empty())
					texture.loaded.numOfChannels = textureData.getNumOfChannels();
				else
					texture.loaded.numOfChannels = textures.textureDataFromFile(textureData).numOfChannels;

				texture.loaded.size = textureData.loaded.size;

				applyTexture(textureData.getRawData(), prevSize, prevNumOfChannels);
			}

			void operator ()(std::monostate) const
			{
				assert(!"uninitialised textureData");
				throw std::runtime_error("Uninitialised textureData.");
			}

		private:
			void applyTexture(const float* data, glm::ivec2 prevSize, int prevNumOfChannels) const
			{
				std::optional<TextureSubData> subData = texture.subImagesF
					? std::optional<TextureSubData>(texture.subImagesF())
					: std::nullopt;

				if (!subData || !subData->exclusiveLoad)
				{
					const auto& finalData = !texture.sourceFragmentCornerAndSizeF
						? data
						: [&]() {
							const auto [fragmentCorner, fragmentSize] = texture.sourceFragmentCornerAndSizeF(texture.loaded.size);
							const size_t rowSize = fragmentSize.x * texture.loaded.numOfChannels * sizeof(float);
							const size_t totalSize = fragmentSize.y * fragmentSize.x * texture.loaded.numOfChannels;

							textures.operationalBuffer.resize(totalSize);

							auto copyRow = [&](const auto y) {
								const float* rowStart = data + ((fragmentCorner.y + y) * texture.loaded.size.x + fragmentCorner.x) * texture.loaded.numOfChannels;
								float* destStart = textures.operationalBuffer.data() + y * fragmentSize.x * texture.loaded.numOfChannels;
								std::memcpy(destStart, rowStart, rowSize);
								};

							if constexpr (parallelProcessing && 1)
							{
								Tools::ItToId itToId(fragmentSize.y);
								std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), copyRow);
							}
							else
								for (int y = 0; y < fragmentSize.y; ++y)
									copyRow(y);

							texture.loaded.size = fragmentSize;

							return textures.operationalBuffer.data();
						}();
					if (prevSize != texture.loaded.size || prevNumOfChannels != texture.loaded.numOfChannels)
						glTexImage2D(GL_TEXTURE_2D, 0, texture.loaded.getFormat(), texture.loaded.size.x, texture.loaded.size.y, 0, texture.loaded.getFormat(), GL_FLOAT, finalData);
					else
						glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.loaded.size.x, texture.loaded.size.y, texture.loaded.getFormat(), GL_FLOAT, finalData);
				}

				if (subData)
				{
					int i = 0;
					for (auto& [imageDataVariant, offsetPos] : subData->imagesData)
					{
						auto& imageData = TextureSubData::GetTextureDataFromVariant(imageDataVariant);

						if (!imageData.file.path.empty())
							textures.textureDataFromFile(imageData);

						const int imageNumOfChannels = imageData.getNumOfChannels();

						assert(texture.loaded.numOfChannels == imageNumOfChannels);

						if (subData->deferredOffsetPosF)
							offsetPos = subData->deferredOffsetPosF(imageData.loaded.size, texture.loaded.size, i++);

						const auto subImageData =
							Tools::ClipSubImage(imageData.getRawData(), imageData.loaded.size, offsetPos,
								texture.loaded.size, imageNumOfChannels, textures.operationalBuffer);

						if (subImageData.data)
						{
							if (subData->updateCallbackF)
								subData->updateCallbackF(subImageData.data, subImageData.size, subImageData.offset, imageNumOfChannels);
							glTexSubImage2D(GL_TEXTURE_2D, 0, subImageData.offset.x, subImageData.offset.y, subImageData.size.x, subImageData.size.y,
								Tools::GetPixelFormat(imageData.getNumOfChannels()), GL_FLOAT, subImageData.data);
						}
					}

					if (!subData->permanent)
						texture.subImagesF = nullptr;
				}
			}

			Textures& textures;
			Components::Texture& texture;
		};

		glActiveTexture(texture.loaded.textureUnit);
		if (initial)
			glGenTextures(1, &texture.loaded.textureObject);
		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		std::visit(DataSourceVisitor{ *this, texture }, texture.source);

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

		for (size_t res = 0; res < (size_t)ResolutionMode::Resolution::COUNT; ++res)
			for (size_t scaling = 0; scaling < (size_t)ResolutionMode::Scaling::COUNT; ++scaling)
				for (size_t blending = 0; blending < (size_t)ResolutionMode::Blending::COUNT; ++blending)
					createTextureFramebuffer(framebuffers.subBuffers[res][scaling][blending], (ResolutionMode::Scaling)scaling == ResolutionMode::Scaling::Nearest ? GL_NEAREST : GL_LINEAR);
	}

	void Textures::updateStaticTextures()
	{
		for (auto& texture : Globals::Components().staticTextures().underlyingContainer() | std::views::drop(staticTexturesOffset))
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
