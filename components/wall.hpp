#pragma once

#include "_physical.hpp"

#include <functional>

namespace Components
{
	struct Wall : Physical
	{
		Wall(Body body,
			AbstractTextureComponentVariant texture = std::monostate{},
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			ShadersUtils::ProgramBase* customShadersProgram = nullptr):
			Physical(std::move(body), texture, std::move(renderingSetupF), renderLayer, customShadersProgram)
		{
		}

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);
			Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Wall(*this));
		}
	};
}
