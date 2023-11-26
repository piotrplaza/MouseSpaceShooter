#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/geometryHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct Decoration : Renderable
	{
		Decoration(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram, std::move(vertices), std::move(texCoord))
		{
		}
	};
}
