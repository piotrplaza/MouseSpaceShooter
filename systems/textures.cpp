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
#include <ranges>

namespace
{
	constexpr bool parallelProcessing = true;

	template<typename ColorType>
	inline const float* getFirstFloatPtr(const std::vector<ColorType>& data)
	{
		assert(!data.empty());
		if (data.empty())
			return nullptr;
		if constexpr (std::is_same_v<ColorType, float>)
			return data.data();
		else
			return &data[0].r;
	};

	inline const float* getFirstFloatPtr(const std::pair<std::vector<float>, int>& data)
	{
		assert(!data.first.empty());
		return data.first.data();
	}

	inline const float* getFirstFloatPtr(const std::pair<float*, int>& data)
	{
		assert(!data.first);
		return data.first;
	}

	inline const float* getFirstFloatPtr(std::monostate)
	{
		assert(!"uninitialised textureData.loaded.data");
		throw std::runtime_error("getFirstFloatPtr - uninitialised textureData.loaded.data.");
	}

	template<typename Variant>
	inline const float* getFirstFloatPtrFromVariant(const Variant& variant)
	{
		return std::visit([&](auto& data) { return getFirstFloatPtr(data); }, variant);
	}

	template<typename Variant>
	inline float* getFirstFloatPtrFromVariant(Variant& variant)
	{
		return const_cast<float*>(getFirstFloatPtrFromVariant(static_cast<const Variant&>(variant)));
	}

	template<typename Variant>
	inline int getNumOfChannelsFromVariant(const Variant& variant)
	{
		if (std::holds_alternative<std::pair<float*, int>>(variant))
			return std::get<std::pair<float*, int>>(variant).second;
		else if (std::holds_alternative<std::pair<std::vector<float>, int>>(variant))
			return std::get<std::pair<std::vector<float>, int>>(variant).second;
		else if (std::holds_alternative<std::vector<glm::vec2>>(variant))
			return 2;
		else if (std::holds_alternative<std::vector<glm::vec3>>(variant))
			return 3;
		else if (std::holds_alternative<std::vector<glm::vec4>>(variant))
			return 4;

		assert(!"unsupported variant type");
		throw std::runtime_error("getNumOfChannelsFromVariant - unsupported variant type.");
	}

	template<typename Variant>
	inline int getFormatFromVariant(const Variant& variant)
	{
		switch (getNumOfChannelsFromVariant(variant))
		{
		case 1: return GL_RED;
		case 2: return GL_RG;
		case 3: return GL_RGB;
		case 4: return GL_RGBA;
		default: assert(!"unsupported number of channels"); return 0;
		}
	}

	template<typename Variant>
	inline const TextureData& getTextureDataFromVariant(const Variant& variant)
	{
		if (std::holds_alternative<TextureData>(variant))
			return std::get<TextureData>(variant);
		else if (std::holds_alternative<TextureData*>(variant))
			return *std::get<TextureData*>(variant);

		assert(!"unsupported variant type");
		throw std::runtime_error("getTextureDataFromVariant - unsupported variant type.");
	}

	template<typename Variant>
	inline TextureData& getTextureDataFromVariant(Variant& variant)
	{
		return const_cast<TextureData&>(getTextureDataFromVariant(static_cast<const Variant&>(variant)));
	}

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

