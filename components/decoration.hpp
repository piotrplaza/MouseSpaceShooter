#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/graphicsHelpers.hpp>

#include <commonTypes/resolutionMode.hpp>

namespace Components
{
	struct Decoration : ComponentBase, Renderable
	{
		Decoration(std::vector<glm::vec3> vertices = {},
			TextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::NearMidground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt) :
			Renderable(texture, renderingSetup, renderLayer, customShadersProgram, std::move(vertices), {}, std::move(texCoord))
		{
		}

		std::function<void()> step;
		std::function<glm::mat4()> modelMatrixF;

		glm::mat4 getModelMatrix() const override
		{
			return modelMatrixF ? modelMatrixF() : glm::mat4(1.0f);
		}
	};
}
