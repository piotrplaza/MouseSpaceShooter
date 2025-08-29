#include "textures.hpp"

#include <components/texture.hpp>
#include <components/renderTexture.hpp>
#include <components/renderTexturesMapper.hpp>
#include <components/systemInfo.hpp>

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

		auto processRow = [&](const auto y_) {
			const int y = (int)y_;
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

		auto processRow = [&](const auto y_) {
			const int y = (int)y_;
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
			auto processRow = [&](const auto y_) {
				const int y = (int)y_;
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

		createAndConfigureStandardRenderTextures();
	}

	void Textures::postInit()
	{
		updateStaticTextures();
		updateStaticRenderTextures();
		updateDynamicTextures();
		updateDynamicRenderTextures();
	}

	void Textures::step()
	{
		for (auto& texture : Globals::Components().staticTextures())
			texture.step();

		for (auto& renderTexture : Globals::Components().staticRenderTextures())
			renderTexture.step();

		for (auto& texture : Globals::Components().textures())
			texture.step();

		for (auto& renderTexture : Globals::Components().renderTextures())
			renderTexture.step();

		updateDynamicTextures();
		updateDynamicRenderTextures();
	}

	void Textures::updateStaticTextures()
	{
		for (auto& texture : Globals::Components().staticTextures().underlyingContainer() | std::views::drop(staticTexturesOffset))
			updateTexture(texture);
		staticTexturesOffset = Globals::Components().staticTextures().size();
	}

	void Textures::updateStaticRenderTextures()
	{
		for (auto& renderTexture : Globals::Components().staticRenderTextures().underlyingContainer() | std::views::drop(staticRenderTexturesOffset))
			updateRenderTexture(renderTexture);
		staticRenderTexturesOffset = Globals::Components().staticRenderTextures().size();
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


	void Textures::updateDynamicTextures()
	{
		for (auto& texture : Globals::Components().textures())
			updateTexture(texture);
	}

	void Textures::updateTexture(Components::Texture& texture)
	{
		if (texture.state == ComponentState::Ongoing)
			return;
		if (texture.state == ComponentState::Changed)
		{
			loadAndConfigureTexture(texture);
			texture.state = ComponentState::Ongoing;
			return;
		}
		if (texture.state == ComponentState::Outdated)
		{
			deleteTexture(texture);
			return;
		}
		if (texture.state == ComponentState::LastShot)
		{
			loadAndConfigureTexture(texture);
			texture.teardownF = [&, prevTeardownF = std::move(texture.teardownF)]() {
				if (prevTeardownF)
					prevTeardownF();
				deleteTexture(texture);
				};
			return;
		}
		assert(!"unsupported texture state");
	}

	void Textures::deleteTexture(Components::Texture& texture)
	{
		glDeleteTextures(1, &texture.loaded.textureObject);
		texture.loaded.textureObject = 0;
	}

	void Textures::loadAndConfigureTexture(Components::Texture& texture)
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

		if (texture.loaded.textureObject == 0)
		{
			const auto& limits = Globals::Components().systemInfo().limits;
			glGenTextures(1, &texture.loaded.textureObject);
			assert(texture.loaded.textureObject);
			if (!texture.loaded.textureObject)
				throw std::runtime_error("Unable to create texture unit.");
		}

		glBindTexture(GL_TEXTURE_2D, texture.loaded.textureObject);

		std::visit(DataSourceVisitor{ *this, texture }, texture.source);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.magFilter);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &texture.borderColor[0]);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (texture.minFilter == GL_LINEAR_MIPMAP_LINEAR ||
			texture.minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			texture.minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			texture.minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Textures::createAndConfigureStandardRenderTextures()
	{
		assert(Globals::Components().staticRenderTextures().empty());

		auto createStandardRenderTexture = [](const StandardRenderMode& standardRenderMode) {
			const auto glScaling = standardRenderMode.scaling == StandardRenderMode::Scaling::Linear ? GL_LINEAR : GL_NEAREST;

			auto& renderTexture = Globals::Components().staticRenderTextures().emplace(standardRenderMode, GL_CLAMP_TO_EDGE, GL_NEAREST, glScaling);
			Globals::Components().renderTexturesMapper().renderTextureIds[(size_t)standardRenderMode.resolution][(size_t)standardRenderMode.scaling][(size_t)standardRenderMode.blending] = renderTexture.getComponentId();

			unsigned textureObject;
			glGenTextures(1, &textureObject);
			glBindTexture(GL_TEXTURE_2D, textureObject);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, renderTexture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, renderTexture.wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, renderTexture.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, renderTexture.magFilter);
			renderTexture.loaded.textureObject = textureObject;

			glGenFramebuffers(1, &renderTexture.loaded.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.loaded.fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture.loaded.textureObject, 0);

			glGenRenderbuffers(1, &renderTexture.loaded.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderTexture.loaded.depthBuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTexture.loaded.depthBuffer);
		};

		for (size_t res = 0; res < (size_t)StandardRenderMode::Resolution::COUNT; ++res)
			for (size_t scaling = 0; scaling < (size_t)StandardRenderMode::Scaling::COUNT; ++scaling)
				for (size_t blending = 0; blending < (size_t)StandardRenderMode::Blending::COUNT; ++blending)
					createStandardRenderTexture({ (StandardRenderMode::Resolution)res, (StandardRenderMode::Scaling)scaling, (StandardRenderMode::Blending)blending });

		staticRenderTexturesOffset = Globals::Components().staticRenderTextures().size();
	}

	void Textures::updateDynamicRenderTextures()
	{
		for (auto& renderTexture : Globals::Components().renderTextures())
			updateRenderTexture(renderTexture);
	}

	void Textures::updateRenderTexture(Components::RenderTexture& renderTexture)
	{
		if (renderTexture.state == ComponentState::Ongoing)
			return;
		if (renderTexture.state == ComponentState::Changed)
		{
			configureRenderTexture(renderTexture);
			renderTexture.state = ComponentState::Ongoing;
			return;
		}
		if (renderTexture.state == ComponentState::Outdated)
		{
			deleteRenderTexture(renderTexture);
			return;
		}
		if (renderTexture.state == ComponentState::LastShot)
		{
			configureRenderTexture(renderTexture);
			renderTexture.teardownF = [&, prevTeardownF = std::move(renderTexture.teardownF)]() {
				if (prevTeardownF)
					prevTeardownF();
				deleteRenderTexture(renderTexture);
			};
			return;
		}
		assert(!"unsupported texture state");
	}

	void Textures::deleteRenderTexture(Components::RenderTexture& renderTexture)
	{
		glDeleteFramebuffers(1, &renderTexture.loaded.fbo);
		renderTexture.loaded.fbo = 0;
		glDeleteRenderbuffers(1, &renderTexture.loaded.depthBuffer);
		renderTexture.loaded.depthBuffer = 0;
		glDeleteTextures(1, &renderTexture.loaded.textureObject);
		renderTexture.loaded.textureObject = 0;
	}

	void Textures::configureRenderTexture(Components::RenderTexture& renderTexture)
	{
		if (renderTexture.stepF)
			renderTexture.stepF();
		assert(renderTexture.size);

		unsigned textureObject;
		glGenTextures(1, &textureObject);
		glBindTexture(GL_TEXTURE_2D, textureObject);
		renderTexture.loaded.textureObject = textureObject;

		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &renderTexture.borderColor[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, renderTexture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, renderTexture.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, renderTexture.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, renderTexture.magFilter);

		glGenFramebuffers(1, &renderTexture.loaded.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.loaded.fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture.loaded.textureObject, 0);

		glGenRenderbuffers(1, &renderTexture.loaded.depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderTexture.loaded.depthBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTexture.loaded.depthBuffer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTexture.size->x, renderTexture.size->y, 0, GL_RGBA, GL_FLOAT, nullptr);

		glBindRenderbuffer(GL_RENDERBUFFER, renderTexture.loaded.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTexture.size->x, renderTexture.size->y);

		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		renderTexture.loaded.size = *renderTexture.size;
		renderTexture.loaded.numOfChannels = 4;
	}
}