	std::tuple<const float*, glm::ivec2, glm::ivec2> getClippedTextureSubData(
		const float* textureSubData,
		const glm::ivec2& textureSubSize,
		const glm::ivec2& offsetPos,
		const glm::ivec2& targetTextureSize,
		const int numOfChannels,
		std::vector<float>& operationalBuffer)
	{
		const int startX = offsetPos.x;
		const int startY = offsetPos.y;
		const int endX = startX + textureSubSize.x;
		const int endY = startY + textureSubSize.y;

		const bool withinBounds = (startX >= 0) && (startY >= 0) && (endX <= targetTextureSize.x) && (endY <= targetTextureSize.y);

		if (withinBounds)
			return std::make_tuple(textureSubData, textureSubSize, offsetPos);
		else
		{
			if (startX >= targetTextureSize.x || startY >= targetTextureSize.y || endX <= 0 || endY <= 0)
				return std::make_tuple(nullptr, glm::ivec2(0), glm::ivec2(0));

			const int clippedStartX = std::max(0, startX);
			const int clippedStartY = std::max(0, startY);
			const int clippedEndX = std::min(targetTextureSize.x, endX);
			const int clippedEndY = std::min(targetTextureSize.y, endY);

			const glm::ivec2 clippedSize(clippedEndX - clippedStartX, clippedEndY - clippedStartY);
			const glm::ivec2 newOffset(std::max(0, startX), std::max(0, startY));

			const int rowSize = clippedSize.x * numOfChannels * sizeof(float);
			const int sourceX = clippedStartX - startX;

			operationalBuffer.resize(clippedSize.x * clippedSize.y * numOfChannels);

			auto processRow = [&](const auto y) {
				const int sourceY = y - startY;
				const int destIndex = (y - clippedStartY) * clippedSize.x * numOfChannels;
				const int sourceIndex = (sourceY * textureSubSize.x + sourceX) * numOfChannels;

				std::memcpy(&operationalBuffer[destIndex], &textureSubData[sourceIndex], rowSize);
			};

			if constexpr (parallelProcessing && 1)
			{
				Tools::ItToId itToId(clippedStartY, clippedEndY);
				std::for_each(std::execution::par_unseq, itToId.begin(), itToId.end(), processRow);
			}
			else
				for (int y = clippedStartY; y < clippedEndY; ++y)
					processRow(y);

			return std::make_tuple(operationalBuffer.data(), clippedSize, newOffset);
		}
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

			optionalAlphaProcessing(textureCache.data.get(), textureCache.size, textureCache.numOfChannels,
				file.convertToPremultipliedAlpha, file.additionalConversion);

			if (file.desiredChannels)
				changeNumOfChannels(textureCache, file.desiredChannels);
		}

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

				texImage2D(textureCache.data.get(), prevSize, prevNumOfChannels);
			}

			void operator()(TextureData& textureData)
			{
				const glm::ivec2 prevSize = texture.loaded.size;
				const int prevNumOfChannels = texture.loaded.numOfChannels;

				if (textureData.file.path.empty())
					texture.loaded.numOfChannels = getNumOfChannelsFromVariant(textureData.loaded.data);
				else
					texture.loaded.numOfChannels = textures.textureDataFromFile(textureData).numOfChannels;

				texture.loaded.size = textureData.loaded.size;

				texImage2D(getFirstFloatPtrFromVariant(textureData.loaded.data), prevSize, prevNumOfChannels);
			}

			void operator ()(std::monostate) const
			{
				assert(!"uninitialised textureData");
				throw std::runtime_error("Uninitialised textureData.");
			}

		private:
			void texImage2D(const float* data, glm::ivec2 prevSize, int prevNumOfChannels) const
			{
				auto* const subImages = texture.subImagesF ? &texture.subImagesF() : nullptr;

				if (!subImages || !subImages->exclusiveLoad)
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

				if (subImages)
				{
					int i = 0;
					for (auto& [textureSubDataVariant, offsetPos] : subImages->textureSubData)
					{
						auto& textureSubData = getTextureDataFromVariant(textureSubDataVariant);

						if (!textureSubData.file.path.empty())
							textures.textureDataFromFile(textureSubData);

						assert(texture.loaded.numOfChannels == getNumOfChannelsFromVariant(textureSubData.loaded.data));

						if (subImages->deferredOffsetPosF)
							offsetPos = subImages->deferredOffsetPosF(textureSubData.loaded.size, i++);

						const auto& [clippedTextureSubData, clippedSize, clippedOffsetPos] =
							getClippedTextureSubData(getFirstFloatPtrFromVariant(textureSubData.loaded.data), textureSubData.loaded.size, offsetPos,
								texture.loaded.size, getNumOfChannelsFromVariant(textureSubData.loaded.data), textures.operationalBuffer);
						if (clippedTextureSubData)
							glTexSubImage2D(GL_TEXTURE_2D, 0, clippedOffsetPos.x, clippedOffsetPos.y, clippedSize.x, clippedSize.y, getFormatFromVariant(textureSubData.loaded.data), GL_FLOAT, clippedTextureSubData);
					}

					subImages->deferredOffsetPosF = nullptr;
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

		createTextureFramebuffer(framebuffers.main, GL_LINEAR);
		createTextureFramebuffer(framebuffers.normalLinearBlend0, GL_LINEAR);
		createTextureFramebuffer(framebuffers.normalLinearBlend1, GL_LINEAR);
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
				getFirstFloatPtrFromVariant(textureData.loaded.data));
		}
		textureData.file = {};

		return textureCache;
	}
}
