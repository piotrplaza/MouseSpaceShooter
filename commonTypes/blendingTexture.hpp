#pragma once

#include <tools/utility.hpp>

struct BlendingTexture
{
	int blendingTexture = -1;
	int textureR = -1;
	int textureG = -1;
	int textureB = -1;
	int textureA = -1;

	bool operator==(const BlendingTexture&) const = default;
};

template<>
struct std::hash<BlendingTexture>
{
	std::size_t operator()(const BlendingTexture& blendingTexture) const noexcept
	{
		std::size_t seed = 0;
		Tools::HashCombine(seed, blendingTexture.blendingTexture);
		Tools::HashCombine(seed, blendingTexture.textureR);
		Tools::HashCombine(seed, blendingTexture.textureG);
		Tools::HashCombine(seed, blendingTexture.textureB);
		Tools::HashCombine(seed, blendingTexture.textureA);
		return seed;
	}
};
