#pragma once

#include "_physical.hpp"

#include <functional>

namespace Components
{
	struct StaticWall : Physical
	{
		StaticWall(Body body,
			TextureComponentVariant texture = std::monostate{},
			std::optional<ComponentId> renderingSetup = std::nullopt,
			RenderLayer renderLayer = RenderLayer::Midground,
			std::optional<Shaders::ProgramId> customShadersProgram = std::nullopt):
			Physical(std::move(body), texture, renderingSetup, renderLayer, customShadersProgram)
		{
		}

		std::function<void()> stepF;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::StaticWall(id, this));
		}

		void step() override
		{
			if (stepF)
				stepF();
		}
	};

	struct DynamicWall : StaticWall
	{
		using StaticWall::StaticWall;

		void init(ComponentId id) override
		{
			ComponentBase::init(id);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
			setBodyComponentVariant(TCM::DynamicWall(id, this));
		}
	};
}
