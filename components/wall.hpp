#pragma once

#include "_componentBase.hpp"
#include "_physical.hpp"

namespace Components
{
	struct Wall : ComponentBase, Physical
	{
		Wall() = default;

		Wall(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), TCM::Wall(getComponentId()), texture, renderingSetup, renderLayer, customShadersProgram)
		{
		}

		std::function<void()> step;
	};
}
