#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <variant>

struct TextureFile
{
	enum class AdditionalConversion { None, DarkToTransparent, TransparentToDark };

	TextureFile() = default;

	TextureFile(std::string path, int desiredChannels = 0, bool convertToPremultipliedAlpha = true, AdditionalConversion additionalConversion = AdditionalConversion::None) :
		path{ std::move(path) },
		desiredChannels{ desiredChannels },
		additionalConversion{ additionalConversion },
		convertToPremultipliedAlpha{ convertToPremultipliedAlpha }
	{
	}

	std::string path;
	int desiredChannels{};
	bool convertToPremultipliedAlpha{};
	AdditionalConversion additionalConversion{};
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

	TextureData(float* data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(data, numOfChannels), size }
	{
	}

	TextureData(TextureFile file) :
		file{ std::move(file) }
	{
	}

	TextureFile file;

	struct
	{
		std::variant<std::monostate, std::pair<float*, int>, std::pair<std::vector<float>, int>, std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>> data;
		glm::ivec2 size = { 0, 0 };
	} loaded;
};

struct TextureSubData
{
	TextureSubData() = default;

	TextureSubData(std::vector<std::pair<std::variant<TextureData, TextureData*>, glm::ivec2>> textureSubData, std::function<glm::ivec2(const glm::ivec2& size, int i)> deferredOffsetPosF = nullptr, bool exclusiveLoad = false) :
		textureSubData{ std::move(textureSubData) },
		deferredOffsetPosF{ std::move(deferredOffsetPosF) },
		exclusiveLoad{ exclusiveLoad }
	{
	}

	std::vector<std::pair<std::variant<TextureData, TextureData*>, glm::ivec2>> textureSubData;
	std::function<glm::ivec2(const glm::ivec2& size, int i)> deferredOffsetPosF;
	bool exclusiveLoad{};
};

using TextureSourceVariant = std::variant<std::monostate, std::string, TextureFile, TextureData>;
