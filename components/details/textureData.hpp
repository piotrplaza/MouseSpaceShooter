#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>

struct TextureData
{
	enum class AdditionalConversion { None, DarkToTransparent, TrasparentToDark };

	template<typename ColorType>
	TextureData(std::vector<ColorType> data, glm::ivec2 size) :
		loaded{ std::move(data), size }
	{
	}

	TextureData(std::vector<float> data, glm::ivec2 size, int numOfChannels = 1) :
		loaded{ std::make_pair(std::move(data), numOfChannels), size }
	{
	}

	TextureData(std::string path, bool convertToPremultipliedAlpha = true, AdditionalConversion additionalConversion = AdditionalConversion::None) :
		toBeLoaded{ std::move(path), convertToPremultipliedAlpha, additionalConversion }
	{
	}

	struct
	{
		std::string path;
		bool convertToPremultipliedAlpha;
		AdditionalConversion additionalConversion;
	} toBeLoaded;

	struct
	{
		std::variant<std::pair<std::vector<float>, int>, std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>> data;
		glm::ivec2 size = { 0, 0 };
	} loaded;
};
