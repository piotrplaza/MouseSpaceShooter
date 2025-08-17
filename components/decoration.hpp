#pragma once

#include "_componentBase.hpp"
#include "_renderable.hpp"

#include <tools/geometryHelpers.hpp>

#include <commonTypes/standardRenderMode.hpp>

namespace Components
{
	struct Decoration : Renderable
	{
		Decoration(std::vector<glm::vec3> vertices = {},
			AbstractTextureComponentVariant texture = std::monostate{},
			std::vector<glm::vec2> texCoord = {},
			RenderingSetupF renderingSetupF = nullptr,
			ShadersUtils::AccessorBase* customShadersProgram = nullptr) :
			Renderable(texture, std::move(renderingSetupF), customShadersProgram, std::move(vertices), std::move(texCoord))
		{
		}
	};
}
