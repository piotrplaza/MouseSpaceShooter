#pragma once

#include <componentId.hpp>

#include <variant>

namespace TypeComponentMappers
{
	struct Texture
	{
		ComponentId id;

		bool operator==(const Texture&) const = default;
	};

	struct AnimationTexture
	{
		ComponentId id;

		bool operator==(const AnimationTexture&) const = default;
	};

	struct BlendingTexture
	{
		ComponentId id;

		bool operator==(const BlendingTexture&) const = default;
	};
}

namespace TCM = TypeComponentMappers;

using TextureVariant = std::variant<std::monostate, TCM::Texture, TCM::AnimationTexture, TCM::BlendingTexture>;

template<>
struct std::hash<TCM::Texture>
{
	std::size_t operator()(const TCM::Texture& texture) const noexcept
	{
		return texture.id;
	}
};

template<>
struct std::hash<TCM::AnimationTexture>
{
	std::size_t operator()(const TCM::AnimationTexture& animationTexture) const noexcept
	{
		return animationTexture.id;
	}
};
template<>
struct std::hash<TCM::BlendingTexture>
{
	std::size_t operator()(const TCM::BlendingTexture& blendingTexture) const noexcept
	{
		return blendingTexture.id;
	}
};
