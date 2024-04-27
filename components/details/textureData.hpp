#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
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
	template<typename ColorType>
	TextureData(std::vector<ColorType> data, glm::ivec2 size) :
		loaded{ std::move(data), size }
	{
	}

	TextureData(std::vector<float> data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(std::move(data), numOfChannels), size }
	{
	}

	TextureData(TextureFile file) :
		file{ std::move(file) }
	{
	}

	TextureFile file;

	struct
	{
		std::variant<std::pair<std::vector<float>, int>, std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>> data;
		glm::ivec2 size = { 0, 0 };
	} loaded;
};

using TextureSourceVariant = std::variant<std::string, TextureFile, TextureData>;
