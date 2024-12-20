#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <variant>
#include <cassert>
#include <stdexcept>

struct TextureFile
{
	enum class AdditionalConversion { None, DarkToTransparent, TransparentToDark };

	TextureFile() = default;

	TextureFile(std::string path, int desiredChannels = 0, bool convertToPremultipliedAlpha = true, AdditionalConversion additionalConversion = AdditionalConversion::None,
		std::function<void(float* data, glm::ivec2 size, int numOfChannels)> customConversionF = nullptr) :
		path{ std::move(path) },
		desiredChannels{ desiredChannels },
		convertToPremultipliedAlpha{ convertToPremultipliedAlpha },
		additionalConversion{ additionalConversion },
		customConversionF{ std::move(customConversionF) }
	{
	}

	std::string path;
	int desiredChannels{};
	bool convertToPremultipliedAlpha{};
	AdditionalConversion additionalConversion{};
	std::function<void(float* data, glm::ivec2 size, int numOfChannels)> customConversionF;
};

struct TextureData
{
	TextureData() = default;

	template<typename ColorType>
	TextureData(std::vector<ColorType> data, glm::ivec2 size) :
		loaded{ std::move(data), size }
	{
	}

	TextureData(std::vector<float> data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(std::move(data), numOfChannels), size }
	{
	}

	TextureData(const float* data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(data, numOfChannels), size }
	{
	}

	TextureData(float* data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(data, numOfChannels), size }
	{
	}

	TextureData(TextureFile file) :
		file{ std::move(file) }
	{
	}

	const float* getRawData() const
	{
		return std::visit([&](auto& data) { return getRawData(data); }, loaded.data);
	}

	float* getRawData()
	{
		// TODO: Could be done better.
		return const_cast<float*>(std::as_const(*this).getRawData());
	}

	int getNumOfChannels()
	{
		if (std::holds_alternative<std::pair<const float*, int>>(loaded.data))
			return std::get<std::pair<const float*, int>>(loaded.data).second;
		else if (std::holds_alternative<std::pair<float*, int>>(loaded.data))
			return std::get<std::pair<float*, int>>(loaded.data).second;
		else if (std::holds_alternative<std::pair<std::vector<float>, int>>(loaded.data))
			return std::get<std::pair<std::vector<float>, int>>(loaded.data).second;
		else if (std::holds_alternative<std::vector<glm::vec2>>(loaded.data))
			return 2;
		else if (std::holds_alternative<std::vector<glm::vec3>>(loaded.data))
			return 3;
		else if (std::holds_alternative<std::vector<glm::vec4>>(loaded.data))
			return 4;

		assert(!"unsupported variant type");
		throw std::runtime_error("getNumOfChannels - unsupported variant type.");
	}

	TextureFile file;

	struct
	{
		std::variant<std::monostate, std::pair<const float*, int>, std::pair<float*, int>, std::pair<std::vector<float>, int>, std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>> data;
		glm::ivec2 size = { 0, 0 };
	} loaded;

private:
	template<typename ColorType>
	const float* getRawData(const std::vector<ColorType>& data) const
	{
		assert(!data.empty());
		if (data.empty())
			return nullptr;
		if constexpr (std::is_same_v<ColorType, float>)
			return data.data();
		else
			return &data[0].r;
	};

	const float* getRawData(const std::pair<std::vector<float>, int>& data) const
	{
		assert(!data.first.empty());
		return data.first.data();
	}

	const float* getRawData(const std::pair<const float*, int>& data) const
	{
		assert(data.first);
		return data.first;
	}

	const float* getRawData(const std::pair<float*, int>& data) const
	{
		assert(data.first);
		return data.first;
	}

	const float* getRawData(std::monostate) const
	{
		assert(!"uninitialised textureData.loaded.data");
		throw std::runtime_error("getRawData - uninitialised textureData.loaded.data.");
	}
};

struct TextureSubData
{
	struct Params
	{
		Params& imagesData(std::vector<std::pair<std::variant<TextureData, TextureData*>, glm::ivec2>> imagesData)
		{
			imagesData_ = std::move(imagesData);
			return *this;
		}

		Params& deferredOffsetPosF(std::function<glm::ivec2(const glm::ivec2& subImageSize, const glm::ivec2& textureSize, int i)> value)
		{
			deferredOffsetPosF_ = std::move(value);
			return *this;
		}

		Params& updateCallbackF(std::function<void(const float* subImageRawData, const glm::ivec2& subImageSize, const glm::ivec2& offsetPos, int numOfChannels)> value)
		{
			updateCallbackF_ = std::move(value);
			return *this;
		}

		Params& exclusiveLoad(bool value)
		{
			exclusiveLoad_ = value;
			return *this;
		}

		Params& permanent(bool value)
		{
			permanent_ = value;
			return *this;
		}

		std::vector<std::pair<std::variant<TextureData, TextureData*>, glm::ivec2>> imagesData_;
		std::function<glm::ivec2(const glm::ivec2& subImageSize, const glm::ivec2& textureSize, int i)> deferredOffsetPosF_;
		std::function<void(const float* subImageRawData, const glm::ivec2& subImageSize, const glm::ivec2& offsetPos, int numOfChannels)> updateCallbackF_;
		bool exclusiveLoad_{};
		bool permanent_{};
	};

	TextureSubData() = default;

	TextureSubData(Params params) :
		imagesData{ std::move(params.imagesData_) },
		deferredOffsetPosF{ std::move(params.deferredOffsetPosF_) },
		updateCallbackF{ std::move(params.updateCallbackF_) },
		exclusiveLoad{ params.exclusiveLoad_ },
		permanent{ params.permanent_ }
	{
	}

	template<typename TextureDataVariant>
	static const TextureData& GetTextureDataFromVariant(const TextureDataVariant& textureData)
	{
		if (std::holds_alternative<TextureData>(textureData))
			return std::get<TextureData>(textureData);
		else if (std::holds_alternative<TextureData*>(textureData))
			return *std::get<TextureData*>(textureData);

		assert(!"unsupported variant type");
		throw std::runtime_error("getTextureDataFromVariant - unsupported variant type.");
	}

	template<typename TextureDataVariant>
	static TextureData& GetTextureDataFromVariant(TextureDataVariant& textureData)
	{
		return const_cast<TextureData&>(GetTextureDataFromVariant(std::as_const(textureData)));
	}

	const TextureData& getImageData(int i) const
	{
		return GetTextureDataFromVariant(imagesData[i].first);
	}

	TextureData& getImageData(int i)
	{
		return const_cast<TextureData&>(std::as_const(*this).getImageData(i));
	}

	std::vector<std::pair<std::variant<TextureData, TextureData*>, glm::ivec2>> imagesData;
	std::function<glm::ivec2(const glm::ivec2& subImageSize, const glm::ivec2& textureSize, int i)> deferredOffsetPosF;
	std::function<void(const float* subImageRawData, const glm::ivec2& subImageSize, const glm::ivec2& offsetPos, int numOfChannels)> updateCallbackF;
	bool exclusiveLoad{};
	bool permanent{};
};

using TextureSourceVariant = std::variant<std::monostate, std::string, TextureFile, TextureData>;
