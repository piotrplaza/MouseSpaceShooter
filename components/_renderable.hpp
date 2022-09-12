#pragma once

#include "_typeComponentMappers.hpp"
#include "_renderableDef.hpp"
#include "_componentBase.hpp"

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>
#include <commonTypes/renderLayer.hpp>

#include <ogl/shaders.hpp>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <deque>
#include <optional>

namespace Buffers
{
	struct GenericBuffers;
}

struct Renderable : ComponentBase, RenderableDef
{
	Renderable() = default;

	Renderable(TextureComponentVariant texture,
		std::optional<ComponentId> renderingSetup,
		RenderLayer renderLayer,
		std::optional<Shaders::ProgramId> customShadersProgram,
		std::vector<glm::vec3> vertices = {},
		std::vector<glm::vec2> texCoord = {}):
		RenderableDef(std::move(vertices), std::move(texCoord), texture, renderingSetup),
		renderLayer(renderLayer),
		customShadersProgram(customShadersProgram)
	{
	}

	std::optional<Shaders::ProgramId> customShadersProgram;

	ResolutionMode resolutionMode = ResolutionMode::Normal;
	RenderLayer renderLayer = RenderLayer::Midground;

	std::deque<RenderableDef> subsequence;
	unsigned subsequenceBegin = 0;
	unsigned posInSubsequence = 0;

	struct
	{
		Buffers::GenericBuffers* buffers = nullptr;
	} loaded;

	virtual void enable(bool value)
	{
		render = value;
		for (auto& e : subsequence)
			e.render = value;
	}
};
