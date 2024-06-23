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
			AbstractTextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<ShadersUtils::ProgramId> customShadersProgram = std::nullopt) :
			Renderable(texture, std::move(renderingSetupF), renderLayer, customShadersProgram, std::move(vertices), std::move(texCoord))
		{
		}
	};
}
