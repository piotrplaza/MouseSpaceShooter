#pragma once

#include <commonTypes/renderLayer.hpp>

#include <array>

template <typename CollectionsType>
struct BuffersColections
{
	std::array<CollectionsType, (size_t)RenderLayer::COUNT> basicPhong;
	std::array<CollectionsType, (size_t)RenderLayer::COUNT> texturedPhong;
	std::array<CollectionsType, (size_t)RenderLayer::COUNT> basic;
	std::array<CollectionsType, (size_t)RenderLayer::COUNT> textured;
	std::array<CollectionsType, (size_t)RenderLayer::COUNT> customShaders;
};
