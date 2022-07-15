#pragma once

#include "_componentBase.hpp"
#include "_decorationDef.hpp"

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct Decoration : ComponentBase, DecorationDef
	{
		using ComponentBase::ComponentBase;

		Decoration(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			DecorationDef(std::move(vertices), texture, std::move(texCoord), renderingSetup),
			renderLayer(renderLayer),
			customShadersProgram(customShadersProgram)
		{
		}

		std::optional<Shaders::ProgramId> customShadersProgram;
		std::function<void()> step;
		ResolutionMode resolutionMode = ResolutionMode::Normal;
		RenderLayer renderLayer = RenderLayer::NearMidground;

		std::vector<DecorationDef> subsequence;
		unsigned posInSubsequence = 0;
	};
}
