#pragma once

#include "_physical.hpp"

#include <functional>

namespace Components
{
	struct Actor : Physical
	{
		Actor() = default;

		Actor(Body body,
			AbstractTextureComponentVariant texture = std::monostate{},
			RenderingSetupF renderingSetupF = nullptr,
			RenderLayer renderLayer = RenderLayer::Midground,
			ShadersUtils::AccessorBase* customShadersProgram = nullptr) :
			Physical(std::move(body), texture, std::move(renderingSetupF), renderLayer, customShadersProgram)
		{
		}

		void init(ComponentId id, bool static_) override
		{
			ComponentBase::init(id, static_);

			if (!body)
				return;

			Tools::SetCollisionFilteringBits(*body, Globals::CollisionBits::actor, Globals::CollisionBits::all);
			setBodyComponentVariant(CM::Actor(*this));
		}
	};
}
