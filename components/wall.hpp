#pragma once

#include "_physical.hpp"

#include <functional>

namespace Components
{
	struct Wall : Physical
	{
		Wall(Body body = {},
			AbstractTextureComponentVariant texture = std::monostate{},
			RenderingSetupF renderingSetupF = nullptr,
			ShadersUtils::AccessorBase* customShadersProgram = nullptr):
			Physical(std::move(body), texture, std::move(renderingSetupF), customShadersProgram)
		{
		}

		void init(ComponentId id, bool static_) override
		{
			Physical::init(id, static_);
			if (this->body)
			{
				Tools::SetCollisionFilteringBits(*this->body, Globals::CollisionBits::wall, Globals::CollisionBits::all);
				setBodyComponentVariant(CM::Wall(*this));
			}
		}
	};
}
