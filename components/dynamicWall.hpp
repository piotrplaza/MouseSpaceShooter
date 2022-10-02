#pragma once

#include "_physical.hpp"

namespace Components
{
	struct DynamicWall : Physical
	{
		DynamicWall() = default;

		DynamicWall(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram)
		{
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
		}

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::DynamicWall(id, this));
		}
	};
}
