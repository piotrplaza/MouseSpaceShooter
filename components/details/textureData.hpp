#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <variant>

struct TextureData
{
	template<typename ColorType>
	TextureData(std::vector<ColorType> data, glm::ivec2 size, int forcedNumOfChannels = 0) :
		loaded{ forcedNumOfChannels ? forcedNumOfChannels : []() {
			if constexpr (std::is_same_v<ColorType, float>)
				return 1;
			else if constexpr (std::is_same_v<ColorType, glm::vec2>)
				return 2;
			else if constexpr (std::is_same_v<ColorType, glm::vec3>)
				return 3;
			else if constexpr (std::is_same_v<ColorType, glm::vec4>)
				return 4;
			else
				/*static_assert(false, "unsupported color type");*/ // Not sure why VS failes to compile this even without any instance.
				assert(!"unsupported color type");
			return 0;
		}(),
		size,
		std::move(data) }
	{
		if constexpr (!std::is_same_v<ColorType, float>)
			assert(forcedNumOfChannels == 0);
	}

	TextureData(std::string path, bool fileWithPremultipliedAlpha = false, bool convertDarkToTransparent = false) :
		toBeLoaded{ std::move(path), fileWithPremultipliedAlpha, convertDarkToTransparent }
	{
	}

	struct
	{
		std::string path;
		bool fileWithPremultipliedAlpha;
		bool convertDarkToTransparent;
	} toBeLoaded;

	struct
	{
		int numOfChannels = 0;
		glm::ivec2 size = { 0, 0 };
		std::variant<std::vector<float>, std::vector<glm::vec2>, std::vector<glm::vec3>, std::vector<glm::vec4>> data;
	} loaded;
};
