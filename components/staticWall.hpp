#pragma once

#include "_physical.hpp"

#include <functional>

namespace Components
{
	struct StaticWall : Physical
	{
		StaticWall() = default;

		StaticWall(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram)
		{
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
		}

		std::function<void()> stepF;

		void setComponentId(ComponentId id) override
		{
			ComponentBase::setComponentId(id);
			setBodyComponentVariant(TCM::StaticWall(id, this));
		}

		void step() override
		{
			if (stepF)
				stepF();
		}
	};
}
